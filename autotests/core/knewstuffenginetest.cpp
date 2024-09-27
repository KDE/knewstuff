/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>
#include <QtGlobal>

#include "enginebase.h"
#include "entry.h"
#include "provider.h"
#include "qtquick/quickengine.h"
#include "question.h"
#include "questionmanager.h"

using namespace KNSCore;

class EngineTest : public QObject
{
    Q_OBJECT
public:
    Engine *engine = nullptr;
    const QString dataDir = QStringLiteral(DATA_DIR);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testPropertiesReading();
    void testProviderFileLoading();
    void testInstallCommand();
    void testUninstallCommand();
};

void EngineTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    engine = new Engine(this);
    engine->setConfigFile(dataDir + QLatin1String("enginetest.knsrc"));
    QVERIFY(engine->isValid());
    QCOMPARE(engine->busyState(), Engine::BusyOperation::Initializing);
    QSignalSpy providersLoaded(engine, &Engine::signalProvidersLoaded);
    QVERIFY(providersLoaded.wait());
    QCOMPARE(engine->busyState(), Engine::BusyState());

    connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, this, [](KNSCore::Question *q) {
        q->setResponse(KNSCore::Question::YesResponse);
    });
}

void EngineTest::cleanupTestCase()
{
    const QString dataPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("icons/"), QStandardPaths::LocateDirectory);
    if (!dataPath.isEmpty()) {
        QDir(dataPath).removeRecursively();
    }
}

void EngineTest::testPropertiesReading()
{
    QCOMPARE(engine->name(), QStringLiteral("InstallCommands"));
    QCOMPARE(static_cast<EngineBase *>(engine)->categories(),
             QStringList({QStringLiteral("KDE Wallpaper 1920x1200"), QStringLiteral("KDE Wallpaper 1600x1200")}));
    QCOMPARE(engine->useLabel(), QStringLiteral("UseLabelTest"));
    QVERIFY(engine->hasAdoptionCommand());
    QCOMPARE(engine->contentWarningType(), EngineBase::ContentWarningType::Executables);
}

void EngineTest::testProviderFileLoading()
{
    const QString providerId = QUrl::fromLocalFile(dataDir + QLatin1String("entry.xml")).toString();
    QSharedPointer<Provider> provider = engine->provider(providerId);
    QVERIFY(provider);
    QCOMPARE(engine->defaultProvider(), provider);
    QVERIFY(engine->isValid());

    engine->setSearchTerm(QStringLiteral("Entry 4"));
    QSignalSpy spy(engine, &Engine::signalEntriesLoaded);
    QVERIFY(spy.wait());
    const QVariantList entries = spy.last().constFirst().toList(); // From last signal emission
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().value<KNSCore::Entry>().name(), QStringLiteral("Entry 4 (ghns included)"));
}

void EngineTest::testInstallCommand()
{
    const QString providerId = QUrl::fromLocalFile(dataDir + QLatin1String("entry.xml")).toString();
    Entry entry;
    entry.setProviderId(providerId);
    entry.setUniqueId(QStringLiteral("0"));
    entry.setName(QStringLiteral("testInstallCommand"));
    entry.setPayload(QUrl::fromLocalFile(QFINDTESTDATA("data/testfile.txt")).toString());

    QSignalSpy spy(engine, &Engine::signalEntryEvent);
    engine->install(entry);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(entry.status(), KNSCore::Entry::Installing);
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 2);
    QCOMPARE(entry.status(), KNSCore::Entry::Installed);
}

void EngineTest::testUninstallCommand()
{
    Entry entry;
    entry.setUniqueId(QStringLiteral("0"));
    QFile file(QStringLiteral("testFile.txt"));
    file.open(QIODevice::WriteOnly);
    file.close();
    entry.setStatus(KNSCore::Entry::Installed);
    entry.setInstalledFiles(QStringList(file.fileName()));
    QVERIFY(QFileInfo(file).exists());

    QSignalSpy spy(engine, &Engine::signalEntryEvent);
    engine->uninstall(entry);
    QVERIFY(spy.wait());
    // There are 3 signals: apparently one changes the status to Installing (transaction.cpp:375)
    // And two to "Deleted" (first installation.cpp:584 then transaction.cpp:383)
    QCOMPARE(spy.count(), 3);
    QCOMPARE(entry.status(), KNSCore::Entry::Deleted);
}

QTEST_MAIN(EngineTest)

#include "knewstuffenginetest.moc"
