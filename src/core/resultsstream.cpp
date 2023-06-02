/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "resultsstream.h"
#include "enginebase_p.h"
#include "knewstuffcore_debug.h"

using namespace KNSCore;

class KNSCore::ResultsStreamPrivate
{
public:
    QList<QSharedPointer<KNSCore::Provider>> providers;
    EngineBase *engine;
    Provider::SearchRequest request;
};

ResultsStream::ResultsStream(const Provider::SearchRequest &request, EngineBase *base)
    : d(new ResultsStreamPrivate)
{
    d->engine = base;
    d->request = request;
    d->providers = base->d->providers.values();
    auto f = [this](const KNSCore::Provider::SearchRequest &request, const KNSCore::Entry::List &entries) {
        d->providers.removeAll(static_cast<Provider *>(sender()));
        if (entries.isEmpty() && d->providers.isEmpty()) {
            finish();
        }

        if (request == d->request) {
            Q_EMIT entriesFound(entries);
        }
    };
    for (const auto &provider : d->providers) {
        connect(provider.data(), &Provider::loadingFinished, this, f);
    }
}

ResultsStream::~ResultsStream() = default;

void ResultsStream::fetch()
{
    if (d->request.filter != Provider::Installed) {
        // when asking for installed entries, never use the cache
        Entry::List cacheEntries = d->engine->cache()->requestFromCache(d->request);
        if (!cacheEntries.isEmpty()) {
            Q_EMIT entriesFound(cacheEntries);
            return;
        }
    }

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (p->isInitialized()) {
            p->loadEntries(d->request);
        } else {
            connect(p.get(), &KNSCore::Provider::providerInitialized, this, [this, p] {
                disconnect(p.get(), &KNSCore::Provider::providerInitialized, this, nullptr);
                p->loadEntries(d->request);
            });
        }
    }
}

void ResultsStream::fetchMore()
{
    d->request.page++;
    fetch();
}

void ResultsStream::finish()
{
    Q_EMIT finished();
    deleteLater();
}
