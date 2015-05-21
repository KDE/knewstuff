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

#include "kmoretoolsmenufactory.h"

#include <QDebug>

#include <KLocalizedString>
#include <KMountPoint>
#include <KRun>
#include <KService>
#include <KNS3/KMoreTools>
#include <KNS3/KMoreToolsPresets>

class KMoreToolsMenuFactoryPrivate
{
public:
    // Note that this object must live long enough in case the user opens
    // the "Configure..." dialog
    KMoreTools* kmt = nullptr;

    QMenu* menu = nullptr;
};

KMoreToolsMenuFactory::KMoreToolsMenuFactory(const QString& uniqueId)
    : d(new KMoreToolsMenuFactoryPrivate())
{
    d->kmt = new KMoreTools(uniqueId);
}

KMoreToolsMenuFactory::~KMoreToolsMenuFactory()
{
    if (d->menu) {
        delete d->menu;
    }
    delete d->kmt;

    delete d;
}

void addItemsFromList(KMoreToolsMenuBuilder* menuBuilder,
                      QMenu* menu,
                      QList<KMoreToolsService*> kmtServiceList,
                      const QUrl& url,
                      bool isMoreSection
                     )
{
    Q_FOREACH(auto kmtService, kmtServiceList) {

        auto menuItem = menuBuilder->addMenuItem(kmtService, isMoreSection ?
                        KMoreTools::MenuSection_More : KMoreTools::MenuSection_Main);

        if (kmtService->isInstalled()) {
            auto kService = kmtService->installedService();

            if (!kService) {
                // if the corresponding desktop file is not installed
                // then the isInstalled was true because of the Exec line check
                // and we use the desktopfile provided by KMoreTools.
                // Otherwise *kService would crash.
                qDebug() << "Desktop file not installed:" << kmtService->desktopEntryName() << "=> Use desktop file provided by KMoreTools";
                kService = kmtService->kmtProvidedService();
            }

            if (!url.isEmpty() && kmtService->maxUrlArgCount() > 0) {
                menu->connect(menuItem->action(), &QAction::triggered, menu,
                [kService, url](bool) {
                    KRun::runService(*kService, { url }, nullptr);
                });
            } else {
                menu->connect(menuItem->action(), &QAction::triggered, menu,
                [kService](bool) {
                    KRun::runService(*kService, { }, nullptr);
                });
            }
        }
    }
}

void addItemsForGroupingName(KMoreToolsMenuBuilder* menuBuilder,
                             QMenu* menu,
                             QList<KMoreToolsService*> kmtServiceList,
                             const QString& groupingName,
                             const QUrl& url,
                             bool isMoreSection
                            )
{
    //
    // special handlings
    //
    if (groupingName == "disk-usage" && !url.isEmpty()) {
        //
        // "disk-usage" plus a given URL. If no url is given there is no need
        // for special handling
        //

        auto filelightAppIter = std::find_if(kmtServiceList.begin(),
                                             kmtServiceList.end(),
        [](KMoreToolsService* s) {
            return s->desktopEntryName() == "org.kde.filelight";
        });

        if (filelightAppIter != kmtServiceList.end()) {
            auto filelightApp = *filelightAppIter;

            // because we later add all remaining items
            kmtServiceList.removeOne(filelightApp);

            if (url.isLocalFile()) { // 2015-01-12: Filelight can handle FTP connections
                // but KIO/kioexec cannot (bug or feature?), so we
                // don't offer it in this case

                const auto filelight1Item = menuBuilder->addMenuItem(filelightApp);

                if (filelightApp->isInstalled()) {
                    const auto filelightService = filelightApp->installedService();

                    filelight1Item->action()->setText(filelightApp->formatString(
                                                          i18nc("@action:inmenu", "$GenericName - current folder")));
                    menu->connect(filelight1Item->action(), &QAction::triggered, menu,
                    [filelightService, url](bool) {
                        KRun::runService(*filelightService, { url }, nullptr);
                    });

                    const auto filelight2Item = menuBuilder->addMenuItem(filelightApp);
                    filelight2Item->action()->setText(filelightApp->formatString(
                                                          i18nc("@action:inmenu", "$GenericName - current device")));
                    menu->connect(filelight2Item->action(), &QAction::triggered, menu,
                    [filelightService, url](bool) {
                        KMountPoint::Ptr mountPoint
                            = KMountPoint::currentMountPoints().findByPath(url.toLocalFile());
                        KRun::runService(*filelightService,
                        { QUrl::fromLocalFile(mountPoint->mountPoint()) },
                        nullptr);
                    });
                }
            }

            auto filelight3Item = menuBuilder->addMenuItem(filelightApp, KMoreTools::MenuSection_More);
            if (filelightApp->isInstalled()) {
                filelight3Item->action()->setText(filelightApp->formatString(
                                                      i18nc("@action:inmenu", "$GenericName - all devices")));
                const auto filelightService = filelightApp->installedService();
                menu->connect(filelight3Item->action(), &QAction::triggered, menu,
                [filelightService](bool) {
                    KRun::runService(*filelightService, { }, nullptr);
                });
            }
        } else {
            qWarning() << "org.kde.filelight should be present in KMoreTools but it is not!";
        }
    } else if (groupingName == "disk-partitions") {
        // better because the Partition editors all have the same GenericName
        menuBuilder->setInitialItemTextTemplate("$GenericName ($Name)");

        addItemsFromList(menuBuilder, menu, kmtServiceList, url, isMoreSection);

        menuBuilder->setInitialItemTextTemplate("$GenericName"); // set back to default

        return; // skip processing remaining list (would result in duplicates)
    }

    //
    // default handling (or process remaining list)
    //
    menuBuilder->setInitialItemTextTemplate("$Name"); // just use the application name
    addItemsFromList(menuBuilder, menu, kmtServiceList, url, isMoreSection);
    menuBuilder->setInitialItemTextTemplate("$GenericName"); // set back to default
}

QMenu* KMoreToolsMenuFactory::createMenuFromGroupingNames(
    const QStringList& groupingNames,
    const QUrl& url)
{
    if (d->menu) {
        delete d->menu;
    }

    d->menu = new QMenu();

    const auto menuBuilder = d->kmt->menuBuilder();
    menuBuilder->clear();

    bool isMoreSection = false;

    Q_FOREACH(auto groupingName, groupingNames) {

        if (groupingName == "more:") {
            isMoreSection = true;
            continue;
        }

        auto kmtServiceList = KMoreToolsPresets::registerServicesByGroupingNames(
                                  d->kmt, { groupingName });

        addItemsForGroupingName(menuBuilder,
                                d->menu,
                                kmtServiceList,
                                groupingName,
                                url,
                                isMoreSection);
    }

    menuBuilder->buildByAppendingToMenu(d->menu);

    return d->menu;
}

