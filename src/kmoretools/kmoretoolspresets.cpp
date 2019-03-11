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
    KmtServiceInfo(const QString &desktopEntryName, const QString &homepageUrl, int maxUrlArgCount, const QString &appstreamId)
        : desktopEntryName(desktopEntryName), homepageUrl(homepageUrl), maxUrlArgCount(maxUrlArgCount), appstreamId(appstreamId)
    {
    }
public:
    QString desktopEntryName;
    QString homepageUrl;
    int maxUrlArgCount;
    QString appstreamId;
};

//
// todo later: add a property "maturity" with values "stable" > "new" > "incubating" or similar
//
KMoreToolsService* KMoreToolsPresets::registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName)
{
    static QHash<QString, KmtServiceInfo> dict;

#define ADD_ENTRY(desktopEntryName, maxUrlArgCount, homepageUrl, appstreamUrl) dict.insert(desktopEntryName, KmtServiceInfo(desktopEntryName, QLatin1String(homepageUrl), maxUrlArgCount, appstreamUrl));

    //
    // definitions begin (sorted alphabetically):
    //                                              .------ If one gives more URL arguments as
    //                                              |       specified here the program will not work.
    //                                              |       Note, that there are some desktop files where _too few_
    //                                              |       arguments also lead to errors. Watch the console
    //                                              v       output for messages from the program.
    //
    ADD_ENTRY(QStringLiteral("angrysearch"),                        0, "https://github.com/DoTheEvo/ANGRYsearch", QString());
    ADD_ENTRY(QStringLiteral("com.uploadedlobster.peek"),           0, "https://github.com/phw/peek", QStringLiteral("com.uploadedlobster.peek.desktop")); // easy to use screen recorder, creates gif
    ADD_ENTRY(QStringLiteral("catfish"),                            1, "http://www.twotoasts.de/index.php/catfish/", QStringLiteral("catfish"));
    ADD_ENTRY(QStringLiteral("ding"),                               0, "https://www-user.tu-chemnitz.de/~fri/ding/", QString()); // Offline dict, Online: http://dict.tu-chemnitz.de/dings.cgi
    ADD_ENTRY(QStringLiteral("disk"),                               0, "https://en.opensuse.org/YaST_Disk_Controller", QString());
    ADD_ENTRY(QStringLiteral("fontinst"),                           0, "https://docs.kde.org/trunk5/en/kde-workspace/kcontrol/fontinst/", QString()); // good for previewing many fonts at once
    ADD_ENTRY(QStringLiteral("fontmatrix"),                         0, "https://github.com/fontmatrix/fontmatrix", QString());
    ADD_ENTRY(QStringLiteral("fsearch"),                            0, "http://www.fsearch.org/", QString());
    ADD_ENTRY(QStringLiteral("giggle"),                             1, "https://wiki.gnome.org/Apps/giggle/", QStringLiteral("giggle.desktop")); // good for searching in history
    ADD_ENTRY(QStringLiteral("git-cola-folder-handler"),            1, "https://git-cola.github.io", QStringLiteral("git-cola.desktop"));
    ADD_ENTRY(QStringLiteral("git-cola-view-history.kmt-edition"),  1, "https://git-cola.github.io", QStringLiteral("git-cola.desktop"));
    ADD_ENTRY(QStringLiteral("gitk.kmt-edition"),                   1, "http://git-scm.com/docs/gitk", QString());
    ADD_ENTRY(QStringLiteral("qgit.kmt-edition"),                   1, "http://libre.tibirna.org/projects/qgit", QString());
    ADD_ENTRY(QStringLiteral("gitg"),                               1, "https://wiki.gnome.org/action/show/Apps/Gitg?action=show&redirect=Gitg", QStringLiteral("gitg.desktop"));
    ADD_ENTRY(QStringLiteral("gnome-search-tool"),                  0, "https://help.gnome.org/users/gnome-search-tool/", QStringLiteral("gnome-search-tool.desktop")); // has good filtering options
    ADD_ENTRY(QStringLiteral("gucharmap"),                          0, "https://wiki.gnome.org/action/show/Apps/Gucharmap", QStringLiteral("gucharmap.desktop"));
    ADD_ENTRY(QStringLiteral("gparted"),                            0, "http://gparted.org", QStringLiteral("gparted.desktop"));
    ADD_ENTRY(QStringLiteral("htop"),                               0, "http://hisham.hm/htop/", QStringLiteral("htop.desktop"));
    ADD_ENTRY(QStringLiteral("hotshots"),                           1, "http://sourceforge.net/projects/hotshots/", QString());
    ADD_ENTRY(QStringLiteral("kaption"),                            0, "http://kde-apps.org/content/show.php/?content=139302", QString());
    ADD_ENTRY(QStringLiteral("kding"),                              0, "", QString()); // Offline dict; unmaintained?
    ADD_ENTRY(QStringLiteral("org.kde.kmousetool"),                 0, "https://www.kde.org/applications/utilities/kmousetool/", QStringLiteral("org.kde.kmousetool"));
    ADD_ENTRY(QStringLiteral("org.gnome.clocks"),                   0, "https://wiki.gnome.org/Apps/Clocks", QStringLiteral("org.gnome.clocks.desktop"));
    ADD_ENTRY(QStringLiteral("org.kde.filelight"),                  1, "https://utils.kde.org/projects/filelight", QStringLiteral("org.kde.filelight.desktop"));
    ADD_ENTRY(QStringLiteral("org.kde.kcharselect"),                0, "https://utils.kde.org/projects/kcharselect/", QStringLiteral("org.kde.kcharselect"));
    ADD_ENTRY(QStringLiteral("org.kde.kdf"),                        0, "https://www.kde.org/applications/system/kdiskfree", QStringLiteral("org.kde.kdf"));
    ADD_ENTRY(QStringLiteral("org.kde.kfind"),                      1, "https://www.kde.org/applications/utilities/kfind/", QStringLiteral("org.kde.kfind.desktop")); // has good filtering options
    ADD_ENTRY(QStringLiteral("org.kde.partitionmanager"),           0, "https://www.kde.org/applications/system/kdepartitionmanager/", QStringLiteral("org.kde.partitionmanager.desktop"));
    ADD_ENTRY(QStringLiteral("org.kde.plasma.cuttlefish.kmt-edition"), 0, "http://vizzzion.org/blog/2015/02/say-hi-to-cuttlefish/", QStringLiteral("org.kde.plasma.cuttlefish"));
    ADD_ENTRY(QStringLiteral("org.kde.ksysguard"),                  0, "https://userbase.kde.org/KSysGuard", QStringLiteral("org.kde.ksysguard"));
    ADD_ENTRY(QStringLiteral("org.kde.ksystemlog"),                 0, "https://www.kde.org/applications/system/ksystemlog/", QStringLiteral("org.kde.ksystemlog"));
    ADD_ENTRY(QStringLiteral("org.kde.ktimer"),                     0, "https://www.kde.org/applications/utilities/ktimer/", QStringLiteral("org.kde.ktimer"));
    ADD_ENTRY(QStringLiteral("org.kde.spectacle"),                  0, "https://www.kde.org/applications/graphics/spectacle", QStringLiteral("org.kde.spectacle.desktop"));
    ADD_ENTRY(QStringLiteral("simplescreenrecorder"),               0, "http://www.maartenbaert.be/simplescreenrecorder/", QStringLiteral("simplescreenrecorder.desktop"));
    ADD_ENTRY(QStringLiteral("shutter"),                            0, "http://shutter-project.org", QStringLiteral("org.shutterproject.shutter")); // good for edit screenshot after capture
    ADD_ENTRY(QStringLiteral("vokoscreen"),                         0, "https://github.com/vkohaupt/vokoscreen", QString()); // feature-rich screen recorder
    ADD_ENTRY(QStringLiteral("xfce4-taskmanager"),                  0, "http://goodies.xfce.org/projects/applications/xfce4-taskmanager", QStringLiteral("xfce4-taskmanager.desktop"));
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
            service->setAppstreamId(kmtServiceInfo.appstreamId);
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

    for (const QString &groupingName : groupingNames) {
        auto iter = dict.constFind(groupingName);
        if (iter != dict.constEnd()) {
            for (const QString &desktopEntryName : qAsConst(*iter)) {
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
            qCWarning(KNEWSTUFF) << "KMoreToolsPresets::registerServicesByGroupingName: groupingName not found: " << groupingName;
        }
    }

    if (resultList.isEmpty()) {
        qCWarning(KNEWSTUFF) << "KMoreToolsPresets::registerServicesByGroupingName: " << groupingNames << ". Nothing found in this groupings. HINT: check for invalid grouping names.";
    }

    return resultList;
}


