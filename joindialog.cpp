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

#include "joindialog.hpp"
#include "ui_joindialog.h"

JoinDialog::JoinDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::JoinDialog)
{
	ui->setupUi(this);

	ui->progressBar->setVisible(false);

	connect(ui->Class, &QComboBox::currentTextChanged, this, &JoinDialog::onListRequest);
}

JoinDialog::~JoinDialog(void)
{
	delete ui;
}

void JoinDialog::open(void)
{
	if (!ui->Values->count()) emit onListRequest(ui->Class->currentText());

	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

	QDialog::open();
}

void JoinDialog::accept(void)
{
	if (ui->Class->currentText().size())
	{
		ui->procedButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->progressBar->setVisible(true);

		QList<int> Values;

		for (int i = 0; i < ui->Values->count(); ++i)
		{
			if (ui->Values->item(i)->checkState() == Qt::Checked)
			{
				Values.append(i);
			}
		}

		emit onJoinRequest(ui->Class->currentText(), Values, ui->Keep->isChecked());
	}
	else QMessageBox::warning(this, tr("Error"), tr("Classes list is empty"));
}

void JoinDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		disconnect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
		disconnect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

		if (Joined) { emit onRefreshRequest(); Joined = 0; }
	}
	else QMessageBox::warning(this, tr("Error"), tr("Work in progress"));
}

void JoinDialog::UpdateFields(const QStringList& Fields)
{
	ui->Values->clear();
	ui->Values->addItems(Fields);

	for (int i = 0; i < ui->Values->count(); i++)
	{
		ui->Values->item(i)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		ui->Values->item(i)->setCheckState(Qt::Unchecked);
	}
}

void JoinDialog::UpdateList(const QStringList& Classes)
{
	ui->Class->clear();
	ui->Class->addItems(Classes);
}

void JoinDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Join progress"),
						tr("Joined %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Joined += Count;
}
