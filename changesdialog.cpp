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

#include "changesdialog.hpp"
#include "ui_changesdialog.h"

ChangesDialog::ChangesDialog(QWidget *Parent)
: QDialog(Parent), ui(new Ui::ChangesDialog)
{
	ui->setupUi(this);

	ui->Items->setColumnWidth(0, 50);
	ui->Items->setColumnWidth(1, 150);
	ui->Items->setColumnWidth(2, 150);

	for (const auto& Item : AppCore::getInstance()->getItems()) AddItem(Item);
}

ChangesDialog::~ChangesDialog(void)
{
	delete ui;
}

void ChangesDialog::AddItem(const AppCore::Entry& Data)
{
	const int Rows = ui->Items->rowCount();

	const bool Sorting = ui->Items->isSortingEnabled();
	const bool Block = ui->Items->blockSignals(true);

	ui->Items->setSortingEnabled(false);
	ui->Items->insertRow(Rows);

	QTableWidgetItem* IndexBox = new QTableWidgetItem();
	QTableWidgetItem* CheckBox = new QTableWidgetItem();

	IndexBox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	IndexBox->setData(Qt::UserRole, Data.ID);
	IndexBox->setData(Qt::EditRole, Data.ID);
	IndexBox->setCheckState(Data.isEnabled ? Qt::Checked : Qt::Unchecked);

	CheckBox->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	CheckBox->setCheckState(Data.isRegExp ? Qt::Checked : Qt::Unchecked);
	CheckBox->setText(Data.isRegExp ? tr("Yes") : tr("No"));

	ui->Items->setItem(Rows, 0, IndexBox);
	ui->Items->setItem(Rows, 1, new QTableWidgetItem(Data.Source));
	ui->Items->setItem(Rows, 2, new QTableWidgetItem(Data.Replace));
	ui->Items->setItem(Rows, 3, CheckBox);

	ui->Items->blockSignals(Block);
	ui->Items->setSortingEnabled(Sorting);

	if (Data.ID == -1) ui->Items->scrollToItem(IndexBox);
}

void ChangesDialog::AddButtonClicked(void)
{
	const AppCore::Entry Default;

	AddItem(Default);
}

void ChangesDialog::RemoveButtonClicked(void)
{
	const int Row = ui->Items->currentRow();

	if (Row != -1)
	{
		if (QMessageBox::question(this, tr("Delete item"), tr("Delete selected entry?")) == QMessageBox::Yes)
		{
			const int ID = ui->Items->item(Row, 0)->data(Qt::UserRole).toInt();

			AppCore::getInstance()->RemoveItem(ID);

			ui->Items->removeRow(Row);
		}
	}
}

void ChangesDialog::FindEditChanged(const QString& String)
{
	const int Rows = ui->Items->rowCount();

	if (String.size()) for (int i = 0; i < Rows; i++)
	{
		const bool Source = ui->Items->item(i, 1)->text().contains(String, Qt::CaseInsensitive);
		const bool Replace = ui->Items->item(i, 2)->text().contains(String, Qt::CaseInsensitive);

		ui->Items->setRowHidden(i, !(Source || Replace));
	}
	else for (int i = 0; i < Rows; i++) ui->Items->setRowHidden(i, false);
}

void ChangesDialog::ItemChanged(QTableWidgetItem* Changed)
{
	const int Row = ui->Items->row(Changed);

	if (ui->Items->rowCount() > Row)
	{
		AppCore::Entry Item;

		Item.ID = ui->Items->item(Row, 0)->data(Qt::UserRole).toInt();
		Item.Source = ui->Items->item(Row, 1)->text();
		Item.Replace = ui->Items->item(Row, 2)->text();
		Item.isRegExp = ui->Items->item(Row, 3)->checkState() & Qt::Checked;
		Item.isEnabled = ui->Items->item(Row, 0)->checkState() & Qt::Checked;

		AppCore::Entry Last = AppCore::getInstance()->getItem(Item.ID);

		if (Item.ID == -1)
		{
			if (AppCore::getInstance()->AddItem(Item) != -1)
			{
				ui->Items->item(Row, 0)->setData(Qt::UserRole, Item.ID);
				ui->Items->item(Row, 0)->setData(Qt::EditRole, Item.ID);
			}
			else QMessageBox::warning(this, tr("Error"), tr("Entered data is incorrect"));
		}
		else if (!AppCore::getInstance()->UpdateItem(Item))
		{
			QMessageBox::warning(this, tr("Error"), tr("Entered data is incorrect"));

			const bool Block = ui->Items->blockSignals(true);

			ui->Items->item(Row, 0)->setCheckState(Last.isRegExp ? Qt::Checked : Qt::Unchecked);
			ui->Items->item(Row, 1)->setText(Last.Source);
			ui->Items->item(Row, 2)->setText(Last.Replace);
			ui->Items->item(Row, 3)->setCheckState(Last.isRegExp ? Qt::Checked : Qt::Unchecked);

			ui->Items->blockSignals(Block);
		}

		ui->Items->item(Row, 3)->setText(ui->Items->item(Row, 3)->checkState() ? tr("Yes") : tr("No"));
	}
}
