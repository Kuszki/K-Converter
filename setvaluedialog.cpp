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

#include "setvaluedialog.hpp"
#include "ui_setvaluedialog.h"

SetvalueDialog::SetvalueDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::SetvalueDialog)
{
	QRegExpValidator* Validator = new QRegExpValidator(QRegExp("[A-z]+(?:,[A-z]+)*"), this);

	ui->setupUi(this);

	ui->progressBar->setVisible(false);
	ui->Class->setValidator(Validator);
}

SetvalueDialog::~SetvalueDialog(void)
{
	delete ui;
}

void SetvalueDialog::AddButtonClicked(void)
{
	const int Rows = ui->Values->rowCount();

	const bool Sorting = ui->Values->isSortingEnabled();

	ui->Values->setSortingEnabled(false);
	ui->Values->insertRow(Rows);

	ui->Values->setItem(Rows, 0, new QTableWidgetItem(tr("key")));
	ui->Values->setItem(Rows, 1, new QTableWidgetItem(tr("value")));

	ui->Values->setSortingEnabled(Sorting);
}

void SetvalueDialog::RemoveButtonClicked(void)
{
	const int Row = ui->Values->currentRow();

	if (Row != -1) ui->Values->removeRow(Row);
}

void SetvalueDialog::open(void)
{
	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

	QDialog::open();
}

void SetvalueDialog::accept(void)
{
	if (ui->Field->text().size())
	{
		const int Count = ui->Values->rowCount();

		QMap<QString, QString> Values;

		for (int i = 0; i < Count; ++i) Values.insert(ui->Values->item(i, 0)->text(), (ui->Values->item(i, 1)->text()));

		if (Values.size() == Count)
		{
			const QStringList Classes = !ui->Class->currentText().isEmpty() ?
									   ui->Class->currentText().split(',', QString::SkipEmptyParts) :
									   QStringList() << ".*";

			ui->procedButton->setEnabled(false);
			ui->cancelButton->setEnabled(false);
			ui->progressBar->setVisible(true);

			emit onSetvalueRequest(ui->Field->text(), ui->Value->text(), Classes, Values);
		}
		else QMessageBox::warning(this, tr("Error"), tr("Found duplicated keys"));
	}
	else QMessageBox::warning(this, tr("Error"), tr("Text to find is empty"));
}

void SetvalueDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		disconnect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
		disconnect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

		if (Replaces) { emit onRefreshRequest(); Replaces = 0; }
	}
	else QMessageBox::warning(this, tr("Error"), tr("Replacing in progress"));
}

void SetvalueDialog::UpdateList(const QStringList& Classes)
{
	ui->Class->clear();
	ui->Class->addItems(Classes);
	ui->Class->setCurrentText("");
}

void SetvalueDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Replace progress"),
						tr("Replaced %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Replaces += Count;
}
