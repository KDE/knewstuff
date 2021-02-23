/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knewstuff_export.h"
#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 79)
#include "downloadmanager.h"

#include <knewstuffcore_debug.h>

#include <QCoreApplication>

#include "engine.h"

namespace KNSCore
{
class DownloadManagerPrivate
{
public:
    DownloadManager *const q;
    Engine *const engine;

    DownloadManagerPrivate(DownloadManager *q)
        : q(q)
        , engine(new Engine)
    {
    }
    ~DownloadManagerPrivate()
    {
        delete engine;
    }

    bool isInitialized = false;
    bool checkForUpdates = false;
    bool checkForInstalled = false;
    bool doSearch = false;

    int page = 0;
    int pageSize = 100;

    void init(const QString &configFile);
};
}

using namespace KNSCore;

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , d(new DownloadManagerPrivate(this))
{
    QString name = QCoreApplication::applicationName();
    d->init(name + QStringLiteral(".knsrc"));
}

DownloadManager::DownloadManager(const QString &configFile, QObject *parent)
    : QObject(parent)
    , d(new DownloadManagerPrivate(this))
{
    d->init(configFile);
}

void DownloadManagerPrivate::init(const QString &configFile)
{
    q->connect(engine, &KNSCore::Engine::signalProvidersLoaded, q, &DownloadManager::slotProvidersLoaded);
    q->connect(engine, &KNSCore::Engine::signalUpdateableEntriesLoaded, q, &DownloadManager::searchResult);
    q->connect(engine, &KNSCore::Engine::signalEntriesLoaded, q, &DownloadManager::searchResult);
    q->connect(engine, &KNSCore::Engine::signalEntryEvent, q, [this](const EntryInternal &entry, EntryInternal::EntryEvent event) {
        if (event == EntryInternal::StatusChangedEvent) {
            q->entryStatusChanged(entry);
        }
    });
    q->connect(engine, &KNSCore::Engine::signalErrorCode, q, [this](const KNSCore::ErrorCode &, const QString &message, const QVariant &) {
        Q_EMIT q->errorFound(message);
    });
    engine->init(configFile);
}

DownloadManager::~DownloadManager()
{
    delete d;
}

void DownloadManager::slotProvidersLoaded()
{
    qCDebug(KNEWSTUFFCORE) << "providers loaded";
    d->isInitialized = true;
    if (d->checkForInstalled) {
        d->engine->checkForInstalled();
    } else if (d->checkForUpdates) {
        d->engine->checkForUpdates();
    } else if (d->doSearch) {
        d->engine->requestData(d->page, d->pageSize);
    }
}

void DownloadManager::checkForUpdates()
{
    if (d->isInitialized) {
        d->engine->checkForUpdates();
    } else {
        d->checkForUpdates = true;
    }
}

void DownloadManager::checkForInstalled()
{
    if (d->isInitialized) {
        d->engine->checkForInstalled();
    } else {
        d->checkForInstalled = true;
    }
}

void DownloadManager::installEntry(const EntryInternal &entry)
{
    if (entry.isValid()) {
        d->engine->install(entry);
    }
}

void DownloadManager::uninstallEntry(const EntryInternal &entry)
{
    if (entry.isValid()) {
        d->engine->uninstall(entry);
    }
}

void DownloadManager::search(int page, int pageSize)
{
    d->page = page;
    d->pageSize = pageSize;

    if (d->isInitialized) {
        d->engine->requestData(page, pageSize);
    } else {
        d->doSearch = true;
    }
}

void DownloadManager::setSearchOrder(DownloadManager::SortOrder order)
{
    switch (order) {
    case Newest:
        d->engine->setSortMode(KNSCore::Provider::Newest);
        break;
    case Rating:
        d->engine->setSortMode(KNSCore::Provider::Rating);
        break;
    case Alphabetical:
        d->engine->setSortMode(KNSCore::Provider::Alphabetical);
        break;
    case Downloads:
        d->engine->setSortMode(KNSCore::Provider::Downloads);
        break;
    }
}

void DownloadManager::setSearchTerm(const QString &searchTerm)
{
    d->engine->setSearchTerm(searchTerm);
}

void DownloadManager::fetchEntryById(const QString &id)
{
    d->engine->fetchEntryById(id);
}
#endif
