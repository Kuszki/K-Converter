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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QAbstractEventDispatcher>
#include <QTreeWidgetItem>
#include <QProgressBar>
#include <QTreeWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QClipboard>
#include <QComboBox>
#include <QThread>
#include <QAction>
#include <QLabel>
#include <QMenu>

#include "changesdialog.hpp"
#include "replacedialog.hpp"
#include "deletedialog.hpp"
#include "unpinndialog.hpp"
#include "aboutdialog.hpp"
#include "appcore.hpp"

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{

		Q_OBJECT

	private:

		Ui::MainWindow* ui;

		AboutDialog* About;
		ReplaceDialog* Replace;
		ReplaceDialog* Setvalue;
		DeleteDialog* Delete;
		UnpinnDialog* Unpinn;

		QProgressBar* Progress;
		QComboBox* Codecs;

		QList<QStringList> loadedData;
		QStringList loadedHeader;

	private:

		void LockUI(bool Lock = true, const QString& Message = QString());

	public:

		explicit MainWindow(QWidget* Parent = nullptr);
		virtual ~MainWindow(void) override;

	private slots:

		void ConvertActionClicked(void);
		void EditActionClicked(void);
		void OpenActionClicked(void);
		void SaveActionClicked(void);

		void TreeMenuRequest(const QPoint& Pos);

		void LoadTree(const QList<QStringList>& Data);
		void LoadHeader(const QStringList& Header);

		void FinishSave(bool OK);

		void InitReplace(const QString& From, const QString& To, bool Case, bool RegExp);
		void FinishReplace(const QList<QStringList>& Data, int Count);
		void InitSetting(const QString& From, const QString& To, bool Case, bool RegExp);
		void FinishSetting(const QList<QStringList>& Data, int Count);
		void InitDeleting(const QStringList& Classes, const QMap<QString, QString>& Values);
		void FinishDeleting(const QList<QStringList>& Data, int Count);
		void InitUnpinning(const QStringList& Classes);
		void FinishUnpinning(const QList<QStringList>& Data, int Count);

		void UpdateTree(void);

	signals:

		void onOpenRequest(const QString&,
					    const QString&);
		void onConvertRequest(const QList<QStringList>&);
		void onSaveRequest(const QString&,
					    const QStringList&,
					    const QList<QStringList>&,
					    const QString&);
		void onReplaceRequest(const QList<QStringList>&,
						  const QString&,
						  const QString&,
						  bool, bool);
		void onSetvalueRequest(const QList<QStringList>&,
						   const QString&,
						   const QString&,
						   bool, bool);
		void onDeleteRequest(const QList<QStringList>&,
						 const QStringList&,
						 const QMap<QString, QString>&);
		void onUnpinnRequest(const QList<QStringList>&,
						 const QStringList&);

		void onReplaceFinish(int);
		void onSettingFinish(int);
		void onDeletingFinish(int);
		void onUnpinningFinish(int);

};

#endif // MAINWINDOW_HPP
