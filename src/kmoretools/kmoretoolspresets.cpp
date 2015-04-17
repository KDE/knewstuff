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

KMoreToolsService* KMoreToolsPresets::registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName)
{
    KMoreToolsService* service = nullptr;
    const QString subdir = "presets-kmoretools";

    if (desktopEntryName == QString("git-cola-folder-handler")) {
        service = kmt->registerServiceByDesktopEntryName(desktopEntryName, subdir);
        service->setHomepageUrl(QUrl(QLatin1String("https://git-cola.github.io")));
    } else if (desktopEntryName == QString("git-cola-view-history")) {
        service = kmt->registerServiceByDesktopEntryName(
                      desktopEntryName + ".kmt-edition",
                      subdir,
                      KMoreTools::ServiceLocatingMode_ByProvidedExecLine); // because kmt-edition
        service->setHomepageUrl(QUrl(QLatin1String("https://git-cola.github.io")));
    } else if (desktopEntryName == QString("gitk")) {
        service = kmt->registerServiceByDesktopEntryName(
                      desktopEntryName + ".kmt-edition", subdir,
                      // use exec line because gitk does not install desktop file of it's own and because kmt-edition
                      KMoreTools::ServiceLocatingMode_ByProvidedExecLine);
        service->setHomepageUrl(QUrl(QLatin1String("http://git-scm.com/docs/gitk")));
    } else if (desktopEntryName == QString("qgit")) {
        service  = kmt->registerServiceByDesktopEntryName(
                       desktopEntryName + ".kmt-edition",
                       subdir,
                       KMoreTools::ServiceLocatingMode_ByProvidedExecLine); // because kmt-edition
        service->setHomepageUrl(QUrl(QLatin1String("http://libre.tibirna.org/projects/qgit")));
        // note that icon is missing (upstream problem)
    }  else if (desktopEntryName == QString("gitg")) {
        service = kmt->registerServiceByDesktopEntryName(desktopEntryName, subdir);
        service->setHomepageUrl(QUrl(QLatin1String("https://wiki.gnome.org/action/show/Apps/Gitg?action=show&redirect=Gitg")));
    } else {
        qDebug() << "KMoreToolsPresets::registerServiceByDesktopEntryName: " << desktopEntryName << "was not found";
    }

    return service;
}

