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

#include "unpinndialog.hpp"
#include "ui_unpinndialog.h"

UnpinnDialog::UnpinnDialog(QWidget *Parent)
: QDialog(Parent), ui(new Ui::UnpinnDialog)
{
	QRegExpValidator* Validator = new QRegExpValidator(QRegExp("[A-z]+(?:,[A-z]+)*"), this);

	ui->setupUi(this);

	ui->progressBar->setVisible(false);
	ui->Class->setValidator(Validator);
}

UnpinnDialog::~UnpinnDialog(void)
{
	delete ui;
}

void UnpinnDialog::open(void)
{
	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

	QDialog::open();
}

void UnpinnDialog::accept(void)
{
	if (ui->Class->text().size())
	{
		ui->procedButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->progressBar->setVisible(true);

		emit onUnpinnRequest(ui->Class->text().split(',', QString::SkipEmptyParts),
						 ui->Delete->isChecked());
	}
	else QMessageBox::warning(this, tr("Error"), tr("Classes list is empty"));
}

void UnpinnDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		disconnect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
		disconnect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

		if (Unpinned) { emit onRefreshRequest(); Unpinned = 0; }
	}
	else QMessageBox::warning(this, tr("Error"), tr("Work in progress"));
}

void UnpinnDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Unpinn progress"),
						tr("Unpinned %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Unpinned += Count;
}
