/*
    This file is part of KNewStuff.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2007-2014 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QApplication>
#include <QPointer>
#include <QtDebug>
#include <QUrl>

#include <klocalizedstring.h>

#include <iostream>

#include <kns3/uploaddialog.h>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QLatin1String("khotnewstuff_upload"));
    QCoreApplication::setApplicationVersion(QLatin1String("0.4"));
    QCoreApplication::setOrganizationDomain(QLatin1String("kde.org"));
    QApplication::setApplicationDisplayName(i18n("KHotNewStuff"));

    QApplication i(argc, argv);

    if (i.arguments().count() > 1) {
        QString configfile = QLatin1String(argv[1]);
        QPointer<KNS3::UploadDialog> dialog = new KNS3::UploadDialog(configfile);
        if (i.arguments().count() > 2) {
            dialog->setUploadFile(QUrl(QLatin1String(argv[2])));
        }
        dialog->exec();
        delete dialog;
    }
    else
    {
        std::cout << "Enter the knsrc file to use followed by a filename to upload\n";
        return -1;
    }
    return 0;
}

