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

#include "deletedialog.hpp"
#include "ui_deletedialog.h"

DeleteDialog::DeleteDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::DeleteDialog)
{
	ui->setupUi(this);

	ui->progressBar->setVisible(false);

	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);
}

DeleteDialog::~DeleteDialog(void)
{
	delete ui;
}

void DeleteDialog::AddButtonClicked(void)
{
	const int Rows = ui->Values->rowCount();

	const bool Sorting = ui->Values->isSortingEnabled();

	ui->Values->setSortingEnabled(false);
	ui->Values->insertRow(Rows);

	ui->Values->setItem(Rows, 0, new QTableWidgetItem(tr("key")));
	ui->Values->setItem(Rows, 1, new QTableWidgetItem(tr("value")));

	ui->Values->setSortingEnabled(Sorting);
}

void DeleteDialog::RemoveButtonClicked(void)
{
	const int Row = ui->Values->currentRow();

	if (Row != -1) ui->Values->removeRow(Row);
}

void DeleteDialog::accept(void)
{
	if (ui->Class->text().size())
	{
		const int Count = ui->Values->rowCount();

		QMap<QString, QString> Values;

		for (int i = 0; i < Count; ++i) Values.insert(ui->Values->item(i, 0)->text(), (ui->Values->item(i, 1)->text()));

		if (Values.size() == Count)
		{
			ui->procedButton->setEnabled(false);
			ui->cancelButton->setEnabled(false);
			ui->progressBar->setVisible(true);

			emit onDeleteRequest(ui->Class->text().split(','), Values);
		}
		else QMessageBox::warning(this, tr("Error"), tr("Found duplicated keys"));
	}
	else QMessageBox::warning(this, tr("Error"), tr("Class to delete is empty"));
}

void DeleteDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		if (Deleted) emit onRefreshRequest();

		Deleted = 0;
	}
	else QMessageBox::warning(this, tr("Error"), tr("Deleting in progress"));
}

void DeleteDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Delete progress"),
						tr("Deleted %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Deleted += Count;
}
