/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QObject>
#include <QSet>

#include "entry.h"

#include "knewstuffcore_export.h"

#include <memory.h>

namespace KNSCore
{
class Cache2Private;
class SearchRequest;

// Exported for our internal QtQuick tech. Do not install this header or use it outside knewstuff!
class KNEWSTUFFCORE_EXPORT Cache2 : public QObject
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
    static QSharedPointer<Cache2> getCache(const QString &appName);

    ~Cache2() override;
    Q_DISABLE_COPY(Cache2)

    /// Read the installed entries (on startup)
    void readRegistry();
    /// All entries that have been installed by a certain provider
    Entry::List registryForProvider(const QString &providerId);

    /// All entries known by the cache (this corresponds with entries which are installed, regardless of status)
    Entry::List registry() const;

    /// Save the list of installed entries
    void writeRegistry();

    void insertRequest(const KNSCore::SearchRequest &, const KNSCore::Entry::List &entries);
    Entry::List requestFromCache(const KNSCore::SearchRequest &);

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

    /**
     * Get the entry which installed the passed file. If no entry lists the
     * passed file as having been installed by it, an invalid entry will be
     * returned.
     * @param installedFile The full path name for an installed file
     * @return An entry if one was found, or an invalid entry if no entry says it installed that file
     * since 5.74
     */
    KNSCore::Entry entryFromInstalledFile(const QString &installedFile) const;

    /**
     * Emitted when the cache has changed underneath us, and need users of the cache to know
     * that this has happened.
     * @param entry The entry which has changed
     * @since 5.75
     */
    Q_SIGNAL void entryChanged(const KNSCore::Entry &entry);

public Q_SLOTS:
    void registerChangedEntry(const KNSCore::Entry &entry);

private:
    Cache2(const QString &appName);

private:
    std::unique_ptr<Cache2Private> d;
};

}
