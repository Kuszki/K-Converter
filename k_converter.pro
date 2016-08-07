#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#*                                                                         *
#*  Tango file converter for DaroD                                         *
#*  Copyright (C) 2016  Łukasz "Kuszki" Dróżdż  l.drozdz@openmailbox.org   *
#*                                                                         *
#*  This program is free software: you can redistribute it and/or modify   *
#*  it under the terms of the GNU General Public License as published by   *
#*  the  Free Software Foundation, either  version 3 of the  License, or   *
#*  (at your option) any later version.                                    *
#*                                                                         *
#*  This  program  is  distributed  in the hope  that it will be useful,   *
#*  but WITHOUT ANY  WARRANTY;  without  even  the  implied  warranty of   *
#*  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the   *
#*  GNU General Public License for more details.                           *
#*                                                                         *
#*  You should have  received a copy  of the  GNU General Public License   *
#*  along with this program. If not, see http://www.gnu.org/licenses/.     *
#*                                                                         *
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

QT		+=	core gui widgets sql concurrent

TARGET	=	kconverter
TEMPLATE	=	app

CONFIG	+=	c++14

SOURCES	+=	main.cpp \
			mainwindow.cpp \
			changesdialog.cpp \
			appcore.cpp \
			replacedialog.cpp \
			aboutdialog.cpp \
			deletedialog.cpp \
			unpinndialog.cpp \
			setvaluedialog.cpp \
			splitdialog.cpp

HEADERS	+=	mainwindow.hpp \
			changesdialog.hpp \
			appcore.hpp \
			replacedialog.hpp \
			aboutdialog.hpp \
			deletedialog.hpp \
			unpinndialog.hpp \
			setvaluedialog.hpp \
			splitdialog.hpp

FORMS	+=	mainwindow.ui \
			changesdialog.ui \
			replacedialog.ui \
			aboutdialog.ui \
			deletedialog.ui \
			unpinndialog.ui \
			setvaluedialog.ui \
			splitdialog.ui

RESOURCES	+=	resources.qrc

QMAKE_CXXFLAGS += -std=c++14 -march=native

TRANSLATIONS	+= k_converter_pl.ts
