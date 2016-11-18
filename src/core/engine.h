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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QSharedPointer>

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

namespace KNSCore
{
class Cache;
class Installation;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 *
 * @internal
 */
class KNEWSTUFFCORE_EXPORT Engine : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit Engine(QObject *parent = 0);

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
     * @return Whether or not installation was started successfully
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

    void loadPreview(const KNSCore::EntryInternal &entry, EntryInternal::PreviewType type);
    void loadDetails(const KNSCore::EntryInternal &entry);

    void setSortMode(Provider::SortMode mode);
    void setFilter(Provider::Filter filter);

    /**
      Set the categories that will be included in searches
      */
    void setCategoriesFilter(const QStringList &categories);
    void setSearchTerm(const QString &searchString);
    void reloadEntries();
    void requestMoreData();
    void requestData(int page, int pageSize);

    void checkForUpdates();
    void checkForInstalled();

    void fetchEntryById(const QString &id);

    /**
     * Try to contact the author of the entry by email or showing their homepage.
     */
    void contactAuthor(const EntryInternal &entry);

    bool userCanVote(const EntryInternal &entry);
    void vote(const EntryInternal &entry, uint rating);
    bool userCanBecomeFan(const EntryInternal &entry);
    void becomeFan(const EntryInternal &entry);

    QStringList categories() const;
    QStringList categoriesFilter() const;

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

    // the name of the app that uses hot new stuff
    QString m_applicationName;

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
