/*
 * Copyright 2014 2015 by Gregor Mi <codestruct@posteo.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <../src/kmoretools/kmoretools.h>
#include <../src/kmoretools/kmoretools_p.h>
#include <../src/kmoretools/kmoretoolspresets.h>

#include <QTest>
#include <QRegularExpression>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

#define _ QLatin1String

class KMoreToolsTest2 : public QObject
{
    Q_OBJECT

private Q_SLOTS: // todo: why does just "slots" not work here? (see http://qt-project.org/forums/viewthread/18432)
    void init();
    void cleanup();

    // corner, error cases and impl details:
    void testDesktopFileWithNoExec();
    void testDesktopFileWithInvalidHeader();
    void testDesktopFileWithNoName();
    void testDesktopFileNotProvided();
    void testDetectByExecLineButNoFileProvided();
    void testRegisterServiceTwice();
    void testMenuBuilderWithConfigPostfix();

    // use cases:
    void testInstalledAppStructure();
    void testInstalledAppSetInitialItemText();
    void testNotInstalledAppStructure();
    void testNotInstalledAppIcon();
    void testUniqueItemIdForOwnActions();

    void test_buildMenu_ShowConfigureMenuItem();
    void test_buildMenu_PruneDuplicateNotInstalledService();

    void test_KMoreToolsPresets_registerServicesByGrouping();


    // kmoretools_p.h tests:
    void testMenuItemIdGen();
    void test_MenuItemDto_removeMenuAmpersand();
    void test_MenuStructureDto_sortListBySection();
    void test_MenuStructureDto_serialize();
    void test_MenuStructureDto_deserialize();

    // GUI (manual / interactive):
    void testConfigDialogAllInstalled();
    void testConfigDialogSomeNotInstalled();
    void testConfigDialogNotInstalled1Service2Items();
    void test_buildMenu_WithQActions_interative1();

public:
    static const bool enableInteractiveTests = false; // default == false for non-interactive testing
};

void KMoreToolsTest2::init()
{
}

void KMoreToolsTest2::cleanup()
{
}

/**
 * no Exec line => not a usable desktop file
 */
void KMoreToolsTest2::testDesktopFileWithNoExec()
{
    KMoreTools kmt(QLatin1String(_("unittest-kmoretools/1")));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(_("The desktop entry file .+ has Type= \"Application\" but no Exec line")));
    // QTest::ignoreMessage(QtCriticalMsg, "KMoreTools::registerServiceByDesktopEntryName: the kmt-desktopfile .+ is provided but no Exec line is specified. The desktop file is probably faulty. Please fix. Return nullptr.");
    auto aApp = kmt.registerServiceByDesktopEntryName(_("a"));
    QVERIFY(!aApp);
}

/**
 * invalid header? => Exec line not found
 */
void KMoreToolsTest2::testDesktopFileWithInvalidHeader()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(_("The desktop entry file .+ has Type= \"Application\" but no Exec line")));
    auto bApp = kmt.registerServiceByDesktopEntryName(_("b"));
    QVERIFY(!bApp);
}

/**
 * no Name line => name() will be filled automatically, everything will be ok
 */
void KMoreToolsTest2::testDesktopFileWithNoName()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    auto cApp = kmt.registerServiceByDesktopEntryName(_("c"));
    QVERIFY(cApp);
    QCOMPARE(cApp->desktopEntryName(), QString(_("c")));
    QVERIFY(cApp->kmtProvidedService());
    QCOMPARE(cApp->kmtProvidedService()->exec(), QString(_("hallo")));
}

/**
 * desktop file not present => warning
 */
void KMoreToolsTest2::testDesktopFileNotProvided()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    //QTest::ignoreMessage(QtWarningMsg, "KMoreTools::registerServiceByDesktopEntryName: desktopEntryName .+ (kmtDesktopfileSubdir= .+ ) not provided (or at the wrong place) in the installed kmt-desktopfiles directory. If the service is also not installed on the system the user won't get nice translated app name and description.");
    auto eeeApp = kmt.registerServiceByDesktopEntryName(_("eee"));
    QVERIFY(eeeApp);
    QCOMPARE(eeeApp->desktopEntryName(), QString(_("eee")));
}

void KMoreToolsTest2::testDetectByExecLineButNoFileProvided()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    //QTest::ignoreMessage(QtCriticalMsg, "KMoreTools::registerServiceByDesktopEntryName: If detectServiceExistenceViaProvidedExecLine is true then a kmt-desktopfile must be provided. Please fix. Return nullptr.");
    auto eeeApp = kmt.registerServiceByDesktopEntryName(_("eee"), QString(), KMoreTools::ServiceLocatingMode_ByProvidedExecLine);
    QVERIFY(!eeeApp);
}

void KMoreToolsTest2::testRegisterServiceTwice()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    /*auto eeeApp1 = */kmt.registerServiceByDesktopEntryName(_("eee"));
    /*auto eeeApp2 = */kmt.registerServiceByDesktopEntryName(_("eee"));
    // todo: verify that there is only the last item in the internal service list
}

void KMoreToolsTest2::testMenuBuilderWithConfigPostfix()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    auto menuBuilder1 = kmt.menuBuilder();
    auto menuBuilder2 = kmt.menuBuilder();
    auto menuBuilder3 = kmt.menuBuilder(_("postfix"));
    auto menuBuilder4 = kmt.menuBuilder(_("postfix"));

    QVERIFY(menuBuilder1 == menuBuilder2);
    QVERIFY(menuBuilder3 != menuBuilder1);
    QVERIFY(menuBuilder3 == menuBuilder4);
}

/**
 * NOTE: we assume kate is installed
 */
void KMoreToolsTest2::testInstalledAppStructure()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto kateApp = kmt.registerServiceByDesktopEntryName(_("kate"));
    const auto menuBuilder = kmt.menuBuilder();
    menuBuilder->addMenuItem(kateApp);
    QString s = menuBuilder->menuStructureAsString(false);
    qDebug() << s;
    QCOMPARE(s, QString(_("|main|:kate.|more|:|notinstalled|:")));
}

/**
 * NOTE: we assume kate is installed
 * and that the translated Name is "Kate"
 */
void KMoreToolsTest2::testInstalledAppSetInitialItemText()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto kateApp = kmt.registerServiceByDesktopEntryName(_("kate"));
    const auto menuBuilder = kmt.menuBuilder();
    auto kateAppItem = menuBuilder->addMenuItem(kateApp);
    kateAppItem->setInitialItemText(kateApp->formatString(_("$Name in super-user mode")));
    auto action = kateAppItem->action();
    QVERIFY(action); // because kate is installed;
    QCOMPARE(action->text(), QString(_("Kate in super-user mode")));
}

void KMoreToolsTest2::testNotInstalledAppStructure()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    const auto menuBuilder = kmt.menuBuilder();
    menuBuilder->addMenuItem(mynotInstalledApp);
    QString s = menuBuilder->menuStructureAsString(false);
    qDebug() << s;
    QCOMPARE(s, QString(_("|main|:|more|:|notinstalled|:mynotinstalledapp.")));
}

void KMoreToolsTest2::testNotInstalledAppIcon()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    QVERIFY(!mynotInstalledApp->icon().isNull());
}

void KMoreToolsTest2::testUniqueItemIdForOwnActions()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    auto menuBuilder = kmt.menuBuilder();
    QAction a1(_("a"), nullptr);
    auto item1 = menuBuilder->addMenuItem(&a1, _("aaa"));
    QAction a2(_("b"), nullptr);
    auto item2 = menuBuilder->addMenuItem(&a2, _("aaa")); // same id to see if it will be made unique

    QCOMPARE(item1->id(), _("aaa0"));
    QCOMPARE(item2->id(), _("aaa1"));
}

bool menuAtLeastOneActionWithText(const QMenu* menu, const QString& text)
{
    Q_FOREACH(auto a, menu->actions())
    {
        if (a->text() == text) {
            return true;
        }
    }

    return false;
}

bool menuAtLeastNoActionWithText(const QMenu* menu, const QString& text)
{
    Q_FOREACH(auto a, menu->actions())
    {
        if (a->text() == text) {
            qDebug() << a->text();
            return false;
        }
    }

    return true;
}

/**
 * NOTE: we assume kate is installed
 */
void KMoreToolsTest2::test_buildMenu_ShowConfigureMenuItem()
{
    {
        KMoreTools kmt(_("unittest-kmoretools/2"));
        const auto kateApp = kmt.registerServiceByDesktopEntryName(_("kate"));
        // porcelain: other (interactive) tests will reuse the kmt id so we make sure that
        // no user configurment disburbs this test:
        const auto menuBuilder = kmt.menuBuilder(_("porcelain"));
        menuBuilder->addMenuItem(kateApp);
        QMenu menu;
        menuBuilder->buildByAppendingToMenu(&menu); // == KMoreTools::ConfigureDialogAccessible_Always
        qDebug() << menuBuilder->menuStructureAsString(true);
        QVERIFY(menuAtLeastOneActionWithText(&menu, _("Configure..."))); // "Kate", "Separator", "Configure..."

        {
            menu.clear();
            menuBuilder->buildByAppendingToMenu(&menu, KMoreTools::ConfigureDialogAccessible_Defensive);
            QVERIFY(menuAtLeastNoActionWithText(&menu, _("Configure...")));
        }
    }

    {
        KMoreTools kmt(_("unittest-kmoretools/2"));
        const auto kateApp = kmt.registerServiceByDesktopEntryName(_("kate"));
        const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
        const auto menuBuilder = kmt.menuBuilder(_("porcelain"));
        menuBuilder->addMenuItem(kateApp);
        menuBuilder->addMenuItem(mynotInstalledApp);
        QMenu menu;
        menuBuilder->buildByAppendingToMenu(&menu); // == KMoreTools::ConfigureDialogAccessible_Always

        auto doAssert = [](QMenu* menu) {
            QCOMPARE(menu->actions().count(), 3); // "Kate", "Separator", "More..."
            auto moreMenu = menu->actions()[2]->menu();
            QCOMPARE(moreMenu->actions().count(), 4); // "Not-installed-section", "Not installed app", "Separator", "Configure menu..."
            auto configureMenu = moreMenu->actions()[3];
            QCOMPARE(configureMenu->data().toString(), QString(_("configureItem")));
        };

        doAssert(&menu);

        {
            menu.clear();
            menuBuilder->buildByAppendingToMenu(&menu, KMoreTools::ConfigureDialogAccessible_Defensive);
            doAssert(&menu); // = same as _Always because there is one not-installed item
        }
    }
}

void KMoreToolsTest2::test_buildMenu_PruneDuplicateNotInstalledService()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    const auto menuBuilder = kmt.menuBuilder();
    menuBuilder->addMenuItem(mynotInstalledApp);
    menuBuilder->addMenuItem(mynotInstalledApp); // duplicate (which can make sense if the service will be called with different arguments)
    QMenu menu;
    menuBuilder->buildByAppendingToMenu(&menu);
    QCOMPARE(menu.actions().count(), 2);
    QVERIFY(menu.actions()[0]->isSeparator()); // might change later: currently, even if there are no installed applications the more menu will be generated
    QCOMPARE(menu.actions()[1]->text(), QString(_("More")));
    auto moreMenu = menu.actions()[1]->menu();
    QCOMPARE(moreMenu->actions().count(), 4); // "Not installed section", "Not installed app" (only once), "Separator", "Configure menu..."
}

void KMoreToolsTest2::test_KMoreToolsPresets_registerServicesByGrouping()
{
    KMoreTools kmt(_("unittest-kmoretools/3"));
    auto list = KMoreToolsPresets::registerServicesByGroupingNames(&kmt, { _("screenshot-take") });
    if (std::find_if(list.begin(), list.end(), [](KMoreToolsService* s) {
    return s->desktopEntryName() == _("org.kde.ksnapshot");
    }) != list.end()) {
        QVERIFY(true); // at least ksnapshot should currently be present
    }
    else {
        QVERIFY(false);
    }
}

void KMoreToolsTest2::test_buildMenu_WithQActions_interative1()
{
    if (!enableInteractiveTests) {
        QSKIP("enableInteractiveTests is false");
    }

    KMoreTools kmt(_("unittest-kmoretools/qactions")); // todo: disable copy-ctor!?

    auto f = [&kmt](QString title) { // NOTE: capture by reference! see https://en.wikipedia.org/wiki/Anonymous_function
        const auto menuBuilder = kmt.menuBuilder();
        menuBuilder->clear();
        QMenu menu;
        menuBuilder->addMenuItem(new QAction(_("Hallo 1"), &menu), _("id1"));
        menuBuilder->addMenuItem(new QAction(_("Hallo 2"), &menu), _("id2"));
        menuBuilder->addMenuItem(new QAction(_("Hallo 3"), &menu), _("id3"));

        menuBuilder->buildByAppendingToMenu(&menu);
        menuBuilder->showConfigDialog(title);
    };

    f(_("test_buildMenu_WithQActions 1"));
    //f(_("test_buildMenu_WithQActions 2"));
}

void KMoreToolsTest2::testMenuItemIdGen()
{
    KmtMenuItemIdGen idGen;
    QCOMPARE(idGen.getId(_("a")), QString(_("a0")));
    QCOMPARE(idGen.getId(_("a")), QString(_("a1")));
    QCOMPARE(idGen.getId(_("b")), QString(_("b0")));
    QCOMPARE(idGen.getId(_("a")), QString(_("a2")));
}

QDebug operator<< (QDebug d, const KmtMenuItemDto &m) {
    d << "id:" << m.id << ", section:" << m.menuSection << ", isInstalled:" << m.isInstalled;
    return d;
}

void KMoreToolsTest2::test_MenuItemDto_removeMenuAmpersand()
{
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&Hallo")), QString(_("Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("Hall&o")), QString(_("Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("H&all&o")), QString(_("Hallo"))); // is this ok for menus items?
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&&Hallo")), QString(_("&Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&Hall&&o")), QString(_("Hall&o")));
}

void KMoreToolsTest2::test_MenuStructureDto_sortListBySection()
{
    KmtMenuStructureDto mstruct;

    KmtMenuItemDto ma1;
    ma1.id = _("main1");
    ma1.menuSection = KMoreTools::MenuSection_Main;
    mstruct.list.append(ma1);

    KmtMenuItemDto mo1;
    mo1.id = _("more1");
    mo1.menuSection = KMoreTools::MenuSection_More;
    mstruct.list.append(mo1);

    KmtMenuItemDto ma3;
    ma3.id = _("main3_ni");
    ma3.menuSection = KMoreTools::MenuSection_Main;
    ma3.isInstalled = false; // !!!
    mstruct.list.append(ma3);

    KmtMenuItemDto mo2;
    mo2.id = _("more2");
    mo2.menuSection = KMoreTools::MenuSection_More;
    mstruct.list.append(mo2);

    KmtMenuItemDto ma2;
    ma2.id = _("main2");
    ma2.menuSection = KMoreTools::MenuSection_Main;
    mstruct.list.append(ma2);

    //qDebug() << mstruct.list;
    mstruct.stableSortListBySection();
    //qDebug() << mstruct.list;

    QCOMPARE(mstruct.list[0].id, QString(_("main1"))); // 1. main
    QCOMPARE(mstruct.list[1].id, QString(_("main2")));
    QCOMPARE(mstruct.list[2].id, QString(_("more1"))); // 2. more
    QCOMPARE(mstruct.list[3].id, QString(_("more2")));
    QCOMPARE(mstruct.list[4].id, QString(_("main3_ni"))); // 3. not installed
}

void KMoreToolsTest2::test_MenuStructureDto_serialize()
{
    KmtMenuStructureDto mstruct;

    KmtMenuItemDto ma1;
    ma1.id = _("main1");
    ma1.menuSection = KMoreTools::MenuSection_Main;
    mstruct.list.append(ma1);

    KmtMenuItemDto mo1;
    mo1.id = _("more1");
    mo1.menuSection = KMoreTools::MenuSection_More;
    mstruct.list.append(mo1);

    QString json = mstruct.serialize();
    QCOMPARE(json, QString(_("{\"menuitemlist\":[{\"id\":\"main1\",\"isInstalled\":true,\"menuSection\":\"main\"},{\"id\":\"more1\",\"isInstalled\":true,\"menuSection\":\"more\"}]}")));
}

void KMoreToolsTest2::test_MenuStructureDto_deserialize()
{
    QString jsonStr(_("{\"menuitemlist\":[{\"id\":\"main1\",\"isInstalled\":true,\"menuSection\":\"main\"},{\"id\":\"more1\",\"isInstalled\":true,\"menuSection\":\"more\"}]}"));
    KmtMenuStructureDto mstruct;
    mstruct.deserialize(jsonStr);
    QCOMPARE(mstruct.list.count(), 2);
    KmtMenuItemDto ma1 = mstruct.list[0];
    QCOMPARE(ma1.id, _("main1"));
    QCOMPARE(ma1.menuSection, KMoreTools::MenuSection_Main);
    QCOMPARE(ma1.isInstalled, true);
}

void testConfigDialogImpl(bool withNotInstalled, bool withMultipleItemsPerNotInstalledService, const QString& description)
{
    if (KMoreToolsTest2::enableInteractiveTests) {
        KMoreTools kmt(_("unittest-kmoretools/2"));
        const auto kateApp = kmt.registerServiceByDesktopEntryName(_("kate"));
        const auto gitgApp = kmt.registerServiceByDesktopEntryName(_("gitg"));
        const auto notinstApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
        const auto notinstApp2 = kmt.registerServiceByDesktopEntryName(_("mynotinstapp2"));
        notinstApp2->setHomepageUrl(QUrl(_("http://www.kde.org")));
        const auto menuBuilder = kmt.menuBuilder();
        menuBuilder->addMenuItem(kateApp);
        menuBuilder->addMenuItem(gitgApp);
        if (withNotInstalled) {
            auto item1 = menuBuilder->addMenuItem(notinstApp);
            item1->setInitialItemText(notinstApp->formatString(_("$Name - item 1")));

            menuBuilder->addMenuItem(notinstApp2);

            if (withMultipleItemsPerNotInstalledService) {
                auto item3 = menuBuilder->addMenuItem(notinstApp);
                item3->setInitialItemText(notinstApp->formatString(_("$Name - second item")));
            }
        }
        auto i1 = menuBuilder->addMenuItem(kateApp, KMoreTools::MenuSection_More);
        i1->setId(_("kate1"));
        i1->setInitialItemText(_("Kate more"));
        auto i2 = menuBuilder->addMenuItem(gitgApp, KMoreTools::MenuSection_More);
        i2->setId(_("gitg1"));
        i2->setInitialItemText(_("gitg more"));
        menuBuilder->showConfigDialog(description);

        // show resulting menu
        auto dlg = new QDialog();
        auto button = new QPushButton(_("Test the menu"), dlg);
        auto menu = new QMenu(dlg);
        menuBuilder->buildByAppendingToMenu(menu);
        button->setMenu(menu); // TODO: connect to the button click signal to always rebuild the menu
        auto label = new QLabel(_("Test the menu and hit Esc to exit if you are done. Note that changes made via the Configure dialog will have no immediate effect."), dlg);
        label->setWordWrap(true);
        auto layout = new QHBoxLayout();
        layout->addWidget(button);
        layout->addWidget(label);
        dlg->setLayout(layout);
        QObject::connect(dlg, &QDialog::finished, dlg, [dlg]() {
            qDebug () << "delete dlg;";
            delete dlg;
        });
        dlg->exec();
    }
}

void KMoreToolsTest2::testConfigDialogAllInstalled()
{
    if (!enableInteractiveTests) {
        QSKIP("enableInteractiveTests is false");
    }

    testConfigDialogImpl(false, false, _("TEST all installed"));
}

void KMoreToolsTest2::testConfigDialogSomeNotInstalled()
{
    if (!enableInteractiveTests) {
        QSKIP("enableInteractiveTests is false");
    }

    testConfigDialogImpl(true, false, _("TEST some not installed"));
}

void KMoreToolsTest2::testConfigDialogNotInstalled1Service2Items()
{
    if (!enableInteractiveTests) {
        QSKIP("enableInteractiveTests is false");
    }

    testConfigDialogImpl(true, true, _("TEST more than one item for one not installed service"));
}

QTEST_MAIN(KMoreToolsTest2)

#include "kmoretoolstest2.moc"
