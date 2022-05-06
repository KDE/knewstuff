/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kmoretoolsmenufactory.h"

#include "kmoretools_p.h"
#include "kmoretoolspresets_p.h"
#include "knewstuff_debug.h"
#include <QDebug>
#include <QStorageInfo>

#include <KDialogJobUiDelegate>
#include <KIO/ApplicationLauncherJob>
#include <KLocalizedString>

#include "kmoretools.h"
#include "kmoretoolspresets.h"

class KMoreToolsMenuFactoryPrivate
{
public:
    // Note that this object must live long enough in case the user opens
    // the "Configure..." dialog
    KMoreTools *kmt = nullptr;

    QMenu *menu = nullptr;
    QWidget *parentWidget = nullptr;
};

class KMoreToolsLazyMenu : public QMenu
{
private Q_SLOTS:
    void onAboutToShow()
    {
        // qDebug() << "onAboutToShow";
        clear();
        m_aboutToShowFunc(this);
    }

public:
    KMoreToolsLazyMenu(QWidget *parent = nullptr)
        : QMenu(parent)
    {
        connect(this, &QMenu::aboutToShow, this, &KMoreToolsLazyMenu::onAboutToShow);
    }

    void setAboutToShowAction(std::function<void(QMenu *)> aboutToShowFunc)
    {
        m_aboutToShowFunc = aboutToShowFunc;
    }

private:
    std::function<void(QMenu *)> m_aboutToShowFunc;
};

KMoreToolsMenuFactory::KMoreToolsMenuFactory(const QString &uniqueId)
    : d(new KMoreToolsMenuFactoryPrivate())
{
    d->kmt = new KMoreTools(uniqueId);
    Q_UNUSED(m_off)
}

KMoreToolsMenuFactory::~KMoreToolsMenuFactory()
{
    if (d->menu && !d->menu->parent()) {
        delete d->menu;
    }

    delete d->kmt;
}

static void runApplication(const KService::Ptr &service, const QList<QUrl> &urls)
{
    auto *job = new KIO::ApplicationLauncherJob(service);
    job->setUrls(urls);
    job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->start();
}

// "file static" => no symbol will be exported
static void addItemFromKmtService(KMoreToolsMenuBuilder *menuBuilder, QMenu *menu, KMoreToolsService *kmtService, const QUrl &url, bool isMoreSection)
{
    auto menuItem = menuBuilder->addMenuItem(kmtService, isMoreSection ? KMoreTools::MenuSection_More : KMoreTools::MenuSection_Main);

    if (kmtService->isInstalled()) {
        auto kService = kmtService->installedService();

        if (!kService) {
            // if the corresponding desktop file is not installed
            // then the isInstalled was true because of the Exec line check
            // and we use the desktopfile provided by KMoreTools.
            // Otherwise *kService would crash.
            qCDebug(KNEWSTUFF) << "Desktop file not installed:" << kmtService->desktopEntryName() << "=> Use desktop file provided by KMoreTools";
            kService = kmtService->kmtProvidedService();
        }

        if (!url.isEmpty() && kmtService->maxUrlArgCount() > 0) {
            menu->connect(menuItem->action(), &QAction::triggered, menu, [kService, url](bool) {
                runApplication(kService, {url});
            });
        } else {
            menu->connect(menuItem->action(), &QAction::triggered, menu, [kService](bool) {
                runApplication(kService, {});
            });
        }
    }
}

// "file static" => no symbol will be exported
static void addItemsFromKmtServiceList(KMoreToolsMenuBuilder *menuBuilder,
                                       QMenu *menu,
                                       const QList<KMoreToolsService *> &kmtServiceList,
                                       const QUrl &url,
                                       bool isMoreSection,
                                       QString firstMoreSectionDesktopEntryName)
{
    for (auto kmtService : kmtServiceList) {
        // Check the pointer just in case a null pointer got in somewhere
        if (!kmtService) {
            continue;
        }
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
static void addItemsForGroupingNameWithSpecialHandling(KMoreToolsMenuBuilder *menuBuilder,
                                                       QMenu *menu,
                                                       QList<KMoreToolsService *> kmtServiceList,
                                                       const QString &groupingName,
                                                       const QUrl &url,
                                                       bool isMoreSection,
                                                       QString firstMoreSectionDesktopEntryName)
{
    //
    // special handlings
    //
    if (groupingName == QLatin1String("disk-usage") && !url.isEmpty()) {
        //
        // "disk-usage" plus a given URL. If no url is given there is no need
        // for special handling
        //

        auto filelightAppIter = std::find_if(kmtServiceList.begin(), kmtServiceList.end(), [](KMoreToolsService *s) {
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

                    filelight1Item->action()->setText(
                        filelightApp->formatString(i18nc("@action:inmenu %1=\"$GenericName\"", "%1 - current folder", QStringLiteral("$GenericName"))));
                    menu->connect(filelight1Item->action(), &QAction::triggered, menu, [filelightService, url](bool) {
                        runApplication(filelightService, {url});
                    });

                    const auto filelight2Item = menuBuilder->addMenuItem(filelightApp);
                    filelight2Item->action()->setText(
                        filelightApp->formatString(i18nc("@action:inmenu %1=\"$GenericName\"", "%1 - current device", QStringLiteral("$GenericName"))));
                    menu->connect(filelight2Item->action(), &QAction::triggered, menu, [filelightService, url](bool) {
                        const QStorageInfo info(url.toLocalFile());

                        if (info.isValid() && info.isReady()) {
                            runApplication(filelightService, {QUrl::fromLocalFile(info.rootPath())});
                        }
                    });
                }
            }

            auto filelight3Item = menuBuilder->addMenuItem(filelightApp, KMoreTools::MenuSection_More);
            if (filelightApp->isInstalled()) {
                filelight3Item->action()->setText(
                    filelightApp->formatString(i18nc("@action:inmenu %1=\"$GenericName\"", "%1 - all devices", QStringLiteral("$GenericName"))));
                const auto filelightService = filelightApp->installedService();
                menu->connect(filelight3Item->action(), &QAction::triggered, menu, [filelightService](bool) {
                    runApplication(filelightService, {});
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

        for (auto kmtService : std::as_const(kmtServiceList)) {
            // Check the pointer just in case a null pointer got in somewhere
            if (!kmtService) {
                continue;
            }
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

QMenu *KMoreToolsMenuFactory::createMenuFromGroupingNames(const QStringList &groupingNames, const QUrl &url)
{
    delete d->menu;

    auto menu = new KMoreToolsLazyMenu(d->parentWidget);
    menu->setAboutToShowAction([this, groupingNames, url](QMenu *m) {
        fillMenuFromGroupingNames(m, groupingNames, url);
    });
    d->menu = menu;

    return d->menu;
}

void KMoreToolsMenuFactory::fillMenuFromGroupingNames(QMenu *menu, const QStringList &groupingNames, const QUrl &url)
{
    const auto menuBuilder = d->kmt->menuBuilder();
    menuBuilder->clear();

    bool isMoreSection = false;

    for (const auto &groupingName : groupingNames) {
        if (groupingName == QLatin1String("more:")) {
            isMoreSection = true;
            continue;
        }

        QString firstMoreSectionDesktopEntryName;
        auto kmtServiceList = KMoreToolsPresetsPrivate::registerServicesByGroupingNames(&firstMoreSectionDesktopEntryName, d->kmt, {groupingName});

        addItemsForGroupingNameWithSpecialHandling(menuBuilder, menu, kmtServiceList, groupingName, url, isMoreSection, firstMoreSectionDesktopEntryName);
    }

    menuBuilder->buildByAppendingToMenu(menu);
}

void KMoreToolsMenuFactory::setParentWidget(QWidget *widget)
{
    d->parentWidget = widget;
}
