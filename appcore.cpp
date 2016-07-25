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
	QFile dataFile(Path);

	objectExpr.setMinimal(true);
	headerExpr.setMinimal(true);

	dataFile.open(QFile::ReadOnly);

	if (dataFile.isOpen() && !isTerminated)
	{
		QTextDecoder* Coder = QTextCodec::codecForName(CoderName.toUtf8())->makeDecoder();
		QString fileText = Coder->toUnicode(dataFile.readAll()).append("\n\n");
		int lastPos = 0;

		emit onProgressInit(0, fileText.size());

		headerExpr.indexIn(fileText);

		emit onHeaderLoad(headerExpr.capturedTexts()[1].remove('\r').split('\n', QString::KeepEmptyParts));

		while (((lastPos = objectExpr.indexIn(fileText, lastPos)) != -1) && !isTerminated)
		{
			Items.append(objectExpr.capturedTexts()[1].remove('\r').split('\n', QString::SkipEmptyParts));
			lastPos += objectExpr.matchedLength();

			emit onProgressUpdate(lastPos);
		}
	}

	emit onObjectsLoad(Items);
}

void AppCore::SaveData(const QString& Path, const QStringList& Header, const QList<QStringList>& Data, const QString& CoderName)
{
	Locker.lock();
	isTerminated = false;
	Locker.unlock();

	QFile dataFile(Path);
	int Step = 0;

	if (dataFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream streamOut(&dataFile);

		streamOut.setCodec(QTextCodec::codecForName(CoderName.toUtf8()));

		emit onProgressInit(0, Data.size());

		streamOut << Header.join('\n') << "[OBIEKTY]\n";

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
			uint32_t Seed = 4294967295u;
			uint32_t Poly[256];

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

	emit onOutputSave(Step == Data.size());
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

	emit onObjectsConvert(Output);
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

void AppCore::UpdateValues(const QList<QStringList>& Data, const QString &Source, const QString &Replace, bool Case, bool RegExp)
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
		QRegExp objectExpr("(C,.*)=(.*)");
		QMap<QString, QString> Values;
		QMap<int, QString> foundList;
		int ID = 0;

		for (auto& String : Item)
		{
			if (objectExpr.indexIn(String) != -1)
			{
				const QString LastKey = objectExpr.capturedTexts()[1];
				bool isFound = false;

				Values.insert(QString("$%1").arg(objectExpr.capturedTexts()[1]),
										   objectExpr.capturedTexts()[2]);

				if (RegExp)
				{
					QRegExp nameExpr(Source, Case ? Qt::CaseSensitive : Qt::CaseInsensitive);

					isFound = nameExpr.indexIn(LastKey) != -1;
				}
				else isFound = !QString::compare(LastKey, Source, Case ? Qt::CaseSensitive : Qt::CaseInsensitive);

				if (isFound) foundList.insert(ID, LastKey);
			}

			++ID;
		}

		for (const auto& Index : foundList.keys())
		{
			Item[Index] = foundList[Index] + "=" + Replace;

			for (const auto& Value : Values.keys())
			{
				Item[Index].replace(Value, Values[Value]);
			}

			Item[Index] = Item[Index].trimmed();
		}

		CountLocker.lock();
		Count += foundList.size();
		CountLocker.unlock();
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onValuesUpdate(Output, Count);
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
		QRegExp classExpr(QString("A,(%1),").arg(Expr));
		QRegExp objectExpr("(C,.*)=(.*)");
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

void AppCore::UnpinnData(const QList<QStringList>& Data, const QStringList& Classes, bool Delete)
{
	QList<QStringList> Output = Data;
	QFutureWatcher<void> Watcher;
	QThread WatcherThread;

	Watcher.moveToThread(&WatcherThread);
	WatcherThread.start();

	connect(this, &AppCore::onTerminateRequest, &Watcher, &QFutureWatcher<void>::cancel, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressRangeChanged, this, &AppCore::onProgressInit, Qt::DirectConnection);
	connect(&Watcher, &QFutureWatcher<void>::progressValueChanged, this, &AppCore::onProgressUpdate, Qt::DirectConnection);

	QRegExp classExpr(QString("A,(%1),\\d*,(\\d+)").arg(Classes.join('|')));
	QList<QString> List;
	int Count = 0;

	for (auto& Item : Output) if (classExpr.indexIn(Item.first()) != -1)
	{
		List.append(classExpr.capturedTexts().last());
		if (Delete) Item = QStringList();

		++Count;
	}

	Output.removeAll(QStringList());

	Watcher.setFuture(QtConcurrent::map(Output, [&List] (auto& Item) -> void
	{
		for (auto& String : Item) if (String[0] == 'B') for (const auto& Pinn : List)
		{
			String.replace(QString("B,%1,").arg(Pinn), "B,,");
		}
	}));

	Watcher.waitForFinished();
	WatcherThread.exit();
	WatcherThread.wait();

	emit onDataUnpinn(Output, Count);
}
