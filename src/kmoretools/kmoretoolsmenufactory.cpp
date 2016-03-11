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

#include "kmoretools_p.h"
#include "kmoretoolspresets_p.h"

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

class KMoreToolsLazyMenu : public QMenu
{
private Q_SLOTS:
    void onAboutToShow() {
        //qDebug() << "onAboutToShow";
        clear();
        m_aboutToShowFunc(this);
    }

public:
    KMoreToolsLazyMenu(QWidget* parent = 0) : QMenu(parent) {
        connect(this, &QMenu::aboutToShow, this, &KMoreToolsLazyMenu::onAboutToShow);
    }

    void setAboutToShowAction(std::function<void(QMenu*)> aboutToShowFunc) {
        m_aboutToShowFunc = aboutToShowFunc;
    }

private:
    std::function<void(QMenu*)> m_aboutToShowFunc;
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

// "file static" => no symbol will be exported
static void addItemFromKmtService(KMoreToolsMenuBuilder* menuBuilder,
                                  QMenu* menu,
                                  KMoreToolsService* kmtService,
                                  const QUrl& url,
                                  bool isMoreSection
                                 )
{
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

// "file static" => no symbol will be exported
static void addItemsFromKmtServiceList(KMoreToolsMenuBuilder* menuBuilder,
                                       QMenu* menu,
                                       QList<KMoreToolsService*> kmtServiceList,
                                       const QUrl& url,
                                       bool isMoreSection,
                                       QString firstMoreSectionDesktopEntryName
                                      )
{
    Q_FOREACH(auto kmtService, kmtServiceList) {
        // Check the pointer just in case a null pointer got in somewhere
        if (!kmtService) continue;
        if (kmtService->desktopEntryName() == firstMoreSectionDesktopEntryName) {
            // once we reach the potential first "more section desktop entry name"
            // all remaining services are added to the more section by default
            isMoreSection = true;
        }
        addItemFromKmtService(menuBuilder, menu, kmtService, url, isMoreSection);
    }
}

/**
* "file static" => no symbol will be exported
* @param isMoreSection: true => all items will be added into the more section
* @param firstMoreSectionDesktopEntryName: only valid when @p isMoreSection is false:
*                                           see KMoreToolsPresets::registerServicesByGroupingNames
*/
static void addItemsForGroupingNameWithSpecialHandling(KMoreToolsMenuBuilder* menuBuilder,
                                    QMenu* menu,
                                    QList<KMoreToolsService*> kmtServiceList,
                                    const QString& groupingName,
                                    const QUrl& url,
                                    bool isMoreSection,
                                    QString firstMoreSectionDesktopEntryName
                                   )
{
    //
    // special handlings
    //
    if (groupingName == QLatin1String("disk-usage") && !url.isEmpty()) {
        //
        // "disk-usage" plus a given URL. If no url is given there is no need
        // for special handling
        //

        auto filelightAppIter = std::find_if(kmtServiceList.begin(),
                                             kmtServiceList.end(),
        [](KMoreToolsService* s) {
            return s->desktopEntryName() == QLatin1String("org.kde.filelight");
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

    } else if (groupingName == QLatin1String("disk-partitions")) {
        // better because the Partition editors all have the same GenericName
        menuBuilder->setInitialItemTextTemplate(QStringLiteral("$GenericName ($Name)"));

        addItemsFromKmtServiceList(menuBuilder, menu, kmtServiceList, url, isMoreSection, firstMoreSectionDesktopEntryName);

        menuBuilder->setInitialItemTextTemplate(QStringLiteral("$GenericName")); // set back to default

        return; // skip processing remaining list (would result in duplicates)

    } else if (groupingName == QLatin1String("git-clients-and-actions")) {
        // Here we change the default item text and make sure that the url
        // argument is properly handled.
        //

        menuBuilder->setInitialItemTextTemplate(QStringLiteral("$Name")); // just use the application name

        Q_FOREACH(auto kmtService, kmtServiceList) {
            // Check the pointer just in case a null pointer got in somewhere
            if (!kmtService) continue;
            QUrl argUrl = url;

            if (url.isLocalFile()) { // this can only be done for local files, remote urls probably won't work for git clients anyway
                // by default we need an URL pointing to a directory
                // (this impl currently leads to wrong behaviour if the root dir of a git repo is chosen because it always goes one level up)
                argUrl = KmtUrlUtil::localFileAbsoluteDir(url); // needs local file

                if (kmtService->desktopEntryName() == _("git-cola-view-history.kmt-edition")) {
                    // in this case we need the file because we would like to see its history
                    argUrl = url;
                }
            }

            addItemFromKmtService(menuBuilder, menu, kmtService, argUrl, isMoreSection);
        }

        menuBuilder->setInitialItemTextTemplate(QStringLiteral("$GenericName")); // set back to default

        return; // skip processing remaining list (would result in duplicates)
    }

    //
    // default handling (or process remaining list)
    //
    menuBuilder->setInitialItemTextTemplate(QStringLiteral("$Name")); // just use the application name
    addItemsFromKmtServiceList(menuBuilder, menu, kmtServiceList, url, isMoreSection, firstMoreSectionDesktopEntryName);
    menuBuilder->setInitialItemTextTemplate(QStringLiteral("$GenericName")); // set back to default
}

QMenu* KMoreToolsMenuFactory::createMenuFromGroupingNames(
    const QStringList& groupingNames,
    const QUrl& url)
{
    if (d->menu) {
        delete d->menu;
    }

    auto menu = new KMoreToolsLazyMenu();
    menu->setAboutToShowAction([this, groupingNames, url](QMenu* m) { fillMenuFromGroupingNames(m, groupingNames, url); });
    d->menu = menu;

    return d->menu;
}

void KMoreToolsMenuFactory::fillMenuFromGroupingNames(QMenu* menu, const QStringList& groupingNames, const QUrl& url)
{
    const auto menuBuilder = d->kmt->menuBuilder();
    menuBuilder->clear();

    bool isMoreSection = false;

    Q_FOREACH(auto groupingName, groupingNames) {

        if (groupingName == QLatin1String("more:")) {
            isMoreSection = true;
            continue;
        }

        QString firstMoreSectionDesktopEntryName;
        auto kmtServiceList = KMoreToolsPresetsPrivate::registerServicesByGroupingNames(
                                  &firstMoreSectionDesktopEntryName, d->kmt, { groupingName });

        addItemsForGroupingNameWithSpecialHandling(menuBuilder,
                                menu,
                                kmtServiceList,
                                groupingName,
                                url,
                                isMoreSection,
                                firstMoreSectionDesktopEntryName);
    }

    menuBuilder->buildByAppendingToMenu(menu);
}
