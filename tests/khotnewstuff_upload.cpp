/*
    This file is part of KNewStuff.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2007-2014 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QPointer>
#include <QUrl>

#include <KLocalizedString>

#include <iostream>

#include <kns3/uploaddialog.h>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("khotnewstuff_upload"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.4"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
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
    } else {
        std::cout << "Enter the knsrc file to use followed by a filename to upload\n";
        return -1;
    }
    return 0;
}
