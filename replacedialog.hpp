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

#ifndef REPLACEDIALOG_HPP
#define REPLACEDIALOG_HPP

#include <QMessageBox>
#include <QDialog>

#include "appcore.hpp"

namespace Ui
{
	class ReplaceDialog;
}

class ReplaceDialog : public QDialog
{

		Q_OBJECT

	private:

		Ui::ReplaceDialog* ui;

		int Replaces = 0;

	public:

		explicit ReplaceDialog(QWidget* Parent = nullptr);
		virtual ~ReplaceDialog(void) override;

	public slots:

		virtual void accept(void) override;
		virtual void reject(void) override;

		void ShowProgress(int Count);
		void UpdateProgress(double Value);

	signals:

		void onReplaceRequest(const QString&, const QString&, bool, bool);
		void onRefreshRequest(void);

};

#endif // REPLACEDIALOG_HPP
