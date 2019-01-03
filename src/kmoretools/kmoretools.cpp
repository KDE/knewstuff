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

#include "kmoretools.h"

#include "kmoretools_p.h"
#include "kmoretoolsconfigdialog_p.h"
#include "knewstuff_debug.h"

#include <QDebug>
#include <QDir>
#include <KService>
#include <QStandardPaths>
#include <QApplication>

#include <klocalizedstring.h>
#include <KConfig>
#include <KConfigGroup>

class KMoreToolsPrivate
{
public:
    QString uniqueId;

    // allocated via new, don't forget to delete
    QList<KMoreToolsService*> serviceList;

    QMap<QString, KMoreToolsMenuBuilder*> menuBuilderMap;

public:
    KMoreToolsPrivate(const QString& uniqueId)
        : uniqueId(uniqueId)
    {
    }

    ~KMoreToolsPrivate()
    {
        qDeleteAll(menuBuilderMap);
        qDeleteAll(serviceList);
    }

    /**
     * @return uniqueId if kmtDesktopfileSubdir is empty
     * else kmtDesktopfileSubdir
     */
    QString kmtDesktopfileSubdirOrUniqueId(const QString& kmtDesktopfileSubdir) {
        if (kmtDesktopfileSubdir.isEmpty()) {
            return uniqueId;
        }

        return kmtDesktopfileSubdir;
    }

    /**
     * Finds a file in the '/usr/share'/kf5/kmoretools/'uniqueId'/ directory.
     * '/usr/share' = "~/.local/share", "/usr/local/share", "/usr/share" (see QStandardPaths::GenericDataLocation)
     * 'uniqueId' = @see uniqueId()
     *
     * @param can be a filename with or without relative path. But no absolute path.
     * @returns the first occurrence if there are more than one found
     */
    QString findFileInKmtDesktopfilesDir(const QString& filename)
    {
        return findFileInKmtDesktopfilesDir(uniqueId, filename);
    }

    static QString findFileInKmtDesktopfilesDir(const QString& kmtDesktopfileSubdir, const QString& filename)
    {
        //qDebug() << "--search locations:" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation); // /usr/share etc.
        const QString kmtDesktopfilesFilename = QLatin1String("kf5/kmoretools/") + kmtDesktopfileSubdir + QLatin1Char('/') + filename;
        //qDebug() << "---search for:" << kmtDesktopfilesFilename;
        const QString foundKmtFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, kmtDesktopfilesFilename);
        //qDebug() << "----QStandardPaths::locate(QStandardPaths::GenericDataLocation, kmtDesktopfilesFilename) -> foundKmtFile" << foundKmtFile;

        return foundKmtFile;
    }
};

KMoreTools::KMoreTools(const QString& uniqueId)
    : d(new KMoreToolsPrivate(uniqueId))
{

}

KMoreTools::~KMoreTools()
{
    delete d;
}

KMoreToolsService* KMoreTools::registerServiceByDesktopEntryName(
    const QString& desktopEntryName,
    const QString& kmtDesktopfileSubdir,
    KMoreTools::ServiceLocatingMode serviceLocatingMode)
{
    //qDebug() << "* registerServiceByDesktopEntryName(desktopEntryName=" << desktopEntryName;

    const QString foundKmtDesktopfilePath = d->findFileInKmtDesktopfilesDir(
            d->kmtDesktopfileSubdirOrUniqueId(kmtDesktopfileSubdir),
            desktopEntryName + QLatin1String(".desktop"));
    const bool isKmtDesktopfileProvided = !foundKmtDesktopfilePath.isEmpty();

    KService::Ptr kmtDesktopfile;

    if (isKmtDesktopfileProvided) {
        kmtDesktopfile = KService::Ptr(new KService(foundKmtDesktopfilePath));
        // todo later: what exactly does "isValid" mean? Valid syntax? Or installed in system?
        //             right now we cannot use it
        //Q_ASSERT_X(kmtDesktopfile->isValid(), "addServiceByDesktopFile", "the kmt-desktopfile is provided but not valid. This must be fixed.");
        //qDebug() << "  INFO: kmt-desktopfile provided and valid.";
        if (kmtDesktopfile->exec().isEmpty()) {
            qCCritical(KNEWSTUFF) << "KMoreTools::registerServiceByDesktopEntryName: the kmt-desktopfile " << desktopEntryName << " is provided but no Exec line is specified. The desktop file is probably faulty. Please fix. Return nullptr.";
            return nullptr;
        }
        //qDebug() << "  INFO: kmt-desktopfile provided.";
    } else {
        qCWarning(KNEWSTUFF) << "KMoreTools::registerServiceByDesktopEntryName: desktopEntryName " << desktopEntryName << " (kmtDesktopfileSubdir=" << kmtDesktopfileSubdir << ") not provided (or at the wrong place) in the installed kmt-desktopfiles directory. If the service is also not installed on the system the user won't get nice translated app name and description.";
        qCDebug(KNEWSTUFF) << "`-- More info at findFileInKmtDesktopfilesDir, QStandardPaths::standardLocations = " << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation); // /usr/share etc.
    }

    bool isInstalled = false;
    KService::Ptr installedService;
    if (serviceLocatingMode == KMoreTools::ServiceLocatingMode_Default) { // == default behaviour: search for installed services
        installedService = KService::serviceByDesktopName(desktopEntryName);
        isInstalled = installedService != nullptr;
        //qDebug() << "----- isInstalled: " << isInstalled;
    } else if (serviceLocatingMode == KMoreTools::ServiceLocatingMode_ByProvidedExecLine) { // only use provided kmt-desktopfile:
        if (!isKmtDesktopfileProvided) {
            qCCritical(KNEWSTUFF) << "KMoreTools::registerServiceByDesktopEntryName for " << desktopEntryName << ": If detectServiceExistenceViaProvidedExecLine is true then a kmt-desktopfile must be provided. Please fix. Return nullptr.";
            return nullptr;
        }

        auto tryExecProp = kmtDesktopfile->property(QStringLiteral("TryExec"), QVariant::String);
        isInstalled = (tryExecProp.isValid() && !QStandardPaths::findExecutable(tryExecProp.toString()).isEmpty())
                      || !QStandardPaths::findExecutable(kmtDesktopfile->exec()).isEmpty();
    } else {
        Q_ASSERT(false); // case not handled
    }

//     if (isInstalled) {
//         qDebug() << "registerServiceByDesktopEntryName:" << desktopEntryName << ": installed.";
//     } else {
//         qDebug() << "registerServiceByDesktopEntryName:" << desktopEntryName << ": NOT installed.";
//     }

    auto registeredService = new KMoreToolsService(
        d->kmtDesktopfileSubdirOrUniqueId(kmtDesktopfileSubdir),
        desktopEntryName,
        isInstalled,
        installedService,
        kmtDesktopfile);

    // add or replace item in serviceList
    auto foundService = std::find_if(d->serviceList.begin(), d->serviceList.end(),
    [desktopEntryName](KMoreToolsService* service) {
        return service->desktopEntryName() == desktopEntryName;
    });
    if (foundService == d->serviceList.end()) {
        //qDebug() << "not found, add new service";
        d->serviceList.append(registeredService);
    } else {
        KMoreToolsService* foundServicePtr = *foundService;
        int i = d->serviceList.indexOf(foundServicePtr);
        //qDebug() << "found: replace it with new service, index=" << i;
        delete foundServicePtr;
        //qDebug() << "   deleted";
        d->serviceList.replace(i, registeredService);
        //qDebug() << "   replaced in list";
    }

    return registeredService;
}

KMoreToolsMenuBuilder* KMoreTools::menuBuilder(const QString& userConfigPostfix) const
{
    if (d->menuBuilderMap.find(userConfigPostfix) == d->menuBuilderMap.end()) {
        d->menuBuilderMap.insert(userConfigPostfix,
                                 new KMoreToolsMenuBuilder(d->uniqueId, userConfigPostfix));
    }
    return d->menuBuilderMap[userConfigPostfix];
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

class KMoreToolsServicePrivate
{
public:
    QString kmtDesktopfileSubdir;
    QString desktopEntryName;
    KService::Ptr installedService;
    KService::Ptr kmtDesktopfile;
    QUrl homepageUrl;
    int maxUrlArgCount = 0;
    bool isInstalled = false;
    QString appstreamId;


public:
    QString getServiceName()
    {
        if (installedService) {
            return installedService->name();
        } else {
            if (kmtDesktopfile) {
                return kmtDesktopfile->name();
            } else {
                return QString();
            }
        }
    }

    QString getServiceGenericName()
    {
        if (installedService) {
            return installedService->genericName();
        } else {
            if (kmtDesktopfile) {
                return kmtDesktopfile->genericName();
            } else {
                return QString();
            }
        }
    }

    /**
     * @return the provided icon or an empty icon if not kmtDesktopfile is available or the icon was not found
     */
    QIcon getKmtProvidedIcon()
    {
        if (!kmtDesktopfile) {
            return QIcon();
        }

        QString iconPath = KMoreToolsPrivate::findFileInKmtDesktopfilesDir(kmtDesktopfileSubdir, kmtDesktopfile->icon() + QLatin1String(".svg"));
        //qDebug() << "kmt iconPath" << iconPath;
        QIcon svgIcon(iconPath);
        if (!svgIcon.isNull()) {
            return svgIcon;
        }

        iconPath = KMoreToolsPrivate::findFileInKmtDesktopfilesDir(kmtDesktopfileSubdir, kmtDesktopfile->icon() + QLatin1String(".png"));
        //qDebug() << "kmt iconPath" << iconPath;
        QIcon pngIcon(iconPath);
        if (!pngIcon.isNull()) {
            return pngIcon;
        }

        return QIcon();
    }
};

KMoreToolsService::KMoreToolsService(const QString& kmtDesktopfileSubdir,
                                     const QString& desktopEntryName,
                                     bool isInstalled,
                                     KService::Ptr installedService,
                                     KService::Ptr kmtDesktopfile)
    : d(new KMoreToolsServicePrivate())
{
    d->kmtDesktopfileSubdir = kmtDesktopfileSubdir;
    d->desktopEntryName = desktopEntryName;
    d->isInstalled = isInstalled;
    d->installedService = installedService;
    d->kmtDesktopfile = kmtDesktopfile;
}

KMoreToolsService::~KMoreToolsService()
{
    delete d;
}

QString KMoreToolsService::desktopEntryName() const
{
    return d->desktopEntryName;
}

bool KMoreToolsService::isInstalled() const
{
    return d->isInstalled;
}

KService::Ptr KMoreToolsService::installedService() const
{
    return d->installedService;
}

KService::Ptr KMoreToolsService::kmtProvidedService() const
{
    return d->kmtDesktopfile;
}

QIcon KMoreToolsService::kmtProvidedIcon() const
{
    return d->getKmtProvidedIcon();
}

QUrl KMoreToolsService::homepageUrl() const
{
    return d->homepageUrl;
}

void KMoreToolsService::setHomepageUrl(const QUrl& url)
{
    d->homepageUrl = url;
}

int KMoreToolsService::maxUrlArgCount() const
{
    return d->maxUrlArgCount;
}

void KMoreToolsService::setMaxUrlArgCount(int maxUrlArgCount)
{
    d->maxUrlArgCount = maxUrlArgCount;
}

QString KMoreToolsService::formatString(const QString& formatString) const
{
    QString result = formatString;

    QString genericName = d->getServiceGenericName();
    if (genericName.isEmpty()) {
        genericName = d->getServiceName();
        if (genericName.isEmpty()) {
            genericName = desktopEntryName();
        }
    }

    QString name = d->getServiceName();
    if (name.isEmpty()) {
        name = desktopEntryName();
    }

    result.replace(QLatin1String("$GenericName"), genericName);
    result.replace(QLatin1String("$Name"), name);
    result.replace(QLatin1String("$DesktopEntryName"), desktopEntryName());

    return result;
}

QIcon KMoreToolsService::icon() const
{
    if (installedService() != nullptr) {
        return QIcon::fromTheme(installedService()->icon());
    } else if (kmtProvidedService() != nullptr) {
        return d->getKmtProvidedIcon();
    } else {
        return QIcon();
    }
}

void KMoreToolsService::setExec(const QString& exec)
{
    auto service = installedService();
    if (service) {
        service->setExec(exec);
    }
}

QString KMoreToolsService::appstreamId() const
{
    return d->appstreamId;
}

void KMoreToolsService::setAppstreamId(const QString& id)
{
    d->appstreamId = id;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

const QString configFile = QStringLiteral("kmoretoolsrc");
const QString configKey = QStringLiteral("menu_structure");

class KMoreToolsMenuBuilderPrivate
{
public:
    QString uniqueId;
    /**
     * default value is "", see KMoreTools::menuBuilder()
     */
    QString userConfigPostfix;
    QList<KMoreToolsMenuItem*> menuItems;
    KmtMenuItemIdGen menuItemIdGen;
    QString initialItemTextTemplate = QStringLiteral("$GenericName");

public:
    KMoreToolsMenuBuilderPrivate()
    {
    }

    ~KMoreToolsMenuBuilderPrivate()
    {
    }

    void deleteAndClearMenuItems()
    {
        Q_FOREACH (auto item, menuItems)
        {
            //qDebug() << item;
            delete item;
        }

        menuItems.clear();
    }

    KmtMenuStructureDto readUserConfig() const
    {
        KConfig config(configFile, KConfig::NoGlobals, QStandardPaths::ConfigLocation);
        auto configGroup = config.group(uniqueId + userConfigPostfix);
        QString json = configGroup.readEntry(configKey, "");
        KmtMenuStructureDto configuredStructure;
        //qDebug() << "read from config: " << json;
        configuredStructure.deserialize(json);
        return configuredStructure;
    }

    void writeUserConfig(const KmtMenuStructureDto& mstruct) const
    {
        KConfig config(configFile, KConfig::NoGlobals, QStandardPaths::ConfigLocation);
        auto configGroup = config.group(uniqueId + userConfigPostfix);
        auto configValue = mstruct.serialize();
        //qDebug() << "write to config: " << configValue;
        configGroup.writeEntry(configKey, configValue);
        configGroup.sync();
    }

    enum CreateMenuStructureOption
    {
        CreateMenuStructure_Default,
        CreateMenuStructure_MergeWithUserConfig
    };

    /**
     * Merge strategy if createMenuStructureOption == CreateMenuStructure_MergeWithUserConfig
     * --------------------------------------------------------------------------------------
     * 1) For each 'main' section item from configStruct
     *      lookup in current structure (all installed items) and if found add to new structure
     *    This means items which are in configStruct but not in current structure will be discarded.
     *
     * 2) Add remaining 'main' section items from current to new structure
     *
     * 3) Do the 1) and 2) analogous for 'more' section
     *
     *
     * How default structure and DTOs play together
     * --------------------------------------------
     * Part 1:
     *
     *   defaultStruct (in memory, defined by application that uses KMoreTools)
     * + configuredStruct (DTO, loaded from disk, from json)
     * = currentStruct (in memory, used to create the actual menu)
     * This is done by KMoreToolsMenuBuilderPrivate::createMenuStructure(mergeWithUserConfig = true).
     *
     * Part 2:
     * defaultStruct => defaultStructDto
     * currentStruct => currentStructDto
     * Both DTOs go to the Configure dialog.
     * Users edits structure => new configuredStruct (DTO => to json => to disk)
     *
     *
     * If createMenuStructureOption == CreateMenuStructure_Default then the default menu structure is returned.
     */
    KmtMenuStructure createMenuStructure(CreateMenuStructureOption createMenuStructureOption) const
    {
        KmtMenuStructureDto configuredStructure; // if this stays empty then the default structure will not be changed
        if (createMenuStructureOption == CreateMenuStructure_MergeWithUserConfig) {
            // fill if should be merged
            configuredStructure = readUserConfig();
        }

        KmtMenuStructure mstruct;

        QList<KMoreToolsMenuItem*> menuItemsSource = menuItems;
        QList<KMoreToolsMenuItem*> menuItemsSortedAsConfigured;

        // presort as in configuredStructure
        //
        Q_FOREACH (const auto& item, configuredStructure.list) {
            auto foundItem = std::find_if(menuItemsSource.begin(), menuItemsSource.end(),
            [item](const KMoreToolsMenuItem* kMenuItem) {
                return kMenuItem->id() == item.id;
            });
            if (foundItem != menuItemsSource.end()) {
                menuItemsSortedAsConfigured.append(*foundItem); // add to final list
                menuItemsSource.removeOne(*foundItem); // remove from source
            }
        }
        // Add remaining items from source. These may be main and more section items
        // so that the resulting list may have [ main items, more items, main items, more items ]
        // instead of only [ main items, more items ]
        // But in the next step this won't matter.
        menuItemsSortedAsConfigured.append(menuItemsSource);

        // build MenuStructure from presorted list
        //
        Q_FOREACH (auto item, menuItemsSortedAsConfigured) {

            const auto registeredService = item->registeredService();

            if ((registeredService && registeredService->isInstalled())
                    || !registeredService) { // if a QAction was registered directly
                auto confItem = configuredStructure.findInstalled(item->id());
                if ((!confItem && item->defaultLocation() == KMoreTools::MenuSection_Main)
                        || (confItem && confItem->menuSection == KMoreTools::MenuSection_Main)) {
                    mstruct.mainItems.append(item);
                } else if ((!confItem && item->defaultLocation() == KMoreTools::MenuSection_More)
                           || (confItem && confItem->menuSection == KMoreTools::MenuSection_More)) {
                    mstruct.moreItems.append(item);
                } else {
                    Q_ASSERT_X(false, "buildAndAppendToMenu", "invalid enum"); // todo/later: apart from static programming error, if the config garbage this might happen
                }
            } else {
                if (!mstruct.notInstalledServices.contains(item->registeredService())) {
                    mstruct.notInstalledServices.append(item->registeredService());
                }
            }
        }

        return mstruct;
    }

    /**
     * @param defaultStructure also contains the currently not-installed items
     */
    void showConfigDialog(KmtMenuStructureDto defaultStructureDto, const QString& title = QString()) const
    {
        // read from config
        //
        auto currentStructure = createMenuStructure(CreateMenuStructure_MergeWithUserConfig);
        auto currentStructureDto = currentStructure.toDto();

        KMoreToolsConfigDialog *dlg = new KMoreToolsConfigDialog(defaultStructureDto, currentStructureDto, title);
        if (dlg->exec() == QDialog::Accepted) {
            currentStructureDto = dlg->currentStructure();
            writeUserConfig(currentStructureDto);
        }

        delete dlg;
    }

    /**
     * Create the 'More' menu with parent as parent
     * @param parent The parent of the menu
     */
    void createMoreMenu(const KmtMenuStructure &mstruct, QMenu *parent)
    {
        Q_FOREACH (auto item, mstruct.moreItems) {
            const auto action = item->action();
            action->setParent(parent);
            parent->addAction(action);
        }

        if (!mstruct.notInstalledServices.isEmpty()) {
            //qDebug() << "notInstalledItems not empty => build 'Not installed' section";
            parent->addSection(i18nc("@action:inmenu", "Not installed:"));

            Q_FOREACH (auto registeredService, mstruct.notInstalledServices) {

                QMenu* submenuForNotInstalled = KmtNotInstalledUtil::createSubmenuForNotInstalledApp(
                                                    registeredService->formatString(QStringLiteral("$Name")), parent, registeredService->icon(), registeredService->homepageUrl(), registeredService->appstreamId());
                parent->addMenu(submenuForNotInstalled);
            }
        }
    }
};

KMoreToolsMenuBuilder::KMoreToolsMenuBuilder()
{
    Q_ASSERT(false);
}

KMoreToolsMenuBuilder::KMoreToolsMenuBuilder(const QString& uniqueId, const QString& userConfigPostfix)
    : d(new KMoreToolsMenuBuilderPrivate())
{
    d->uniqueId = uniqueId;
    d->userConfigPostfix = userConfigPostfix;
}

KMoreToolsMenuBuilder::~KMoreToolsMenuBuilder()
{
    d->deleteAndClearMenuItems();
    delete d;
}

void KMoreToolsMenuBuilder::setInitialItemTextTemplate(const QString& templateText)
{
    d->initialItemTextTemplate = templateText;
}

KMoreToolsMenuItem* KMoreToolsMenuBuilder::addMenuItem(KMoreToolsService* registeredService, KMoreTools::MenuSection defaultLocation)
{
    auto kmtMenuItem = new KMoreToolsMenuItem(registeredService, defaultLocation, d->initialItemTextTemplate);
    kmtMenuItem->setId(d->menuItemIdGen.getId(registeredService->desktopEntryName()));
    d->menuItems.append(kmtMenuItem);
    return kmtMenuItem;
}

KMoreToolsMenuItem* KMoreToolsMenuBuilder::addMenuItem(QAction* action, const QString& itemId, KMoreTools::MenuSection defaultLocation)
{
    auto kmtMenuItem = new KMoreToolsMenuItem(action, d->menuItemIdGen.getId(itemId), defaultLocation);
    d->menuItems.append(kmtMenuItem);
    return kmtMenuItem;
}

void KMoreToolsMenuBuilder::clear()
{
    //qDebug() << "----KMoreToolsMenuBuilder::clear()";
    //qDebug() << "d" << d;
    //qDebug() << "d->menuItems" << d->menuItems.count();
    d->deleteAndClearMenuItems();
    //qDebug() << "----after d->menuItems.clear();";
    d->menuItemIdGen.reset();
}

QString KMoreToolsMenuBuilder::menuStructureAsString(bool mergeWithUserConfig) const
{
    KmtMenuStructure mstruct = d->createMenuStructure(mergeWithUserConfig ?
                            KMoreToolsMenuBuilderPrivate::CreateMenuStructure_MergeWithUserConfig
                            : KMoreToolsMenuBuilderPrivate::CreateMenuStructure_Default);
    QString s;
    s += QLatin1String("|main|:");
    Q_FOREACH (auto item, mstruct.mainItems) {
        s += item->registeredService()->desktopEntryName() + QLatin1Char('.');
    }
    s += QLatin1String("|more|:");
    Q_FOREACH (auto item, mstruct.moreItems) {
        s += item->registeredService()->desktopEntryName() + QLatin1Char('.');
    }
    s += QLatin1String("|notinstalled|:");
    Q_FOREACH (auto regService, mstruct.notInstalledServices) {
        s += regService->desktopEntryName() + QLatin1Char('.');
    }
    return s;
}

// TMP / for unit test
void KMoreToolsMenuBuilder::showConfigDialog(const QString& title)
{
    d->showConfigDialog(d->createMenuStructure(KMoreToolsMenuBuilderPrivate::CreateMenuStructure_Default).toDto(), title);
}

void KMoreToolsMenuBuilder::buildByAppendingToMenu(QMenu* menu,
        KMoreTools::ConfigureDialogAccessibleSetting configureDialogAccessibleSetting, QMenu** outMoreMenu)
{
    KmtMenuStructure mstruct = d->createMenuStructure(KMoreToolsMenuBuilderPrivate::CreateMenuStructure_MergeWithUserConfig);

    Q_FOREACH (auto item, mstruct.mainItems) {
        const auto action = item->action();
        if (!action->parent()) { // if the action has no parent, set it to the menu to be filled
            action->setParent(menu);
        }
        menu->addAction(action);
    }

    QMenu* moreMenu = new QMenu(i18nc("@action:inmenu", "More"), menu);

    if (!mstruct.moreItems.isEmpty() || !mstruct.notInstalledServices.isEmpty()) {

        if (mstruct.mainItems.isEmpty()) {
            d->createMoreMenu(mstruct, menu);
        } else {
            menu->addSeparator();
            menu->addMenu(moreMenu);
            d->createMoreMenu(mstruct, moreMenu);
        }
    }

    if (moreMenu->isEmpty()) {
        if (outMoreMenu) {
            *outMoreMenu = nullptr;
        }
    } else {
        if (outMoreMenu) {
            *outMoreMenu = moreMenu;
        }
    }

    QMenu* baseMenu;
    // either the "Configure..." menu should be shown via setting or the Ctrl key is pressed
    if (configureDialogAccessibleSetting == KMoreTools::ConfigureDialogAccessible_Always
            || QApplication::keyboardModifiers() & Qt::ControlModifier
            || (configureDialogAccessibleSetting == KMoreTools::ConfigureDialogAccessible_Defensive && !mstruct.notInstalledServices.empty())) {
        if (moreMenu->isEmpty()) { // "more" menu was not created...
            // ...then we add the configure menu to the main menu
            baseMenu = menu;
        } else { // more menu has items
            // ...then it was added to main menu and has got at least on item
            baseMenu = moreMenu;
        }

        if (!baseMenu->isEmpty()) {
            baseMenu->addSeparator();
            auto configureAction = baseMenu->addAction(QIcon::fromTheme(QStringLiteral("configure")), i18nc("@action:inmenu", "Configure..."));
            configureAction->setData(QStringLiteral("configureItem")); // tag the action (currently only used in unit-test)
            KmtMenuStructure mstructDefault = d->createMenuStructure(KMoreToolsMenuBuilderPrivate::CreateMenuStructure_Default);
            KmtMenuStructureDto mstructDefaultDto = mstructDefault.toDto(); // makes sure the "Reset" button works as expected
            QObject::connect(configureAction, &QAction::triggered, configureAction, [this, mstructDefaultDto](bool) {
                this->d->showConfigDialog(mstructDefaultDto);
            });
        }
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

class KMoreToolsMenuItemPrivate
{
public:
    QString id;
    KMoreToolsService* registeredService = nullptr;
    QString initialItemText;
    QAction* action = nullptr;
    KMoreTools::MenuSection defaultLocation;
    bool actionAutoCreated = false; // action might stay nullptr even if actionCreated is true
};

KMoreToolsMenuItem::KMoreToolsMenuItem(KMoreToolsService* registeredService, KMoreTools::MenuSection defaultLocation, const QString& initialItemTextTemplate)
    : d(new KMoreToolsMenuItemPrivate())
{
    d->registeredService = registeredService;
    d->defaultLocation = defaultLocation;

    // set menu item caption (text)
    QString defaultName = registeredService->formatString(initialItemTextTemplate); // e.g. "$GenericName", "$Name"
    d->initialItemText = registeredService->formatString(defaultName);
}

KMoreToolsMenuItem::KMoreToolsMenuItem(QAction* action, const QString& itemId, KMoreTools::MenuSection defaultLocation)
    : d(new KMoreToolsMenuItemPrivate())
{
    d->action = action;
    d->id = itemId;
    d->defaultLocation = defaultLocation;
}

KMoreToolsMenuItem::~KMoreToolsMenuItem()
{
    if (d->actionAutoCreated && d->action) { // Only do this if KMoreTools created the action. Other actions must be deleted by client.
        // d->action can already be nullptr in some cases.
        // Disconnects the 'connect' event (and potentially more; is this bad?)
        // that was connected in action() to detect action deletion.
        d->action->disconnect(d->action);
        delete d;
    }
}

QString KMoreToolsMenuItem::id() const
{
    return d->id;
}

void KMoreToolsMenuItem::setId(const QString& id)
{
    d->id = id;
}

KMoreToolsService* KMoreToolsMenuItem::registeredService() const
{
    return d->registeredService;
}

KMoreTools::MenuSection KMoreToolsMenuItem::defaultLocation() const
{
    return d->defaultLocation;
}

QString KMoreToolsMenuItem::initialItemText() const
{
    return d->initialItemText;
}

void KMoreToolsMenuItem::setInitialItemText(const QString& itemText)
{
    d->initialItemText = itemText;
}

QAction* KMoreToolsMenuItem::action() const
{
    // currently we assume if a registeredService is given we auto-create the QAction once
    if (d->registeredService && !d->actionAutoCreated) {
        d->actionAutoCreated = true;

        if (d->registeredService->isInstalled()) {
            d->action = new QAction(d->registeredService->icon(), d->initialItemText, nullptr);
            // reset the action cache when action gets destroyed
            // this happens in unit-tests where menu.clear() is called before another buildByAppendingToMenu call
            // WARN: see also destructor! (might be a source of bugs?)
            QObject::connect(d->action, &QObject::destroyed, d->action, [this]() {
                this->d->actionAutoCreated = false;
                this->d->action = nullptr;
            });
        } else {
            d->action = nullptr;
        }
    }
    // else:
    // !d->registeredService => action will be provided by user
    // or d->actionAutoCreated => action was autocreated (or set to nullptr if service not installed)

    return d->action;
}

