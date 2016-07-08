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

#include "replacedialog.hpp"
#include "ui_replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::ReplaceDialog)
{
	ui->setupUi(this);

	ui->progressBar->setVisible(false);

	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, this, &ReplaceDialog::UpdateProgress);
}

ReplaceDialog::~ReplaceDialog(void)
{
	delete ui;
}

void ReplaceDialog::accept(void)
{
	if (ui->Find->text().size())
	{
		ui->procedButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->progressBar->setVisible(true);

		emit onReplaceRequest(ui->Find->text(),
						  ui->Replace->text(),
						  ui->caseCheck->isChecked(),
						  ui->regexpCheck->isChecked());
	}
	else QMessageBox::warning(this, tr("Error"), tr("Text to find is empty"));
}

void ReplaceDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		if (Replaces) emit onRefreshRequest();

		Replaces = 0;
	}
	else QMessageBox::warning(this, tr("Error"), tr("Replacing in progress"));
}

void ReplaceDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Replace progress"),
						tr("Replaced %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Replaces += Count;
}

void ReplaceDialog::UpdateProgress(double Value)
{
	ui->progressBar->setValue(100 * Value);
}
