/*
    This file is part of KNewStuff3
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KSharedConfig>
#include <QDir>
#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "core/entryinternal.h"
#include "core/installation.h"
#include "core/itemsmodel.h"
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
    void testInstallCommandArchive();
    void testInstallCommandTopLevelFilesInArchive();
    void testUninstallCommand();
    void testUninstallCommandDirectory();
};

void InstallationTest::initTestCase()
{
    // Just in case a previous test crashed
    cleanupTestCase();
    qRegisterMetaType<EntryInternal>();
    QStandardPaths::setTestModeEnabled(true);
    installation = new Installation(this);
    KConfigGroup grp = KSharedConfig::openConfig(dataDir + "installationtest.knsrc")->group("KNewStuff3");
    QVERIFY(installation->readConfig(grp));
    connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, this, [](KNSCore::Question *q) {
        q->setResponse(KNSCore::Question::YesResponse);
    });
}

void InstallationTest::cleanupTestCase()
{
    QFile::remove("installed.txt");
    QFile::remove("uninstalled.txt");
    const QString dataPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "demo/", QStandardPaths::LocateDirectory);
    if (!dataPath.isEmpty()) {
        QDir(dataPath).removeRecursively();
    }
}

void InstallationTest::testConfigFileReading()
{
    QCOMPARE(installation->uncompressionSetting(), Installation::UncompressIntoSubdirIfArchive);
    const QString actualPath = installation->targetInstallationPath();
    const QString expectedPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/demo/";
    QCOMPARE(actualPath, expectedPath);
}

void InstallationTest::testInstallCommand()
{
    EntryInternal entry;
    entry.setUniqueId("0");
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/testfile.txt")).toString());
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    installation->install(entry);
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

void InstallationTest::testUninstallCommandDirectory()
{
    static const QLatin1String testDir{"testDirectory"};

    // This will be left over from the previous test, so clean it up first
    QFile::remove("uninstalled.txt");

    EntryInternal entry;
    entry.setUniqueId("0");
    QDir().mkdir(testDir);
    entry.setStatus(KNS3::Entry::Installed);
    entry.setInstalledFiles(QStringList(testDir));
    QVERIFY(QFileInfo(testDir).exists());
    QVERIFY(!QFileInfo::exists("uninstalled.txt"));

    installation->uninstall(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());
    QCOMPARE(entry.status(), KNS3::Entry::Deleted);
    QVERIFY(!QFileInfo(testDir).exists());
    QVERIFY(QFileInfo::exists("uninstalled.txt"));
}

void InstallationTest::testInstallCommandArchive()
{
    EntryInternal entry;
    entry.setUniqueId("0");
    entry.setStatus(KNS3::Entry::Downloadable);
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/archive_dir.tar.gz")).toString());

    installation->install(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());

    QCOMPARE(entry.installedFiles().count(), 1);
    const QString file = entry.installedFiles().constFirst();
    const QFileInfo fileInfo(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "demo/data", QStandardPaths::LocateDirectory));
    QCOMPARE(file, fileInfo.absoluteFilePath() + "/*");
    QVERIFY(fileInfo.exists());
    QVERIFY(fileInfo.isDir());

    // Check if the files that are in the archive exist
    const QStringList files = QDir(fileInfo.absoluteFilePath()).entryList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(files, QStringList({"test1.txt", "test2.txt"}));
}

void InstallationTest::testInstallCommandTopLevelFilesInArchive()
{
    EntryInternal entry;
    entry.setUniqueId("0");
    entry.setStatus(KNS3::Entry::Downloadable);
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/archive_toplevel_files.tar.gz")).toString());

    installation->install(entry);
    QSignalSpy spy(installation, &Installation::signalEntryChanged);
    QVERIFY(spy.wait());

    QCOMPARE(entry.installedFiles().count(), 1);
    const QString file = entry.installedFiles().constFirst();
    QVERIFY(file.endsWith("/*"));

    // The file is given a random name, so we can't easily check that
    const QFileInfo fileOnDisk(file.left(file.size() - 2));
    QVERIFY(fileOnDisk.exists());
    QVERIFY(fileOnDisk.isDir());
    // The by checking the parent dir we can check if it is properly in a subdir uncompressed
    QCOMPARE(fileOnDisk.absoluteDir().path(), QStandardPaths::locate(QStandardPaths::GenericDataLocation, "demo", QStandardPaths::LocateDirectory));
    // Check if the files that are in the archive exist
    const QStringList files = QDir(fileOnDisk.absoluteFilePath()).entryList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(files, QStringList({"test1.txt", "test2.txt"}));
}

QTEST_MAIN(InstallationTest)

#include "installationtest.moc"
