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

#ifndef CHANGESDIALOG_HPP
#define CHANGESDIALOG_HPP

#include <QTableWidgetItem>
#include <QTableWidget>
#include <QMessageBox>
#include <QDialog>

#include "appcore.hpp"

namespace Ui
{
	class ChangesDialog;
}

class ChangesDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::ChangesDialog* ui;

	private:

		void AddItem(const AppCore::Entry& Data);

	public:

		explicit ChangesDialog(QWidget* Parent = nullptr);
		virtual ~ChangesDialog(void) override;

	private slots:

		void AddButtonClicked(void);
		void RemoveButtonClicked(void);

		void FindEditChanged(const QString& String);

		void ItemChanged(QTableWidgetItem* Changed);

};

#endif // CHANGESDIALOG_HPP
