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

#include "setvaluedialog.hpp"
#include "changesdialog.hpp"
#include "replacedialog.hpp"
#include "deletedialog.hpp"
#include "unpinndialog.hpp"
#include "splitdialog.hpp"
#include "insertdialog.hpp"
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

		Ui::MainWindow* ui = nullptr;

		AboutDialog* About = nullptr;
		ReplaceDialog* Replace = nullptr;
		SetvalueDialog* Setvalue = nullptr;
		DeleteDialog* Delete = nullptr;
		UnpinnDialog* Unpinn = nullptr;
		SplitDialog* Split = nullptr;
		InsertDialog* Insert = nullptr;

		QProgressBar* Progress = nullptr;
		QComboBox* Codecs = nullptr;

		QList<QStringList>* loadedData = nullptr;
		QStringList loadedHeader;

		QList<QList<QStringList>*> Undo;
		QList<QList<QStringList>*> Redo;
		QStringList SavedActions;

	private:

		void LockUI(bool Lock = true, const QString& Message = QString());
		void SaveDAT(const QList<QStringList>& Data, const QString& Action);
		void ClearDAT(void);

	public:

		explicit MainWindow(QWidget* Parent = nullptr);
		virtual ~MainWindow(void) override;

	private slots:

		void ConvertActionClicked(void);
		void EditActionClicked(void);
		void OpenActionClicked(void);
		void SaveActionClicked(void);
		void UndoActionClicked(void);
		void RedoActionClicked(void);
		void ClearActionClicked(void);

		void TreeMenuRequest(const QPoint& Pos);

		void LoadTree(void);

		void FinishLoad(const QStringList Head, const QList<QStringList>& Data);
		void FinishSave(unsigned Crc32);
		void FinishConvert(const QList<QStringList>& Data);

		void InitReplace(const QString& From, const QString& To, bool Case, bool RegExp);
		void FinishReplace(const QList<QStringList>& Data, int Count);
		void InitSetting(const QString& Field, const QString& Value, const QStringList& Classes, const QMap<QString, QString>& Values);
		void FinishSetting(const QList<QStringList>& Data, int Count);
		void InitDeleting(const QStringList& Classes, const QMap<QString, QString>& Values);
		void FinishDeleting(const QList<QStringList>& Data, int Count);
		void InitUnpinning(const QStringList& Classes, bool Delete, bool Keep);
		void FinishUnpinning(const QList<QStringList>& Data, int Count);
		void InitSplitting(const QStringList& Classes, bool Keep, bool Hide);
		void FinishSplitting(const QList<QStringList>& Data, int Count);
		void InitInserting(const QStringList& Classes, const QString& Insert);
		void FinishInserting(const QList<QStringList>& Data, int Count);

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
						   const QStringList&,
						   const QMap<QString, QString>&);
		void onDeleteRequest(const QList<QStringList>&,
						 const QStringList&,
						 const QMap<QString, QString>&);
		void onUnpinnRequest(const QList<QStringList>&,
						 const QStringList&,
						 bool, bool);
		void onSplitRequest(const QList<QStringList>&,
						const QStringList&,
						bool, bool);
		void onInsertRequest(const QList<QStringList>&,
						 const QStringList&,
						 const QString&);

		void onReplaceFinish(int);
		void onSettingFinish(int);
		void onDeletingFinish(int);
		void onUnpinningFinish(int);
		void onSplittingFinish(int);
		void onInsertingFinish(int);

};

#endif // MAINWINDOW_HPP
