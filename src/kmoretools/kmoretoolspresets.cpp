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
#include "kmoretoolspresets_p.h"
#include "knewstuff_debug.h"
#include <QDebug>
#include <QHash>

#include <KNS3/KMoreTools>

#define _ QStringLiteral

class KmtServiceInfo
{
public:
    KmtServiceInfo(const QString &desktopEntryName, const QString &homepageUrl, int maxUrlArgCount)
        : desktopEntryName(desktopEntryName), homepageUrl(homepageUrl), maxUrlArgCount(maxUrlArgCount)
    {
    }
public:
    QString desktopEntryName;
    QString homepageUrl;
    int maxUrlArgCount;
};

//
// todo later: add a property "maturity" with values "stable" > "new" > "incubating" or similar
//
KMoreToolsService* KMoreToolsPresets::registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName)
{
    static QHash<QString, KmtServiceInfo> dict;

#define ADD_ENTRY(desktopEntryName, maxUrlArgCount, homepageUrl) dict.insert(desktopEntryName, KmtServiceInfo(desktopEntryName, QLatin1String(homepageUrl), maxUrlArgCount));

    //
    // definitions begin (sorted alphabetically):
    //                                              .------ If one gives more URL arguments as
    //                                              |       specified here the program will not work.
    //                                              |       Note, that there are some desktop files where _too few_
    //                                              |       arguments also lead to errors. Watch the console
    //                                              v       output for messages from the program.
    //
    ADD_ENTRY("angrysearch",                        0, "https://github.com/DoTheEvo/ANGRYsearch");
    ADD_ENTRY("com.uploadedlobster.peek",           0, "https://github.com/phw/peek"); // easy to use screen recorder, creates gif
    ADD_ENTRY("catfish",                            1, "http://www.twotoasts.de/index.php/catfish/");
    ADD_ENTRY("ding",                               0, "https://www-user.tu-chemnitz.de/~fri/ding/"); // Offline dict, Online: http://dict.tu-chemnitz.de/dings.cgi
    ADD_ENTRY("disk",                               0, "https://en.opensuse.org/YaST_Disk_Controller");
    ADD_ENTRY("fontinst",                           0, "https://docs.kde.org/trunk5/en/kde-workspace/kcontrol/fontinst/"); // good for previewing many fonts at once
    ADD_ENTRY("fontmatrix",                         0, "https://github.com/fontmatrix/fontmatrix");
    ADD_ENTRY("fsearch",                            0, "http://www.fsearch.org/");
    ADD_ENTRY("giggle",                             1, "https://wiki.gnome.org/Apps/giggle/"); // good for searching in history
    ADD_ENTRY("git-cola-folder-handler",            1, "https://git-cola.github.io");
    ADD_ENTRY("git-cola-view-history.kmt-edition",  1, "https://git-cola.github.io");
    ADD_ENTRY("gitk.kmt-edition",                   1, "http://git-scm.com/docs/gitk");
    ADD_ENTRY("qgit.kmt-edition",                   1, "http://libre.tibirna.org/projects/qgit");
    ADD_ENTRY("gitg",                               1, "https://wiki.gnome.org/action/show/Apps/Gitg?action=show&redirect=Gitg");
    ADD_ENTRY("gnome-search-tool",                  0, "https://help.gnome.org/users/gnome-search-tool/"); // has good filtering options
    ADD_ENTRY("gucharmap",                          0, "https://wiki.gnome.org/action/show/Apps/Gucharmap");
    ADD_ENTRY("gparted",                            0, "http://gparted.org");
    ADD_ENTRY("htop",                               0, "http://hisham.hm/htop/");
    ADD_ENTRY("hotshots",                           1, "http://sourceforge.net/projects/hotshots/");
    ADD_ENTRY("kaption",                            0, "http://kde-apps.org/content/show.php/?content=139302");
    ADD_ENTRY("kding",                              0, ""); // Offline dict; unmaintained?
    ADD_ENTRY("org.kde.kmousetool",                 0, "https://www.kde.org/applications/utilities/kmousetool/");
    ADD_ENTRY("org.gnome.clocks",                   0, "https://wiki.gnome.org/Apps/Clocks");
    ADD_ENTRY("org.kde.filelight",                  1, "https://utils.kde.org/projects/filelight");
    ADD_ENTRY("org.kde.kcharselect",                0, "https://utils.kde.org/projects/kcharselect/");
    ADD_ENTRY("org.kde.kdf",                        0, "https://www.kde.org/applications/system/kdiskfree");
    ADD_ENTRY("org.kde.kfind",                      1, "https://www.kde.org/applications/utilities/kfind/"); // has good filtering options
    ADD_ENTRY("org.kde.partitionmanager",           0, "https://www.kde.org/applications/system/kdepartitionmanager/");
    ADD_ENTRY("org.kde.plasma.cuttlefish.kmt-edition", 0, "http://vizzzion.org/blog/2015/02/say-hi-to-cuttlefish/");
    ADD_ENTRY("org.kde.ksysguard",                  0, "https://userbase.kde.org/KSysGuard");
    ADD_ENTRY("org.kde.ksystemlog",                 0, "https://www.kde.org/applications/system/ksystemlog/");
    ADD_ENTRY("org.kde.ktimer",                     0, "https://www.kde.org/applications/utilities/ktimer/");
    ADD_ENTRY("org.kde.spectacle",                  0, "https://www.kde.org/applications/graphics/spectacle");
    ADD_ENTRY("simplescreenrecorder",               0, "http://www.maartenbaert.be/simplescreenrecorder/");
    ADD_ENTRY("shutter",                            0, "http://shutter-project.org"); // good for edit screenshot after capture
    ADD_ENTRY("vokoscreen",                         0, "https://github.com/vkohaupt/vokoscreen"); // feature-rich screen recorder
    ADD_ENTRY("xfce4-taskmanager",                  0, "http://goodies.xfce.org/projects/applications/xfce4-taskmanager");
    //
    // ...definitions end
    //

#undef ADD_ENTRY

    auto iter = dict.constFind(desktopEntryName);
    if (iter != dict.constEnd()) {
        auto kmtServiceInfo = *iter;
        const QString subdir = QStringLiteral("presets-kmoretools");
        auto serviceLocatingMode = desktopEntryName.endsWith(QLatin1String(".kmt-edition")) ?
                                   KMoreTools::ServiceLocatingMode_ByProvidedExecLine : KMoreTools::ServiceLocatingMode_Default;
        auto service = kmt->registerServiceByDesktopEntryName(desktopEntryName, subdir, serviceLocatingMode);
        if (service) { // We might get nullptr in case of missing or broken .desktop files
            service->setHomepageUrl(QUrl(kmtServiceInfo.homepageUrl));
            service->setMaxUrlArgCount(kmtServiceInfo.maxUrlArgCount);
        }
        return service;
    } else {
        qCDebug(KNEWSTUFF) << "KMoreToolsPresets::registerServiceByDesktopEntryName: " << desktopEntryName << "was not found. Return nullptr.";
        return nullptr;
    }
}

QList<KMoreToolsService*> KMoreToolsPresets::registerServicesByGroupingNames(KMoreTools* kmt, const QStringList& groupingNames)
{
    QString firstMoreSectionDesktopEntryName;
    return KMoreToolsPresetsPrivate::registerServicesByGroupingNames(&firstMoreSectionDesktopEntryName, kmt, groupingNames);
}

QList<KMoreToolsService*> KMoreToolsPresetsPrivate::registerServicesByGroupingNames(QString* firstMoreSectionDesktopEntryName, KMoreTools* kmt, const QStringList& groupingNames)
{
    static QHash<QString, QList<QString>> dict;

    // The corresponding desktop files are located here:
    // 'knewstuff/data/kmoretools-desktopfiles/'

    // Use KMoreToolsTest2::testDialogForGroupingNames to see if the settings
    // here are correct.

    // NOTE that the desktopentry names must be registered in
    // registerServiceByDesktopEntryName above.

    // For special handlings about naming in the menu etc. see kmoretoolsmenufactory.cpp/addItemsForGroupingNameWithSpecialHandling

    //
    // grouping definitions begin (sorted alphabetically):
    //
    dict.insert(_("disk-usage"), { _("org.kde.kdf"), _("org.kde.filelight") });
    dict.insert(_("disk-partitions"), { _("gparted"), _("org.kde.partitionmanager"), _("disk") });
    dict.insert(_("files-find"), { _("org.kde.kfind"), _("fsearch"), _("more:"), _("gnome-search-tool"), _("catfish"), _("angrysearch") });
    dict.insert(_("font-tools"), { _("fontinst"), _("gucharmap"), _("more:"), _("org.kde.kcharselect"), _("fontmatrix") });
    dict.insert(_("git-clients-for-folder"), { _("git-cola-folder-handler"), _("gitk.kmt-edition"),
                _("giggle"), _("qgit.kmt-edition"), _("gitg") });
    dict.insert(_("git-clients-and-actions"), { _("git-cola-folder-handler"), _("git-cola-view-history.kmt-edition"),
                _("giggle"), _("gitk.kmt-edition"), _("qgit.kmt-edition"), _("gitg") });
    dict.insert(_("icon-browser"), { _("org.kde.plasma.cuttlefish.kmt-edition") });
    dict.insert(_("language-dictionary"), { _("ding"), _("kding") });
    dict.insert(_("mouse-tools"), { _("org.kde.kmousetool") }); // todo: add program "xbanish" to remove mouse cursor while typing
    dict.insert(_("screenrecorder"), { _("com.uploadedlobster.peek"), _("simplescreenrecorder"), _("vokoscreen") });
    dict.insert(_("screenshot-take"), { _("org.kde.spectacle"), _("shutter"), _("kaption"), _("hotshots") });
    dict.insert(_("system-monitor-processes"), { _("org.kde.ksysguard"), _("more:"), _("htop"), _("xfce4-taskmanager") });
    dict.insert(_("system-monitor-logs"), { _("org.kde.ksystemlog") });
    dict.insert(_("time-countdown"), { _("org.gnome.clocks"), _("org.kde.ktimer") });
    //
    // ...grouping definitions end
    //

    QList<KMoreToolsService*> resultList;
    QSet<QString> alreadyUsedDesktopEntryNames; // including the "more:" keyword
    bool nextIsMore = false;

    Q_FOREACH (const QString &groupingName, groupingNames) {
        auto iter = dict.constFind(groupingName);
        if (iter != dict.constEnd()) {
            Q_FOREACH(const QString &desktopEntryName, *iter) {
                if (!alreadyUsedDesktopEntryNames.contains(desktopEntryName)) {
                    if (desktopEntryName == _("more:")) {
                        nextIsMore = true;
                    } else {
                        if (nextIsMore) { // this will be only set once
                            *firstMoreSectionDesktopEntryName = desktopEntryName;
                            nextIsMore = false;
                        }
                        KMoreToolsService *kmtService = KMoreToolsPresets::registerServiceByDesktopEntryName(kmt, desktopEntryName);
                        if (kmtService) { // Do not add null pointers caused by missing or broken .desktop files
                            resultList << kmtService;
                        }
                    }
                } else {
                    alreadyUsedDesktopEntryNames.insert(desktopEntryName);
                }
            }
        } else {
            qWarning() << "KMoreToolsPresets::registerServicesByGroupingName: groupingName not found: " << groupingName;
        }
    }

    if (resultList.isEmpty()) {
        qWarning() << "KMoreToolsPresets::registerServicesByGroupingName: " << groupingNames << ". Nothing found in this groupings. HINT: check for invalid grouping names.";
    }

    return resultList;
}


