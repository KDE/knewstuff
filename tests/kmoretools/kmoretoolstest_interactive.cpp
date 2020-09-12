/*
    SPDX-FileCopyrightText: 2014-2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <../src/kmoretools/kmoretools.h>
#include <../src/kmoretools/kmoretools_p.h>
#include <../src/kmoretools/kmoretoolspresets.h>
#include <../src/kmoretools/kmoretoolsmenufactory.h>

#include <QTest>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>

#define _ QLatin1String

/**
 * Each test case starts a test GUI.
 * Run kmoretoolstest_interactive with the test case name as first parameter
 * (e.g. testDialogForGroupingNames) to run only this test GUI.
 */
class KMoreToolsTestInteractive : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init();
    void cleanup();

    void testConfigDialogAllInstalled();
    void testConfigDialogSomeNotInstalled();
    void testConfigDialogNotInstalled1Service2Items();

    void test_buildMenu_WithQActions_interative1();

    void testDialogForGroupingNames();

    void testLazyMenu();

private:
    void testConfigDialogImpl(bool withNotInstalled, bool withMultipleItemsPerNotInstalledService, const QString& description);
};

void KMoreToolsTestInteractive::init()
{
}

void KMoreToolsTestInteractive::cleanup()
{
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

void KMoreToolsTestInteractive::test_buildMenu_WithQActions_interative1()
{
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
}

void KMoreToolsTestInteractive::testConfigDialogImpl(bool withNotInstalled, bool withMultipleItemsPerNotInstalledService, const QString& description)
{

    KMoreTools kmt(_("unittest-kmoretools/2"));
    const auto kateApp = kmt.registerServiceByDesktopEntryName(_("org.kde.kate"));
    const auto gitgApp = kmt.registerServiceByDesktopEntryName(_("gitg"));
    const auto notinstApp = kmt.registerServiceByDesktopEntryName(_("mynotinstalledapp"));
    const auto notinstApp2 = kmt.registerServiceByDesktopEntryName(_("mynotinstapp2"));
    notinstApp2->setHomepageUrl(QUrl(_("https://www.kde.org")));
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

void KMoreToolsTestInteractive::testConfigDialogAllInstalled()
{
    testConfigDialogImpl(false, false, _("TEST all installed"));
}

void KMoreToolsTestInteractive::testConfigDialogSomeNotInstalled()
{
    testConfigDialogImpl(true, false, _("TEST some not installed"));
}

void KMoreToolsTestInteractive::testConfigDialogNotInstalled1Service2Items()
{
    testConfigDialogImpl(true, true, _("TEST more than one item for one not installed service"));
}

void KMoreToolsTestInteractive::testDialogForGroupingNames()
{
    // show resulting menu
    auto dlg = new QDialog();
    auto labelInfo = new QLabel(_("First, select a URL (leave the URL box empty to give no URL; don't forget to add file:// or https://). Then, select a grouping name. => A menu will be created that you can try out. KDE4/KF5: If an application does not start even there is the launch indicator, try: $ eval `dbus-launch`"), dlg);
    labelInfo->setWordWrap(true);
    auto selectButton = new QPushButton(_("Select grouping name..."), dlg);
    auto labelLineEdit = new QLabel(_("URL 1 (file://..., https://...)"), dlg);
    auto urlLineEdit = new QLineEdit(dlg);
    urlLineEdit->setText(_("file:///etc/bash.bashrc"));
    auto menuButton = new QPushButton(_("<wait for selection>"), dlg);

    const auto groupingNamesList = {
        _("disk-usage"),
        _("disk-partitions"),
        _("files-find"),
        _("font-tools"),
        _("git-clients-for-folder"),
        _("git-clients-and-actions"),
        _("icon-browser"),
        _("language-dictionary"),
        _("mouse-tools"),
        _("screenrecorder"),
        _("screenshot-take"),
        _("system-monitor-processes"),
        _("system-monitor-logs"),
        _("time-countdown")
    };

    KMoreToolsMenuFactory menuFactory(_("unittest-kmoretools/3"));

    auto groupingNamesMenu = new QMenu(dlg);
    QMenu* moreToolsMenu = nullptr;
    for (auto groupingName : groupingNamesList) {
        auto action = new QAction(groupingName, groupingNamesMenu);
        action->setData(groupingName);
        groupingNamesMenu->addAction(action);

        QObject::connect(action, &QAction::triggered, action,
        [action, &menuFactory, &moreToolsMenu, urlLineEdit, menuButton]() {
            auto groupingName = action->data().toString();
            QUrl url;
            if (!urlLineEdit->text().isEmpty()) {
                url.setUrl(urlLineEdit->text());
            }
            moreToolsMenu = menuFactory.createMenuFromGroupingNames( { groupingName }, url);
            menuButton->setText(QString(_("menu for: '%1' (URL arg: %2)...")).arg(groupingName, url.isEmpty() ? _("<empty>") : _("<see URL 1>")));
            menuButton->setMenu(moreToolsMenu);
        });
    }

    selectButton->setMenu(groupingNamesMenu);

    auto hLayout = new QHBoxLayout();
    auto vLayout = new QVBoxLayout();
    hLayout->addWidget(labelInfo);
    hLayout->addLayout(vLayout);
    vLayout->addWidget(labelLineEdit);
    vLayout->addWidget(urlLineEdit);
    vLayout->addWidget(selectButton);
    vLayout->addWidget(menuButton);
    dlg->setLayout(hLayout);
    dlg->setBaseSize(300, 150);
    QObject::connect(dlg, &QDialog::finished, dlg, [dlg]() {
        qDebug () << "delete dlg;";
        delete dlg;
    });
    dlg->exec();
}

void KMoreToolsTestInteractive::testLazyMenu()
{
    KMoreToolsMenuFactory menuFactory(_("unittest-kmoretools/4"));

    auto moreToolsMenu = menuFactory.createMenuFromGroupingNames( { _("git-clients-for-folder") } );

    auto dlg = new QDialog();
    auto button = new QPushButton(_("Test the lazy menu"), dlg);
    button->setMenu(moreToolsMenu);
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

QTEST_MAIN(KMoreToolsTestInteractive)

#include "kmoretoolstest_interactive.moc"
