/*
    SPDX-FileCopyrightText: 2014, 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <../src/kmoretools/kmoretools.h>
#include <../src/kmoretools/kmoretools_p.h>
#include <../src/kmoretools/kmoretoolspresets.h>

#include <QTest>
#include <QRegularExpression>
#include <QPushButton>

#define _ QLatin1String

class KMoreToolsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    // corner, error cases and impl details:
    void testDesktopFileWithNoExec();
    void testDesktopFileWithInvalidHeader();
    void testDesktopFileWithNoName();
    void testDesktopFileNotProvided();
    void testDetectByExecLineButNoFileProvided();
    void testRegisterServiceTwice();
    void testMenuBuilderWithConfigPostfix();

    // use cases:
    void testNotInstalledAppStructure();
    void testNotInstalledAppIcon();
    void testUniqueItemIdForOwnActions();
    void test_buildMenu_PruneDuplicateNotInstalledService();
    void test_KMoreToolsPresets_registerServicesByGrouping();

    // kmoretools_p.h tests:
    void testMenuItemIdGen();
    void test_MenuItemDto_removeMenuAmpersand();
    void test_MenuStructureDto_sortListBySection();
    void test_MenuStructureDto_serialize();
    void test_MenuStructureDto_deserialize();
    void test_KmtUrlUtil_localFileAbsoluteDir();
};

void KMoreToolsTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    const QString dest = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kf5/kmoretools/unittest-kmoretools/1/";
    QVERIFY(QDir(dest).removeRecursively());
    QVERIFY(QDir().mkpath(dest));
    for (const QString& fileName : {"a.desktop", "b.desktop", "c.desktop"}) {
        const QString srcFile = QFINDTESTDATA("1/" + fileName + ".notranslate");
        QVERIFY(!srcFile.isEmpty());
        QVERIFY(QFile::copy(srcFile, dest + fileName));
    }


    const QString dest2 = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kf5/kmoretools/unittest-kmoretools/2/";
    QVERIFY(QDir(dest2).removeRecursively());
    QVERIFY(QDir().mkpath(dest2));
    for (const QString& fileName : {"org.kde.kate.desktop", "org.kde.kate.png", "mynotinstalledapp.desktop", "mynotinstalledapp.png", "mynotinstapp2.desktop"}) {
        const QString origFile = fileName.endsWith(QLatin1String("desktop")) ? fileName + _(".notranslate") : fileName;
        const QString srcFile = QFINDTESTDATA("2/" + origFile);
        QVERIFY(!srcFile.isEmpty());
        QVERIFY(QFile::copy(srcFile, dest2 + fileName));
    }
}

/**
 * no Exec line => not a usable desktop file
 */
void KMoreToolsTest::testDesktopFileWithNoExec()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(_("The desktop entry file .+ has Type= \"Application\" but no Exec line")));
    auto aApp = kmt.registerServiceByDesktopEntryName(_("a"));
    QVERIFY(!aApp);
}

/**
 * invalid header? => Exec line not found
 */
void KMoreToolsTest::testDesktopFileWithInvalidHeader()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(_("The desktop entry file .+ has Type= \"Application\" but no Exec line")));
    auto bApp = kmt.registerServiceByDesktopEntryName(_("b"));
    QVERIFY(!bApp);
}

/**
 * no Name line => name() will be filled automatically, everything will be ok
 */
void KMoreToolsTest::testDesktopFileWithNoName()
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
void KMoreToolsTest::testDesktopFileNotProvided()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    auto eeeApp = kmt.registerServiceByDesktopEntryName(_("eee"));
    QVERIFY(eeeApp);
    QCOMPARE(eeeApp->desktopEntryName(), QString(_("eee")));
}

void KMoreToolsTest::testDetectByExecLineButNoFileProvided()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    auto eeeApp = kmt.registerServiceByDesktopEntryName(_("eee"), QString(), KMoreTools::ServiceLocatingMode_ByProvidedExecLine);
    QVERIFY(!eeeApp);
}

void KMoreToolsTest::testRegisterServiceTwice()
{
    KMoreTools kmt(_("unittest-kmoretools/1"));
    /*auto eeeApp1 = */kmt.registerServiceByDesktopEntryName(_("eee"));
    /*auto eeeApp2 = */kmt.registerServiceByDesktopEntryName(_("eee"));
    // todo: verify that there is only the last item in the internal service list
}

void KMoreToolsTest::testMenuBuilderWithConfigPostfix()
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

void KMoreToolsTest::testNotInstalledAppStructure()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    const auto menuBuilder = kmt.menuBuilder();
    menuBuilder->addMenuItem(mynotInstalledApp);
    QString s = menuBuilder->menuStructureAsString(false);
    qDebug() << s;
    QCOMPARE(s, QString(_("|main|:|more|:|notinstalled|:mynotinstalledapp.")));
}

void KMoreToolsTest::testNotInstalledAppIcon()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    QVERIFY(!mynotInstalledApp->icon().isNull());
}

void KMoreToolsTest::testUniqueItemIdForOwnActions()
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
    const auto lstActions = menu->actions();
    for (auto a : lstActions)
    {
        if (a->text() == text) {
            return true;
        }
    }

    return false;
}

bool menuAtLeastNoActionWithText(const QMenu* menu, const QString& text)
{
    const auto lstActions = menu->actions();
    for (auto a : lstActions)
    {
        if (a->text() == text) {
            qDebug() << a->text();
            return false;
        }
    }

    return true;
}

void KMoreToolsTest::test_buildMenu_PruneDuplicateNotInstalledService()
{
    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto mynotInstalledApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    const auto menuBuilder = kmt.menuBuilder();
    menuBuilder->addMenuItem(mynotInstalledApp);
    menuBuilder->addMenuItem(mynotInstalledApp); // duplicate (which can make sense if the service will be called with different arguments)
    QMenu menu;
    menuBuilder->buildByAppendingToMenu(&menu);
    QCOMPARE(menu.actions().count(), 4); // "Not installed section", "Not installed app" (only once), "Separator", "Configure menu..."
}

void KMoreToolsTest::test_KMoreToolsPresets_registerServicesByGrouping()
{
    KMoreTools kmt(_("unittest-kmoretools/3"));
    auto list = KMoreToolsPresets::registerServicesByGroupingNames(&kmt, { _("screenshot-take") });

    if (std::find_if(list.begin(), list.end(), [](KMoreToolsService* s) {
    return s->desktopEntryName() == _("org.kde.spectacle");
    }) != list.end()) {
        QVERIFY(true); // at least spectacle should currently be present
    }
    else {
        QVERIFY(false);
    }
}

void KMoreToolsTest::testMenuItemIdGen()
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

void KMoreToolsTest::test_MenuItemDto_removeMenuAmpersand()
{
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&Hallo")), QString(_("Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("Hall&o")), QString(_("Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("H&all&o")), QString(_("Hallo"))); // is this ok for menus items?
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&&Hallo")), QString(_("&Hallo")));
    QCOMPARE(KmtMenuItemDto::removeMenuAmpersand(_("&Hall&&o")), QString(_("Hall&o")));
}

// Support method for KMoreToolsTest::test_MenuStructureDto_sortListBySection,
// generates a list of menu items ordered by the passed-in indexes; this
// helps test multiple permutations of the same list. Don't try *all*
// permutations, since it's a stable sort -- items inside a section
// must remain in the same relative order.
//
static void sortListBySection(int indexes[5])
{
    KmtMenuStructureDto mstruct;

    KmtMenuItemDto ma1;
    ma1.id = QStringLiteral("main1");
    ma1.menuSection = KMoreTools::MenuSection_Main;

    KmtMenuItemDto mo1;
    mo1.id = QStringLiteral("more1");
    mo1.menuSection = KMoreTools::MenuSection_More;

    KmtMenuItemDto ma3;
    ma3.id = QStringLiteral("main3_ni");
    ma3.menuSection = KMoreTools::MenuSection_Main;
    ma3.isInstalled = false; // !!!

    KmtMenuItemDto mo2;
    mo2.id = QStringLiteral("more2");
    mo2.menuSection = KMoreTools::MenuSection_More;

    KmtMenuItemDto ma2;
    ma2.id = QStringLiteral("main2");
    ma2.menuSection = KMoreTools::MenuSection_Main;

    KmtMenuItemDto* items[5] = { &ma1, &mo1, &ma3, &mo2, &ma2 };

    mstruct.list.clear();
    for (unsigned int i=0; i<5; ++i)
    {
        mstruct.list.append(*items[indexes[i]]);
    }
    mstruct.stableSortListBySection();

    QCOMPARE(mstruct.list[0].id, QString(_("main1"))); // 1. main
    QCOMPARE(mstruct.list[1].id, QString(_("main2")));
    QCOMPARE(mstruct.list[2].id, QString(_("more1"))); // 2. more
    QCOMPARE(mstruct.list[3].id, QString(_("more2")));
    QCOMPARE(mstruct.list[4].id, QString(_("main3_ni"))); // 3. not installed
}

void KMoreToolsTest::test_MenuStructureDto_sortListBySection()
{
    int indexes_plain[5] = { 0, 1, 2, 3, 4 };  // In normal order
    int indexes_presorted[5] = { 0, 4, 1, 3, 2 };
    int indexes_interleave[5] = { 0, 1, 3, 4, 2 };
    int indexes_morefirst[5] = { 1, 3, 0, 4, 2 };
    int indexes_uninstalledfirst[5] = { 2, 1, 0, 4, 3 };
    // Permutations of where the uninstalled item is inserted
    int indexes_uninstalled_p0[5] = { 2, 0, 1, 3, 4 };
    int indexes_uninstalled_p1[5] = { 0, 2, 1, 3, 4 };
    int indexes_uninstalled_p2[5] = { 0, 1, 2, 3, 4 };
    int indexes_uninstalled_p3[5] = { 0, 1, 3, 2, 4 };
    int indexes_uninstalled_p4[5] = { 0, 1, 3, 4, 2 };


    qDebug() << "Plain";
    sortListBySection(indexes_plain);
    qDebug() << "Presorted";
    sortListBySection(indexes_presorted);
    qDebug() << "Interleaved";
    sortListBySection(indexes_interleave);
    qDebug() << "'More' first";
    sortListBySection(indexes_morefirst);
    qDebug() << "'Uninstalled' first";
    sortListBySection(indexes_uninstalledfirst);
    qDebug() << "'Uninstalled' first, p0";
    sortListBySection(indexes_uninstalled_p0);
    qDebug() << "'Uninstalled' first, p1";
    sortListBySection(indexes_uninstalled_p1);
    qDebug() << "'Uninstalled' first, p2";
    sortListBySection(indexes_uninstalled_p2);
    qDebug() << "'Uninstalled' first, p3";
    sortListBySection(indexes_uninstalled_p3);
    qDebug() << "'Uninstalled' first, p4";
    sortListBySection(indexes_uninstalled_p4);
}

void KMoreToolsTest::test_MenuStructureDto_serialize()
{
    KmtMenuStructureDto mstruct;

    KmtMenuItemDto ma1;
    ma1.id = QStringLiteral("main1");
    ma1.menuSection = KMoreTools::MenuSection_Main;
    mstruct.list.append(ma1);

    KmtMenuItemDto mo1;
    mo1.id = QStringLiteral("more1");
    mo1.menuSection = KMoreTools::MenuSection_More;
    mstruct.list.append(mo1);

    QString json = mstruct.serialize();
    QCOMPARE(json, QString(_("{\"menuitemlist\":[{\"id\":\"main1\",\"isInstalled\":true,\"menuSection\":\"main\"},{\"id\":\"more1\",\"isInstalled\":true,\"menuSection\":\"more\"}]}")));
}

void KMoreToolsTest::test_MenuStructureDto_deserialize()
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

void KMoreToolsTest::test_KmtUrlUtil_localFileAbsoluteDir()
{
    {
        auto urlIn = QUrl::fromLocalFile(QStringLiteral("/etc/bash.bashrc"));
        QCOMPARE(urlIn.toString(), QString(_("file:///etc/bash.bashrc")));

        auto urlOut = KmtUrlUtil::localFileAbsoluteDir(urlIn);
#ifdef Q_OS_WINDOWS
        QCOMPARE(urlOut.toString(), QString(_("file:///C:/etc")));
#else
        QCOMPARE(urlOut.toString(), QString(_("file:///etc")));
#endif
    }

    {
        auto urlIn = QUrl::fromLocalFile(QStringLiteral("/home/u1/dev/kf5/src/kde/applications/dolphin/.reviewboardrc"));
        QCOMPARE(urlIn.toString(), QString(_("file:///home/u1/dev/kf5/src/kde/applications/dolphin/.reviewboardrc")));

        auto urlOut = KmtUrlUtil::localFileAbsoluteDir(urlIn);
#ifdef Q_OS_WINDOWS
        QCOMPARE(urlOut.toString(), QString(_("file:///C:/home/u1/dev/kf5/src/kde/applications/dolphin")));
#else
        QCOMPARE(urlOut.toString(), QString(_("file:///home/u1/dev/kf5/src/kde/applications/dolphin")));
#endif
    }

    {
        auto urlIn2 = QUrl::fromLocalFile(QStringLiteral("aaa/bbb"));
        QCOMPARE(urlIn2.toString(), QString(_("file:aaa/bbb")));
    }
}

QTEST_MAIN(KMoreToolsTest)

#include "kmoretoolstest.moc"

