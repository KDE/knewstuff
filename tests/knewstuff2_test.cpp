/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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

#include "knewstuff2_test.h"

#include <knewstuff2/core/ktranslatable.h>
#include <knewstuff2/core/providerhandler.h>
#include <knewstuff2/core/entryhandler.h>
#include <knewstuff2/core/coreengine.h>

#include <kstandarddirs.h>
#include <QApplication>
#include <QDebug>

#include <QFile>

#include <unistd.h> // for exit()
#include <stdio.h> // for stdout

KNewStuff2Test::KNewStuff2Test()
    : QObject()
{
    m_engine = NULL;
    m_testall = false;
}

void KNewStuff2Test::setTestAll(bool testall)
{
    m_testall = testall;
}

void KNewStuff2Test::entryTest()
{
    // qCDebug(KNEWSTUFF) << "-- test kns2 entry class";

    QDomDocument doc;
    QFile f(QString("%1/testdata/entry.xml").arg(KNSSRCDIR));
    if (!f.open(QIODevice::ReadOnly)) {
        // qCDebug(KNEWSTUFF) << "Error loading entry file.";
        quitTest();
    }
    if (!doc.setContent(&f)) {
        // qCDebug(KNEWSTUFF) << "Error parsing entry file.";
        f.close();
        quitTest();
    }
    f.close();

    KNS::EntryHandler eh(doc.documentElement());
    KNS::Entry e = eh.entry();

    // qCDebug(KNEWSTUFF) << "-- xml->entry test result: " << eh.isValid();

    KNS::EntryHandler eh2(e);
    QDomElement exml = eh2.entryXML();

    // qCDebug(KNEWSTUFF) << "-- entry->xml test result: " << eh.isValid();

    if (!eh.isValid()) {
        quitTest();
    } else {
        QTextStream out(stdout);
        out << exml;
    }
}

void KNewStuff2Test::providerTest()
{
    // qCDebug(KNEWSTUFF) << "-- test kns2 provider class";

    QDomDocument doc;
    QFile f(QString("%1/testdata/provider.xml").arg(KNSSRCDIR));
    if (!f.open(QIODevice::ReadOnly)) {
        // qCDebug(KNEWSTUFF) << "Error loading provider file.";
        quitTest();
    }
    if (!doc.setContent(&f)) {
        // qCDebug(KNEWSTUFF) << "Error parsing provider file.";
        f.close();
        quitTest();
    }
    f.close();

    KNS::ProviderHandler ph(doc.documentElement());
    KNS::Provider p = ph.provider();

    // qCDebug(KNEWSTUFF) << "-- xml->provider test result: " << ph.isValid();

    KNS::ProviderHandler ph2(p);
    QDomElement pxml = ph2.providerXML();

    // qCDebug(KNEWSTUFF) << "-- provider->xml test result: " << ph.isValid();

    if (!ph.isValid()) {
        quitTest();
    } else {
        QTextStream out(stdout);
        out << pxml;
    }
}

void KNewStuff2Test::engineTest()
{
    // qCDebug(KNEWSTUFF) << "-- test kns2 engine";

    m_engine = new KNS::CoreEngine(NULL);
    bool ret = m_engine->init("knewstuff2_test.knsrc");

    // qCDebug(KNEWSTUFF) << "-- engine test result: " << ret;

    if (ret) {
        connect(m_engine,
                &KNS::CoreEngine::signalProviderLoaded,
                this, &KNewStuff2Test::slotProviderLoaded);
        connect(m_engine,
                &KNS::CoreEngine::signalProvidersFailed,
                this, &KNewStuff2Test::slotProvidersFailed);
        connect(m_engine,
                &KNS::CoreEngine::signalEntryLoaded,
                this, &KNewStuff2Test::slotEntryLoaded);
        connect(m_engine,
                &KNS::CoreEngine::signalEntriesFinished,
                this, &KNewStuff2Test::slotEntriesFinished);
        connect(m_engine,
                &KNS::CoreEngine::signalEntriesFailed,
                this, &KNewStuff2Test::slotEntriesFailed);
        connect(m_engine,
                &KNS::CoreEngine::signalPayloadLoaded,
                this, &KNewStuff2Test::slotPayloadLoaded);
        connect(m_engine,
                &KNS::CoreEngine::signalPayloadFailed,
                this, &KNewStuff2Test::slotPayloadFailed);
        connect(m_engine,
                &KNS::CoreEngine::signalInstallationFinished,
                this, &KNewStuff2Test::slotInstallationFinished);
        connect(m_engine,
                &KNS::CoreEngine::signalInstallationFailed,
                this, &KNewStuff2Test::slotInstallationFailed);

        m_engine->start();
    } else {
        qWarning() << "ACHTUNG: you probably need to 'make install' the knsrc file first.";
        qWarning() << "Although this is not required anymore, so something went really wrong.";
        quitTest();
    }
}

void KNewStuff2Test::slotProviderLoaded(KNS::Provider *provider)
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotProviderLoaded";
    // qCDebug(KNEWSTUFF) << "-- provider: " << provider->name().representation();

    m_engine->loadEntries(provider);
}

void KNewStuff2Test::slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider)
{
    Q_UNUSED(feed);
    Q_UNUSED(provider);

    // qCDebug(KNEWSTUFF) << "SLOT: slotEntryLoaded";
    // qCDebug(KNEWSTUFF) << "-- entry: " << entry->name().representation();

    if (m_testall) {
        // qCDebug(KNEWSTUFF) << "-- now, download the entry's preview and payload file";

        if (!entry->preview().isEmpty()) {
            m_engine->downloadPreview(entry);
        }
        if (!entry->payload().isEmpty()) {
            m_engine->downloadPayload(entry);
        }
    }
}

void KNewStuff2Test::slotEntriesFinished()
{
    // Wait for installation if requested
    if (!m_testall) {
        quitTest();
    }
}

void KNewStuff2Test::slotPayloadLoaded(QUrl payload)
{
    // qCDebug(KNEWSTUFF) << "-- entry downloaded successfully";
    // qCDebug(KNEWSTUFF) << "-- downloaded to " << payload.prettyUrl();

    // qCDebug(KNEWSTUFF) << "-- run installation";

    bool ret = m_engine->install(payload.path());

    // qCDebug(KNEWSTUFF) << "-- installation result: " << ret;
    // qCDebug(KNEWSTUFF) << "-- now, wait for installation to finish...";
}

void KNewStuff2Test::slotPayloadFailed()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotPayloadFailed";
    quitTest();
}

void KNewStuff2Test::slotProvidersFailed()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotProvidersFailed";
    quitTest();
}

void KNewStuff2Test::slotEntriesFailed()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotEntriesFailed";
    quitTest();
}

void KNewStuff2Test::slotInstallationFinished()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotInstallationFinished";
    // qCDebug(KNEWSTUFF) << "-- OK, finish test";
    quitTest();
}

void KNewStuff2Test::slotInstallationFailed()
{
    // qCDebug(KNEWSTUFF) << "SLOT: slotInstallationFailed";
    quitTest();
}

void KNewStuff2Test::quitTest()
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
    //options.add("testall", qi18n("Downloads all previews and payloads"));

    QApplication app(argc, argv);

    // Take source directory into account
    // qCDebug(KNEWSTUFF) << "-- adding source directory " << KNSSRCDIR;
    // qCDebug(KNEWSTUFF) << "-- adding build directory " << KNSBUILDDIR;
    KGlobal::dirs()->addResourceDir("config", KNSSRCDIR);
    KGlobal::dirs()->addResourceDir("config", KNSBUILDDIR);

    KNewStuff2Test *test = new KNewStuff2Test();
    if (app.arguments().contains("--testall")) {
        test->setTestAll(true);
        test->entryTest();
        test->providerTest();
    }
    test->engineTest();

    return app.exec();
}

