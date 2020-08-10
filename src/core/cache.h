/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef CACHE_H
#define CACHE_H

#include <QObject>
#include <QSet>

#include "engine.h"
#include "entryinternal.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{

class KNEWSTUFFCORE_EXPORT Cache : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns an instance of a shared cache for appName
     * That way it is made sure, that there do not exist different
     * instances of cache, with different contents
     * @param appName The file name of the registry - this is usually
     * the application name, it will be stored in "apps/knewstuff3/appname.knsregistry"
     */
    static QSharedPointer<Cache> getCache(const QString &appName);

    ~Cache();

    /// Read the installed entries (on startup)
    void readRegistry();
    /// All entries that have been installed by a certain provider
    EntryInternal::List registryForProvider(const QString &providerId);

    /// Save the list of installed entries
    void writeRegistry();

    void insertRequest(const KNSCore::Provider::SearchRequest &, const KNSCore::EntryInternal::List &entries);
    EntryInternal::List requestFromCache(const KNSCore::Provider::SearchRequest &);

    /**
     * This will run through all entries in the cache, and remove all entries
     * where all the installed files they refer to no longer exist.
     *
     * This cannot be done wholesale for all caches, as some consumers will allow
     * this to happen (or indeed expect it to), and so we have to do this on a
     * per-type basis
     *
     * This will also cause the cache store to be updated
     *
     * @since 5.71
     */
    void removeDeletedEntries();
public Q_SLOTS:
    void registerChangedEntry(const KNSCore::EntryInternal &entry);

private:
    Q_DISABLE_COPY(Cache)
    Cache(const QString &appName);

    // compatibility with KNS2
    void readKns2MetaFiles();

private:
    // The file that is used to keep track of downloaded entries
    QString registryFile;

    // The component name that was used in KNS2 to keep track of .meta files
    // This is only for compatibility with the former version - KNewStuff2.
    QString m_kns2ComponentName;

    QSet<EntryInternal> cache;
    QHash<QString, EntryInternal::List> requestCache;
};

}

#endif
