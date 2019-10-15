/*
    knewstuff3/engine.h.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

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

#ifndef KNEWSTUFF3_ENGINE_P_H
#define KNEWSTUFF3_ENGINE_P_H

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QHash>

#include "provider.h"
#include "entryinternal.h"
#include "errorcode.h"

#include "knewstuffcore_export.h"

class QTimer;
class KJob;
class EnginePrivate;

namespace Attica
{
class ProviderManager;
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
class Installation;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 */
class KNEWSTUFFCORE_EXPORT Engine : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit Engine(QObject *parent = nullptr);

    /**
     * Destructor. Frees up all the memory again which might be taken
     * by cached entries and providers.
     */
    ~Engine();

    /**
     * Initializes the engine. This step is application-specific and relies
     * on an external configuration file, which determines all the details
     * about the initialization.
     *
     * @param configfile KNewStuff2 configuration file (*.knsrc)
     * @return \b true if any valid configuration was found, \b false otherwise
     */
    bool init(const QString &configfile);

    /**
     * The name as defined by the knsrc file
     * @return The name associated with the engine's configuration file
     * @since 5.63
     */
    QString name() const;

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
    void install(KNSCore::EntryInternal entry, int linkId = 1);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    void uninstall(KNSCore::EntryInternal entry);

    /**
     * Attempt to load a specific preview for the specified entry.
     *
     * @param entry The entry to fetch a preview for
     * @param type The particular preview to fetch
     *
     * @see signalEntryPreviewLoaded(KNSCore::EntryInternal, KNSCore::EntryInternal::PreviewType);
     * @see signalPreviewFailed();
     */
    void loadPreview(const KNSCore::EntryInternal &entry, EntryInternal::PreviewType type);
    /**
     * Get the full details of a specific entry
     *
     * @param entry The entry to get full details for
     *
     * @see Entry::signalEntryDetailsLoaded(KNSCore::EntryInternal)
     */
    void loadDetails(const KNSCore::EntryInternal &entry);

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
     * Set a filter for results, which filters out all entries which do not match
     * the filter, as applied to the tags for the entry. This filters only on the
     * tags specified for the entry itself. To filter the downloadlinks, use
     * setDownloadTagFilter(QStringList).
     *
     * @note The default filter if one is not set from your knsrc file will filter
     * out entries marked as ghns_excluded=1. To retain this when setting a custom
     * filter, add "ghns_excluded!=1" as one of the filters.
     *
     * @note Some tags provided by OCS do not supply a value (and are simply passed
     * as a key). These will be interpreted as having the value 1 for filtering
     * purposes. An example of this might be ghns_excluded, which in reality will
     * generally be passed through ocs as "ghns_excluded" rather than "ghns_excluded=1"
     *
     * @note As tags are metadata, they are provided in the form of adjectives. They
     * are never supplied as action verbs or instructions (as an example, a good tag
     * to suggest that for example a wallpaper is painted would be "painted" as opposed
     * to "paint", and another example might be that an item should be "excluded" as
     * opposed to "exclude").
     *
     * == Examples of use ==
     * Value for tag "tagname" must be exactly "tagdata":
     * tagname==tagdata
     *
     * Value for tag "tagname" must be different from "tagdata":
     * tagname!=tagdata
     *
     * == KNSRC entry ==
     * A tag filter line in a .knsrc file, which is a comma semarated list of
     * tag/value pairs, might look like:
     *
     * TagFilter=ghns_excluded!=1,data##mimetype==application/cbr+zip,data##mimetype==application/cbr+rar
     * which would honour the exclusion and filter out anything that does not
     * include a comic book archive in either zip or rar format in one or more
     * of the download items.
     * Notice in particular that there are two data##mimetype entries. Use this
     * for when a tag may have multiple values.
     *
     * TagFilter=application##architecture==x86_64
     * which would not honour the exclusion, and would filter out all entries
     * which do not mark themselves as having a 64bit application binary in at
     * least one download item.
     *
     * The value does not current suppport wildcards. The list should be considered
     * a binary AND operation (that is, all filter entries must match for the data
     * entry to be included in the return data)
     *
     * @param filter The filter in the form of a list of strings
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void setTagFilter(const QStringList &filter);
    /**
     * Gets the current tag filter list
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    QStringList tagFilter() const;
    /**
     * Add a single filter entry to the entry tag filter. The filter should be in
     * the same form as the filter lines in the list used by setTagFilter(QStringList)
     * @param filter The filter in the form of a string
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    void addTagFilter(const QString &filter);
    /**
     * Sets a filter to be applied to the downloads for an entry. The logic is the
     * same as used in setTagFilter(QStringList), but vitally, only one downloadlink
     * is required to match the filter for the list to be valid. If you do not wish
     * to show the others in your client, you must hide them yourself.
     *
     * For an entry to be accepted when a download tag filter is set, it must also
     * be accepted by the entry filter (so, for example, while a list of downloads
     * might be accepted, if the entry has ghns_excluded set, and the default entry
     * filter is set, the entry will still be filtered out).
     *
     * In your knsrc file, set DownloadTagFilter to the filter you wish to apply,
     * using the same logic as described for the entry tagfilter.
     *
     * @param filter The filter in the form of a list of strings
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    void setDownloadTagFilter(const QStringList &filter);
    /**
     * Gets the current downloadlink tag filter list
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    QStringList downloadTagFilter() const;
    /**
     * Add a single filter entry to the download tag filter. The filter should be in
     * the same form as the filter lines in the list used by setDownloadsTagFilter(QStringList)
     * @param filter The filter in the form of a string
     * @see setTagFilter(QStringList)
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void addDownloadTagFilter(const QString &filter);

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
     * @note it will reset the engine state
     *
     * @param id The ID of the entry you wish to fetch
     */
    void fetchEntryById(const QString &id);

    /**
     * Try to contact the author of the entry by email or showing their homepage.
     */
    void contactAuthor(const EntryInternal &entry);

    /**
     * Whether or not a user is able to vote on the passed entry.
     *
     * @param entry The entry to check votability on
     * @return True if the user is able to vote on the entry
     */
    bool userCanVote(const EntryInternal &entry);
    /**
     * Cast a vote on the passed entry.
     *
     * @param entry The entry to vote on
     * @param rating A number from 0 to 100, 50 being neutral, 0 being most negative and 100 being most positive.
     */
    void vote(const EntryInternal &entry, uint rating);

    /**
     * Whether or not the user is allowed to become a fan of
     * a particular entry.
     * Not all providers (and consequently entries) support the fan functionality
     * and you can use this function to determine this ability.
     * @param entry The entry the user might wish to be a fan of
     * @return Whether or not it is possible for the user to become a fan of that entry
     */
    bool userCanBecomeFan(const EntryInternal &entry);
    /**
     * This will mark the user who is currently authenticated as a fan
     * of the entry passed to the function.
     * @param entry The entry the user wants to be a fan of
     */
    void becomeFan(const EntryInternal &entry);
    // FIXME There is currently no exposed API to remove the fan status

    /**
     * The list of the server-side names of the categories handled by this
     * engine instance. This corresponds directly to the list of categories
     * in your knsrc file. This is not supposed to be used as user-facing
     * strings - @see categoriesMetadata() for that.
     *
     * @return The categories which this instance of Engine handles
     */
    QStringList categories() const;
    /**
     * The list of categories searches will actually show results from. This
     * is a subset of the categories() list.
     *
     * @see KNSCore::Engine::setCategoriesFilter(QString)
     */
    QStringList categoriesFilter() const;

    /**
     * The list of metadata for the categories handled by this engine instance.
     * If you wish to show the categories to the user, this is the data to use.
     * The category name is the string used to set categories for the filter,
     * and also what is returned by both categories() and categoriesFilter().
     * The human-readable name is displayName, and the only thing which should
     * be shown to the user.
     *
     * @return The metadata for all categories handled by this engine
     */
    QList<Provider::CategoryMetadata> categoriesMetadata();

    /**
     * The adoption command can be used to allow a user to make use of an entry's
     * installed data. For example, this command might be used to ask the system to
     * switch to a wallpaper or icon theme which was installed with KNS.
     *
     * The following is how this might look in a knsrc file. The example shows how
     * an external tool is called on the directory containing the installed file
     * represented by %d. If you wish to directly point to the installed file, the
     * substitution variable is %f.
     * <pre>
       AdoptionCommand=/usr/lib64/libexec/plasma-changeicons %d
     * </pre>
     *
     * @param entry The entry to return an adoption command for
     * @return The command to run to adopt this entry's installed data
     */
    QString adoptionCommand(const KNSCore::EntryInternal &entry) const;
    /**
     * Whether or not an adoption command exists for this engine
     *
     * @see adoptionCommand(KNSCore::EntryInternal)
     * @return True if an adoption command exists
     */
    bool hasAdoptionCommand() const;

    /**
     * Set the page size for requests not made explicitly with requestData(int,int)
     * @param pageSize the default number of entries to request from the provider
     * @see requestData(int,int)
     */
    void setPageSize(int pageSize);

    /**
     * Get a list of all the locations which will be used when searching for knsrc
     * files, in the order in which the search will occur.
     *
     * @param includeFallbackLocations Whether or not the deprecated search locations are included
     * @return The search list for knsrc files
     * @since 5.57
     */
    static QStringList configSearchLocations(bool includeFallbackLocations = false);
    /**
     * Sets whether or not the config file location discovery fallback should be active.
     * If enabled (default), if the config file is not found in the knsrcfiles location,
     * then the engine will also look in the systemwide config location (usually /etc/xdg
     * on linux). If disabled, this fallback location will not be searched.
     *
     * @param enableFallback Whether or not the fallback discovery should be enabled
     * @since 5.57
     */
    void setConfigLocationFallback(bool enableFallback);

    /**
     * The Provider instance with the passed ID
     *
     * @param providerId The ID of the Provider to fetch
     * @return The Provider with the passed ID, or null if non such Provider exists
     * @since 5.63
     */
    QSharedPointer<Provider> provider(const QString &providerId) const;

    /**
     * Return the first provider in the providers list (usually the default provider)
     * @return The first Provider (or null if the engine is not initialized)
     * @since 5.63
     */
    QSharedPointer<Provider> defaultProvider() const;

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
    CommentsModel *commentsForEntry(const KNSCore::EntryInternal &entry);

Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString &message);

    void signalProvidersLoaded();
    void signalEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void signalUpdateableEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void signalEntryChanged(const KNSCore::EntryInternal &entry);
    void signalEntryDetailsLoaded(const KNSCore::EntryInternal &entry);

    // a new search result is there, clear the list of items
    void signalResetView();

    void signalEntryPreviewLoaded(const KNSCore::EntryInternal &, KNSCore::EntryInternal::PreviewType);
    void signalPreviewFailed();

    void signalEntryUploadFinished();
    void signalEntryUploadFailed();

    void signalDownloadDialogDone(KNSCore::EntryInternal::List);
    void jobStarted(KJob *, const QString &);

#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(5, 53)
    KNEWSTUFFCORE_DEPRECATED_VERSION(5, 53, "Use Engine::signalErrorCode(const KNSCore::ErrorCode &, const QString &, const QVariant &)")
    void signalError(const QString &);
#endif
    void signalBusy(const QString &);
    void signalIdle(const QString &);
    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::EntryInternal::ErrorCode for the metadata details)
     * @see KNSCore::EntryInternal::ErrorCode
     * @since 5.53
     */
    void signalErrorCode(const KNSCore::ErrorCode &errorCode, const QString &message, const QVariant &metadata);

    void signalCategoriesMetadataLoded(const QList<Provider::CategoryMetadata> &categories);

private Q_SLOTS:
    // the .knsrc file was loaded
    void slotProviderFileLoaded(const QDomDocument &doc);
    // instead of getting providers from knsrc, use what was configured in ocs systemsettings
    void atticaProviderLoaded(const Attica::Provider &provider);

    // loading the .knsrc file failed
    void slotProvidersFailed();

    // called when a provider is ready to work
    void providerInitialized(KNSCore::Provider *);

    void slotEntriesLoaded(const KNSCore::Provider::SearchRequest &, KNSCore::EntryInternal::List);
    void slotEntryDetailsLoaded(const KNSCore::EntryInternal &entry);
    void slotPreviewLoaded(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::PreviewType type);

    void slotSearchTimerExpired();

    void slotEntryChanged(const KNSCore::EntryInternal &entry);
    void slotInstallationFinished();
    void slotInstallationFailed(const QString &message);
    void downloadLinkLoaded(const KNSCore::EntryInternal &entry);

    void providerJobStarted(KJob *);

private:
    /**
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();

    /**
      Add a provider and connect it to the right slots
     */
    void addProvider(QSharedPointer<KNSCore::Provider> provider);

    void updateStatus();

    void doRequest();

    //FIXME KF6: move all of this in EnginePrivate
    // handle installation of entries
    Installation *m_installation;
    // read/write cache of entries
    QSharedPointer<Cache> m_cache;
    QTimer *m_searchTimer;
    // The url of the file containing information about content providers
    QString m_providerFileUrl;
    // Categories from knsrc file
    QStringList m_categories;

    QHash<QString, QSharedPointer<KNSCore::Provider> > m_providers;

    QString m_adoptionCommand;

    // the current request from providers
    Provider::SearchRequest m_currentRequest;

    EnginePrivate * const d;

    // the page that is currently displayed, so it is not requested repeatedly
    int m_currentPage;

    // when requesting entries from a provider, how many to ask for
    int m_pageSize;

    int m_numDataJobs;
    int m_numPictureJobs;
    int m_numInstallJobs;
    // If the provider is ready to be used
    bool m_initialized;

    Q_DISABLE_COPY(Engine)
};

}

#endif
