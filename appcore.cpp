/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Tango file converter for DaroD                                         *
 *  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the  Free Software Foundation, either  version 3 of the  License, or   *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This  program  is  distributed  in the hope  that it will be useful,   *
 *  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have  received a copy  of the  GNU General Public License   *
 *  along with this program. If not, see http://www.gnu.org/licenses/.     *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "appcore.hpp"

AppCore* AppCore::THIS = nullptr;

AppCore::AppCore(void)
{
	const QString DB = QSettings("K-Converter").value("database", "sources.sqlite").toString();

	if (THIS) qFatal("Core object duplicated"); else THIS = this;
	if (!QFile::exists(DB)) QFile::copy(":/data/database", DB);

	Database = QSqlDatabase::addDatabase("QSQLITE");

	Database.setDatabaseName(DB);
	Database.open();
}

int AppCore::AddItem(Entry& Item)
{
	QSqlQuery Query(Database);

	Query.prepare(
			"INSERT INTO "
				"sources (source, expr, isregexp, isenabled) "
			"VALUES "
				"(:source, :expr, :isregexp, :isenabled)");

	Query.bindValue(":source", Item.Source);
	Query.bindValue(":expr", Item.Replace);
	Query.bindValue(":isregexp", Item.isRegExp);
	Query.bindValue(":isenabled", Item.isEnabled);

	return Query.exec() ? Item.ID = Query.lastInsertId().toInt() : -1;
}

bool AppCore::RemoveItem(int ID)
{
	QSqlQuery Query(Database);

	Query.prepare(
			"DELETE FROM "
				"sources "
			"WHERE "
				"ID = :ID");

	Query.bindValue(":ID", ID);

	return Query.exec();
}

bool AppCore::UpdateItem(Entry& Item)
{
	QSqlQuery Query(Database);

	Query.prepare(
			"UPDATE "
				"sources "
			"SET "
				"source = :source, "
				"expr = :expr, "
				"isregexp = :isregexp, "
				"isenabled = :isenabled "
			"WHERE "
				"ID = :ID");

	Query.bindValue(":ID", Item.ID);

	Query.bindValue(":source", Item.Source);
	Query.bindValue(":expr", Item.Replace);
	Query.bindValue(":isregexp", Item.isRegExp);
	Query.bindValue(":isenabled", Item.isEnabled);

	return Query.exec();
}

AppCore::Entry AppCore::getItem(int ID)
{
	QSqlQuery Query(Database);

	Query.prepare(
			"SELECT "
				"ID, source, expr, isregexp, isenabled "
			"FROM "
				"sources "
			"WHERE "
				"ID = :ID");

	Query.bindValue(":ID", ID);

	if (Query.exec() && Query.next())
	{
		Entry Item;

		Item.ID = Query.value(0).toInt();
		Item.Source = Query.value(1).toString();
		Item.Replace = Query.value(2).toString();
		Item.isRegExp = Query.value(3).toBool();
		Item.isEnabled = Query.value(4).toBool();

		return Item;
	}
	else return Entry();
}

QMap<int, AppCore::Entry> AppCore::getItems(void)
{
	QSqlQuery Query(Database);
	QMap<int, AppCore::Entry> Items;

	Query.prepare(
			"SELECT "
				"ID, source, expr, isregexp, isenabled "
			"FROM "
				"sources");

	if (Query.exec()) while(Query.next())
	{
		Entry Item;

		Item.ID = Query.value(0).toInt();
		Item.Source = Query.value(1).toString();
		Item.Replace = Query.value(2).toString();
		Item.isRegExp = Query.value(3).toBool();
		Item.isEnabled = Query.value(4).toBool();

		Items.insert(Item.ID, Item);
	}

	return Items;
}

QStringList AppCore::getClasses(const QList<QStringList>& Data)
{
	QRegExp classExpr("^A,(\\w+),");
	QStringList Items;

	for (const auto& Item : Data)
	{
		if (classExpr.indexIn(Item.first()) != -1)
		{
			const QString Class = classExpr.capturedTexts().last();

			if (!Items.contains(Class)) Items.append(Class);
		}
	}

	qSort(Items);

	return Items;
}

QStringList AppCore::getFields(const QList<QStringList>& Data, const QString& Class)
{
	QRegExp classExpr(QString("^A,%1,").arg(Class));
	QRegExp fieldExpr("^C,(.+)=");
	QStringList Items;

	fieldExpr.setMinimal(true);

	for (const auto& Item : Data)
	{
		if (classExpr.indexIn(Item.first()) != -1)
		{
			for (const auto& Field : Item)
			{
				if (fieldExpr.indexIn(Field) != -1) Items.append(fieldExpr.capturedTexts().last());
			}

			return Items;
		}
	}

	return Items;
}

void AppCore::Terminate(void)
{
	Locker.lock();
	isTerminated = true;
	Locker.unlock();

	emit onTerminateRequest();
}

AppCore* AppCore::getInstance(void)
{
	return THIS;
}

void AppCore::LoadItems(void)
{
	emit onItemsLoad(getItems());
}

void AppCore::LoadData(const QString& Path, const QString& CoderName)
{
	Locker.lock();
	isTerminated = false;
	Locker.unlock();

	QRegExp headerExpr("(.*)\\[OBIEKTY\\]");
	QRegExp objectExpr("(A,.*)(\\n|\\r|\\r\\n)\\2");

	QList<QStringList> Items;
	QStringList Header;
	QFile dataFile(Path);

	objectExpr.setMinimal(true);
	headerExpr.setMinimal(true);

	if (dataFile.open(QFile::ReadOnly | QFile::Text) && !isTerminated)
	{
		QTextDecoder* Coder = QTextCodec::codecForName(CoderName.toUtf8())->makeDecoder();
		QString fileText = Coder->toUnicode(dataFile.readAll()).append("\n\n");
		int lastPos = 0;

		emit onProgressInit(0, fileText.size());

		headerExpr.indexIn(fileText);

		Header = headerExpr.capturedTexts()[1].remove('\r').split('\n', QString::SkipEmptyParts);

		for (auto& Line : Header) if (Line[0] == ';') Line.clear();

		Header.removeAll(QString());

		while (((lastPos = objectExpr.indexIn(fileText, lastPos)) != -1) && !isTerminated)
		{
			Items.append(objectExpr.capturedTexts()[1].remove('\r').split('\n', QString::SkipEmptyParts));
			lastPos += objectExpr.matchedLength();

			for (auto& Line : Items.last()) if (Line[0] == ';') Line.clear();

			Items.last().removeAll(QString());

			emit onProgressUpdate(lastPos);
		}
	}

	emit onClassesLoad(getClasses(Items));
	emit onObjectsLoad(Header, Items);
}

void AppCore::SaveData(const QString& Path, const QStringList& Header, const QList<QStringList>& Data, const QString& CoderName)
{
	Locker.lock();
	isTerminated = false;
	Locker.unlock();

	QFile dataFile(Path);
	uint32_t Seed = 4294967295u;
	uint32_t Poly[256];
	int Step = 0;

	if (dataFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream streamOut(&dataFile);

		streamOut.setCodec(QTextCodec::codecForName(CoderName.toUtf8()));

		emit onProgressInit(0, Data.size());

		streamOut << QString("; Genated by K-Converter ") << QDateTime::currentDateTimeUtc().toString() << "\n"
				<< QString("; Developed by Łukasz \"Kuszki\" Dróżdż l.drozdz@openmailbox.org\n")
				<< QString("; See more at https://github.com/Kuszki/K-Converter\n\n")
				<< Header.join('\n') << "\n\n[OBIEKTY]\n\n";

		for (const auto& Item : Data) if (!isTerminated)
		{
			streamOut << Item.join('\n') << "\n\n";

			emit onProgressUpdate(++Step);
		}

		streamOut << "; Crc32Sum=";

		streamOut.flush();
		dataFile.close();

		if (dataFile.open(QFile::ReadWrite))
		{
			for (int i = 0; i < 256; i++)
			{
				uint32_t crc = i;

				for (int j = 0; j < 8; j++)
					crc = crc & 1 ? (crc >> 1) ^ 3988292384u : crc >> 1;

				Poly[i] = crc;
			}

			for (const auto& Value : dataFile.readAll()) if (Value != 10 && Value != 13)
			{
				Seed = Poly[(Seed ^ Value) & 0xFF] ^ (Seed >> 8);
			}

			dataFile.write(QString("%1").arg(~Seed, 8, 16, QChar('0')).toUtf8());
		}
	}

	emit onOutputSave(Step == Data.size() ? ~Seed : 0);
}

void AppCore::ConvertData(const QList<QStringList>& Data)
{
	const QMap<int, AppCore::Entry> Tasks = getItems();

	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	Watcher.setFuture(QtConcurrent::map(Output, [&Tasks] (auto& Item) -> void
	{
		for (auto& String : Item) for (const auto& Task : Tasks) if (Task.isEnabled)
		{
			if (!Task.isRegExp) String.replace(Task.Source, Task.Replace);
			else String.replace(QRegExp(Task.Source), Task.Replace);
		}

		Item.removeAll("");
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onDataConvert(Output);
}

void AppCore::ReplaceData(const QList<QStringList>& Data, const QString &Source, const QString &Replace, bool Case, bool RegExp)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Count, &Source, &Replace, Case, RegExp] (auto& Item) -> void
	{
		for (auto& String : Item)
		{
			const QString Old = String;

			if (RegExp) String.replace(QRegExp(Source, Case ? Qt::CaseSensitive : Qt::CaseInsensitive), Replace);
			else String.replace(Source, Replace, Case ? Qt::CaseSensitive : Qt::CaseInsensitive);

			const bool Changed = String != Old;

			CountLocker.lock();
			Count += Changed;
			CountLocker.unlock();
		}

		Item.removeAll("");
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onDataReplace(Output, Count);
}

void AppCore::UpdateData(const QList<QStringList>& Data, const QString& Field, const QString& Setto, const QStringList& Classes, const QMap<QString, QString>& List)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	const QString Expr = Classes.join('|');

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Count, &Field, &Setto, &List, &Expr] (auto& Item) -> void
	{
		QMap<QString, QString> ListCopy = List;
		QRegExp classExpr(QString("^A,(%1),").arg(Expr));
		QRegExp objectExpr("(C,[^=]*)=(.*)");
		QMap<QString, QString> Values;
		bool Checked = true;
		int Index = -1;
		int ID = 0;

		if (classExpr.indexIn(Item.first()) != -1)
		{

			for (auto& String : Item)
			{
				if (objectExpr.indexIn(String) != -1)
				{
					const QString Key = objectExpr.capturedTexts()[1];

					Values.insert(Key, objectExpr.capturedTexts()[2]);

					if (Key == Field)
					{
						Index = ID;
					}
				}

				++ID;
			}

			for (const auto& Key : Values.keys())
			{
				for (auto& Value : ListCopy)
				{
					Value.replace(QString("$%1").arg(Key), Values[Key], Qt::CaseSensitive);
				}
			}

			for (const auto& Key : ListCopy.keys()) if (Checked)
			{
				if (!(Values.contains(Key) && Values[Key] == ListCopy[Key])) Checked = false;
			}
			else break;

			for (const auto& Key : ListCopy.keys()) if (Checked)
			{
				if (!(Values.contains(Key) && Values[Key] == ListCopy[Key])) Checked = false;
			}
			else break;

			if (Checked)
			{
				if (Index == -1)
				{
					Item.append(QString());
					Index = Item.count() - 1;
				}

				Item[Index] = Setto;

				for (const auto& Value : Values.keys())
				{
					Item[Index].replace(QString("$%1").arg(Value), Values[Value]);
				}

				Item[Index].remove(QRegExp("\\$\\S+"));
				Item[Index] = Field + "=" + Item[Index].simplified();

				CountLocker.lock();
				++Count;
				CountLocker.unlock();
			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onDataUpdate(Output, Count);
}

void AppCore::DeleteData(const QList<QStringList>& Data, const QStringList& Classes, const QMap<QString, QString>& List)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	const QString Expr = Classes.join('|');

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Count, &List, &Expr] (auto& Item) -> void
	{
		QMap<QString, QString> ListCopy = List;
		QRegExp classExpr(QString("^A,(%1),").arg(Expr));
		QRegExp objectExpr("(C,[^=]*)=(.*)");
		QMap<QString, QString> Values;
		bool Delete = true;

		if (classExpr.indexIn(Item.first()) != -1)
		{

			for (auto& String : Item) if (objectExpr.indexIn(String) != -1)
			{
				Values.insert(objectExpr.capturedTexts()[1], objectExpr.capturedTexts()[2]);
			}

			for (const auto& Key : Values.keys())
			{
				for (auto& Value : ListCopy)
				{
					Value.replace(QString("$%1").arg(Key), Values[Key], Qt::CaseSensitive);
				}
			}

			for (const auto& Key : ListCopy.keys()) if (Delete)
			{
				if (!(Values.contains(Key) && Values[Key] == ListCopy[Key])) Delete = false;
			}
			else break;

			if (Delete)
			{
				Item = QStringList();

				CountLocker.lock();
				Count += Delete;
				CountLocker.unlock();
			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	Output.removeAll(QStringList());

	emit onDataDelete(Output, Count);
}

void AppCore::UnpinnData(const QList<QStringList>& Data, const QStringList& Classes, bool Delete, bool Keep)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	QRegExp classExpr(QString("^A,(%1),\\d*,(\\d+)").arg(Classes.join('|')));
	QMap<int, QString> List;
	int ID = 0;

	for (auto& Item : Output)
	{
		if (classExpr.indexIn(Item.first()) != -1)
		{
			List.insert(ID, classExpr.capturedTexts().last());

			if (Delete && !Keep) Item = QStringList();
		}

		++ID;
	}

	Watcher.setFuture(QtConcurrent::map(Output, [&List, &CountLocker, &Count, Keep] (auto& Item) -> void
	{
		int Stop = Item.lastIndexOf(QRegExp("^B,.*"));
		int Start = 1;

		if (Keep)
		{
			++Start;
			--Stop;
		}

		for (int i = Start; i <= Stop; ++i) for (const auto& Pinn : List)
		{
			const QString Source = Item[i];

			Item[i].replace(QString("B,%1,").arg(Pinn), "B,,");

			if (Source != Item[i])
			{
				CountLocker.lock();
				++Count;
				CountLocker.unlock();
			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	if (Delete && Keep)
	{
		static QStringList (*Task)(const QStringList&) = [] (const QStringList& Item) -> QStringList
		{
			QRegExp pinExpr("^B,(\\d+),.*");
			QStringList Return;

			for (const auto& String : Item) if (pinExpr.indexIn(String) != -1)
			{
				Return.append(pinExpr.capturedTexts().last());
			}

			return Return;
		};

		static void (*Reduce)(QStringList&, const QStringList&) = [] (QStringList& Resoult, const QStringList& Part) -> void
		{
			for (const auto& String : Part) if (!Resoult.contains(String)) Resoult.append(String);
		};

		emit onProgressInit(0, 0);

		QStringList Save = QtConcurrent::blockingMappedReduced(Output, Task, Reduce);

		for (const auto& Index : List.keys())
		{
			if (!Save.contains(List[Index]))
			{
				Output[Index] = QStringList();
			}
		}

		emit onProgressInit(0, 1);
		emit onProgressUpdate(1);

	}

	Output.removeAll(QStringList());

	emit onDataUnpinn(Output, Count);
}

void AppCore::SplitData(const QList<QStringList> &Data, const QStringList &Classes, bool Keep, bool Hide)
{
	QList<QStringList> Output = Data;
	QList<QStringList> Divided;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	QRegExp classExpr(QString("^A,(%1),\\d*,(\\d+)").arg(Classes.join('|')));
	QStringList List;

	for (auto& Item : Output) if (classExpr.indexIn(Item.first()) != -1)
	{
		List.append(classExpr.capturedTexts().last());
	}

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Count, &Divided, &List, Keep, Hide] (auto& Item) -> void
	{
		QStringList Geometry = Item.filter(QRegExp("^B,.*"));
		QRegExp pinExpr("^B,(\\d+),");
		QList<int> Cuts;
		int ID = 0;

		for (const auto& Pin : Geometry)
		{
			if (pinExpr.indexIn(Pin) != -1 && List.contains(pinExpr.capturedTexts().last()))
			{
				if (ID != 0 && ID != Geometry.size() - 1) Cuts.append(ID);
			}

			++ID;
		}

		if (!Cuts.isEmpty())
		{
			QStringList Copy = Item;
			QString Header = Copy.takeFirst().replace(QRegExp("^A,(\\w+),(\\d+),(\\d+),(.*)"), "A,\\1,\\2,,\\4");

			ID = Cuts.size() + 1;

			Copy.replaceInStrings(QRegExp("^C,_identifier=.*"), "C,_identifier=")
			    .replaceInStrings(QRegExp("^B,.*"), QString())
			    .removeAll(QString());

			if (Keep)
			{
				Item.replaceInStrings(QRegExp("^C,_status=.*"), "C,_status=0");
			}
			else
			{
				Item = QStringList();
			}

			QList<QStringList> NewItems;
			QStringList NewGeometry;

			for (int i = 0; i < Geometry.size(); ++i)
			{
				NewGeometry << Geometry[i];

				if (!Cuts.isEmpty() && i == Cuts.first())
				{
					if (Hide)
					{
						QRegExp attrExpr("^B,(.+),(\\d+)");
						const int PreLast = NewGeometry.size() - 2;

						if (attrExpr.indexIn(NewGeometry[PreLast]) != -1)
						{
							const int Flags = attrExpr.capturedTexts().last().toInt() & ~int(0x1);

							NewGeometry[PreLast] = QString("B,%1,%2").arg(attrExpr.capturedTexts()[1]).arg(Flags);
						}

						if (attrExpr.indexIn(Geometry[i]) != -1)
						{
							const int Flags = attrExpr.capturedTexts().last().toInt() & ~int(0x1);

							Geometry[i] = QString("B,%1,%2").arg(attrExpr.capturedTexts()[1]).arg(Flags);
						}
					}

					NewItems.append(QStringList() << Header << NewGeometry << Copy);

					NewGeometry.clear();
					Cuts.removeFirst();

					NewGeometry << Geometry[i];
				}
			}

			if (NewGeometry.size() > 1)
			{
				NewItems.append(QStringList() << Header << NewGeometry << Copy);
			}

			CountLocker.lock();
			Count += ID;
			Divided.append(NewItems);
			CountLocker.unlock();
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	Output.removeAll(QStringList());

	for (const auto& Item : Divided) if (!Output.contains(Item)) Output.append(Item);

	emit onDataSplit(Output, Count);
}

void AppCore::InsertData(const QList<QStringList> &Data, const QStringList &Classes, const QString &Insert)
{
	QList<QStringList> Output = Data;
	QList<QStringList> Inserted;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	const QString Class = QString("^A,(%1),.*").arg(Classes.join('|'));

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Inserted, &Class, &Insert] (auto& Item) -> void
	{
		QRegExp classExpr(Class);

		if (classExpr.indexIn(Item.first()) != -1)
		{
			QStringList Geometry = Item.filter(QRegExp("^B,,.*"));
			QStringList Attributes = Item.filter(QRegExp("^C,.*"));

			if (!Geometry.isEmpty())
			{
				QList<QStringList> NewItems;

				Geometry.replaceInStrings(QRegExp("^B,,(.*),\\d+$"), "B,,\\1,196609");

				for (const auto& Pin : Geometry) NewItems.append(QStringList()
							<< QString("A,%1,1,,,").arg(Insert)
							<< Pin << Attributes);

				CountLocker.lock();
				Inserted.append(NewItems);
				CountLocker.unlock();
			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	for (const auto& Item : Inserted) if (!Output.contains(Item)) Output.insert(0, Item);

	emit onDataInsert(Output, Output.size() - Data.size());
}

void AppCore::RevertData(const QList<QStringList>& Data, const QStringList& Classes, const QStringList& Begins, const QStringList& Ends)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;
	QMap<QString, int> List;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	volatile int Count = 0;

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	const QString Class = QString("^A,(%1),.*").arg(Classes.join('|'));
	const QString Begin = QString("^A,(%1),\\d*,(\\d+)").arg(Begins.join('|'));
	const QString End = QString("^A,(%1),\\d*,(\\d+)").arg(Ends.join('|'));

	for (const auto& Item : Output)
	{
		QRegExp beginExpr(Begin);
		QRegExp endExpr(End);
		QPair<QString, int> Return;

		if (beginExpr.indexIn(Item.first()) != -1)
		{
			Return.first = beginExpr.capturedTexts()[2];
			Return.second |= 0x2;
		}

		if (endExpr.indexIn(Item.first()) != -1)
		{
			Return.first = endExpr.capturedTexts()[2];
			Return.second |= 0x4;
		}

		if (Return.second) List.insert(Return.first, Return.second);
	}

	List.insert(QString(), (Begins.contains(".*") << 1) | (Ends.contains(".*") << 2));

	Watcher.setFuture(QtConcurrent::map(Output, [&CountLocker, &Count, &List, &Class] (auto& Item) -> void
	{
		QRegExp geometryExpr("^B,(\\d+),.*");
		QRegExp classExpr(Class);

		if (classExpr.indexIn(Item.first()) != -1)
		{
			QStringList Geometry = Item.filter(QRegExp("^B,.*"));
			QStringList Attributes = Item.filter(QRegExp("^C,.*"));

			if (Geometry.size() > 1)
			{
				QString First, Last;

				if (geometryExpr.indexIn(Geometry.first()) != -1)
				{
					First = geometryExpr.capturedTexts().last();
				}

				if (geometryExpr.indexIn(Geometry.last()) != -1)
				{
					Last = geometryExpr.capturedTexts().last();
				}

				if (List.contains(First) && List.contains(Last))
				{
					if (List[First] & 0x2 && List[Last] && 0x4)
					{
						QRegExp attribExpr("^B,.*,(\\d+)$");
						QString Saved, Current;

						if (attribExpr.indexIn(Geometry.last()) != -1)
						{
							Saved = attribExpr.capturedTexts().last();
						}

						for (int i = 0; i < Geometry.size(); ++i)
						{
							if (attribExpr.indexIn(Geometry[i]) != -1)
							{
								Current = attribExpr.capturedTexts().last();

								Geometry[i].replace(QRegExp("^B,(.*),\\d+$"),
												QString("B,\\1,%1").arg(Saved));

								Saved = Current;
							}
							else Saved = QString();
						}

						std::reverse(Geometry.begin(), Geometry.end());

						Item = QStringList() << Item.first() << Geometry << Attributes;

						CountLocker.lock();
						++Count;
						CountLocker.unlock();
					}
				}
			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onDataRevert(Output, Count);
}

void AppCore::JoinData(const QList<QStringList> &Data, const QString &Class, const QList<int>& Values, bool Keep)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;
	QMutex CountLocker;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	QVector<int> Indexes(Output.size() / 4);
	QList<QPair<int, int>> Joins;

	const QString Header = QString("A,%1,").arg(Class);

	int ID = 0; for (const auto& Item : Output)
	{
		if (Item.first().startsWith(Header))
		{
			Indexes.append(ID);
		}

		++ID;
	}

	Watcher.setFuture(QtConcurrent::map(Indexes, [&Indexes, &Output, &Values, &Joins, &CountLocker] (const auto Current) -> void
	{
		const QString First = Output[Current].filter(QRegExp("^B,.*")).first();
		const QString Last = Output[Current].filter(QRegExp("^B,.*")).last();

		const QStringList Attr = Output[Current].filter(QRegExp("^C,.*"));

		bool OK = true;

		for (const auto Item : Indexes) if (Item != Current)
		{
			QStringList Geometry = Output[Item].filter(QRegExp("^B,.*"));
			QStringList Attributes = Output[Item].filter(QRegExp("^C,.*"));

			if (First == Geometry.first() || Last == Geometry.first() ||
			    First == Geometry.last() || Last == Geometry.last())
			{

				if (Attr.size() != Attributes.size() || (Values.size() && Attr.size() < Values.last()))
				{
					OK = false;
				}
				else for (const auto Check : Values) if (OK)
				{
					if (Attr[Check] != Attributes[Check]) OK = false;
				}

				if (OK)
				{
					CountLocker.lock();
					Joins.append(QPair<int, int>(qMin(Current, Item), qMax(Current, Item)));
					CountLocker.unlock();
				}

			}
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	auto Set = Joins.toSet().toList(); qSort(Set);
	const int Count = Set.size();

	qDebug() << Set;

	while (Set.size())
	{
		const auto Task = Set.takeFirst();

		QList<int> Parts;
		int Iter = 0;

		Parts.append(Task.first);
		Parts.append(Task.second);

		while (Iter < Set.size())
		{
			if (Parts.last() == Set[Iter].first)
			{
				Parts.append(Set.takeAt(Iter).second); Iter = 0;
			}
			else if (Parts.last() == Set[Iter].second)
			{
				Parts.append(Set.takeAt(Iter).first); Iter = 0;
			}
			else ++Iter;
		}

		QStringList Attributes;
		QStringList Geometry;
		QString Header;

		Attributes = Output[Parts.first()].filter(QRegExp("^C,.*"));
		Attributes.replaceInStrings(QRegExp("^C,_identifier=.*"), "C,_identifier=");

		Header = Output[Parts.first()].first();
		Header.replace(QRegExp("^A,(\\w+),(\\d+),.*"), "A,\\1,\\2,,,");

		qDebug() << Parts;

		for (const auto Item : Parts)
		{
			QStringList Current = Output[Item].filter(QRegExp("^B,.*"));

			if (Geometry.isEmpty()) Geometry = Current;
			else
			{
				if (Geometry.last() == Current.last())
				{
					std::reverse(Current.begin(), Current.end());

				}
				else if (Geometry.first() == Current.first())
				{
					std::reverse(Geometry.begin(), Geometry.end());
				}
				else if (Geometry.first() == Current.last())
				{
					std::reverse(Current.begin(), Current.end());
					std::reverse(Geometry.begin(), Geometry.end());
				}

				Current.removeFirst();
				Geometry.append(Current);
			}

			if (Keep) Output[Item].replaceInStrings(QRegExp("^C,_status=.*"), "C,_status=0");
			else Output[Item] = QStringList();
		}

		QStringList Joined = QStringList() << Header << Geometry << Attributes;

		if (!Output.contains(Joined)) Output.append(Joined);
	}

	Output.removeAll(QStringList());

	emit onDataJoin(Output, Count ? Count + 1 : 0);
}
