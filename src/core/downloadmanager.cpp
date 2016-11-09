/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (C) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

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

#include "downloadmanager.h"

#include <knewstuffcore_debug.h>

#include <QCoreApplication>

#include "engine_p.h"
#include "entryinternal_p.h"

namespace KNSCore
{
class DownloadManagerPrivate
{
public:
    DownloadManager *q;
    Engine *engine;

    DownloadManagerPrivate(DownloadManager *q)
        : q(q)
        , engine(new Engine)
        , isInitialized(false)
        , checkForUpdates(false)
        , checkForInstalled(false)
        , doSearch(false)
        , page(0)
        , pageSize(100)
    {}
    ~DownloadManagerPrivate()
    {
        delete engine;
    }

    bool isInitialized;
    bool checkForUpdates;
    bool checkForInstalled;
    bool doSearch;

    int page;
    int pageSize;

    void init(const QString &configFile);
    void _k_slotProvidersLoaded();
    void _k_slotEngineError(const QString &error);
    void _k_slotUpdatesLoaded(const KNSCore::EntryInternal::List &entries);
    void _k_slotEntryStatusChanged(const KNSCore::EntryInternal &entry);
    void _k_slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
};
}

using namespace KNSCore;

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , d(new DownloadManagerPrivate(this))
{
    QString name = QCoreApplication::applicationName();
    d->init(name + ".knsrc");
}

DownloadManager::DownloadManager(const QString &configFile, QObject *parent)
    : QObject(parent)
    , d(new DownloadManagerPrivate(this))
{
    d->init(configFile);
}

void DownloadManagerPrivate::init(const QString &configFile)
{
    q->connect(engine, SIGNAL(signalProvidersLoaded()), q, SLOT(_k_slotProvidersLoaded()));
    q->connect(engine, SIGNAL(signalUpdateableEntriesLoaded(KNSCore::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNSCore::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntriesLoaded(KNSCore::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNSCore::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntryChanged(KNSCore::EntryInternal)), q, SLOT(_k_slotEntryStatusChanged(KNSCore::EntryInternal)));
    q->connect(engine, SIGNAL(signalError(QString)), q, SLOT(_k_slotEngineError(QString)));
    engine->init(configFile);
}

DownloadManager::~DownloadManager()
{
    delete d;
}

void DownloadManagerPrivate::_k_slotEngineError(const QString &error)
{
    qCWarning(KNEWSTUFFCORE) << "engine error" << error;

    Q_EMIT q->errorFound(error);
}

void DownloadManagerPrivate::_k_slotProvidersLoaded()
{
    qCDebug(KNEWSTUFFCORE) << "providers loaded";
    isInitialized = true;
    if (checkForInstalled) {
        engine->checkForInstalled();
    } else if (checkForUpdates) {
        engine->checkForUpdates();
    } else if (doSearch) {
        engine->requestData(page, pageSize);
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

void DownloadManagerPrivate::_k_slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    EntryInternal::List result;
    result.reserve(entries.size());
    foreach (const EntryInternal &entry, entries) {
        result.append(entry);
    }
    emit q->searchResult(result);
}

void DownloadManagerPrivate::_k_slotEntryStatusChanged(const KNSCore::EntryInternal &entry)
{
    emit q->entryStatusChanged(entry);
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

void DownloadManager::fetchEntryById(const QString& id)
{
    d->engine->fetchEntryById(id);
}

#include "moc_downloadmanager.cpp"
