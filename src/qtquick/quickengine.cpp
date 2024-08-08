/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickengine.h"
#include "cache.h"
#include "errorcode.h"
#include "imageloader_p.h"
#include "installation_p.h"
#include "knewstuffquick_debug.h"
#include "quicksettings.h"

#include <KLocalizedString>
#include <QQmlInfo>
#include <QTimer>

#include "categoriesmodel.h"
#include "quickquestionlistener.h"
#include "searchpresetmodel.h"

#include "../core/enginebase_p.h"
#include "../core/providerbase_p.h"
#include "../core/providercore.h"
#include "../core/providercore_p.h"

// Could be made :public EngineBasePrivate so we don't have two distinct d pointers
class EnginePrivate
{
public:
    bool isValid = false;
    CategoriesModel *categoriesModel = nullptr;
    SearchPresetModel *searchPresetModel = nullptr;
    QString configFile;
    QTimer searchTimer;
    Engine::BusyState busyState;
    QString busyMessage;
    // the current request from providers
    KNSCore::SearchRequest currentRequest;
    KNSCore::SearchRequest storedRequest;
    // the page that is currently displayed, so it is not requested repeatedly
    int currentPage = -1;

    // when requesting entries from a provider, how many to ask for
    int pageSize = 20;

    int numDataJobs = 0;
    int numPictureJobs = 0;
    int numInstallJobs = 0;
};

Engine::Engine(QObject *parent)
    : KNSCore::EngineBase(parent)
    , d(new EnginePrivate)
    , dd(KNSCore::EngineBase::d.get())
{
    connect(this, &KNSCore::EngineBase::providerAdded, this, [this](auto core) {
        connect(core->d->base, &KNSCore::ProviderBase::loadingFinished, this, [this](const auto &request, const auto &entries) {
            d->currentPage = qMax<int>(request.page(), d->currentPage);
            qCDebug(KNEWSTUFFQUICK) << "loaded page " << request.page() << "current page" << d->currentPage << "count:" << entries.count();

            if (request.filter() != KNSCore::Filter::Updates) {
                dd->cache->insertRequest(request, entries);
            }
            Q_EMIT signalEntriesLoaded(entries);

            --d->numDataJobs;
            updateStatus();
        });
        connect(core->d->base, &KNSCore::ProviderBase::entryDetailsLoaded, this, [this](const auto &entry) {
            --d->numDataJobs;
            updateStatus();
            Q_EMIT signalEntryEvent(entry, KNSCore::Entry::DetailsLoadedEvent);
        });
    });

    const auto setBusy = [this](Engine::BusyState state, const QString &msg) {
        setBusyState(state);
        d->busyMessage = msg;
    };
    setBusy(BusyOperation::Initializing, i18n("Loading data")); // For the user this should be the same as initializing

    KNewStuffQuick::QuickQuestionListener::instance();
    d->categoriesModel = new CategoriesModel(this);
    connect(d->categoriesModel, &QAbstractListModel::modelReset, this, &Engine::categoriesChanged);
    d->searchPresetModel = new SearchPresetModel(this);
    connect(d->searchPresetModel, &QAbstractListModel::modelReset, this, &Engine::searchPresetModelChanged);

    d->searchTimer.setSingleShot(true);
    d->searchTimer.setInterval(1000);
    connect(&d->searchTimer, &QTimer::timeout, this, &Engine::reloadEntries);
    connect(installation(), &KNSCore::Installation::signalInstallationFinished, this, [this]() {
        --d->numInstallJobs;
        updateStatus();
    });
    connect(installation(), &KNSCore::Installation::signalInstallationFailed, this, [this](const QString &message) {
        --d->numInstallJobs;
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::InstallationError, message, QVariant());
    });
    connect(this, &EngineBase::signalProvidersLoaded, this, &Engine::updateStatus);
    connect(this, &EngineBase::signalProvidersLoaded, this, [this]() {
        d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                                   d->currentRequest.filter(),
                                                   d->currentRequest.searchTerm(),
                                                   EngineBase::categories(),
                                                   d->currentRequest.page(),
                                                   d->currentRequest.pageSize());
    });

    connect(this,
            &KNSCore::EngineBase::signalErrorCode,
            this,
            [setBusy, this](const KNSCore::ErrorCode::ErrorCode &error, const QString &message, const QVariant &metadata) {
                Q_EMIT errorCode(error, message, metadata);
                if (error == KNSCore::ErrorCode::ProviderError || error == KNSCore::ErrorCode::ConfigFileError) {
                    // This means loading the config or providers file failed entirely and we cannot complete the
                    // initialisation. It also means the engine is done loading, but that nothing will
                    // work, and we need to inform the user of this.
                    setBusy({}, QString());
                }

                // Emit the signal later, currently QML is not connected to the slot
                if (error == KNSCore::ErrorCode::ConfigFileError) {
                    QTimer::singleShot(0, [this, error, message, metadata]() {
                        Q_EMIT errorCode(error, message, metadata);
                    });
                }
            });

    connect(this, &Engine::signalEntryEvent, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
        // Just forward the event but not do anything more
        Q_EMIT entryEvent(entry, event);
    });
    //
    // And finally, let's just make sure we don't miss out the various things here getting changed
    // In other words, when we're asked to reset the view, actually do that
    connect(this, &Engine::signalResetView, this, &Engine::categoriesFilterChanged);
    connect(this, &Engine::signalResetView, this, &Engine::filterChanged);
    connect(this, &Engine::signalResetView, this, &Engine::sortOrderChanged);
    connect(this, &Engine::signalResetView, this, &Engine::searchTermChanged);
}

bool Engine::init(const QString &configfile)
{
    const bool valid = EngineBase::init(configfile);
    if (valid) {
        connect(this, &Engine::signalEntryEvent, dd->cache.get(), [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            if (event == KNSCore::Entry::StatusChangedEvent) {
                dd->cache->registerChangedEntry(entry);
            }
        });
        const auto slotEntryChanged = [this](const KNSCore::Entry &entry) {
            Q_EMIT signalEntryEvent(entry, KNSCore::Entry::StatusChangedEvent);
        };
        // Don't connect KNSCore::Installation::signalEntryChanged as is already forwarded to
        // Transaction, which in turn is forwarded to our slotEntryChanged, so avoids a double emission
        connect(dd->cache.get(), &KNSCore::Cache2::entryChanged, this, slotEntryChanged);
    }
    return valid;
}
void Engine::updateStatus()
{
    QString busyMessage;
    BusyState state;
    if (d->numPictureJobs > 0) {
        // If it is loading previews or data is irrelevant for the user
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingPreview;
    }
    if (d->numInstallJobs > 0) {
        busyMessage = i18n("Installing");
        state |= BusyOperation::InstallingEntry;
    }
    if (d->numDataJobs > 0) {
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingData;
    }
    d->busyMessage = busyMessage;
    setBusyState(state);
}

bool Engine::needsLazyLoadSpinner()
{
    return d->numDataJobs > 0 || d->numPictureJobs;
}

Engine::~Engine() = default;

void Engine::setBusyState(BusyState state)
{
    d->busyState = state;
    Q_EMIT busyStateChanged();
}
Engine::BusyState Engine::busyState() const
{
    return d->busyState;
}
QString Engine::busyMessage() const
{
    return d->busyMessage;
}

QString Engine::configFile() const
{
    return d->configFile;
}

void Engine::setConfigFile(const QString &newFile)
{
    if (d->configFile != newFile) {
        d->configFile = newFile;
        Q_EMIT configFileChanged();

        if (KNewStuffQuick::Settings::instance()->allowedByKiosk()) {
            d->isValid = init(newFile);
            Q_EMIT categoriesFilterChanged();
            Q_EMIT filterChanged();
            Q_EMIT sortOrderChanged();
            Q_EMIT searchTermChanged();
        } else {
            // This is not an error message in the proper sense, and the message is not intended to look like an error (as there is really
            // nothing the user can do to fix it, and we just tell them so they're not wondering what's wrong)
            Q_EMIT errorCode(
                KNSCore::ErrorCode::ConfigFileError,
                i18nc("An informational message which is shown to inform the user they are not authorized to use GetHotNewStuff functionality",
                      "You are not authorized to Get Hot New Stuff. If you think this is in error, please contact the person in charge of your permissions."),
                QVariant());
        }
    }
}

CategoriesModel *Engine::categories() const
{
    return d->categoriesModel;
}

QStringList Engine::categoriesFilter() const
{
    return d->currentRequest.categories();
}

void Engine::setCategoriesFilter(const QStringList &newCategoriesFilter)
{
    if (d->currentRequest.categories() != newCategoriesFilter) {
        d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                                   d->currentRequest.filter(),
                                                   d->currentRequest.searchTerm(),
                                                   newCategoriesFilter,
                                                   d->currentRequest.page(),
                                                   d->currentRequest.pageSize());
        reloadEntries();
        Q_EMIT categoriesFilterChanged();
    }
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
KNSCore::Provider::Filter Engine::filter() const
{
    return [filter = filter2()] {
        switch (filter) {
        case KNSCore::Filter::None:
            return KNSCore::Provider::None;
        case KNSCore::Filter::Installed:
            return KNSCore::Provider::Installed;
        case KNSCore::Filter::Updates:
            return KNSCore::Provider::Updates;
        case KNSCore::Filter::ExactEntryId:
            return KNSCore::Provider::ExactEntryId;
        }
        return KNSCore::Provider::None;
    }();
}
#endif

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
void Engine::setFilter(KNSCore::Provider::Filter newFilter_)
{
    setFilter2([newFilter_] {
        switch (newFilter_) {
        case KNSCore::Provider::None:
            return KNSCore::Filter::None;
        case KNSCore::Provider::Installed:
            return KNSCore::Filter::Installed;
        case KNSCore::Provider::Updates:
            return KNSCore::Filter::Updates;
        case KNSCore::Provider::ExactEntryId:
            return KNSCore::Filter::ExactEntryId;
        }
        return KNSCore::Filter::None;
    }());
}
#endif

KNSCore::Filter Engine::filter2() const
{
    return d->currentRequest.filter();
}

void Engine::setFilter2(KNSCore::Filter newFilter)
{
    if (d->currentRequest.filter() != newFilter) {
        d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                                   newFilter,
                                                   d->currentRequest.searchTerm(),
                                                   d->currentRequest.categories(),
                                                   d->currentRequest.page(),
                                                   d->currentRequest.pageSize());
        reloadEntries();
        Q_EMIT filterChanged();
    }
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
KNSCore::Provider::SortMode Engine::sortOrder() const
{
    return [mode = sortOrder2()] {
        switch (mode) {
        case KNSCore::SortMode::Newest:
            return KNSCore::Provider::Newest;
        case KNSCore::SortMode::Alphabetical:
            return KNSCore::Provider::Alphabetical;
        case KNSCore::SortMode::Rating:
            return KNSCore::Provider::Rating;
        case KNSCore::SortMode::Downloads:
            return KNSCore::Provider::Downloads;
        }
        return KNSCore::Provider::Rating;
    }();
}
#endif

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
void Engine::setSortOrder(KNSCore::Provider::SortMode mode_)
{
    setSortOrder2([mode_] {
        switch (mode_) {
        case KNSCore::Provider::Newest:
            return KNSCore::SortMode::Newest;
        case KNSCore::Provider::Alphabetical:
            return KNSCore::SortMode::Alphabetical;
        case KNSCore::Provider::Rating:
            return KNSCore::SortMode::Rating;
        case KNSCore::Provider::Downloads:
            return KNSCore::SortMode::Downloads;
        }
        return KNSCore::SortMode::Rating;
    }());
}
#endif

KNSCore::SortMode Engine::sortOrder2() const
{
    return d->currentRequest.sortMode();
}

void Engine::setSortOrder2(KNSCore::SortMode mode)
{
    if (d->currentRequest.sortMode() != mode) {
        d->currentRequest = KNSCore::SearchRequest(mode,
                                                   d->currentRequest.filter(),
                                                   d->currentRequest.searchTerm(),
                                                   d->currentRequest.categories(),
                                                   d->currentRequest.page(),
                                                   d->currentRequest.pageSize());
        reloadEntries();
        Q_EMIT sortOrderChanged();
    }
}

QString Engine::searchTerm() const
{
    return d->currentRequest.searchTerm();
}

void Engine::setSearchTerm(const QString &searchTerm)
{
    if (d->isValid && d->currentRequest.searchTerm() != searchTerm) {
        d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                                   d->currentRequest.filter(),
                                                   searchTerm,
                                                   d->currentRequest.categories(),
                                                   d->currentRequest.page(),
                                                   d->currentRequest.pageSize());
        Q_EMIT searchTermChanged();
    }
    KNSCore::Entry::List cacheEntries = dd->cache->requestFromCache(d->currentRequest);
    if (!cacheEntries.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer.start();
    }
}

SearchPresetModel *Engine::searchPresetModel() const
{
    return d->searchPresetModel;
}

bool Engine::isValid()
{
    return d->isValid;
}

void Engine::updateEntryContents(const KNSCore::Entry &entry)
{
    const auto core = dd->providerCores.value(entry.providerId());
    if (!core) {
        qCWarning(KNEWSTUFFQUICK) << "Provider was not found" << entry.providerId();
        return;
    }

    const auto base = core->d->base;
    if (!base->isInitialized()) {
        qCWarning(KNEWSTUFFQUICK) << "Provider was not initialized" << base << entry.providerId();
        return;
    }

    base->loadEntryDetails(entry);
}

void Engine::reloadEntries()
{
    Q_EMIT signalResetView();
    d->currentPage = -1;
    d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                               d->currentRequest.filter(),
                                               d->currentRequest.searchTerm(),
                                               d->currentRequest.categories(),
                                               0,
                                               d->currentRequest.pageSize());
    d->numDataJobs = 0;

    const auto providersList = dd->providerCores;
    for (const auto &core : providersList) {
        const auto &base = core->d->base;
        if (base->isInitialized()) {
            if (d->currentRequest.filter() == KNSCore::Filter::Installed || d->currentRequest.filter() == KNSCore::Filter::Updates) {
                // when asking for installed entries, never use the cache
                base->loadEntries(d->currentRequest);
            } else {
                // take entries from cache until there are no more
                KNSCore::Entry::List cacheEntries;
                KNSCore::Entry::List lastCache = dd->cache->requestFromCache(d->currentRequest);
                while (!lastCache.isEmpty()) {
                    qCDebug(KNEWSTUFFQUICK) << "From cache";
                    cacheEntries << lastCache;

                    d->currentPage = d->currentRequest.page();
                    d->currentRequest = d->currentRequest.nextPage();
                    lastCache = dd->cache->requestFromCache(d->currentRequest);
                }

                // Since the cache has no more pages, reset the request's page
                if (d->currentPage >= 0) {
                    d->currentRequest = KNSCore::SearchRequest(d->currentRequest.sortMode(),
                                                               d->currentRequest.filter(),
                                                               d->currentRequest.searchTerm(),
                                                               d->currentRequest.categories(),
                                                               d->currentPage,
                                                               d->currentRequest.pageSize());
                }

                if (!cacheEntries.isEmpty()) {
                    Q_EMIT signalEntriesLoaded(cacheEntries);
                } else {
                    qCDebug(KNEWSTUFFQUICK) << "From provider";
                    base->loadEntries(d->currentRequest);

                    ++d->numDataJobs;
                    updateStatus();
                }
            }
        }
    }
}

void Engine::loadPreview(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFQUICK) << "START  preview: " << entry.name() << type;
    auto l = new KNSCore::ImageLoader(entry, type, this);
    connect(l, &KNSCore::ImageLoader::signalPreviewLoaded, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type) {
        qCDebug(KNEWSTUFFQUICK) << "FINISH preview: " << entry.name() << type;
        Q_EMIT signalEntryPreviewLoaded(entry, type);
        --d->numPictureJobs;
        updateStatus();
    });
    connect(l, &KNSCore::ImageLoader::signalError, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type, const QString &errorText) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ImageError, errorText, QVariantList() << entry.name() << type);
        qCDebug(KNEWSTUFFQUICK) << "ERROR preview: " << errorText << entry.name() << type;
        --d->numPictureJobs;
        updateStatus();
    });
    l->start();
    ++d->numPictureJobs;
    updateStatus();
}

void Engine::adoptEntry(const KNSCore::Entry &entry)
{
    registerTransaction(KNSCore::Transaction::adopt(this, entry));
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
void Engine::install(const KNSCore::Entry &entry, int linkId)
{
    qmlWarning(this) << "org.kde.newstuff.core.Engine.install is deprecated. Use installLinkId or installLatest";
    auto transaction = KNSCore::Transaction::install(this, entry, linkId);
    registerTransaction(transaction);
    if (!transaction->isFinished()) {
        ++d->numInstallJobs;
    }
}
#endif

void Engine::installLinkId(const KNSCore::Entry &entry, quint8 linkId)
{
    auto transaction = KNSCore::Transaction::installLinkId(this, entry, linkId);
    registerTransaction(transaction);
    if (!transaction->isFinished()) {
        ++d->numInstallJobs;
    }
}

void Engine::installLatest(const KNSCore::Entry &entry)
{
    auto transaction = KNSCore::Transaction::installLatest(this, entry);
    registerTransaction(transaction);
    if (!transaction->isFinished()) {
        ++d->numInstallJobs;
    }
}

void Engine::uninstall(const KNSCore::Entry &entry)
{
    registerTransaction(KNSCore::Transaction::uninstall(this, entry));
}
void Engine::registerTransaction(KNSCore::Transaction *transaction)
{
    connect(transaction, &KNSCore::Transaction::signalErrorCode, this, &EngineBase::signalErrorCode);
    connect(transaction, &KNSCore::Transaction::signalMessage, this, &EngineBase::signalMessage);
    connect(transaction, &KNSCore::Transaction::signalEntryEvent, this, &Engine::signalEntryEvent);
}

void Engine::requestMoreData()
{
    qCDebug(KNEWSTUFFQUICK) << "Get more data! current page: " << d->currentPage << " requested: " << d->currentRequest.page();

    if (d->currentPage < d->currentRequest.page()) {
        return;
    }

    d->currentRequest = d->currentRequest.nextPage();
    doRequest();
}

void Engine::doRequest()
{
    const auto cores = dd->providerCores;
    for (const auto &core : cores) {
        const auto &base = core->d->base;
        if (base->isInitialized()) {
            base->loadEntries(d->currentRequest);
            ++d->numDataJobs;
            updateStatus();
        }
    }
}

void Engine::revalidateCacheEntries()
{
    // This gets called from QML, because in QtQuick we reuse the engine, BUG: 417985
    // We can't handle this in the cache, because it can't access the configuration of the engine
    if (dd->cache) {
        const auto cores = dd->providerCores;
        for (const auto &core : cores) {
            const auto &base = core->d->base;
            if (base && base->isInitialized()) {
                const KNSCore::Entry::List cacheBefore = dd->cache->registryForProvider(base->id());
                dd->cache->removeDeletedEntries();
                const KNSCore::Entry::List cacheAfter = dd->cache->registryForProvider(base->id());
                // If the user has deleted them in the background we have to update the state to deleted
                for (const auto &oldCachedEntry : cacheBefore) {
                    if (!cacheAfter.contains(oldCachedEntry)) {
                        KNSCore::Entry removedEntry = oldCachedEntry;
                        removedEntry.setEntryDeleted();
                        Q_EMIT signalEntryEvent(removedEntry, KNSCore::Entry::StatusChangedEvent);
                    }
                }
            }
        }
    }
}

void Engine::restoreSearch()
{
    d->searchTimer.stop();
    d->currentRequest = d->storedRequest;
    if (dd->cache) {
        KNSCore::Entry::List cacheEntries = dd->cache->requestFromCache(d->currentRequest);
        if (!cacheEntries.isEmpty()) {
            reloadEntries();
        } else {
            d->searchTimer.start();
        }
    } else {
        qCWarning(KNEWSTUFFQUICK) << "Attempted to call restoreSearch() without a correctly initialized engine. You will likely get unexpected behaviour.";
    }
}

void Engine::storeSearch()
{
    d->storedRequest = d->currentRequest;
}
