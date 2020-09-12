/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "khotnewstuff_test.h"

#include <KNSCore/Provider>
#include <KNSCore/EntryInternal>
#include <KNSCore/Engine>
#include "../src/staticxml/staticxmlprovider_p.h"

#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QFile>
#include <QXmlStreamReader>

KNewStuff2Test::KNewStuff2Test(const QString &configFile)
    : QObject()
{
    m_messages = new QStandardItemModel(this);
    m_configFile = configFile;
    m_engine = nullptr;
    m_testall = false;
}

void KNewStuff2Test::setTestAll(bool testall)
{
    m_testall = testall;
    Q_EMIT testAllChanged();
}

bool KNewStuff2Test::testAll() const
{
    return m_testall;
}

void KNewStuff2Test::entryTest()
{
    addMessage(QStringLiteral("-- test kns2 entry class"), QStringLiteral("msg_info"));

    QFile f(QString::fromLatin1("%1/testdata/entry.xml").arg(QStringLiteral(KNSSRCDIR)));
    if (!f.open(QIODevice::ReadOnly)) {
        addMessage(QString::fromUtf8("Error loading entry file: %1").arg(f.fileName()), QStringLiteral("msg_error"));
        return;
    }

    QXmlStreamReader reader(&f);
    KNSCore::EntryInternal e;
    reader.readNextStartElement(); // Skip the first (the external OCS container)
    bool xmlResult = reader.readNextStartElement() && e.setEntryXML(reader);
    e.setProviderId(QStringLiteral("test-provider"));

    f.close();
    if (!xmlResult) {
        addMessage(QString::fromUtf8("Error parsing entry file."), QStringLiteral("msg_error"));
        return;
    }

    addMessage(QString::fromUtf8("-- entry->xml test result: %1").arg(e.isValid()), e.isValid() ? QStringLiteral("msg_info") : QStringLiteral("msg_error"));
    if (!e.isValid()) {
        return;
    } else {
        QTextStream out(stdout);
        out << e.entryXML();
    }
}

void KNewStuff2Test::providerTest()
{
    addMessage(QString::fromUtf8("-- test kns2 provider class"), QStringLiteral("msg_info"));

    QDomDocument doc;
    QFile f(QString::fromLatin1("%1/testdata/provider.xml").arg(QStringLiteral(KNSSRCDIR)));
    if (!f.open(QIODevice::ReadOnly)) {
        addMessage(QString::fromUtf8("Error loading provider file: %1").arg(f.fileName()), QStringLiteral("msg_error"));
        return;
    }
    if (!doc.setContent(&f)) {
        addMessage(QString::fromUtf8("Error parsing provider file: %1").arg(f.fileName()), QStringLiteral("msg_error"));
        f.close();
        return;
    }
    f.close();

    KNSCore::StaticXmlProvider p;
    p.setProviderXML(doc.documentElement());

    addMessage(QString::fromUtf8("-- xml->provider test result: %1").arg(p.isInitialized()), p.isInitialized()? QStringLiteral("msg_info") : QStringLiteral("msg_error"));
}

void KNewStuff2Test::engineTest()
{
    addMessage(QString::fromUtf8("-- test kns2 engine"), QStringLiteral("msg_info"));

    m_engine = new KNSCore::Engine(this);

    connect(m_engine,
            &KNSCore::Engine::signalError,
            this, &KNewStuff2Test::slotEngineError);
    connect(m_engine,
            &KNSCore::Engine::signalProvidersLoaded,
            this, &KNewStuff2Test::slotProvidersLoaded);
    connect(m_engine,
            &KNSCore::Engine::signalEntriesLoaded,
            this, &KNewStuff2Test::slotEntriesLoaded);
    connect(m_engine,
            &KNSCore::Engine::signalEntryChanged,
            this, &KNewStuff2Test::slotInstallationFinished);

    bool ret = m_engine->init(m_configFile);

    addMessage(QString::fromUtf8("-- engine test result: %1").arg(ret), ret ? QStringLiteral("msg_info") : QStringLiteral("msg_error"));

    if (!ret) {
        addMessage(QString::fromUtf8("ACHTUNG: you probably need to 'make install' the knsrc file first. Although this is not required anymore, so something went really wrong."), QStringLiteral("msg_warning"));
    }
    addMessage(QString::fromUtf8("-- initial engine test completed"), QStringLiteral("msg_info"));
}

void KNewStuff2Test::slotProvidersLoaded()
{
    addMessage(QString::fromUtf8("SLOT: slotProvidersLoaded"), QStringLiteral("msg_info"));
//     qDebug() << "-- provider: " << provider->name().representation();

    m_engine->reloadEntries();
}

void KNewStuff2Test::slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    addMessage(QString::fromUtf8("SLOT: slotEntriesLoaded. Number of entries %1").arg(entries.count()), QStringLiteral("msg_info"));

    if (m_testall) {
        addMessage(QString::fromUtf8("-- now, download the entries' previews and payload files"), QStringLiteral("msg_info"));

        for (const KNSCore::EntryInternal &entry : entries) {
            addMessage(QString::fromUtf8("-- entry: %1").arg(entry.name()), QStringLiteral("msg_info"));
            if (!entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
                m_engine->loadPreview(entry, KNSCore::EntryInternal::PreviewSmall1);
            }
            if (!entry.payload().isEmpty()) {
                m_engine->install(entry);
            }
        }
    }
}

void KNewStuff2Test::slotInstallationFinished()
{
    addMessage(QString::fromUtf8("SLOT: slotInstallationFinished"));
}

void KNewStuff2Test::slotEngineError(const QString &error)
{
    addMessage(QString::fromUtf8("SLOT: slotEngineError %1").arg(error), QStringLiteral("msg_error"));
}

QObject *KNewStuff2Test::messages()
{
    return m_messages;
}

void KNewStuff2Test::addMessage(const QString &message, const QString &iconName)
{
    QStandardItem *item = new QStandardItem(message);
    item->setData(iconName, Qt::WhatsThisRole);
    m_messages->appendRow(item);
}

KNewStuff2Test *test = nullptr;
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler(nullptr);
void debugOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (test) {
        test->addMessage(msg, QStringLiteral("msg_info"));
    }
    // Call the default handler.
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->addOption(QCommandLineOption(QStringLiteral("testall"), i18n("Downloads all previews and payloads")));
    parser->addPositionalArgument(QStringLiteral("knsrcfile"), i18n("The KNSRC file you want to use for testing. If none is passed, we will use khotnewstuff_test.knsrc, which must be installed."));
    parser->process(app);

    if (parser->positionalArguments().count() > 0) {
        test = new KNewStuff2Test(parser->positionalArguments().first());
    } else {
        test = new KNewStuff2Test(QString::fromLatin1("%1/khotnewstuff_test.knsrc").arg(QStringLiteral(KNSBUILDDIR)));
    }
    test->setTestAll(parser->isSet(QStringLiteral("testall")));

    QQmlApplicationEngine *appengine = new QQmlApplicationEngine();
    appengine->rootContext()->setContextProperty(QStringLiteral("testObject"), test);
    appengine->load(QUrl::fromLocalFile(QString::fromLatin1("%1/khotnewstuff_test-ui/main.qml").arg(QStringLiteral(KNSSRCDIR))));

    // Don't really want to add messages until the tester
    // begins to actually request stuff in the UI,
    // so let's just install it here
    qInstallMessageHandler(debugOutputHandler);

    return app.exec();
}
