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
	Setvalue = new ReplaceDialog(this);
	Delete = new DeleteDialog(this);
	Progress = new QProgressBar(this);
	Codecs = new QComboBox(this);
	Newline = new QComboBox(this);

	Progress->setRange(0, 100);
	Progress->setVisible(false);

	Codecs->addItems(QStringList() << "Windows-1250" << "UTF-8");
	Codecs->setLayoutDirection(Qt::LeftToRight);

	Newline->setLayoutDirection(Qt::LeftToRight);
	Newline->addItem("CRLF", "\r\n");
	Newline->addItem("LF", "\n");
	Newline->addItem("CR", "\r");

	ui->Tree->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->statusBar->addPermanentWidget(Progress);
	ui->actionStop->setEnabled(false);
	ui->optionTools->addWidget(Newline);
	ui->optionTools->addWidget(Codecs);

	connect(this, &MainWindow::onOpenRequest, AppCore::getInstance(), &AppCore::LoadData);
	connect(this, &MainWindow::onSaveRequest, AppCore::getInstance(), &AppCore::SaveData);
	connect(this, &MainWindow::onConvertRequest, AppCore::getInstance(), &AppCore::ConvertData);
	connect(this, &MainWindow::onReplaceRequest, AppCore::getInstance(), &AppCore::ReplaceData);
	connect(this, &MainWindow::onSetvalueRequest, AppCore::getInstance(), &AppCore::UpdateValues);
	connect(this, &MainWindow::onDeleteRequest, AppCore::getInstance(), &AppCore::DeleteData);

	connect(AppCore::getInstance(), &AppCore::onHeaderLoad, this, &MainWindow::LoadHeader);
	connect(AppCore::getInstance(), &AppCore::onObjectsLoad, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onObjectsConvert, this, &MainWindow::LoadTree);
	connect(AppCore::getInstance(), &AppCore::onOutputSave, this, &MainWindow::FinishSave);
	connect(AppCore::getInstance(), &AppCore::onDataReplace, this, &MainWindow::FinishReplace);
	connect(AppCore::getInstance(), &AppCore::onValuesUpdate, this, &MainWindow::FinishSetting);
	connect(AppCore::getInstance(), &AppCore::onDataDelete, this, &MainWindow::FinishDeleting);

	connect(ui->actionAbout, &QAction::triggered, About, &AboutDialog::show);

	connect(ui->actionReplace, &QAction::triggered, Replace, &ReplaceDialog::open);
	connect(Replace, &ReplaceDialog::onReplaceRequest, this, &MainWindow::InitReplace);
	connect(this, &MainWindow::onReplaceFinish, Replace, &ReplaceDialog::ShowProgress);
	connect(Replace, &ReplaceDialog::onRefreshRequest, this, &MainWindow::UpdateTree);

	connect(ui->actionSetvalue, &QAction::triggered, Setvalue, &ReplaceDialog::open);
	connect(Setvalue, &ReplaceDialog::onReplaceRequest, this, &MainWindow::InitSetting);
	connect(this, &MainWindow::onSettingFinish, Setvalue, &ReplaceDialog::ShowProgress);
	connect(Setvalue, &ReplaceDialog::onRefreshRequest, this, &MainWindow::UpdateTree);

	connect(ui->actionDelete, &QAction::triggered, Delete, &DeleteDialog::open);
	connect(Delete, &DeleteDialog::onDeleteRequest, this, &MainWindow::InitDeleting);
	connect(this, &MainWindow::onDeletingFinish, Delete, &DeleteDialog::ShowProgress);
	connect(Delete, &DeleteDialog::onRefreshRequest, this, &MainWindow::UpdateTree);

	connect(ui->Tree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::TreeMenuRequest);

	connect(AppCore::getInstance(), &AppCore::onProgressInit, Progress, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, Progress, &QProgressBar::setValue);

	connect(ui->actionStop, &QAction::triggered, AppCore::getInstance(), &AppCore::Terminate, Qt::DirectConnection);
}

MainWindow::~MainWindow(void)
{
	delete ui;
}

void MainWindow::LockUI(bool Lock, const QString& Message)
{
	Progress->setVisible(Lock);
	Codecs->setVisible(!Lock);

	ui->Tree->setEnabled(!Lock);

	ui->actionConvert->setEnabled(!Lock);
	ui->actionEdit->setEnabled(!Lock);
	ui->actionOpen->setEnabled(!Lock);
	ui->actionReplace->setEnabled(!Lock);
	ui->actionSave->setEnabled(!Lock);
	ui->actionTools->setEnabled(!Lock);

	ui->actionStop->setEnabled(Lock);

	if (Message.size()) ui->statusBar->showMessage(Message);
}

void MainWindow::ConvertActionClicked(void)
{
	if (!loadedData.isEmpty())
	{
		LockUI(true, tr("Converting data..."));
		emit onConvertRequest(loadedData);
	}
	else QMessageBox::warning(this, tr("Error"), tr("No data in buffer"));
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
		emit onSaveRequest(Path, loadedHeader, loadedData, Codecs->currentText(), Newline->currentData().toString());
	}
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

void MainWindow::LoadTree(const QList<QStringList>& Data)
{
	QList<QTreeWidgetItem*> Items;
	int Step = 0;
	int ID = 0;

	loadedData = Data;

	ui->statusBar->showMessage(tr("Clearing tree..."));
	ui->Tree->clear();
	ui->statusBar->showMessage(tr("Populating tree..."));
	ui->actionStop->setEnabled(false);

	Progress->setRange(0, loadedData.size());

	for (const auto& Item : loadedData)
	{
		QTreeWidgetItem* Entry = new QTreeWidgetItem(QStringList(QString::number(++ID)));
		QList<QTreeWidgetItem*> Values;

		for (const auto& Data : Item)
		{
			if (Data[0] == 'A' || Data[0] == 'B')
			{
				Values.append(new QTreeWidgetItem(QStringList() << Data[0] << Data.mid(2)));
			}
			else if (Data[0] == 'C')
			{
				QRegExp objectExpr("C,(.*)=(.*)");

				objectExpr.indexIn(Data);

				QStringList Atributes = objectExpr.capturedTexts();

				Atributes.removeFirst();

				Values.append(new QTreeWidgetItem(Atributes));
			}
		}

		Entry->addChildren(Values);
		Items.append(Entry);

		Progress->setValue(++Step);

		thread()->eventDispatcher()->processEvents(QEventLoop::AllEvents);
	}

	ui->Tree->addTopLevelItems(Items);

	LockUI(false, tr("Action complete. Loaded %n item(s)", 0, loadedData.count()));
}

void MainWindow::LoadHeader(const QStringList& Header)
{
	loadedHeader = Header;
}

void MainWindow::FinishSave(bool OK)
{
	if (OK)
	{
		LockUI(false, tr("File saved"));
	}
	else
	{
		QMessageBox::warning(this, tr("Error"), tr("Error while saving file"));
		LockUI(false, tr("Error while saving file"));
	}
}

void MainWindow::InitReplace(const QString& From, const QString& To, bool Case, bool RegExp)
{
	emit onReplaceRequest(loadedData, From, To, Case, RegExp);
}

void MainWindow::FinishReplace(const QList<QStringList>& Data, int Count)
{
	loadedData = Data;

	emit onReplaceFinish(Count);
}

void MainWindow::InitSetting(const QString &From, const QString &To, bool Case, bool RegExp)
{
	emit onSetvalueRequest(loadedData, From, To, Case, RegExp);
}

void MainWindow::FinishSetting(const QList<QStringList>& Data, int Count)
{
	loadedData = Data;

	emit onSettingFinish(Count);
}

void MainWindow::InitDeleting(const QStringList& Classes, const QMap<QString, QString>& Values)
{
	emit onDeleteRequest(loadedData, Classes, Values);
}

void MainWindow::FinishDeleting(const QList<QStringList>& Data, int Count)
{
	loadedData = Data;

	emit onDeletingFinish(Count);
}

void MainWindow::UpdateTree(void)
{
	LockUI(true, tr("Loading data..."));
	LoadTree(loadedData);
	LockUI(false, tr("Action complete. Loaded %n item(s)", 0, loadedData.count()));
}
