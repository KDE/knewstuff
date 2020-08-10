/*
    This file is part of KNewStuff.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2007-2014 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QPointer>
#include <knewstuff_debug.h>

#include <KLocalizedString>

#include <iostream>

#include <kns3/downloaddialog.h>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("khotnewstuff"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.4"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QApplication::setApplicationDisplayName(i18n("KHotNewStuff"));

    QApplication i(argc, argv);

    if (i.arguments().count() > 1) {
        QString configfile = QLatin1String(argv[1]);
        QPointer<KNS3::DownloadDialog> dialog = new KNS3::DownloadDialog(configfile);
        dialog->exec();
        const auto lst = dialog->changedEntries();
        for (const KNS3::Entry& e : lst) {
            qCDebug(KNEWSTUFF) << "Changed Entry: " << e.name();
        }
        delete dialog;
    }
    else
    {
        std::cout << "Enter the filename of a .knsrc file to read configuration from\n";
        return -1;
    }

    return 0;
}

