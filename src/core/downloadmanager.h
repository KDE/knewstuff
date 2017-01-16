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

#ifndef KNSCORE_DownloadManager_H
#define KNSCORE_DownloadManager_H

#include "knewstuffcore_export.h"
#include "entryinternal.h"

namespace KNSCore
{
class DownloadManagerPrivate;
/**
 * KNewStuff update checker.
 * This class can be used to search for KNewStuff items
 * without using the widgets and to look for updates of
 * already installed items without showing the dialog.
 */
class KNEWSTUFFCORE_EXPORT DownloadManager : public QObject
{
    Q_OBJECT

public:
    enum SortOrder {
        Newest,
        Alphabetical,
        Rating,
        Downloads
    };

    /**
     * Create a DownloadManager
     * It will try to find a appname.knsrc file.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     */
    explicit DownloadManager(QObject *parent = nullptr);

    /**
     * Create a DownloadManager. Manually specifying the name of the .knsrc file.
     *
     * @param configFile the name of the configuration file
     * @param parent the parent of the dialog
     */
    explicit DownloadManager(const QString &configFile, QObject *parent = nullptr);

    /**
     * destructor
     */
    ~DownloadManager();

    /**
      Search for a list of entries. searchResult will be emitted with the requested list.
    */
    void search(int page = 0, int pageSize = 100);

    /**
      Check for available updates.
      Use searchResult to get notified as soon as an update has been found.
      */
    void checkForUpdates();

    /**
      Check for installed resources
      Use searchResult to get notified about installed entries.

      @since 5.28
      */
    void checkForInstalled();

    /**
      Installs or updates an entry
      @param entry The entry you wish to install or update
      */
    void installEntry(const EntryInternal &entry);

    /**
     * Uninstalls the given entry.
     * @param entry The entry which will be uninstalled.
     */
    void uninstallEntry(const EntryInternal &entry);

    /**
      Sets the search term to filter the results on the server.
      Note that this function does not trigger a search. Use search after setting this.
      @param searchTerm The term you wish to search for
      */
    void setSearchTerm(const QString &searchTerm);

    /**
      Set the sort order of the results. This depends on the server.
      Note that this function does not trigger a search. Use search after setting this.
      @see SortOrder
      @param order The way you want the results to be sorted
      */
    void setSearchOrder(SortOrder order);

    /**
     * Triggers a search for an entry with @p id as its unique id
     *
     * @see searchResult
     *
     * @since 5.28
     */
    void fetchEntryById(const QString &id);

Q_SIGNALS:
    /**
      Returns the search result.
      This can be the list of updates after checkForUpdates or the result of a search.
      @param entries the list of results. entries is empty when nothing was found.
     */
    void searchResult(const EntryInternal::List &entries);

    /**
      The entry status has changed: emitted when the entry has been installed, updated or removed.
      Use EntryInternal::status() to check the current status.
      @param entry the item that has been updated.
     */
    void entryStatusChanged(const EntryInternal &entry);

    /**
     * Notifies that the engine couldn't be loaded properly and won't be suitable
     */
    void errorFound(const QString &errorMessage);

public Q_SLOTS:
    void slotProvidersLoaded();

private:
    DownloadManagerPrivate *const d;
    Q_DISABLE_COPY(DownloadManager)
};

}

#endif
