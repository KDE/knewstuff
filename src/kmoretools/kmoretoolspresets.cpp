/*
    Copyright 2015 by Gregor Mi <codestruct@posteo.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kmoretoolspresets.h"

#include <QDebug>

#include <KNS3/KMoreTools>

KMoreToolsService* registerImpl(KMoreTools* kmt, const QString& desktopEntryName, const QString& homepageUrl)
{
    const QString subdir = "presets-kmoretools";
    auto serviceLocatingMode = desktopEntryName.endsWith(".kmt-edition") ?
                               KMoreTools::ServiceLocatingMode_ByProvidedExecLine : KMoreTools::ServiceLocatingMode_Default;
    auto service = kmt->registerServiceByDesktopEntryName(desktopEntryName, subdir, serviceLocatingMode);
    service->setHomepageUrl(QUrl(homepageUrl));
    return service;
}

KMoreToolsService* KMoreToolsPresets::registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName)
{
    KMoreToolsService* service = nullptr;

    if (desktopEntryName == QString("git-cola-folder-handler")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("https://git-cola.github.io"));
    } else if (desktopEntryName == QString("git-cola-view-history.kmt-edition")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("https://git-cola.github.io"));
    } else if (desktopEntryName == QString("gitk.kmt-edition")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("http://git-scm.com/docs/gitk"));
    } else if (desktopEntryName == QString("qgit.kmt-edition")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("http://libre.tibirna.org/projects/qgit"));
    }  else if (desktopEntryName == QString("gitg")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("https://wiki.gnome.org/action/show/Apps/Gitg?action=show&redirect=Gitg"));
    } else if (desktopEntryName == QString("gparted")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("http://gparted.org/"));
    } else if (desktopEntryName == QString("kdf")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("https://www.kde.org/applications/system/kdiskfree"));
    } else if (desktopEntryName == QString("org.kde.filelight")) {
        return registerImpl(kmt, desktopEntryName, QLatin1String("https://utils.kde.org/projects/filelight"));
    } else {
        qDebug() << "KMoreToolsPresets::registerServiceByDesktopEntryName: " << desktopEntryName << "was not found. Return nullptr.";
    }

    return service;
}

QList<KMoreToolsService*> KMoreToolsPresets::registerServicesByCategory(KMoreTools* kmt, const QStringList& categories)
{
    QList<KMoreToolsService*> resultList;

    if (categories.contains("git-clients")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("git-cola-folder-handler"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("git-cola-view-history.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gitk.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("qgit.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gitg"));
    }

    if (categories.contains("disk-usage")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("kdf"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("org.kde.filelight"));
    }

    if (categories.contains("disk-partitions")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gparted"));
    }

    if (resultList.isEmpty()) {
        qDebug() << "KMoreToolsPresets::registerServicesByCategory: " << categories << ". Nothing found in this categories. TODO: check for invalid category strings.";
    }

    return resultList;
}

