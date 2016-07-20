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

#ifndef APPCORE_HPP
#define APPCORE_HPP

#include <QtConcurrent>

#include <QSqlDatabase>
#include <QStringList>
#include <QTextCodec>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QVariant>
#include <QObject>
#include <QRegExp>
#include <QMutex>
#include <QFile>
#include <QMap>

#include <QDebug>

class AppCore : public QObject
{

		Q_OBJECT

	public: struct Entry
	{
		int ID = -1;

		QString Source = tr("source");
		QString Replace = tr("replace");

		bool isRegExp = false;
		bool isEnabled = true;
	};

	private:

		static AppCore* THIS;

		QSqlDatabase Database;
		QMutex Locker;

		volatile bool isTerminated = false;

	public:

		AppCore(void);

		int AddItem(Entry& Item);
		bool RemoveItem(int ID);
		bool UpdateItem(Entry& Item);

		Entry getItem(int ID);
		QMap<int, Entry> getItems(void);

		static AppCore* getInstance(void);

	public slots:

		void LoadItems(void);

		void LoadData(const QString& Path,
				    const QString& CoderName);
		void SaveData(const QString& Path,
				    const QStringList& Header,
				    const QList<QStringList>& Data,
				    const QString& CoderName,
				    const QString& Newline);
		void ConvertData(const QList<QStringList>& Data);
		void ReplaceData(const QList<QStringList>& Data,
					  const QString& Source,
					  const QString& Replace,
					  bool Case, bool RegExp);
		void UpdateValues(const QList<QStringList>& Data,
					   const QString &Source,
					   const QString &Replace,
					   bool Case, bool RegExp);
		void DeleteData(const QList<QStringList>& Data,
					 const QStringList& Classes,
					 const QMap<QString, QString>& List);

		void Terminate(void);

	signals:

		void onHeaderLoad(const QStringList&);
		void onItemsLoad(const QMap<int, Entry>&);
		void onObjectsLoad(const QList<QStringList>&);

		void onObjectsConvert(const QList<QStringList>&);
		void onDataReplace(const QList<QStringList>, int);
		void onValuesUpdate(const QList<QStringList>, int);
		void onDataDelete(const QList<QStringList>, int);

		void onOutputSave(bool);

		void onProgressInit(int, int);
		void onProgressUpdate(int);

		void onTerminateRequest(void);

};

#endif // APPCORE_HPP
