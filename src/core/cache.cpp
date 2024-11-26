/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "cache.h"

#include "cache2_p.h"
#include "compat_p.h"

class KNSCore::CachePrivate
{
public:
    QSharedPointer<Cache2> cache2;
};

using namespace KNSCore;

Cache::Cache(const QString &appName)
    : QObject(nullptr)
    , d(new CachePrivate(Cache2::getCache(appName)))
{
}

QSharedPointer<Cache> Cache::getCache(const QString &appName)
{
    return QSharedPointer<Cache>(new Cache(appName)); // internally this hits the cache2 registry
}

Cache::~Cache() = default;

void Cache::readRegistry()
{
    d->cache2->readRegistry();
}

Entry::List Cache::registryForProvider(const QString &providerId)
{
    return d->cache2->registryForProvider(providerId);
}

Entry::List Cache::registry() const
{
    return d->cache2->registry();
}

void Cache::writeRegistry()
{
    d->cache2->writeRegistry();
}

void Cache::registerChangedEntry(const KNSCore::Entry &entry)
{
    d->cache2->registerChangedEntry(entry);
}

void Cache::insertRequest(const KNSCore::Provider::SearchRequest &request, const KNSCore::Entry::List &entries)
{
    d->cache2->insertRequest(KNSCompat::searchRequestFromLegacy(request), entries);
}

Entry::List Cache::requestFromCache(const KNSCore::Provider::SearchRequest &request)
{
    return d->cache2->requestFromCache(KNSCompat::searchRequestFromLegacy(request));
}

void KNSCore::Cache::removeDeletedEntries()
{
    d->cache2->removeDeletedEntries();
}

KNSCore::Entry KNSCore::Cache::entryFromInstalledFile(const QString &installedFile) const
{
    return d->cache2->entryFromInstalledFile(installedFile);
}

#include "moc_cache.cpp"
