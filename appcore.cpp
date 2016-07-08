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

int AppCore::getMaxIndex(void)
{
	QSqlQuery Query(Database);

	Query.prepare(
			"SELECT "
				"MAX(ID) "
			"FROM "
				"sources");

	if (Query.exec() && Query.next())
	{
		return Query.value(0).toInt();
	}
	else return -1;
}

void AppCore::terminate(void)
{
	Locker.lock();
	Terminate = true;
	Locker.unlock();
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
	QRegExp headerExpr("\\[OPCJE\\](.*)\\[OBIEKTY\\]");
	QRegExp objectExpr("(A,.*)\\n{2}");

	QFile dataFile(Path);

	objectExpr.setMinimal(true);
	headerExpr.setMinimal(true);

	dataFile.open(QFile::ReadOnly);

	Terminate = false;

	if (dataFile.isOpen())
	{
		emit onProgressUpdate(0.0);

		QTextDecoder* Coder = QTextCodec::codecForName(CoderName.toUtf8())->makeDecoder();
		QString fileText = Coder->toUnicode(dataFile.readAll());

		headerExpr.indexIn(fileText);

		emit onHeaderLoad(headerExpr.capturedTexts().last().remove('\r').split('\n', QString::SkipEmptyParts));

		QList<QStringList> Items;
		const double Size = fileText.size();
		int lastPos = 0;
		int Step = 0;

		fileText.remove('\r').append("\n\n");

		while (((lastPos = objectExpr.indexIn(fileText, lastPos)) != -1) && !Terminate)
		{
			Items.append(objectExpr.capturedTexts().last().split('\n', QString::SkipEmptyParts));
			lastPos += objectExpr.matchedLength();

			if (!(++Step % 100)) emit onProgressUpdate(lastPos / Size);
		}

		emit onProgressUpdate(1.0);

		emit onObjectsLoad(Items);
	}
}

void AppCore::SaveData(const QString& Path, const QStringList& Header, const QList<QStringList>& Data, const QString& CoderName)
{
	Terminate = false;

	const QString NL = QSettings("K-Converter").value("new_line", "\r\n").toString();
	const double Size = Data.size();
	int Step = 0;
	QFile dataFile(Path);


	if (dataFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream streamOut(&dataFile);
		streamOut.setCodec(QTextCodec::codecForName(CoderName.toUtf8()));

		emit onProgressUpdate(0);

		streamOut << "[OPCJE]" << NL << NL
				<< Header.join(NL) << NL << NL
				<< "[OBIEKTY]" << NL << NL;

		for (const auto& Item : Data) if (!Terminate)
		{
			streamOut << Item.join(NL) << NL << NL;

			if (!(++Step % 100)) emit onProgressUpdate(Step / Size);
		}

		emit onProgressUpdate(1);

		emit onOutputSave(Step == Size);
	}
	else emit onOutputSave(false);
}

void AppCore::ConvertData(const QList<QStringList>& Data)
{
	Terminate = false;

	const QMap<int, AppCore::Entry> Tasks = getItems();
	QList<QStringList> Output = Data;
	const double Size = Output.size();
	const QString Empty;
	int Step = 0;

	emit onProgressUpdate(0);

	for (auto& Item : Output) if (!Terminate)
	{
		for (auto& String : Item) for (const auto& Task : Tasks) if (Task.isEnabled)
		{
			if (!Task.isRegExp) String.replace(Task.Source, Task.Replace);
			else String.replace(QRegExp(Task.Source), Task.Replace);
		}

		Item.removeAll(Empty);

		if (!(++Step % 100)) emit onProgressUpdate(Step / Size);
	}

	emit onProgressUpdate(1);

	emit onObjectsConvert(Output);
}

void AppCore::ReplaceData(const QList<QStringList>& Data, const QString &Source, const QString &Replace, bool Case, bool RegExp)
{
	Terminate = false;

	QList<QStringList> Output = Data;
	const double Size = Data.size();
	const QString Empty;
	int Step = 0;
	int Count = 0;

	emit onProgressUpdate(0);

	for (auto& Item : Output) if (!Terminate)
	{
		for (auto& String : Item)
		{
			const QString Old = String;

			if (RegExp)
			{
				QRegExp Exp(Source);

				Exp.setCaseSensitivity(Case ? Qt::CaseSensitive : Qt::CaseInsensitive);
				String.replace(Exp, Replace);
			}
			else String.replace(Source, Replace, Case ? Qt::CaseSensitive : Qt::CaseInsensitive);

			Count += String != Old;
		}

		Item.removeAll(Empty);

		if (!(++Step % 100)) emit onProgressUpdate(Step / Size);
	}

	emit onProgressUpdate(1);

	emit onDataReplace(Output, Count);
}

void AppCore::UpdateValues(const QList<QStringList>& Data, const QString &Source, const QString &Replace, bool Case, bool RegExp)
{
	Terminate = false;

	QRegExp objectExpr("(C,.*)=(.*)");
	QList<QStringList> Output = Data;
	const double Size = Data.size();
	int Step = 0;
	int Count = 0;

	emit onProgressUpdate(0);

	for (auto& Item : Output) if (!Terminate)
	{
		QMap<QString, QString> Values;
		QList<int> foundList;

		for (auto& String : Item)
		{
			bool isFound;

			if (objectExpr.indexIn(String) != -1)
			{
				Values.insert(QString("$%1").arg(objectExpr.capturedTexts()[1]),
										   objectExpr.capturedTexts().last().trimmed());
			}
			if (RegExp)
			{
				QRegExp nameExpr(Source);

				nameExpr.setCaseSensitivity(Case ? Qt::CaseSensitive : Qt::CaseInsensitive);
				nameExpr.indexIn(Values.lastKey());

				isFound = nameExpr.capturedTexts().first() == Values.lastKey();
			}
			else isFound = Values.lastKey().compare(Source, Case ? Qt::CaseSensitive : Qt::CaseInsensitive);

			if (isFound) foundList.append(Values.size() - 1);
		}
qDebug() << "item ========\n" << Values;
		for (const auto& Index : foundList)
		{
			for (const auto& Value : Values.keys())
			{
				Item[Index].replace(Value, Values[Value]);
			}
		}

		Count += foundList.size();

		if (!(++Step % 100)) emit onProgressUpdate(Step / Size);
	}

	emit onProgressUpdate(1);

	emit onValuesUpdate(Output, Count);
}
