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

#ifndef UNPINNDIALOG_HPP
#define UNPINNDIALOG_HPP

#include <QMessageBox>
#include <QDialog>

#include "appcore.hpp"

namespace Ui
{
	class UnpinnDialog;
}

class UnpinnDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::UnpinnDialog *ui;

		int Unpinned = 0;

	public:

		explicit UnpinnDialog(QWidget* Parent = nullptr);
		virtual ~UnpinnDialog(void) override;

	public slots:

		virtual void open(void) override;
		virtual void accept(void) override;
		virtual void reject(void) override;

		void ShowProgress(int Count);

	signals:

		void onUnpinnRequest(const QStringList&, bool);
		void onRefreshRequest(void);

};

#endif // UNPINNDIALOG_HPP
