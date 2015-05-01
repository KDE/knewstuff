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
#include <QHash>

#include <KNS3/KMoreTools>

class KmtServiceInfo
{
public:
    KmtServiceInfo(QString desktopEntryName, QString homepageUrl)
        : desktopEntryName(desktopEntryName), homepageUrl(homepageUrl)
    {
    }
public:
    QString desktopEntryName;
    QString homepageUrl;
};

//
// todo later: add a property "maturity" with values "stable" > "new" > "incubating" or similar
//
KMoreToolsService* KMoreToolsPresets::registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName)
{
#define ADD_ENTRY(desktopEntryName, homepageUrl) dict.insert(desktopEntryName, KmtServiceInfo(desktopEntryName, QLatin1String(homepageUrl)));

    static QHash<QString, KmtServiceInfo> dict;
    ADD_ENTRY("git-cola-folder-handler", "https://git-cola.github.io");
    ADD_ENTRY("git-cola-view-history.kmt-edition", "https://git-cola.github.io");
    ADD_ENTRY("gitk.kmt-edition", "http://git-scm.com/docs/gitk");
    ADD_ENTRY("qgit.kmt-edition", "http://libre.tibirna.org/projects/qgit");
    ADD_ENTRY("gitg", "https://wiki.gnome.org/action/show/Apps/Gitg?action=show&redirect=Gitg");
    ADD_ENTRY("gparted", "http://gparted.org");
    ADD_ENTRY("partitionmanager", "http://www.partitionmanager.org");
    ADD_ENTRY("disk", "https://en.opensuse.org/YaST_Disk_Controller");
    ADD_ENTRY("kdf", "https://www.kde.org/applications/system/kdiskfree");
    ADD_ENTRY("org.kde.filelight", "https://utils.kde.org/projects/filelight");
    ADD_ENTRY("hotshots", "http://sourceforge.net/projects/hotshots/");
    ADD_ENTRY("kaption", "http://kde-apps.org/content/show.php/?content=139302");
    ADD_ENTRY("org.kde.kscreengenie", "http://quickgit.kde.org/?p=kscreengenie.git");
    ADD_ENTRY("org.kde.ksnapshot", "https://www.kde.org/applications/graphics/ksnapshot/");
    ADD_ENTRY("shutter", "http://shutter-project.org");

    auto iter = dict.find(desktopEntryName);
    if (iter != dict.end()) {
        auto kmtServiceInfo = *iter;
        const QString subdir = "presets-kmoretools";
        auto serviceLocatingMode = desktopEntryName.endsWith(".kmt-edition") ?
                                   KMoreTools::ServiceLocatingMode_ByProvidedExecLine : KMoreTools::ServiceLocatingMode_Default;
        auto service = kmt->registerServiceByDesktopEntryName(desktopEntryName, subdir, serviceLocatingMode);
        service->setHomepageUrl(QUrl(kmtServiceInfo.homepageUrl));
        return service;
    } else {
        qDebug() << "KMoreToolsPresets::registerServiceByDesktopEntryName: " << desktopEntryName << "was not found. Return nullptr.";
        return nullptr;
    }
}

QList<KMoreToolsService*> KMoreToolsPresets::registerServicesByGroupingName(KMoreTools* kmt, const QStringList& groupingNames)
{
    QList<KMoreToolsService*> resultList;

    if (groupingNames.contains("git-clients")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("git-cola-folder-handler"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("git-cola-view-history.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gitk.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("qgit.kmt-edition"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gitg"));
    }

    if (groupingNames.contains("disk-usage")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("kdf"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("org.kde.filelight"));
    }

    if (groupingNames.contains("disk-partitions")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("gparted"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("partitionmanager"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("disk"));
    }

    if (groupingNames.contains("screenshot-take")) {
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("org.kde.ksnapshot"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("org.kde.kscreengenie"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("shutter"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("kaption"));
        resultList << registerServiceByDesktopEntryName(kmt, QLatin1String("hotshots"));
    }

    if (resultList.isEmpty()) {
        qDebug() << "KMoreToolsPresets::registerServicesByGroupingName: " << groupingNames << ". Nothing found in this groupings. TODO: check for invalid grouping names.";
    }

    return resultList;
}

