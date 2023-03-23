/*
    knewstuff3/engine.h.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENGINE_P_H
#define KNEWSTUFF3_ENGINE_P_H

#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "enginebase.h"
#include "entry.h"
#include "errorcode.h"
#include "provider.h"

#include "knewstuffcore_export.h"

#include <memory>

class KJob;
class EnginePrivate;
class QDomDocument;

namespace Attica
{
class Provider;
}

/**
 * Contains the core functionality for handling interaction with NewStuff providers.
 * The entrypoint for most things will be the creation of an instance of KNSCore::Engine
 * which will other classes then either use or get instantiated from directly.
 *
 * NOTE: When implementing anything on top of KNSCore, without using either KNS3 or the
 * Qt Quick components, you will need to implement a custom QuestionListener (see that
 * class for instructions)
 *
 * @see KNSCore::Engine
 * @see KNSCore::ItemsModel
 * @see KNSCore::QuestionListener
 */
namespace KNSCore
{
class Cache;
class CommentsModel;
class Transaction;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 */
class KNEWSTUFFCORE_EXPORT Engine : public EngineBase
{
    Q_OBJECT

    /**
     * Current state of the engine, the state con contain multiple operations
     * an empty BusyState represents the idle status
     * @since 5.74
     */
    Q_PROPERTY(BusyState busyState READ busyState WRITE setBusyState NOTIFY busyStateChanged)

    /**
     * String representation of the engines busy state, in the case of idle this string is empty
     * @since 5.74
     */
    Q_PROPERTY(QString busyMessage READ busyMessage WRITE setBusyMessage NOTIFY busyMessageChanged)

public:
    enum class BusyOperation {
        Initializing,
        LoadingData,
        LoadingPreview,
        InstallingEntry,
    };
    Q_DECLARE_FLAGS(BusyState, BusyOperation)

    bool init(const QString &configfile) override;

    /**
     * String representation of the engines busy state
     * @since 5.74
     */
    QString busyMessage() const;

    /**
     * @since 5.74
     * @see setBusy
     * @see setBusyState
     */
    void setBusyMessage(const QString &busyMessage);

    /**
     * Signal gets emitted when the busy message changes
     * @since 5.74 String representation of the engines busy state
     */
    Q_SIGNAL void busyMessageChanged();

    /**
     * Busy state of the engine
     * @since 5.74
     */
    BusyState busyState() const;

    /**
     * Sets the busy state of the engine
     * @since 5.74
     * @see setBusy
     * @see setBusyMessage
     */
    void setBusyState(BusyState state);

    /**
     * Utility method to set both the state and busyMessage
     * @since 5.74
     */
    void setBusy(BusyState state, const QString &busyMessage);

    /**
     * Signal gets emitted when the busy state changes
     * @since 5.74
     */
    Q_SIGNAL void busyStateChanged();

    /**
     * Constructor.
     */
    explicit Engine(QObject *parent = nullptr);

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~Engine() override;

    /**
     * Attempt to load a specific preview for the specified entry.
     *
     * @param entry The entry to fetch a preview for
     * @param type The particular preview to fetch
     *
     * @see signalEntryPreviewLoaded(KNSCore::Entry, KNSCore::Entry::PreviewType);
     * @see signalPreviewFailed();
     */
    void loadPreview(const KNSCore::Entry &entry, Entry::PreviewType type);
    /**
     * Get the full details of a specific entry
     *
     * @param entry The entry to get full details for
     *
     * @see Entry::signalEntryDetailsLoaded(KNSCore::Entry)
     */
    void loadDetails(const KNSCore::Entry &entry);

    /**
     * Set the order the search results are returned in.
     *
     * Search requests default to showing the newest entries first.
     *
     * Note: This will automatically launch a search, which means
     * you do not need to call requestData manually.
     *
     * @see KNSCore::Provider::SearchRequest
     * @param mode The order you want search results to come back in.
     */
    void setSortMode(Provider::SortMode mode);
    /**
     * The sort mode set on the current request
     * @see setSortMode(Provider::SortMode)
     * @since 5.63
     */
    Provider::SortMode sortMode() const;
    /**
     * Set a filter for results (defaults to none), which will allow you
     * to show only installed entries, installed entries which have updates,
     * or a specific item with a specified ID. The latter further requires
     * the search term to be the exact ID of the entry you wish to retrieve.
     *
     * Note: This will automatically launch a search, which means
     * you do not need to call requestData manually.
     *
     * @see fetchEntryById(QString)
     * @see setSearchTerm(QString)
     * @param filter The type of results you wish to see
     */
    void setFilter(Provider::Filter filter);
    /**
     * The result filter set on the current request
     * @see setFilter(Provider::Filter)
     * @since 5.63
     */
    Provider::Filter filter() const;

    /**
     * Set the categories that will be included in searches
     *
     * Note: This will automatically launch a search, which means
     * you do not need to call requestData manually.
     *
     * @see KNSCore::Engine::categories()
     * @param categories A list of strings of categories
     */
    void setCategoriesFilter(const QStringList &categories);
    /**
     * Sets a string search term.
     *
     * Note: This will automatically launch a search, which means
     * you do not need to call requestData manually.
     *
     * @param searchString The search term you wish to search for
     */
    void setSearchTerm(const QString &searchString);
    /**
     * The search term for the current search (empty if none is set)
     * @return The current search term
     * @since 5.63
     */
    QString searchTerm() const;
    void reloadEntries();
    void requestMoreData();
    void requestData(int page, int pageSize);

    /**
     * Request for packages that are installed and need update
     *
     * These will be reported through the signal @see signalUpdateableEntriesLoaded().
     */
    void checkForUpdates();

    /**
     * Requests installed packages with an up to date state
     *
     * @see signalEntriesLoaded()
     */
    void checkForInstalled();

    /**
     * Convenience method to launch a search for one specific entry.
     *
     * @note it will reset the engine state (use storeSearch() and restoreSearch() to handle this if needed)
     *
     * @param id The ID of the entry you wish to fetch
     */
    Q_INVOKABLE void fetchEntryById(const QString &id);

    /**
     * Restore a previously saved search to be current
     * Also emits the appropriate signals so any views depending
     * on the information can be updated)
     *
     * @since 5.79
     */
    Q_INVOKABLE void restoreSearch();

    /**
     * Stores the current search parameters internally
     * This might for example be used to allow you to restore the current view state
     * after performing a single-entry fetch with fetchEntryById(QString). That function
     * does not perform this action itself, because it may well not be the desired
     * outcome.
     *
     * @since 5.79
     */
    Q_INVOKABLE void storeSearch();

    /**
     * The list of categories searches will actually show results from. This
     * is a subset of the categories() list.
     *
     * @see KNSCore::Engine::setCategoriesFilter(QString)
     */
    QStringList categoriesFilter() const;

    /**
     * Set the page size for requests not made explicitly with requestData(int,int)
     * @param pageSize the default number of entries to request from the provider
     * @see requestData(int,int)
     */
    void setPageSize(int pageSize);

    /**
     * @returns the page size we previously set
     * @see requestData(int,int)
     *
     * @since 5.95
     */
    int pageSize() const;

    /**
     * This function will return an instance of a model which contains comments for
     * the entry passed to it. The model may be empty (if there are no comments for
     * the entry, which also covers situations where the entry's provider does not
     * support commenting)
     *
     * @param entry The entry to fetch comments for
     * @return A model which contains the comments for the specified entry
     * @since 5.63
     */
    CommentsModel *commentsForEntry(const KNSCore::Entry &entry);

    /**
     * If the same engine gets reused and the user could have used the delete functionality of the KCMs the cache could
     * be out of sync. If the RemoveDeadEntries option is set to true this will remove deleted entries from the cache
     * and the signalEntryChanged slot will be emitted with the updated entry
     * @since 5.74
     */
    Q_INVOKABLE void revalidateCacheEntries();

    /**
     * Try to contact the author of the entry by email or showing their homepage.
     */
    void contactAuthor(const Entry &entry);

    /**
     * Adopt an entry using the adoption command. This will also take care of displaying error messages
     * @param entry Entry that should be adopted
     * @see signalErrorCode
     * @see signalEntryEvent
     * @since 5.77
     */
    Q_INVOKABLE void adoptEntry(const KNSCore::Entry &entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     *
     * @param entry Entry to be installed
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(const KNSCore::Entry &entry, int linkId = 1);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    void uninstall(const KNSCore::Entry &entry);

Q_SIGNALS:
    void signalEntriesLoaded(const KNSCore::Entry::List &entries);
    void signalUpdateableEntriesLoaded(const KNSCore::Entry::List &entries);

    // a new search result is there, clear the list of items
    void signalResetView();

    void signalEntryPreviewLoaded(const KNSCore::Entry &, KNSCore::Entry::PreviewType);
    void signalPreviewFailed();

    void signalEntryUploadFinished();
    void signalEntryUploadFailed();

    void signalDownloadDialogDone(KNSCore::Entry::List);
    void jobStarted(KJob *, const QString &);
    /**
     * This is fired for any event related directly to a single Entry instance
     * @see Entry::EntryEvent for details on which specific event is being notified
     * @since 5.77
     */
    void signalEntryEvent(const Entry &entry, Entry::EntryEvent event);

private Q_SLOTS:
    KNEWSTUFFCORE_NO_EXPORT void slotEntriesLoaded(const KNSCore::Provider::SearchRequest &, KNSCore::Entry::List);
    KNEWSTUFFCORE_NO_EXPORT void slotEntryDetailsLoaded(const KNSCore::Entry &entry);
    KNEWSTUFFCORE_NO_EXPORT void slotPreviewLoaded(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type);

    KNEWSTUFFCORE_NO_EXPORT void slotSearchTimerExpired();

    KNEWSTUFFCORE_NO_EXPORT void slotInstallationFinished();
    KNEWSTUFFCORE_NO_EXPORT void slotInstallationFailed(const QString &message);

    KNEWSTUFFCORE_NO_EXPORT void providerJobStarted(KJob *);
    KNEWSTUFFCORE_NO_EXPORT void slotEntryChanged(const KNSCore::Entry &entry);

private:
    void addProvider(QSharedPointer<KNSCore::Provider> provider) override;
    void updateStatus() override;
    void registerTransaction(Transaction *transactions);

    void doRequest();

    const std::unique_ptr<EnginePrivate> d;

    Q_DISABLE_COPY(Engine)
};

}

#endif
