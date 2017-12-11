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

#include "engine.h"

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
    q->connect(engine, &KNSCore::Engine::signalEntryChanged, q, &DownloadManager::entryStatusChanged);
    q->connect(engine, &KNSCore::Engine::signalError, q, &DownloadManager::errorFound);
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

void DownloadManager::fetchEntryById(const QString& id)
{
    d->engine->fetchEntryById(id);
}
