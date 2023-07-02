/*
    knewstuff3/engine.cpp
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "engine.h"

#include "cache.h"
#include "commentsmodel.h"
#include "enginebase_p.h"
#include "imageloader_p.h"
#include "installation_p.h"
#include "question.h"
#include "transaction.h"
#include "xmlloader_p.h"

#include <KConfig>
#include <KConfigGroup>
#include <KFileUtils>
#include <KFormat>
#include <KLocalizedString>
#include <KShell>
#include <QDesktopServices>
#include <knewstuffcore_debug.h>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QThreadStorage>
#include <QTimer>
#include <QUrlQuery>
#include <qdom.h>

#if defined(Q_OS_WIN)
#include <shlobj.h>
#include <windows.h>
#endif

// libattica
#include <qstandardpaths.h>

// own
#include "../attica/atticaprovider_p.h"
#include "../staticxml/staticxmlprovider_p.h"
#ifdef SYNDICATION_FOUND
#include "../opds/opdsprovider_p.h"
#endif

using namespace KNSCore;

class EnginePrivate
{
public:
    QMap<Entry, CommentsModel *> commentsModels;
    KNSCore::Provider::SearchRequest storedRequest;

    Engine::BusyState busyState;
    QString busyMessage;
    QString configFileName;

    QTimer *searchTimer;

    // the current request from providers
    Provider::SearchRequest currentRequest;

    // the page that is currently displayed, so it is not requested repeatedly
    int currentPage = -1;

    // when requesting entries from a provider, how many to ask for
    int pageSize = 20;

    int numDataJobs = 0;
    int numPictureJobs = 0;
    int numInstallJobs = 0;
    // If the provider is ready to be used
    bool initialized = false;
};

Engine::Engine(QObject *parent)
    : EngineBase(parent)
    , d(new EnginePrivate)
{
    setBusy(BusyOperation::Initializing, i18n("Initializing"));

    d->searchTimer = new QTimer(this);
    d->searchTimer->setSingleShot(true);
    d->searchTimer->setInterval(1000);
    connect(d->searchTimer, &QTimer::timeout, this, &Engine::slotSearchTimerExpired);
    connect(EngineBase::d->installation, &Installation::signalInstallationFinished, this, &Engine::slotInstallationFinished);
    connect(EngineBase::d->installation, &Installation::signalInstallationFailed, this, &Engine::slotInstallationFailed);
    connect(this, &EngineBase::signalProvidersLoaded, this, &Engine::updateStatus);
    connect(this, &EngineBase::loadingProvider, this, [this] {
        setBusy(BusyOperation::LoadingData, i18n("Loading provider information"));
    });
}

Engine::~Engine() = default;

QStringList Engine::categoriesFilter() const
{
    return d->currentRequest.categories;
}

void Engine::providerJobStarted(KJob *job)
{
    Q_EMIT jobStarted(job, i18n("Loading data from provider"));
}

void Engine::slotEntriesLoaded(const KNSCore::Provider::SearchRequest &request, KNSCore::Entry::List entries)
{
    d->currentPage = qMax<int>(request.page, d->currentPage);
    qCDebug(KNEWSTUFFCORE) << "loaded page " << request.page << "current page" << d->currentPage << "count:" << entries.count();

    if (request.filter == Provider::Updates) {
        Q_EMIT signalUpdateableEntriesLoaded(entries);
    } else {
        cache()->insertRequest(request, entries);
        Q_EMIT signalEntriesLoaded(entries);
    }

    --d->numDataJobs;
    updateStatus();
}

void Engine::reloadEntries()
{
    Q_EMIT signalResetView();
    d->currentPage = -1;
    d->currentRequest.pageSize = d->pageSize;
    d->currentRequest.page = 0;
    d->numDataJobs = 0;

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(EngineBase::d->providers)) {
        if (p->isInitialized()) {
            if (d->currentRequest.filter == Provider::Installed) {
                // when asking for installed entries, never use the cache
                p->loadEntries(d->currentRequest);
            } else {
                // take entries from cache until there are no more
                Entry::List cacheEntries;
                Entry::List lastCache = cache()->requestFromCache(d->currentRequest);
                while (!lastCache.isEmpty()) {
                    qCDebug(KNEWSTUFFCORE) << "From cache";
                    cacheEntries << lastCache;

                    d->currentPage = d->currentRequest.page;
                    ++d->currentRequest.page;
                    lastCache = cache()->requestFromCache(d->currentRequest);
                }

                // Since the cache has no more pages, reset the request's page
                if (d->currentPage >= 0) {
                    d->currentRequest.page = d->currentPage;
                }

                if (!cacheEntries.isEmpty()) {
                    Q_EMIT signalEntriesLoaded(cacheEntries);
                } else {
                    qCDebug(KNEWSTUFFCORE) << "From provider";
                    p->loadEntries(d->currentRequest);

                    ++d->numDataJobs;
                    updateStatus();
                }
            }
        }
    }
}

void Engine::setCategoriesFilter(const QStringList &categories)
{
    d->currentRequest.categories = categories;
    reloadEntries();
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (d->currentRequest.sortMode != mode) {
        d->currentRequest.page = -1;
    }
    d->currentRequest.sortMode = mode;
    reloadEntries();
}

Provider::SortMode KNSCore::Engine::sortMode() const
{
    return d->currentRequest.sortMode;
}

void KNSCore::Engine::setFilter(Provider::Filter filter)
{
    if (d->currentRequest.filter != filter) {
        d->currentRequest.page = -1;
    }
    d->currentRequest.filter = filter;
    reloadEntries();
}

Provider::Filter KNSCore::Engine::filter() const
{
    return d->currentRequest.filter;
}

void KNSCore::Engine::fetchEntryById(const QString &id)
{
    d->searchTimer->stop();
    d->currentRequest = KNSCore::Provider::SearchRequest(KNSCore::Provider::Newest, KNSCore::Provider::ExactEntryId, id);
    d->currentRequest.pageSize = d->pageSize;

    Entry::List cacheEntries = cache()->requestFromCache(d->currentRequest);
    if (!cacheEntries.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer->start();
    }
}

void KNSCore::Engine::restoreSearch()
{
    d->searchTimer->stop();
    d->currentRequest = d->storedRequest;
    if (cache()) {
        Entry::List cacheEntries = cache()->requestFromCache(d->currentRequest);
        if (!cacheEntries.isEmpty()) {
            reloadEntries();
        } else {
            d->searchTimer->start();
        }
    } else {
        qCWarning(KNEWSTUFFCORE) << "Attempted to call restoreSearch() without a correctly initialized engine. You will likely get unexpected behaviour.";
    }
}

void KNSCore::Engine::storeSearch()
{
    d->storedRequest = d->currentRequest;
}

void Engine::setSearchTerm(const QString &searchString)
{
    d->searchTimer->stop();
    d->currentRequest.searchTerm = searchString;
    Entry::List cacheEntries = cache()->requestFromCache(d->currentRequest);
    if (!cacheEntries.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer->start();
    }
}

QString KNSCore::Engine::searchTerm() const
{
    return d->currentRequest.searchTerm;
}

void Engine::slotSearchTimerExpired()
{
    reloadEntries();
}

void Engine::requestMoreData()
{
    qCDebug(KNEWSTUFFCORE) << "Get more data! current page: " << d->currentPage << " requested: " << d->currentRequest.page;

    if (d->currentPage < d->currentRequest.page) {
        return;
    }

    d->currentRequest.page++;
    doRequest();
}

void Engine::requestData(int page, int pageSize)
{
    d->currentRequest.page = page;
    d->currentRequest.pageSize = pageSize;
    doRequest();
}

void Engine::doRequest()
{
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(EngineBase::d->providers)) {
        if (p->isInitialized()) {
            p->loadEntries(d->currentRequest);
            ++d->numDataJobs;
            updateStatus();
        }
    }
}

void Engine::slotInstallationFinished()
{
    --d->numInstallJobs;
    updateStatus();
}

void Engine::slotInstallationFailed(const QString &message)
{
    --d->numInstallJobs;
    Q_EMIT signalErrorCode(KNSCore::InstallationError, message, QVariant());
}

void Engine::slotEntryDetailsLoaded(const KNSCore::Entry &entry)
{
    --d->numDataJobs;
    updateStatus();
    Q_EMIT signalEntryEvent(entry, Entry::DetailsLoadedEvent);
}

void Engine::loadDetails(const KNSCore::Entry &entry)
{
    QSharedPointer<Provider> p = EngineBase::d->providers.value(entry.providerId());
    p->loadEntryDetails(entry);
}

void Engine::loadPreview(const KNSCore::Entry &entry, Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "START  preview: " << entry.name() << type;
    ImageLoader *l = new ImageLoader(entry, type, this);
    connect(l, &ImageLoader::signalPreviewLoaded, this, &Engine::slotPreviewLoaded);
    connect(l, &ImageLoader::signalError, this, [this](const KNSCore::Entry &entry, Entry::PreviewType type, const QString &errorText) {
        Q_EMIT signalErrorCode(KNSCore::ImageError, errorText, QVariantList() << entry.name() << type);
        qCDebug(KNEWSTUFFCORE) << "ERROR preview: " << errorText << entry.name() << type;
        --d->numPictureJobs;
        updateStatus();
    });
    l->start();
    ++d->numPictureJobs;
    updateStatus();
}

void Engine::slotPreviewLoaded(const KNSCore::Entry &entry, Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "FINISH preview: " << entry.name() << type;
    Q_EMIT signalEntryPreviewLoaded(entry, type);
    --d->numPictureJobs;
    updateStatus();
}

// TODO Belongs to the Entry class
void Engine::contactAuthor(const Entry &entry)
{
    if (!entry.author().email().isEmpty()) {
        // invoke mail with the address of the author
        QUrl mailUrl;
        mailUrl.setScheme(QStringLiteral("mailto"));
        mailUrl.setPath(entry.author().email());
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("subject"), i18n("Re: %1", entry.name()));
        mailUrl.setQuery(query);
        QDesktopServices::openUrl(mailUrl);
    } else if (!entry.author().homepage().isEmpty()) {
        QDesktopServices::openUrl(QUrl(entry.author().homepage()));
    }
}

void Engine::slotEntryChanged(const Entry &entry)
{
    Q_EMIT signalEntryEvent(entry, Entry::StatusChangedEvent);
}

bool Engine::init(const QString &configfile)
{
    const bool valid = EngineBase::init(configfile);
    if (valid) {
        connect(this, &Engine::signalEntryEvent, EngineBase::d->cache.data(), [this](const Entry &entry, Entry::EntryEvent event) {
            if (event == Entry::StatusChangedEvent) {
                EngineBase::d->cache->registerChangedEntry(entry);
            }
        });
        connect(EngineBase::d->installation, &Installation::signalEntryChanged, this, &Engine::slotEntryChanged);
        connect(EngineBase::d->cache.data(), &Cache::entryChanged, this, &Engine::slotEntryChanged);
    }
    return valid;
}

void Engine::updateStatus()
{
    BusyState state;
    QString busyMessage;
    if (d->numInstallJobs > 0) {
        busyMessage = i18n("Installing");
        state |= BusyOperation::InstallingEntry;
    }
    if (d->numPictureJobs > 0) {
        busyMessage = i18np("Loading one preview", "Loading %1 previews", d->numPictureJobs);
        state |= BusyOperation::LoadingPreview;
    }
    if (d->numDataJobs > 0) {
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingPreview;
    }
    setBusy(state, busyMessage);
}

void Engine::checkForUpdates()
{
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(EngineBase::d->providers)) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Updates);
        p->loadEntries(request);
    }
}

void KNSCore::Engine::checkForInstalled()
{
    Entry::List entries = cache()->registry();
    std::remove_if(entries.begin(), entries.end(), [](const auto &entry) {
        return entry.status() != KNSCore::Entry::Installed && entry.status() != KNSCore::Entry::Updateable;
    });
    Q_EMIT signalEntriesLoaded(entries);
}

void KNSCore::Engine::setPageSize(int pageSize)
{
    d->pageSize = pageSize;
}

int KNSCore::Engine::pageSize() const
{
    return d->pageSize;
}

KNSCore::CommentsModel *KNSCore::Engine::commentsForEntry(const KNSCore::Entry &entry)
{
    CommentsModel *model = d->commentsModels[entry];
    if (!model) {
        model = new CommentsModel(this);
        model->setEntry(entry);
        connect(model, &QObject::destroyed, this, [=]() {
            d->commentsModels.remove(entry);
        });
        d->commentsModels[entry] = model;
    }
    return model;
}

QString Engine::busyMessage() const
{
    return d->busyMessage;
}

void Engine::setBusyMessage(const QString &busyMessage)
{
    if (busyMessage != d->busyMessage) {
        d->busyMessage = busyMessage;
        Q_EMIT busyMessageChanged();
    }
}

Engine::BusyState Engine::busyState() const
{
    return d->busyState;
}

void Engine::setBusyState(Engine::BusyState state)
{
    if (d->busyState != state) {
        d->busyState = state;
        Q_EMIT busyStateChanged();
    }
}

void Engine::setBusy(Engine::BusyState state, const QString &busyMessage)
{
    setBusyState(state);
    setBusyMessage(busyMessage);
}

void KNSCore::Engine::revalidateCacheEntries()
{
    // This gets called from QML, because in QtQuick we reuse the engine, BUG: 417985
    // We can't handle this in the cache, because it can't access the configuration of the engine
    if (cache() && EngineBase::d->shouldRemoveDeletedEntries) {
        for (const auto &provider : std::as_const(EngineBase::d->providers)) {
            if (provider && provider->isInitialized()) {
                const Entry::List cacheBefore = cache()->registryForProvider(provider->id());
                cache()->removeDeletedEntries();
                const Entry::List cacheAfter = cache()->registryForProvider(provider->id());
                // If the user has deleted them in the background we have to update the state to deleted
                for (const auto &oldCachedEntry : cacheBefore) {
                    if (!cacheAfter.contains(oldCachedEntry)) {
                        Entry removedEntry = oldCachedEntry;
                        removedEntry.setEntryDeleted();
                        Q_EMIT signalEntryEvent(removedEntry, Entry::StatusChangedEvent);
                    }
                }
            }
        }
    }
}

void Engine::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    EngineBase::addProvider(provider);
    connect(provider.data(), &Provider::loadingFinished, this, &Engine::slotEntriesLoaded);
    connect(provider.data(), &Provider::entryDetailsLoaded, this, &Engine::slotEntryDetailsLoaded);
}

void Engine::adoptEntry(const Entry &entry)
{
    registerTransaction(Transaction::adopt(this, entry));
}

void Engine::install(const KNSCore::Entry &entry, int linkId)
{
    auto transaction = Transaction::install(this, entry, linkId);
    registerTransaction(transaction);
    if (!transaction->isFinished()) {
        ++d->numInstallJobs;
    }
}

void Engine::uninstall(const KNSCore::Entry &entry)
{
    registerTransaction(Transaction::uninstall(this, entry));
}

void Engine::registerTransaction(Transaction *transaction)
{
    connect(transaction, &Transaction::signalErrorCode, this, &EngineBase::signalErrorCode);
    connect(transaction, &Transaction::signalMessage, this, &EngineBase::signalMessage);
    connect(transaction, &Transaction::signalEntryEvent, this, &Engine::signalEntryEvent);
}

#include "moc_engine.cpp"
