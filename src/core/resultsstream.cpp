/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "resultsstream.h"
#include "enginebase_p.h"
#include "knewstuffcore_debug.h"

#include <iostream>

#include <QTimer>

#include "providerbase_p.h"
#include "providercore.h"
#include "providercore_p.h"
#include "searchrequest.h"
#include "searchrequest_p.h"

using namespace KNSCore;

class KNSCore::ResultsStreamPrivate
{
public:
    QList<QSharedPointer<KNSCore::ProviderCore>> providers;
    EngineBase const *engine;
    SearchRequest request;
    bool finished = false;
    int queuedFetch = 0;
};

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
ResultsStream::ResultsStream([[maybe_unused]] const Provider::SearchRequest &request, EngineBase *base)
    : KNSCore::ResultsStream(SearchRequest(), base)
{
    // This ctor should not be used. It is private and we don't use. Nobody else should either. Here for ABI stability.
    Q_ASSERT(false);
    qFatal("Do not use private constructors!");
}
#endif

ResultsStream::ResultsStream(const SearchRequest &request, EngineBase *base)
    : d(new ResultsStreamPrivate{
          .providers = base->d->providerCores.values(),
          .engine = base,
          .request = request,
      })
{
    auto entriesLoaded = [this](const KNSCore::SearchRequest &request, const KNSCore::Entry::List &entries) {
        if (request.d != d->request.d) {
            return;
        }
        Q_EMIT entriesFound(entries);
    };

    auto done = [this](const KNSCore::SearchRequest &request) {
        if (request.d != d->request.d) {
            return;
        }

        qWarning() << this << "Finishing" << sender() << request.d->id;

        auto base = qobject_cast<ProviderBase *>(sender());
        Q_ASSERT_X(base, Q_FUNC_INFO, "Sender failed to cast to ProviderBase");
        if (const auto coresRemoved = d->providers.removeIf([base](const auto &core) {
                return core->d->base == base;
            });
            coresRemoved <= 0) {
            qCWarning(KNEWSTUFFCORE) << "Request finished twice, check your provider" << sender() << d->engine;

            Q_ASSERT(false);
            return;
        }

        if (d->providers.isEmpty()) {
            d->finished = true;
            if (d->queuedFetch > 0) {
                d->queuedFetch--;
                fetchMore();
                return;
            }

            d->request = {}; // prevent this stream from making more requests
            d->finished = true;
            finish();
        }
    };
    auto failed = [this](const KNSCore::SearchRequest &request) {
        if (request.d == d->request.d) {
            finish();
        }
    };

    auto seenProviders = d->providers;
    seenProviders.clear();
    for (const auto &provider : d->providers) {
        Q_ASSERT(!seenProviders.contains(provider));
        seenProviders.append(provider);

        connect(provider->d->base, &ProviderBase::entriesLoaded, this, entriesLoaded);
        connect(provider->d->base, &ProviderBase::loadingDone, this, done);
        connect(provider->d->base, &ProviderBase::entryDetailsLoaded, this, [this](const KNSCore::Entry &entry) {
            if (d->request.d->filter == KNSCore::Filter::ExactEntryId && d->request.d->searchTerm == entry.uniqueId()) {
                if (entry.isValid()) {
                    Q_EMIT entriesFound({entry});
                }
                finish();
            }
        });
        connect(provider->d->base, &ProviderBase::loadingFailed, this, failed);
    }
}

ResultsStream::~ResultsStream() = default;

void ResultsStream::fetch()
{
    if (d->finished) {
        Q_ASSERT_X(false, Q_FUNC_INFO, "Called fetch on an already finished stream. Call fetchMore.");
        return;
    }

    qDebug() << this << "fetching" << d->request;
    if (d->request.d->filter != Filter::Installed) {
        // when asking for installed entries, never use the cache
        Entry::List cacheEntries = d->engine->d->cache->requestFromCache(d->request);
        if (!cacheEntries.isEmpty()) {
            Q_EMIT entriesFound(cacheEntries);
            return;
        }
    }

    for (const auto &providerCore : std::as_const(d->providers)) {
        auto provider = providerCore->d->base;
        qDebug() << this << "loading entries from provider" << provider;
        if (provider->isInitialized()) {
            QTimer::singleShot(0, this, [this, provider] {
                provider->loadEntries(d->request);
            });
        } else {
            connect(provider, &KNSCore::ProviderBase::providerInitialized, this, [this, provider] {
                disconnect(provider, &KNSCore::ProviderBase::providerInitialized, this, nullptr);
                provider->loadEntries(d->request);
            });
        }
    }
}

void ResultsStream::fetchMore()
{
    // fetchMore requires some extra tinkering but this is worthwhile. By offering a fetchMore we can fully encapsulate
    // a search state so the caller doesn't have to worry about persisting SearchRequests. Instead we'll do it for them.
    if (!d->finished) {
        d->queuedFetch++;
        return;
    }
    d->finished = false;
    const auto nextPage = d->request.d->page + 1;
    d->request =
        SearchRequest(d->request.d->sortMode, d->request.d->filter, d->request.d->searchTerm, d->request.d->categories, nextPage, d->request.d->pageSize);
    d->providers = d->engine->d->providerCores.values();
    fetch();
}

void ResultsStream::finish()
{
    Q_EMIT finished();
    deleteLater();
}

#include "moc_resultsstream.cpp"
