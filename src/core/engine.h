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
#include <QMap>
#include <QSharedPointer>

#include "provider.h"
#include "entryinternal.h"

#include "knewstuffcore_export.h"

class QTimer;
class KJob;

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
     * an external tool is called on the installed file represented by %d.
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

    void signalError(const QString &);
    void signalBusy(const QString &);
    void signalIdle(const QString &);

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
    Attica::ProviderManager *m_atticaProviderManager;

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
