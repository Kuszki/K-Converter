/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  {description}                                                          *
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

#include "revertdialog.hpp"
#include "ui_revertdialog.h"

RevertDialog::RevertDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::RevertDialog)
{
	QRegExpValidator* Validator = new QRegExpValidator(QRegExp("[A-z]+(?:,[A-z]+)*"), this);

	ui->setupUi(this);

	ui->progressBar->setVisible(false);
	ui->Class->setValidator(Validator);
	ui->Begin->setValidator(Validator);
	ui->End->setValidator(Validator);
}

RevertDialog::~RevertDialog(void)
{
	delete ui;
}

void RevertDialog::open(void)
{
	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

	QDialog::open();
}

void RevertDialog::accept(void)
{
	if (ui->Class->currentText().size())
	{
		ui->procedButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->progressBar->setVisible(true);

		const QStringList Begins = !ui->Begin->currentText().isEmpty() ?
								  ui->Begin->currentText().split(',', QString::SkipEmptyParts) :
								  QStringList() << ".*";

		const QStringList Ends = !ui->End->currentText().isEmpty() ?
								   ui->End->currentText().split(',', QString::SkipEmptyParts) :
								   QStringList() << ".*";

		emit onRevertRequest(ui->Class->currentText().split(',', QString::SkipEmptyParts), Begins, Ends);
	}
	else QMessageBox::warning(this, tr("Error"), tr("Classes list is empty"));
}

void RevertDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		disconnect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
		disconnect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

		if (Reverted) { emit onRefreshRequest(); Reverted = 0; }
	}
	else QMessageBox::warning(this, tr("Error"), tr("Work in progress"));
}

void RevertDialog::UpdateList(const QStringList& Classes)
{
	ui->Class->clear();
	ui->Class->addItems(Classes);
	ui->Class->setCurrentText("");

	ui->Begin->clear();
	ui->Begin->addItems(Classes);
	ui->Begin->setCurrentText("");

	ui->End->clear();
	ui->End->addItems(Classes);
	ui->End->setCurrentText("");
}

void RevertDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Revert progress"),
						tr("Reverted %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Reverted += Count;
}
