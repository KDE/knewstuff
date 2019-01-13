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

#ifndef KMORETOOLS_H
#define KMORETOOLS_H

#include <QString>
#include <QUrl>
#include <QMenu>

#include <KService>

#include "knewstuff_export.h"

class KMoreToolsService;
class KMoreToolsMenuBuilder;
class KMoreToolsPrivate;

/**
 * Helps to create user-configurable menus with tools which are potentially not yet installed.
 *
 * This class is one entry point of the KMoreTools API.
 * See also KMoreToolsMenuFactory.
 *
 * @note This is a new API (published within KNewStuff since April 2015). Its current
 * target are KDE applications which are part of the kdesrcbuild infrastructure.
 * Here, it is possible to find all usages and to adapt to API changes when needed.
 * So, if you use this in your own application, beware that there might be API
 * changes when more use cases are developed.
 *
 *
 * Introduction
 * ------------
 * KMoreTools helps to to build user-configurable menus with tools which
 * might not installed yet. These tools may also take URL arguments supplied
 * by the application.
 *
 * The user will see a menu item for a tool even if it is not installed (in the
 * 'More' section). Furthermore, it makes long menus shorter by providing a
 * main and more section.
 * It provides a 'Configure menu' dialog to make the menu user-configurable.
 *
 * It does this in the following ways:
 * - Provide an API to define external applications for a given context.
 * - If a defined application is not installed (yet) the application is (optionally)
 *    still presented to the user with a hint that it is not installed and a link
 *    to the homepage (later with integration to package management).
 *    This increases the discoverability of useful applications the user never
 *    heard about yet.
 * - In case of many applications for a given context, it provides a GUI to the
 *    user to hand-pick favorite tools.
 *    This makes it easier for application developers to add alternative
 *    application/tool suggestions without worrying about cluttered menus.
 * - Menu items can be (automatically) moved to the "More" submenu.
 * - Reduce translation effort by re-using .desktop files of the services added
 *    to the menu.
 *
 *
 * Details
 * -------
 * The term "kmt-desktopfile" refers to a 1:1 copy of a .desktop file. The
 * kmt-desktopfile is provided by the application that uses KMoreTools
 * and must be installed to subdirectories of /usr/share/kf5/kmoretools/
 * - e.g. /usr/share/kf5/kmoretools/dolphin/statusbar-diskspace-menu/
 * - e.g. /usr/share/kf5/kmoretools/kate/addons/project/git-tools/
 * - generally, 'QStandardPaths::GenericDataLocation'/kf5/kmoretools/'uniqueId'
 *
 * See KMoreTools::KMoreTools for hints of how to install this correctly
 * using cmake.
 *
 * The kmt-desktopfiles are used to get ready-made translations for application
 * name and description even if the application is not installed. You can
 * also provide an icon which is used in the not-installed section when the
 * application is not installed yet.
 *
 * For details about the resulting menu structure, see KMoreToolsMenuBuilder.
 *
 * See also https://community.kde.org/Scratchpad/KMoreToolsFramework (outdated)
 *
 *
 * Rationale for the "Not installed" section
 * -----------------------------------------
 * - Increase discoverability and visibility of useful free software that have
 *   inherently low budget for marketing.
 * - Make interconnection of different free software packages as effortless as
 *   possible (in terms of creating and maintaining the menu).
 * - Provide expert (i.e. your) knowledge to useful free software alternatives
 *   to solve a certain task.
 * - Give novice users hints about tools that are useful in a particular
 *   context even if they are not installed.
 * - Improve self-documentation of applications.
 *
 *
 * Presets
 * -------
 * Before installing desktop files in your application you might take a look
 * at KMoreToolsPresets or KMoreToolsMenuFactory which might already contain
 * the needed tools.
 *
 *
 * Screenshots
 * -----------
 * This section shows screenshots of usage examples.
 *
 * ### KSnapshot's Send To... menu
 *
 * Last updated: 2015-04-17, uncommited demo, source code:
 * src/kde/kdegraphics/ksnapshot/ksnapshotsendtoactions.cpp
 *
 * Note, that the last item in the 'More' menu in the following screenshot was
 * added by KSnapshot's code.
 *
 * \image html kmoretools-ksnapshot-sendto-1.png "Send To menu" width=100px
 *
 * ### Dolphins's Space info menu
 *
 * Last updated: 2015-04-17, uncommited demo, source code: src/kde/applications/dolphin/src/statusbar/spaceinfotoolsmenu.cpp
 *
 * \image html kmoretools-dolphin-spaceinfo-1.png "Space info menu" width=100px
 *
 * ### Kate's Project plugin git menu
 *
 * Last updated: 2015-03-25, uncommited demo, source code:
 * src/kde/applications/kate/addons/project/kateprojecttreeviewcontextmenu.cpp
 *
 * \image html kmoretools-kate-project-1-all-installed.png "All git tools installed" width=100px
 *
 * \image html kmoretools-kate-project-2-two-not-installed.png "Not all git tools installed" width=100px
 *
 * \image html kmoretools-kate-project-3-config-dialog-all-installed.png "'Configure menu' dialog" width=100px
 *
 * ### Kate's Project plugin git menu
 *
 * Last updated: 2015-04-17, source code: src/frameworks/knewstuff/tests/kmoretools/kmoretoolstest.cpp
 *
 * \image html kmoretools-tests-configure-dialog-notinstalledapps.png "Configure dialog when there are non-installed apps" width=100px
 *
 *
 * FAQ
 * ---
 * ### Why is everything based on desktopfiles?
 *
 * - With desktopfiles translation can be reused.
 * - Definition of application icon can be reused.
 * - They provide a unified interface for dealing with program arguments.
 *
 *
 * todo later
 * ----------
 * - question: KMoreTools::registerServiceByDesktopEntryName():
 *      - warn if service is not of Type=Application (KService::isApplication()) or just leave it?
  * Add support for package managers to install software (e.g. muon discover)
 * - maybe: kmt-desktopfiles: add a config file that can configure the homepage URLs
 *   and e.g. the package name if needed for package manager support
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreTools
{
    friend class KMoreToolsService;
    friend class KMoreToolsServicePrivate;

public:
    /**
     * Specify how should be determined if a service is installed or not
     */
    enum ServiceLocatingMode
    {
        /**
         * by existence of desktop file (discoverable by KService)
         */
        ServiceLocatingMode_Default,

        /**
         * by existence of executable defined in the TryExec or Exec line of
         * the provided kmt-desktopfile
         */
        ServiceLocatingMode_ByProvidedExecLine
    };

    /**
     * Specify where a menu item be placed by default
     */
    enum MenuSection
    {
        /**
         * The item is placed in the main section (default)
         */
        MenuSection_Main,

        /**
         * The item is placed in the "More" submenu.
         */
        MenuSection_More
    };

//     /* *
//      * todo/later: introduce when needed
//      */
//     enum NotInstalledSectionOption
//     {
//         /* *
//          * default
//          */
//         NotInstalledSection_Show,
//
//         /* *
//          * Even if there are non-installed apps the Not-Installed section will
//          * not be shown
//          */
//         NotInstalledSection_Hide
//     };

    /**
     * Specify if the Configure dialog be accessible from the menu
     * (via a "Configure..." menu item)
     */
    enum ConfigureDialogAccessibleSetting
    {
        /**
         * Always show the "Configure..." menu item
         * (default)
         */
        ConfigureDialogAccessible_Always,

        /**
         * Defensively show the "Configure..." menu item
         *
         * The "Configure..." menu item will only be shown if there are non-installed
         * apps.
         * Rationale (suggestion): Do not clutter menu more than needed in standard
         *    cases. But when there are not-installed apps the configure dialog can
         *    be used to find out more about these apps.
         *
         * Note, that the "Configure..." menu item still becomes visible when the
         * user holds the Ctrl key while opening the menu.
         */
        ConfigureDialogAccessible_Defensive
    };

public:
    /**
     * @param uniqueId defines two things
     * 1) the config section name where the user settings done by the Configure
     *    dialog will be stored.
     * 2) the location where the kmt-desktopfiles should be installed because
     *    there they will be searched by default.
     *    If @p uniqueId contains slashes they will result in subdirectories.
     *    The default location can be overridden by
     *    registerServiceByDesktopEntryName's kmtDesktopfileSubdir parameter.
     *    This is currently used in KMoreToolsPresets implementation to
     *    separate the kmt-desktopfiles location from the user's config section
     *    name.
     *
     * Install Desktopfiles
     * --------------------
     * Example 1 (CMakeLists.txt if uniqueId = "dolphin/statusbar-diskspace-menu"):
     * \verbatim
        # note the trailing slash       ------------. (it makes sure only the contents of the directory is copied)
        #                                           |                                 ----fix---
        #                                           v                                            ------ uniqueId-----------------
        install(DIRECTORY statusbar/kmt-desktopfiles/ DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/dolphin/statusbar-diskspace-menu)
        \endverbatim

        Example 2:
        \verbatim
                                                                                       ------ uniqueId--------------
        install(DIRECTORY kmt-desktopfiles/ DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/kate/addons/project/git-tools)
        \endverbatim
     *
     * ### About ${KDE_INSTALL_DATADIR_KF5}
     *
     * In general, ${KDE_INSTALL_DATADIR_KF5}/kmoretools/hallo ends up in /usr/share/kf5/kmoretools/hallo.
     *
     * To use it, you need to add \verbatim include(KDEInstallDirs) \endverbatim to your CMakeLists.txt.
     */
    explicit KMoreTools(const QString& uniqueId);

    ~KMoreTools();

    KMoreTools(const KMoreTools &) = delete;
    KMoreTools& operator=(const KMoreTools &) = delete;

    /**
     * Registers a service with KMoreTools.
     *
     * If the method is called more than once for the same desktopEntryName
     * the service is located again and the old service is replaced with the
     * new one.
     *
     * @param desktopEntryName is the name of the desktopfile (without the
     * .desktop extension)
     * The desktop file is
     * 1. either already installed. Then the information of the installed file
     *    is used.
     * 2. or not installed and kmt-desktopfile is present. Then the information
     *     of the app-local copy of desktopfile located in the kmt-desktopfiles
     *     directory is used
     * 3. or not installed and no kmt-desktopfile provided. In this case
     *    KMoreToolsService::setHomepageUrl should be used so that at least a
     *    website link can be displayed.
     *
     * @param kmtDesktopfileSubdir when not empty overrides the @p uniqueId
     * parameter from the ctor when it comes to searching a kmt-desktopfile.
     * Default value is the empty string.
     *
     * @param serviceLocatingMode == ServiceLocatingMode_ByProvidedExecLine:
     *   Some programs don't install a desktop file of their own (e.g. gitk).
     *   If set to true then installed desktop files are not searched
     *   but the provided in kmt-desktopfiles will be used to extract exec line.
     *   The exec line will be used to determine if the executable is installed.
     *
     * @return a KMoreToolsService pointer which lives as long as KMoreTools, so
     *         do not store it for later use.
     * @return nullptr if the kmt provided desktop file is faulty.
     *         This kind of error must be fixed before you ship your application.
     *         This case is only used for unit tests.
     */
    KMoreToolsService* registerServiceByDesktopEntryName(
        const QString& desktopEntryName,
        const QString& kmtDesktopfileSubdir = QString(),
        ServiceLocatingMode serviceLocatingMode = ServiceLocatingMode_Default);

    /**
     * @returns the interface to build the menu. It is a singleton instance
     * for each different @p userConfigPostfix (which is "" by default).
     * So repeated calls with same parameter will return the same object.
     *
     * The pointer lives as long as KMoreTools.
     *
     * @param userConfigPostfix is empty by default. You can use it to specify
     * a postfix for the user config section. So you can build different menus
     * which can be configured separately. (This is used in unit tests to
     * separated test cases.)
     *
     * @sa KMoreToolsMenuBuilder::clear()
     */
    KMoreToolsMenuBuilder* menuBuilder(const QString& userConfigPostfix = QString()) const;

private:
    KMoreToolsPrivate* d;
};

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

class KMoreToolsServicePrivate;

/**
 * A service described in a .desktop file (kmt-desktopfile) which will be
 * called "registered service".
 *
 * A registered service can either be installed (isInstalled() == true)
 * or - if not found on the system - not installed (isInstalled() == false).
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreToolsService
{
    friend class KMoreTools;
    friend class KMoreToolsPrivate;

public:
    ~KMoreToolsService();

    KMoreToolsService(const KMoreToolsService &) = delete;
    KMoreToolsService& operator=(const KMoreToolsService &) = delete;

    /**
     * @return the desktop entry name which the service is identified by and with which
     * it was registered (see registerServiceByDesktopEntryName).
     *
     * Filename without .desktop: e.g. if the desktop file is named
     * "org.kde.ksnapshot.desktop" then the desktop entry name is
     * "org.kde.ksnapshot".
     */
    QString desktopEntryName() const;

    /**
     * @returns true if the desktopfile with the given
     * desktopname (name of the .desktop file without the .desktop)
     * is installed on the system
     */
    bool isInstalled() const;

    /**
     * @returns the KService represented by an installed desktop file.
     *
     * @note that this might be nullptr even if isInstalled() is true.
     * This can only happen when ServiceLocatingMode::ServiceLocatingMode_ByProvidedExecLine
     * is used in registerServiceByDesktopEntryName. (Then the kmt-desktopfile's
     * Exec line is used to determine if a program is installed)
     */
    KService::Ptr installedService() const;

    /**
     * @returns a non-null KService::Ptr if app-local kmt-desktopfile is
     * found and valid
     */
    KService::Ptr kmtProvidedService() const;

    /**
     * @return the icon provided by the KMoreTools' user and not the installed one.
     * (e.g. QGit currently has got a blank icon installed)
     */
    QIcon kmtProvidedIcon() const;

    /**
     * @see setHomepageUrl()
     */
    QUrl homepageUrl() const;

    /**
     * Sets the homepage url the user is shown when a service is not installed.
     * This way the user gets some information of how to install the
     * application.
     */
    void setHomepageUrl(const QUrl& url);

    /**
     * @see setMaxUrlArgCount()
     */
    int maxUrlArgCount() const;

    /**
     * In KMoreToolsMenuFactory some minor magic is done. In the context of
     * connecting the action trigger signal we need to know the maximum number
     * of URL arguments a given service can accept. Usually a number between
     * 0 and 1. Sometimes 2.
     * E.g. kdf must not be called with any positional argument.
     * E.g. gitg can be called with zero or one arguments.
     */
    void setMaxUrlArgCount(int maxUrlArgCount);

    /**
     * @param formatString supports the following placeholders:
     *
     * 1. $GenericName
     * 2. $Name
     * 3. $DesktopEntryName
     *
     * which are replaced by the corresponding desktop file entries.
     *
     * If a value for a placeholder is not available (or empty)
     * (e.g. if no desktop file is available (not installed or not provided
     * via kmt-desktopfiles)) then the next one is used until 3. is reached which
     * is always available. Example: the formatString is "$GenericName", but
     * the GenericName field is not available. So $Name is used. If this is
     * also not available, $DesktopEntryName is used.
     *
     * @sa KMoreToolsMenuItem::setInitialItemText
     * @sa KMoreToolsMenuBuilder::setInitialItemTextTemplate
     */
    QString formatString(const QString& formatString) const;

    /**
     * 1. Icon from installed desktop file
     * If 1. is not found not found then...
     * 2. icon from kmt desktop file (which is then searched in the kmt-desktopfiles
     *    directory, must have extension .svg or .png)
     * If 2. is not not found then...
     * 3. no icon
     */
    QIcon icon() const;

    /**
     * Will override the "Exec=" line of the service. Will only apply if the
     * service is installed.
     *
     * @see KService::setExec(...)
     */
    void setExec(const QString& exec);

    /**
     * Returns the associated appstream id that was previously set with setAppstreamId().
     * If no appstream id was set, an empty string is returned.
     *
     * @return The service's appstream id.
     *
     * @since 5.48
     */
    QString appstreamId() const;

    /**
     * Sets the appstream id of the service. This is used to create a
     * appstream url for installing the service via a software store
     * (e.g. Discover). For instance, the appstream id for filelight is
     * "org.kde.filelight.desktop".
     *
     * @param id the appstream id
     *
     * @since 5.48
     */
    void setAppstreamId(const QString&);

private:
    /**
     * @param kmtDesktopfileSubdir where to find kmt-desktopfiles
     * @param desktopEntryName name of the desktopfile without the .desktop extension
     * @param isInstalled true if desktop file is installed
     * @param installedService not nullptr if @p isInstalled is true
     * @param kmtDesktopfile not null if app-local kmt-desktopfile is found and valid
     */
    KMoreToolsService(const QString& kmtDesktopfileSubdir,
                      const QString& desktopEntryName,
                      bool isInstalled, KService::Ptr installedService,
                      KService::Ptr kmtDesktopfile);

    /**
     * No copy semantic => private and no implementation
     */
    KMoreToolsService(const KMoreTools&);

    KMoreToolsServicePrivate* d;
};

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

class KMoreToolsMenuItem;
class KMoreToolsMenuBuilderPrivate;

/**
 * Define how the default structure of the menu should look like.
 *
 * Depending on if the added service is installed or not a "Not installed" section
 * will be automatically added to the generated menu.
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreToolsMenuBuilder
{
    friend class KMoreToolsPrivate;
    friend class KMoreTools;
    friend class KMoreToolsTest;
    friend class KMoreToolsTest2;
    friend class KMoreToolsTestInteractive;

public:
    ~KMoreToolsMenuBuilder();

    KMoreToolsMenuBuilder(const KMoreToolsMenuBuilder &) = delete;
    KMoreToolsMenuBuilder& operator=(const KMoreToolsMenuBuilder &) = delete;

    /**
     * Affects addMenuItem() if called before it.
     *
     * see KMoreToolsService::formatString, see KMoreToolsMenuItem::setInitialItemText
     *
     * The default template text is "$GenericName".
     */
    void setInitialItemTextTemplate(const QString& templateText);

    /**
     * Adds a registered service (which can installed or not) to the menu.
     * If the service is not installed it will be shown in the "Not installed"
     * section.
     *
     * @param registeredService will be added to a the menu. A unique menu
     * itemId will be generated automatically from the desktopEntryName.
     * See also KMoreToolsMenuItem::id().
     *
     * @param defaultLocation is KMoreTools::MenuSection_Main by default.
     *
     * The registeredService->isInstalled() result will be respected. E.g. if the service
     * is not installed it will be placed in the "Not installed" section in the more
     * location of the menu even if @p defaultLocation was main location.
     *
     * See also KMoreToolsMenuItem ctor
     *
     * @sa KMoreToolsMenuItem::action()
     */
    KMoreToolsMenuItem* addMenuItem(KMoreToolsService* registeredService,
                                    KMoreTools::MenuSection defaultLocation = KMoreTools::MenuSection_Main);

    /**
     * Adds an action to the menu which is created and managed by the caller.
     *
     * @param action to be added to the menu.
     *
     * @param itemId is a unique (for this menu) id for the item. The itemId
     * _may_ be not unique. Then a unique id is generated automatically by
     * using some postfix. But it is better if you specify something sensible
     * because the itemId is used to find the items in the user config.
     * Otherwise the user config can be messed up if the order or number
     * of default menu items changes. NOTE, that the QAction::text is NOT
     * used to generate the unique id because the text is translated and
     * therefore not stable.
     *
     * @sa KMoreToolsMenuItem::action()
     */
    KMoreToolsMenuItem* addMenuItem(QAction* action, const QString& itemId,
                                    KMoreTools::MenuSection defaultLocation = KMoreTools::MenuSection_Main);

    /**
     * Clears all added menu items. This can be useful if the menuBuilder is reused more than once.
     *
     * @sa KMoreToolsService::menuBuilder
     */
    void clear();

    /**
     * Builds the actual menu and appends all items (main items,
     * more submenu with a potential "not installed" section) to the @p menu.
     *
     * @param menu the menu where the items should be appended to
     *
     * @param configureDialogAccessibleSetting determines when the
     * "Configure..." menu item should be added to the menu
     *
     * @param moreMenu if not nullptr then it will be set to the pointer to the
     * "More" menu in case it was created.
     * Otherwise the pointer will set to nullptr.
     * This can be used to add some custom items to the @p menu.
     */
    void buildByAppendingToMenu(
        QMenu* menu,
        KMoreTools::ConfigureDialogAccessibleSetting configureDialogAccessibleSetting
        = KMoreTools::ConfigureDialogAccessible_Always,
        QMenu** outMoreMenu = nullptr);

private:
    /**
     * for unit testing / get as debug string
     */
    QString menuStructureAsString(bool mergeWithUserConfig) const;

    /**
     * for unit testing
     */
    void showConfigDialog(const QString& title);

    /**
     * (needed because QMap needs a default ctor)
     */
    KMoreToolsMenuBuilder();

    /**
     * internal usage
     */
    KMoreToolsMenuBuilder(const QString& uniqueId, const QString& userConfigPostfix);

    /**
     * No copy semantic => private and no implementation
     */
    KMoreToolsMenuBuilder(const KMoreTools&);

    KMoreToolsMenuBuilderPrivate* d;
};

// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------

class KMoreToolsMenuItemPrivate;

/**
 * Represents a menu item of a service (application, tool or variant of the same
 * service with different parameters).
 *
 * The service might be installed or not.
 *
 * The corresponding QAction will be created for installed services.
 *
 * @note that for not-installed services action() returns nullptr.
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreToolsMenuItem
{
    friend class KMoreToolsMenuBuilderPrivate;
    friend class KMoreToolsMenuBuilder;

public:
    KMoreToolsMenuItem(const KMoreToolsMenuItem &) = delete;
    KMoreToolsMenuItem& operator=(const KMoreToolsMenuItem &) = delete;

    /**
     * Auto-generated unique id that tries to be as stable as possible even if the
     * menu gets restructured after the user did some customization that was
     * persisted in a config file.
     *
     * @note It is possible to add the same service more than once (and then
     * hopefully change the action text). When the order of those are changed,
     * the id will not be consistent (because internally an increasing number is used)
     * If you have issues with this you can solve this by manually
     * calling setId (e.g. 'desktopEntryName' + 'x').
     */
    QString id() const;

    /**
     * (Optional) to help with stable ids (see id())
     *
     * todo: make sure that if this is called, uniqueness of ids will be assured.
     * todo: make sure to show error if the id contains characters other than
     *       alphanumerica, dashes and underscores etc.
     */
    void setId(const QString& id);

    /**
     * @return the underlying KMoreToolsService instance,
     * see KMoreToolsMenuBuilder::addMenuItem (with KKmoreToolsService* argument).
     * Or nullptr when KMoreToolsMenuBuilder::addMenuItem (with QAction* argument
     * was used).
     */
    KMoreToolsService* registeredService() const;

    /**
     * see KMoreToolsMenuBuilder::addMenuItem
     */
    KMoreTools::MenuSection defaultLocation() const;

    /**
     * see setInitialItemText()
     */
    QString initialItemText() const;

    /**
     * Sets the initial text of a menu item.
     *
     * Menu items of a non-installed service will get this text.
     * If the service is installed and you would like to change the item text,
     * you can retrieve the created QAction (action())
     * and modify the text using QAction's methods (QAction::setText()).
     *
     * @see
     * - initialItemText()
     * - action()
     * - You can use the static method KMoreToolsService::formatString here.
     */
    void setInitialItemText(const QString& itemText);

    /**
     * Case 1
     * ------
     * KMoreToolsMenuBuilder::addMenuItem was called with KKmoreToolsService* argument.
     *
     * the corresponding QAction which will be added to the actual menu when
     * underlying service is installed or else - if not installed - nullptr.
     *
     * So you can change the created action as you desire.
     *
     * We return nullptr because not-installed services will get a submenu with
     * other items like opening a website instead of an single action.
     *
     * To change the item's text even for not-installed services use initialItemText()
     *
     * Note, that once the method was invoked the first time the action is created
     * an then reused.
     *
     * Case 2
     * ------
     * KMoreToolsMenuBuilder::addMenuItem was called with QAction* argument.
     * The added action will be returned.
     *
     * @see KMoreToolsService::isInstalled
     */
    QAction* action() const;

private: // internal usage
    /**
     * Sets the initial item text.
     */
    KMoreToolsMenuItem(KMoreToolsService* registeredService, KMoreTools::MenuSection defaultLocation, const QString& initialItemTextTemplate);

    KMoreToolsMenuItem(QAction* action, const QString& itemId, KMoreTools::MenuSection defaultLocation);

    ~KMoreToolsMenuItem();

private:
    KMoreToolsMenuItemPrivate* d;
};

#endif // KMORETOOLS_H
