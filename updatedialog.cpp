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

#include "updatedialog.hpp"
#include "ui_updatedialog.h"

UpdateDialog::UpdateDialog(QWidget* Parent)
: QDialog(Parent), ui(new Ui::UpdateDialog)
{
	ui->setupUi(this);

	ui->progressBar->setVisible(false);

	connect(ui->Class, &QComboBox::currentTextChanged, this, &UpdateDialog::onListRequest);
}

UpdateDialog::~UpdateDialog(void)
{
	delete ui;
}

void UpdateDialog::OpenSource(void)
{
	Source = QFileDialog::getOpenFileName(this, tr("Open source file"), Source, tr("GIV (*.giv);;Text (*.txt)"));

	ui->openButton->setToolTip(Source);
}

void UpdateDialog::open(void)
{
	if (!ui->Values->count()) emit onListRequest(ui->Class->currentText());

	connect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
	connect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

	QDialog::open();
}

void UpdateDialog::accept(void)
{
	if (!Source.isEmpty())
	{
		if (ui->Class->currentText().size() && ui->Field->currentText().size())
		{
			QList<int> Values;

			for (int i = 0; i < ui->Values->count(); ++i)
			{
				if (ui->Values->item(i)->checkState() == Qt::Checked)
				{
					Values.append(i);
				}
			}

			if (!Values.isEmpty() || ui->Geometry->isChecked())
			{
				ui->procedButton->setEnabled(false);
				ui->cancelButton->setEnabled(false);
				ui->progressBar->setVisible(true);

				emit onUpdateRequest(ui->Class->currentText(), Source, Values, ui->Geometry->isChecked(), ui->Field->currentIndex());
			}
			else QMessageBox::warning(this, tr("Error"), tr("Select at least one field to update"));
		}
		else QMessageBox::warning(this, tr("Error"), tr("Set class and field parameters"));
	}
	else QMessageBox::warning(this, tr("Error"), tr("Select data source file"));
}

void UpdateDialog::reject(void)
{
	if (ui->cancelButton->isEnabled())
	{
		QDialog::reject();

		disconnect(AppCore::getInstance(), &AppCore::onProgressInit, ui->progressBar, &QProgressBar::setRange);
		disconnect(AppCore::getInstance(), &AppCore::onProgressUpdate, ui->progressBar, &QProgressBar::setValue);

		if (Updated) { emit onRefreshRequest(); Updated = 0; }
	}
	else QMessageBox::warning(this, tr("Error"), tr("Work in progress"));
}

void UpdateDialog::UpdateFields(const QStringList& Fields)
{
	ui->Values->clear();
	ui->Values->addItems(Fields);

	ui->Field->clear();
	ui->Field->addItems(Fields);

	for (int i = 0; i < ui->Values->count(); i++)
	{
		ui->Values->item(i)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		ui->Values->item(i)->setCheckState(Qt::Unchecked);
	}
}

void UpdateDialog::UpdateList(const QStringList& Classes)
{
	ui->Class->clear();
	ui->Class->addItems(Classes);
}

void UpdateDialog::ShowProgress(int Count)
{
	QMessageBox::information(this, tr("Update progress"),
						tr("Updated %n item(s)", 0, Count));

	ui->procedButton->setEnabled(true);
	ui->cancelButton->setEnabled(true);
	ui->progressBar->setVisible(false);

	Updated += Count;
}
