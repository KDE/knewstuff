/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "downloadmanager.h"

#include <QCoreApplication>
#include <knewstuff_debug.h>

#include "core/engine.h"
#include "entry_p.h"
#include "ui/widgetquestionlistener.h"

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 29)
namespace KNS3
{
class DownloadManagerPrivate
{
public:
    DownloadManager *const q;
    KNSCore::Engine *const engine;

    DownloadManagerPrivate(DownloadManager *q)
        : q(q)
        , engine(new KNSCore::Engine)
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
    void _k_slotProvidersLoaded();
    void _k_slotEngineError(const QString &error);
    void _k_slotUpdatesLoaded(const KNSCore::EntryInternal::List &entries);
    void _k_slotEntryStatusChanged(const KNSCore::EntryInternal &entry);
    void _k_slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
};
}

using namespace KNS3;

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
    q->connect(engine, SIGNAL(signalProvidersLoaded()), q, SLOT(_k_slotProvidersLoaded()));
    q->connect(engine, SIGNAL(signalUpdateableEntriesLoaded(KNSCore::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNSCore::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntriesLoaded(KNSCore::EntryInternal::List)), q, SLOT(_k_slotEntriesLoaded(KNSCore::EntryInternal::List)));
    q->connect(engine, SIGNAL(signalEntryChanged(KNSCore::EntryInternal)), q, SLOT(_k_slotEntryStatusChanged(KNSCore::EntryInternal)));
    q->connect(engine, SIGNAL(signalError(QString)), q, SLOT(_k_slotEngineError(QString)));
    engine->init(configFile);
    WidgetQuestionListener::instance();
}

DownloadManager::~DownloadManager()
{
    delete d;
}

void DownloadManagerPrivate::_k_slotEngineError(const QString &error)
{
    qCWarning(KNEWSTUFF) << "engine error" << error;

    Q_EMIT q->errorFound(error);
}

void DownloadManagerPrivate::_k_slotProvidersLoaded()
{
    qCDebug(KNEWSTUFF) << "providers loaded";
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

void KNS3::DownloadManager::checkForInstalled()
{
    if (d->isInitialized) {
        d->engine->checkForInstalled();
    } else {
        d->checkForInstalled = true;
    }
}

void DownloadManagerPrivate::_k_slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    KNS3::Entry::List result;
    result.reserve(entries.size());
    for (const KNSCore::EntryInternal &entry : entries) {
        result.append(EntryPrivate::fromInternal(&entry));
    }
    Q_EMIT q->searchResult(result);
}

void KNS3::DownloadManagerPrivate::_k_slotEntryStatusChanged(const KNSCore::EntryInternal &entry)
{
    Q_EMIT q->entryStatusChanged(EntryPrivate::fromInternal(&entry));
}

void DownloadManager::installEntry(const KNS3::Entry &entry)
{
    KNSCore::EntryInternal entryInternal = KNSCore::EntryInternal::fromEntry(entry);
    if (entryInternal.isValid()) {
        d->engine->install(entryInternal);
    }
}

void DownloadManager::uninstallEntry(const KNS3::Entry &entry)
{
    KNSCore::EntryInternal entryInternal = KNSCore::EntryInternal::fromEntry(entry);
    if (entryInternal.isValid()) {
        d->engine->uninstall(entryInternal);
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

void KNS3::DownloadManager::fetchEntryById(const QString &id)
{
    d->engine->fetchEntryById(id);
}

#include "moc_downloadmanager.cpp"
#endif
