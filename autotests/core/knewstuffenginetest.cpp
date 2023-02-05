/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "engine.h"
#include "entry.h"

using namespace KNSCore;

class EngineTest : public QObject
{
    Q_OBJECT
public:
    Engine *engine = nullptr;
    const QString dataDir = QStringLiteral(DATA_DIR);

private Q_SLOTS:
    void initTestCase();
    void testPropertiesReading();
    void testProviderFileLoading();
};

void EngineTest::initTestCase()
{
    engine = new Engine(this);
    QVERIFY(engine->init(dataDir + "enginetest.knsrc"));
    QCOMPARE(engine->busyState(), Engine::BusyOperation::LoadingData);
    QSignalSpy providersLoaded(engine, &Engine::signalProvidersLoaded);
    QVERIFY(providersLoaded.wait());
    QCOMPARE(engine->busyState(), Engine::BusyState());
}

void EngineTest::testPropertiesReading()
{
    QCOMPARE(engine->name(), QStringLiteral("InstallCommands"));
    QCOMPARE(engine->categories(), QStringList({"KDE Wallpaper 1920x1200", "KDE Wallpaper 1600x1200"}));
    QCOMPARE(engine->useLabel(), QStringLiteral("UseLabelTest"));
    QVERIFY(engine->hasAdoptionCommand());
}

void EngineTest::testProviderFileLoading()
{
    const QString providerId = QUrl::fromLocalFile(dataDir + "entry.xml").toString();
    QSharedPointer<Provider> provider = engine->provider(providerId);
    QVERIFY(provider);
    QCOMPARE(engine->defaultProvider(), provider);

    KNSCore::Entry::List list;
    connect(
        engine,
        &Engine::signalEntriesLoaded,
        this,
        [&list](const KNSCore::Entry::List &loaded) {
            list = loaded;
        },
        Qt::DirectConnection);

    engine->setSearchTerm(QStringLiteral("Entry 4"));
    QSignalSpy spy(engine, &Engine::signalEntriesLoaded);
    QVERIFY(spy.wait());
    QCOMPARE(list.size(), 1);
    QCOMPARE(list.constFirst().name(), QStringLiteral("Entry 4 (ghns included)"));
}

QTEST_MAIN(EngineTest)

#include "knewstuffenginetest.moc"
