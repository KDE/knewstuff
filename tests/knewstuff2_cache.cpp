/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knewstuff2_cache.h"

#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

#include <KStandardDirs>
#include <QApplication>
#include <QDebug>


#include <unistd.h> // for exit()

KNewStuff2Cache::KNewStuff2Cache()
    : QObject()
{
    m_engine = NULL;
}

void KNewStuff2Cache::run()
{
    // qCDebug(KNEWSTUFF) << "-- start the engine";

    m_engine = new KNS::CoreEngine(0);
    m_engine->setCachePolicy(KNS::CoreEngine::CacheOnly);
    bool ret = m_engine->init("knewstuff2_test.knsrc");

    // qCDebug(KNEWSTUFF) << "-- engine initialisation result: " << ret;

    if (ret) {
        connect(m_engine,
                &KNS::CoreEngine::signalEntryLoaded,
                this, &KNewStuff2Cache::slotEntryLoaded);
        connect(m_engine,
                &KNS::CoreEngine::signalEntriesFailed,
                this, &KNewStuff2Cache::slotEntriesFailed);
        connect(m_engine,
                &KNS::CoreEngine::signalEntriesFinished,
                this, &KNewStuff2Cache::slotEntriesFinished);

        m_engine->start();
    } else {
        qWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first.";
        qWarning() << "Although this is not required anymore, so something went really wrong.";
        quitTest();
    }
}

void KNewStuff2Cache::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    Q_UNUSED(feed);
    Q_UNUSED(provider);

    // qCDebug(KNEWSTUFF) << "SLOT: slotEntryLoaded";
    // qCDebug(KNEWSTUFF) << "-- entry: " << entry->name().representation();
}

void KNewStuff2Cache::slotEntriesFailed()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotEntriesFailed";
    quitTest();
}

void KNewStuff2Cache::slotEntriesFinished()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotEntriesFinished";
    quitTest();
}

void KNewStuff2Cache::quitTest()
{
    // qCDebug(KNEWSTUFF) << "-- quitting now...";
    if (1 == 0) {
        // this would be the soft way out...
        delete m_engine;
        deleteLater();
        qApp->quit();
    } else {
        exit(1);
    }
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName("knewstuff2_cache");
    QApplication app(argc, argv);

    // Take source directory into account
    // qCDebug(KNEWSTUFF) << "-- adding source directory " << KNSSRCDIR;
    // qCDebug(KNEWSTUFF) << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Cache *test = new KNewStuff2Cache();
    test->run();

    return app.exec();
}

