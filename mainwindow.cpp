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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* Parent)
: QMainWindow(Parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	About = new AboutDialog(this);
	Replace = new ReplaceDialog(this);
	Setvalue = new SetvalueDialog(this);
	Delete = new DeleteDialog(this);
	Unpinn = new UnpinnDialog(this);
	Split = new SplitDialog(this);
	Insert = new InsertDialog(this);
	Progress = new QProgressBar(this);
	Codecs = new QComboBox(this);

	Progress->setRange(0, 100);

	Codecs->addItems(QStringList() << "Windows-1250" << "UTF-8");
	Codecs->setLayoutDirection(Qt::LeftToRight);

	ui->Tree->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->statusBar->addPermanentWidget(Progress);
	ui->actionStop->setEnabled(false);
	ui->optionTools->addWidget(Codecs);

	LockUI(false);

	connect(this, &MainWindow::onOpenRequest, AppCore::getInstance(), &AppCore::LoadData);
	connect(this, &MainWindow::onSaveRequest, AppCore::getInstance(), &AppCore::SaveData);
	connect(this, &MainWindow::onConvertRequest, AppCore::getInstance(), &AppCore::ConvertData);
	connect(this, &MainWindow::onReplaceRequest, AppCore::getInstance(), &AppCore::ReplaceData);
	connect(this, &MainWindow::onSetvalueRequest, AppCore::getInstance(), &AppCore::UpdateData);
	connect(this, &MainWindow::onDeleteRequest, AppCore::getInstance(), &AppCore::DeleteData);
	connect(this, &MainWindow::onUnpinnRequest, AppCore::getInstance(), &AppCore::UnpinnData);
	connect(this, &MainWindow::onSplitRequest, AppCore::getInstance(), &AppCore::SplitData);
	connect(this, &MainWindow::onInsertRequest, AppCore::getInstance(), &AppCore::InsertData);

	connect(AppCore::getInstance(), &AppCore::onObjectsLoad, this, &MainWindow::FinishLoad);
	connect(AppCore::getInstance(), &AppCore::onDataConvert, this, &MainWindow::FinishConvert);
	connect(AppCore::getInstance(), &AppCore::onOutputSave, this, &MainWindow::FinishSave);
	connect(AppCore::getInstance(), &AppCore::onDataReplace, this, &MainWindow::FinishReplace);
	connect(AppCore::getInstance(), &AppCore::onDataUpdate, this, &MainWindow::FinishSetting);
	connect(AppCore::getInstance(), &AppCore::onDataDelete, this, &MainWindow::FinishDeleting);
	connect(AppCore::getInstance(), &AppCore::onDataUnpinn, this, &MainWindow::FinishUnpinning);
	connect(AppCore::getInstance(), &AppCore::onDataSplit, this, &MainWindow::FinishSplitting);
	connect(AppCore::getInstance(), &AppCore::onDataInsert, this, &MainWindow::FinishInserting);

	connect(ui->actionAbout, &QAction::triggered, About, &AboutDialog::show);

	connect(ui->actionReplace, &QAction::triggered, Replace, &ReplaceDialog::open);
	connect(Replace, &ReplaceDialog::onReplaceRequest, this, &MainWindow::InitReplace);
	connect(this, &MainWindow::onReplaceFinish, Replace, &ReplaceDialog::ShowProgress);
	connect(Replace, &ReplaceDialog::onRefreshRequest, this, &MainWindow::LoadTree);

	connect(ui->actionSetvalue, &QAction::triggered, Setvalue, &SetvalueDialog::open);
	connect(Setvalue, &SetvalueDialog::onSetvalueRequest, this, &MainWindow::InitSetting);
	connect(this, &MainWindow::onSettingFinish, Setvalue, &SetvalueDialog::ShowProgress);
	connect(Setvalue, &SetvalueDialog::onRefreshRequest, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onClassesLoad, Setvalue, &SetvalueDialog::UpdateList);

	connect(ui->actionDelete, &QAction::triggered, Delete, &DeleteDialog::open);
	connect(Delete, &DeleteDialog::onDeleteRequest, this, &MainWindow::InitDeleting);
	connect(this, &MainWindow::onDeletingFinish, Delete, &DeleteDialog::ShowProgress);
	connect(Delete, &DeleteDialog::onRefreshRequest, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onClassesLoad, Delete, &DeleteDialog::UpdateList);

	connect(ui->actionUnpinn, &QAction::triggered, Unpinn, &UnpinnDialog::open);
	connect(Unpinn, &UnpinnDialog::onUnpinnRequest, this, &MainWindow::InitUnpinning);
	connect(this, &MainWindow::onUnpinningFinish, Unpinn, &UnpinnDialog::ShowProgress);
	connect(Unpinn, &UnpinnDialog::onRefreshRequest, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onClassesLoad, Unpinn, &UnpinnDialog::UpdateList);

	connect(ui->actionSplit, &QAction::triggered, Split, &SplitDialog::open);
	connect(Split, &SplitDialog::onSplitRequest, this, &MainWindow::InitSplitting);
	connect(this, &MainWindow::onSplittingFinish, Split, &SplitDialog::ShowProgress);
	connect(Split, &SplitDialog::onRefreshRequest, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onClassesLoad, Split, &SplitDialog::UpdateList);

	connect(ui->actionInsert, &QAction::triggered, Insert, &InsertDialog::open);
	connect(Insert, &InsertDialog::onInsertRequest, this, &MainWindow::InitInserting);
	connect(this, &MainWindow::onInsertingFinish, Insert, &InsertDialog::ShowProgress);
	connect(Insert, &InsertDialog::onRefreshRequest, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onClassesLoad, Insert, &InsertDialog::UpdateList);

	connect(ui->Tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::TreeMenuRequest);

	connect(AppCore::getInstance(), &AppCore::onProgressInit, Progress, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, Progress, &QProgressBar::setValue);

	connect(ui->actionStop, &QAction::triggered, AppCore::getInstance(), &AppCore::Terminate, Qt::DirectConnection);
}

MainWindow::~MainWindow(void)
{
	ClearDAT(); delete ui;
}

void MainWindow::LockUI(bool Lock, const QString& Message)
{
	Progress->setVisible(Lock);
	Codecs->setEnabled(!Lock);

	ui->Tree->setEnabled(!Lock);

	ui->actionOpen->setEnabled(!Lock);
	ui->actionEdit->setEnabled(!Lock);
	ui->actionStop->setEnabled(Lock);

	if (Lock)
	{
		ui->actionConvert->setEnabled(false);
		ui->actionReplace->setEnabled(false);
		ui->actionSave->setEnabled(false);
		ui->actionDelete->setEnabled(false);
		ui->actionUnpinn->setEnabled(false);
		ui->actionSplit->setEnabled(false);
		ui->actionInsert->setEnabled(false);
		ui->actionSetvalue->setEnabled(false);
		ui->actionClear->setEnabled(false);
	}
	else
	{
		ui->actionConvert->setEnabled(loadedData);
		ui->actionReplace->setEnabled(loadedData);
		ui->actionSave->setEnabled(loadedData);
		ui->actionDelete->setEnabled(loadedData);
		ui->actionUnpinn->setEnabled(loadedData);
		ui->actionSplit->setEnabled(loadedData);
		ui->actionInsert->setEnabled(loadedData);
		ui->actionSetvalue->setEnabled(loadedData);
		ui->actionClear->setEnabled(loadedData);
	}

	ui->actionUndo->setEnabled(!Lock && !Undo.isEmpty());
	ui->actionRedo->setEnabled(!Lock && !Redo.isEmpty());

	if (Message.size()) ui->statusBar->showMessage(Message);
}

void MainWindow::SaveDAT(const QList<QStringList>& Data, const QString& Action)
{
	if (loadedData)
	{
		SavedActions = SavedActions.mid(0, Undo.size()) << Action;

		for (auto I : Redo) delete I;

		Undo.append(loadedData);
		Redo.clear();

		loadedData = nullptr;

		ui->actionUndo->setToolTip(tr("Undo action \"%1\".").arg(Action));
		ui->actionRedo->setToolTip(tr("Redo last commit."));
	}

	loadedData = new QList<QStringList>(Data);
}

void MainWindow::ClearDAT(void)
{
	for (auto I : Redo) delete I;
	for (auto I : Undo) delete I;

	if (loadedData) delete loadedData;

	loadedData = nullptr;
	Redo.clear();
	Undo.clear();
	SavedActions.clear();

	ui->Tree->clear();

	LockUI(false);
}

void MainWindow::ConvertActionClicked(void)
{
	if (!loadedData->isEmpty())
	{
		LockUI(true, tr("Converting data..."));

		emit onConvertRequest(*loadedData);
	}
	else QMessageBox::warning(this, tr("Error"),
						 tr("No data in buffer"));
}

void MainWindow::EditActionClicked(void)
{
	ChangesDialog* Dialog = new ChangesDialog(this);

	connect(Dialog, &ChangesDialog::accepted,
		   Dialog, &ChangesDialog::deleteLater);

	connect(Dialog, &ChangesDialog::rejected,
		   Dialog, &ChangesDialog::deleteLater);

	Dialog->open();
}

void MainWindow::OpenActionClicked(void)
{
	const QString Path = QFileDialog::getOpenFileName(this);

	if (Path.size())
	{
		LockUI(true, tr("Loading data..."));
		emit onOpenRequest(Path, Codecs->currentText());
	}
}

void MainWindow::SaveActionClicked(void)
{
	const QString Path = QFileDialog::getSaveFileName(this);

	if (!Path.isEmpty())
	{
		LockUI(true, tr("Saving data..."));

		emit onSaveRequest(Path, loadedHeader, *loadedData,
					    Codecs->currentText());
	}
}

void MainWindow::UndoActionClicked(void)
{
	if (!Undo.isEmpty())
	{
		Redo.append(loadedData);

		loadedData = Undo.takeLast();

		LoadTree();
	}

	ui->actionUndo->setEnabled(!Undo.isEmpty());

	if (!Undo.isEmpty())
	{
		ui->actionUndo->setToolTip(tr("Undo action \"%1\".").arg(SavedActions[Undo.size() - 1]));
	}
	else
	{
		ui->actionUndo->setToolTip(tr("Undo last commit."));
	}

	ui->actionRedo->setToolTip(tr("Redo action \"%1\".").arg(SavedActions[Redo.size() - 1]));
}

void MainWindow::RedoActionClicked(void)
{
	if (!Redo.isEmpty())
	{
		Undo.append(loadedData);

		loadedData = Redo.takeLast();

		LoadTree();
	}

	ui->actionRedo->setEnabled(!Redo.isEmpty());

	if (!Redo.isEmpty())
	{
		ui->actionRedo->setToolTip(tr("Redo action \"%1\".").arg(SavedActions[Redo.size() - 1]));
	}
	else
	{
		ui->actionRedo->setToolTip(tr("Redo last commit."));
	}

	ui->actionUndo->setToolTip(tr("Undo action \"%1\".").arg(SavedActions[Undo.size() - 1]));
}

void MainWindow::ClearActionClicked(void)
{
	ClearDAT(); ui->statusBar->showMessage(tr("Data cleared"));
}

void MainWindow::TreeMenuRequest(const QPoint& Pos)
{
	QTreeWidgetItem* I = ui->Tree->itemAt(Pos);

	if (I && I->parent())
	{
		QMenu Menu(this);

		QAction* allCopy = Menu.addAction(QIcon::fromTheme("edit-copy"), tr("Copy all"));
		Menu.addSeparator();
		QAction* keyCopy = Menu.addAction(QIcon::fromTheme("dialog-password"), tr("Copy key"));
		QAction* valueCopy = Menu.addAction(QIcon::fromTheme("accessories-calculator"), tr("Copy value"));

		QAction* A = Menu.exec(ui->Tree->mapToGlobal(Pos));

		if (A == allCopy)
		{
			if (I->text(0)[0] == 'A' || I->text(0)[0] == 'B')
				QApplication::clipboard()->setText(QString("%1,%2").arg(I->text(0)).arg(I->text(1)));
			else
				QApplication::clipboard()->setText(QString("C,%1=%2").arg(I->text(0)).arg(I->text(1)));
		}
		else if (A == keyCopy)
		{
			if (I->text(0)[0] == 'A' || I->text(0)[0] == 'B')
				QApplication::clipboard()->setText(QString(I->text(0)));
			else
				QApplication::clipboard()->setText(QString("C,%1").arg(I->text(0)));
		}
		else if (A == valueCopy)
		{
			QApplication::clipboard()->setText(I->text(1));
		}
	}
}

void MainWindow::LoadTree(void)
{
	QList<QTreeWidgetItem*> Items;
	QRegExp objectExpr("^C,([^=]*)=(.*)");

	int Step = 0;
	int ID = 0;

	LockUI(true, tr("Populating tree..."));

	ui->Tree->clear();

	Progress->setRange(0, loadedData->size());

	for (const auto& Item : *loadedData)
	{
		QTreeWidgetItem* Entry = new QTreeWidgetItem(QStringList(QString::number(++ID)));
		QList<QTreeWidgetItem*> Values;

		for (const auto& Data : Item)
		{
			if (Data[0] == 'C')
			{
				objectExpr.indexIn(Data);

				QStringList Atributes = objectExpr.capturedTexts();

				Atributes.removeFirst();

				Values.append(new QTreeWidgetItem(Atributes));
			}
			else
			{
				Values.append(new QTreeWidgetItem(QStringList() << Data[0] << Data.mid(2)));
			}
		}

		Entry->addChildren(Values);
		Items.append(Entry);

		Progress->setValue(++Step);

		thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
	}

	ui->Tree->addTopLevelItems(Items);

	LockUI(false, tr("Action complete, loaded %n item(s)", 0, loadedData->count()));
}

void MainWindow::FinishLoad(const QStringList Head, const QList<QStringList>& Data)
{
	ClearActionClicked();

	loadedHeader = Head;

	SaveDAT(Data, tr("Open data file"));
	LoadTree();
}

void MainWindow::FinishSave(unsigned Crc32)
{
	if (Crc32)
	{
		LockUI(false, tr("File saved, checksum is %1").arg(Crc32, 8, 16, QChar(0)));
	}
	else
	{
		QMessageBox::warning(this, tr("Error"), tr("Error while saving file"));
		LockUI(false, tr("Error while saving file"));
	}
}

void MainWindow::FinishConvert(const QList<QStringList>& Data)
{
	SaveDAT(Data, tr("Convert data")); LoadTree();
}

void MainWindow::InitReplace(const QString& From, const QString& To, bool Case, bool RegExp)
{
	emit onReplaceRequest(*loadedData, From, To, Case, RegExp);
}

void MainWindow::FinishReplace(const QList<QStringList>& Data, int Count)
{
	SaveDAT(Data, tr("Replace data")); emit onReplaceFinish(Count);
}

void MainWindow::InitSetting(const QString &Field, const QString& Value, const QStringList& Classes, const QMap<QString, QString>& Values)
{
	emit onSetvalueRequest(*loadedData, Field, Value, Classes, Values);
}

void MainWindow::FinishSetting(const QList<QStringList>& Data, int Count)
{
	SaveDAT(Data, tr("Set value")); emit onSettingFinish(Count);
}

void MainWindow::InitDeleting(const QStringList& Classes, const QMap<QString, QString>& Values)
{
	emit onDeleteRequest(*loadedData, Classes, Values);
}

void MainWindow::FinishDeleting(const QList<QStringList>& Data, int Count)
{
	SaveDAT(Data, tr("Delete data")); emit onDeletingFinish(Count);
}

void MainWindow::InitUnpinning(const QStringList& Classes, bool Delete, bool Keep)
{
	emit onUnpinnRequest(*loadedData, Classes, Delete, Keep);
}

void MainWindow::FinishUnpinning(const QList<QStringList>& Data, int Count)
{
	SaveDAT(Data, tr("Unpinn data")); emit onUnpinningFinish(Count);
}

void MainWindow::InitSplitting(const QStringList& Classes, bool Keep, bool Hide)
{
	emit onSplitRequest(*loadedData, Classes, Keep, Hide);
}

void MainWindow::FinishSplitting(const QList<QStringList>& Data, int Count)
{
	SaveDAT(Data, tr("Split data")); emit onSplittingFinish(Count);
}

void MainWindow::InitInserting(const QStringList &Classes, const QString &Insert)
{
	emit onInsertRequest(*loadedData, Classes, Insert);
}

void MainWindow::FinishInserting(const QList<QStringList> &Data, int Count)
{
	SaveDAT(Data, tr("Insert data")); emit onInsertingFinish(Count);
}
