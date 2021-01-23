/*
    This file is part of KNewStuff3
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include <QTest>
#include <QDir>
#include <QtGlobal>
#include <QSignalSpy>
#include <KSharedConfig>

#include "core/installation.h"
#include "core/itemsmodel.h"
#include "core/entryinternal.h"
#include "core/questionmanager.h"

using namespace KNSCore;

class InstallationTest : public QObject
{
Q_OBJECT
public:
    Installation *installation = nullptr;
    const QString dataDir = QStringLiteral(DATA_DIR);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testConfigFileReading();
    void testInstallCommand();
    void testUninstallCommand();
};

void InstallationTest::initTestCase()
{
    qRegisterMetaType<EntryInternal>();
    QStandardPaths::setTestModeEnabled(true);
    installation = new Installation();
    KConfigGroup grp = KSharedConfig::openConfig(dataDir + "installationtest.knsrc")->group("KNewStuff3");
    QVERIFY(installation->readConfig(grp));
    connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, this, [](KNSCore::Question* q) {
        q->setResponse(KNSCore::Question::YesResponse);
    });
}

void InstallationTest::cleanupTestCase()
{
    QFile::remove("installed.txt");
    QFile::remove("uninstalled.txt");
}

void InstallationTest::testConfigFileReading()
{
    QCOMPARE(installation->uncompressionSetting(), Installation::NeverUncompress);
    const QString actualPath = installation->targetInstallationPath();
    const QString expectedPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/demo/";
    QCOMPARE(actualPath, expectedPath);
}

void InstallationTest::testInstallCommand()
{
    EntryInternal entry;
    entry.setUniqueId("0");
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/testfile.txt")).toString());
    installation->install(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNS3::Entry::Installed);
    QVERIFY(QFileInfo::exists("installed.txt"));
}

void InstallationTest::testUninstallCommand()
{
    EntryInternal entry;
    entry.setUniqueId("0");
    QFile file("testFile.txt");
    file.open(QIODevice::WriteOnly);
    file.close();
    entry.setStatus(KNS3::Entry::Installed);
    entry.setInstalledFiles(QStringList(file.fileName()));
    QVERIFY(QFileInfo(file).exists());
    QVERIFY(!QFileInfo::exists("uninstalled.txt"));

    installation->uninstall(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNS3::Entry::Deleted);
    QVERIFY(!QFileInfo(file).exists());
    QVERIFY(QFileInfo::exists("uninstalled.txt"));
}

QTEST_MAIN(InstallationTest)

#include "installationtest.moc"
