/*
    knewstuff3/engine.cpp
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "engine.h"

#include "commentsmodel.h"
#include "installation.h"
#include "question.h"
#include "xmlloader.h"
#include "imageloader_p.h"

#include <memory>
#include <KConfig>
#include <KConfigGroup>
#include <knewstuffcore_debug.h>
#include <KLocalizedString>
#include <KShell>
#include <QDesktopServices>

#include <QTimer>
#include <QProcess>
#include <QDir>
#include <qdom.h>
#include <QUrlQuery>
#include <QThreadStorage>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shlobj.h>
#endif

// libattica
#include <attica/providermanager.h>
#include <qstandardpaths.h>

// own
#include "../attica/atticaprovider_p.h"
#include "cache.h"
#include "../staticxml/staticxmlprovider_p.h"

using namespace KNSCore;

typedef QHash<QString, XmlLoader*> EngineProviderLoaderHash;
Q_GLOBAL_STATIC(QThreadStorage<EngineProviderLoaderHash>, s_engineProviderLoaders)

class EnginePrivate {
public:
    QString getAdoptionCommand(const QString &command, const KNSCore::EntryInternal& entry, Installation *inst)
    {
        auto adoption = command;
        if(adoption.isEmpty())
            return {};

        const QLatin1String dirReplace("%d");
        if (adoption.contains(dirReplace)) {
            QString installPath = sharedDir(entry.installedFiles(), inst->targetInstallationPath()).path();
            adoption.replace(dirReplace, KShell::quoteArg(installPath));
        }

        const QLatin1String fileReplace("%f");
        if (adoption.contains(fileReplace)) {
            if (entry.installedFiles().isEmpty()) {
                qCWarning(KNEWSTUFFCORE) << "no installed files to adopt";
                return {};
            } else if (entry.installedFiles().count() != 1) {
                qCWarning(KNEWSTUFFCORE) << "can only adopt one file, will be using the first" << entry.installedFiles().at(0);
            }

            adoption.replace(fileReplace, KShell::quoteArg(entry.installedFiles().at(0)));
        }
        return adoption;
    }
    /**
     * we look for the directory where all the resources got installed.
     * assuming it was extracted into a directory
     */
    static QDir sharedDir(QStringList dirs, QString rootPath)
    {
        // Ensure that rootPath definitely is a clean path with a slash at the end
        rootPath = QDir::cleanPath(rootPath) + QStringLiteral("/");
        qCInfo(KNEWSTUFFCORE) << Q_FUNC_INFO << dirs << rootPath;
        while(!dirs.isEmpty()) {
            QString thisDir(dirs.takeLast());
            if (thisDir.endsWith(QStringLiteral("*"))) {
                qCInfo(KNEWSTUFFCORE) << "Directory entry" << thisDir << "ends in a *, indicating this was installed from an archive - see Installation::archiveEntries";
                thisDir.chop(1);
            }

            const QString currentPath = QDir::cleanPath(thisDir);
            qCInfo(KNEWSTUFFCORE) << "Current path is" << currentPath;
            if (!currentPath.startsWith(rootPath)) {
                qCInfo(KNEWSTUFFCORE) << "Current path" << currentPath << "does not start with" << rootPath << "and should be ignored";
                continue;
            }

            const QFileInfo current(currentPath);
            qCInfo(KNEWSTUFFCORE) << "Current file info is" << current;
            if (!current.isDir()) {
                qCInfo(KNEWSTUFFCORE) << "Current path" << currentPath << "is not a directory, and should be ignored";
                continue;
            }

            const QDir dir(currentPath);
            if (dir.path()==(rootPath+dir.dirName())) {
                qCDebug(KNEWSTUFFCORE) << "Found directory" << dir;
                return dir;
            }
        }
        qCWarning(KNEWSTUFFCORE) << "Failed to locate any shared installed directory in" << dirs << "and this is almost certainly very bad.";
        return {};
    }

    QList<Provider::CategoryMetadata> categoriesMetadata;
    Attica::ProviderManager *m_atticaProviderManager = nullptr;
    QStringList tagFilter;
    QStringList downloadTagFilter;
    bool configLocationFallback = true;
    QString name;
    QMap<EntryInternal, CommentsModel*> commentsModels;
    bool shouldRemoveDeletedEntries = false;

    // Used for updating purposes - we ought to be saving this information, but we also have to deal with old stuff, and so... this will have to do for now, and so
    // TODO KF6: Installed state needs to move onto a per-downloadlink basis rather than per-entry
    QMap<EntryInternal, QStringList> payloads;
    QMap<EntryInternal, QString> payloadToIdentify;
    Engine::BusyState busyState;
    QString busyMessage;
    QString useLabel;
};

Engine::Engine(QObject *parent)
    : QObject(parent)
    , m_installation(new Installation)
    , m_cache()
    , m_searchTimer(new QTimer)
    , d(new EnginePrivate)
    , m_currentPage(-1)
    , m_pageSize(20)
    , m_numDataJobs(0)
    , m_numPictureJobs(0)
    , m_numInstallJobs(0)
    , m_initialized(false)
{
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(1000);
    connect(m_searchTimer, &QTimer::timeout, this, &Engine::slotSearchTimerExpired);
    connect(m_installation, &Installation::signalInstallationFinished, this, &Engine::slotInstallationFinished);
    connect(m_installation, &Installation::signalInstallationFailed, this, &Engine::slotInstallationFailed);
    connect(m_installation, &Installation::signalInstallationError, this, [this](const QString &message){ Q_EMIT signalErrorCode(ErrorCode::InstallationError, i18n("An error occurred during the installation process:\n%1", message), QVariant()); });
#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 53)
    // Pass along old error signal for compatibility
    connect(this, &Engine::signalErrorCode, this, [this] (const KNSCore::ErrorCode &, const QString &msg, const QVariant &) {
        Q_EMIT signalError(msg);
    });
#endif

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 77)
    connect(this, &Engine::signalEntryEvent, this,
            [this](const EntryInternal &entry, EntryInternal::EntryEvent event) {
        if (event == EntryInternal::StatusChangedEvent) {
            Q_EMIT signalEntryChanged(entry);
        } else if (event == EntryInternal::DetailsLoadedEvent){
            Q_EMIT signalEntryDetailsLoaded(entry);
        }
    });
#endif
}

Engine::~Engine()
{
    if (m_cache) {
        m_cache->writeRegistry();
    }
    delete d->m_atticaProviderManager;
    delete m_searchTimer;
    delete m_installation;
    delete d;
}

bool Engine::init(const QString &configfile)
{
    qCDebug(KNEWSTUFFCORE) << "Initializing KNSCore::Engine from '" << configfile << "'";

    setBusy(BusyOperation::Initializing, i18n("Initializing"));

    QScopedPointer<KConfig> conf;
    // TODO KF6: This is fallback logic for an old location for the knsrc files. This is deprecated in KF5 and should be removed in KF6
    bool isRelativeConfig = QFileInfo(configfile).isRelative();
    QString actualConfig;
    if (isRelativeConfig) {
        // Don't do the expensive search unless the config is relative
        actualConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile));
    }
    QString configFileName{configfile};
    if (isRelativeConfig && d->configLocationFallback && actualConfig.isEmpty()) {
        conf.reset(new KConfig(configfile));
        qCWarning(KNEWSTUFFCORE) << "Using a deprecated location for the knsrc file" << configfile << " - please contact the author of the software which provides this file to get it updated to use the new location";
    } else if (isRelativeConfig) {
        configFileName = QFileInfo(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile))).baseName();
        conf.reset(new KConfig(QStringLiteral("knsrcfiles/%1").arg(configfile), KConfig::FullConfig, QStandardPaths::GenericDataLocation));
    } else {
        configFileName = QFileInfo(configfile).baseName();
        conf.reset(new KConfig(configfile));
    }

    if (conf->accessMode() == KConfig::NoAccess) {
        Q_EMIT signalErrorCode(KNSCore::ConfigFileError, i18n("Configuration file exists, but cannot be opened: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file '" << configfile << "' was found but could not be opened.";
        return false;
    }

    KConfigGroup group;
    if (conf->hasGroup("KNewStuff3")) {
        qCDebug(KNEWSTUFFCORE) << "Loading KNewStuff3 config: " << configfile;
        group = conf->group("KNewStuff3");
    } else if (conf->hasGroup("KNewStuff2")) {
        qCDebug(KNEWSTUFFCORE) << "Loading KNewStuff2 config: " << configfile;
        group = conf->group("KNewStuff2");
    } else {
        Q_EMIT signalErrorCode(KNSCore::ConfigFileError, i18n("Configuration file is invalid: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << configfile << " doesn't contain a KNewStuff3 section.";
        return false;
    }

    d->name = group.readEntry("Name");
    m_categories = group.readEntry("Categories", QStringList());
    m_adoptionCommand = group.readEntry("AdoptionCommand");
    d->useLabel = group.readEntry("UseLabel", i18n("Use"));
    Q_EMIT useLabelChanged();

    qCDebug(KNEWSTUFFCORE) << "Categories: " << m_categories;
    m_providerFileUrl = group.readEntry("ProvidersUrl");
    if (m_providerFileUrl == QLatin1String("https://download.kde.org/ocs/providers.xml")) {
        m_providerFileUrl = QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml");
        qCWarning(KNEWSTUFFCORE) << "Please make sure" << configfile << "has ProvidersUrl=https://autoconfig.kde.org/ocs/providers.xml";
    }
    d->tagFilter = group.readEntry("TagFilter", QStringList(QStringLiteral("ghns_excluded!=1")));
    d->downloadTagFilter = group.readEntry("DownloadTagFilter", QStringList());

    // Make sure that config is valid
    if (!m_installation->readConfig(group)) {
        Q_EMIT signalErrorCode(ErrorCode::ConfigFileError,
                               i18n("Could not initialise the installation handler for %1\n"
                                "This is a critical error and should be reported to the application author", configfile),
                                configfile);
        return false;
    }

    connect(m_installation, &Installation::signalEntryChanged, this, &Engine::slotEntryChanged);

    m_cache = Cache::getCache(configFileName);
    qCDebug(KNEWSTUFFCORE) << "Cache is" << m_cache << "for" << configFileName;
    connect(this, &Engine::signalEntryEvent, m_cache.data(), [this] (const EntryInternal &entry, EntryInternal::EntryEvent event) {
        if (event == EntryInternal::StatusChangedEvent) {
            m_cache->registerChangedEntry(entry);
        }
    });
    connect(m_cache.data(), &Cache::entryChanged, this, &Engine::slotEntryChanged);
    m_cache->readRegistry();

    // Cache cleanup option, to help work around people deleting files from underneath KNewStuff (this
    // happens a lot with e.g. wallpapers and icons)
    if (m_installation->uncompressionSetting() == Installation::UseKPackageUncompression) {
        d->shouldRemoveDeletedEntries = true;
    }

    d->shouldRemoveDeletedEntries = group.readEntry("RemoveDeadEntries", d->shouldRemoveDeletedEntries);
    if (d->shouldRemoveDeletedEntries) {
        m_cache->removeDeletedEntries();
    }

    m_initialized = true;

    // load the providers
    loadProviders();

    return true;
}

QString KNSCore::Engine::name() const
{
    return d->name;
}

QStringList Engine::categories() const
{
    return m_categories;
}

QStringList Engine::categoriesFilter() const
{
    return m_currentRequest.categories;
}

QList<Provider::CategoryMetadata> Engine::categoriesMetadata()
{
    return d->categoriesMetadata;
}

void Engine::loadProviders()
{
    if (m_providerFileUrl.isEmpty()) {
        // it would be nicer to move the attica stuff into its own class
        qCDebug(KNEWSTUFFCORE) << "Using OCS default providers";
        delete d->m_atticaProviderManager;
        d->m_atticaProviderManager = new Attica::ProviderManager;
        connect(d->m_atticaProviderManager, &Attica::ProviderManager::providerAdded, this, &Engine::atticaProviderLoaded);
        connect(d->m_atticaProviderManager, &Attica::ProviderManager::failedToLoad, this, &Engine::slotProvidersFailed);
        d->m_atticaProviderManager->loadDefaultProviders();
    } else {
        qCDebug(KNEWSTUFFCORE) << "loading providers from " << m_providerFileUrl;
        setBusy(BusyOperation::LoadingData, i18n("Loading provider information"));

        XmlLoader *loader = s_engineProviderLoaders()->localData().value(m_providerFileUrl);
        if (!loader) {
            qCDebug(KNEWSTUFFCORE) << "No xml loader for this url yet, so create one and temporarily store that" << m_providerFileUrl;
            loader = new XmlLoader(this);
            s_engineProviderLoaders()->localData().insert(m_providerFileUrl, loader);
            connect(loader, &XmlLoader::signalLoaded, this, [this](){ s_engineProviderLoaders()->localData().remove(m_providerFileUrl); });
            connect(loader, &XmlLoader::signalFailed, this, [this](){ s_engineProviderLoaders()->localData().remove(m_providerFileUrl); });
            loader->load(QUrl(m_providerFileUrl));
        }
        connect(loader, &XmlLoader::signalLoaded, this, &Engine::slotProviderFileLoaded);
        connect(loader, &XmlLoader::signalFailed, this, &Engine::slotProvidersFailed);
    }
}

void Engine::slotProviderFileLoaded(const QDomDocument &doc)
{
    qCDebug(KNEWSTUFFCORE) << "slotProvidersLoaded";

    bool isAtticaProviderFile = false;

    // get each provider element, and create a provider object from it
    QDomElement providers = doc.documentElement();

    if (providers.tagName() == QLatin1String("providers")) {
        isAtticaProviderFile = true;
    } else if (providers.tagName() != QLatin1String("ghnsproviders") && providers.tagName() != QLatin1String("knewstuffproviders")) {
        qWarning() << "No document in providers.xml.";
        Q_EMIT signalErrorCode(KNSCore::ProviderError, i18n("Could not load get hot new stuff providers from file: %1", m_providerFileUrl), m_providerFileUrl);
        return;
    }

    QDomElement n = providers.firstChildElement(QStringLiteral("provider"));
    while (!n.isNull()) {
        qCDebug(KNEWSTUFFCORE) << "Provider attributes: " << n.attribute(QStringLiteral("type"));

        QSharedPointer<KNSCore::Provider> provider;
        if (isAtticaProviderFile || n.attribute(QStringLiteral("type")).toLower() == QLatin1String("rest")) {
            provider.reset(new AtticaProvider(m_categories, d->name));
            connect(provider.data(), &Provider::categoriesMetadataLoded,
                    this, [this](const QList<Provider::CategoryMetadata> &categories){
                        d->categoriesMetadata = categories;
                        Q_EMIT signalCategoriesMetadataLoded(categories);
                    });
        } else {
            provider.reset(new StaticXmlProvider);
        }

        if (provider->setProviderXML(n)) {
            addProvider(provider);
        } else {
            Q_EMIT signalErrorCode(KNSCore::ProviderError, i18n("Error initializing provider."), m_providerFileUrl);
        }
        n = n.nextSiblingElement();
    }
    setBusy(BusyOperation::LoadingData, i18n("Loading data"));
}

void Engine::atticaProviderLoaded(const Attica::Provider &atticaProvider)
{
    qCDebug(KNEWSTUFFCORE) << "atticaProviderLoaded called";
    if (!atticaProvider.hasContentService()) {
        qCDebug(KNEWSTUFFCORE) << "Found provider: " << atticaProvider.baseUrl() << " but it does not support content";
        return;
    }
    QSharedPointer<KNSCore::Provider> provider =
        QSharedPointer<KNSCore::Provider> (new AtticaProvider(atticaProvider, m_categories, d->name));
    connect(provider.data(), &Provider::categoriesMetadataLoded,
            this, [this](const QList<Provider::CategoryMetadata> &categories){
                d->categoriesMetadata = categories;
                Q_EMIT signalCategoriesMetadataLoded(categories);
            });
    addProvider(provider);
}

void Engine::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    qCDebug(KNEWSTUFFCORE) << "Engine addProvider called with provider with id " << provider->id();
    m_providers.insert(provider->id(), provider);
    provider->setTagFilter(d->tagFilter);
    provider->setDownloadTagFilter(d->downloadTagFilter);
    connect(provider.data(), &Provider::providerInitialized, this, &Engine::providerInitialized);
    connect(provider.data(), &Provider::loadingFinished, this, &Engine::slotEntriesLoaded);
    connect(provider.data(), &Provider::entryDetailsLoaded, this, &Engine::slotEntryDetailsLoaded);
    connect(provider.data(), &Provider::payloadLinkLoaded, this, &Engine::downloadLinkLoaded);

    connect(provider.data(), &Provider::signalError, this, [this, provider](const QString &msg) {
        Q_EMIT signalErrorCode(ErrorCode::ProviderError, msg, m_providerFileUrl);
    });
    connect(provider.data(), &Provider::signalErrorCode, this, &Engine::signalErrorCode);
    connect(provider.data(), &Provider::signalInformation, this, [this](const QString &message) {
        Q_EMIT signalMessage(message);
    });
}

void Engine::providerJobStarted(KJob *job)
{
    Q_EMIT jobStarted(job, i18n("Loading data from provider"));
}

void Engine::slotProvidersFailed()
{
    Q_EMIT signalErrorCode(KNSCore::ProviderError, i18n("Loading of providers from file: %1 failed", m_providerFileUrl), m_providerFileUrl);
}

void Engine::providerInitialized(Provider *p)
{
    qCDebug(KNEWSTUFFCORE) << "providerInitialized" << p->name();
    p->setCachedEntries(m_cache->registryForProvider(p->id()));
    updateStatus();

    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        if (!p->isInitialized()) {
            return;
        }
    }
    Q_EMIT signalProvidersLoaded();
}

void Engine::slotEntriesLoaded(const KNSCore::Provider::SearchRequest &request, KNSCore::EntryInternal::List entries)
{
    m_currentPage = qMax<int>(request.page, m_currentPage);
    qCDebug(KNEWSTUFFCORE) << "loaded page " << request.page << "current page" << m_currentPage << "count:" << entries.count();

    if (request.filter == Provider::Updates) {
        Q_EMIT signalUpdateableEntriesLoaded(entries);
    } else {
        m_cache->insertRequest(request, entries);
        Q_EMIT signalEntriesLoaded(entries);
    }

    --m_numDataJobs;
    updateStatus();
}

void Engine::reloadEntries()
{
    Q_EMIT signalResetView();
    m_currentPage = -1;
    m_currentRequest.pageSize = m_pageSize;
    m_currentRequest.page = 0;
    m_numDataJobs = 0;

    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        if (p->isInitialized()) {
            if (m_currentRequest.filter == Provider::Installed) {
                // when asking for installed entries, never use the cache
                p->loadEntries(m_currentRequest);
            } else {
                // take entries from cache until there are no more
                EntryInternal::List cache;
                EntryInternal::List lastCache = m_cache->requestFromCache(m_currentRequest);
                while (!lastCache.isEmpty()) {
                    qCDebug(KNEWSTUFFCORE) << "From cache";
                    cache << lastCache;

                    m_currentPage = m_currentRequest.page;
                    ++m_currentRequest.page;
                    lastCache = m_cache->requestFromCache(m_currentRequest);
                }

                // Since the cache has no more pages, reset the request's page
                if (m_currentPage >= 0) {
                    m_currentRequest.page = m_currentPage;
                }

                if (!cache.isEmpty()) {
                    Q_EMIT signalEntriesLoaded(cache);
                } else {
                    qCDebug(KNEWSTUFFCORE) << "From provider";
                    p->loadEntries(m_currentRequest);

                    ++m_numDataJobs;
                    updateStatus();
                }
            }
        }
    }
}

void Engine::setCategoriesFilter(const QStringList &categories)
{
    m_currentRequest.categories = categories;
    reloadEntries();
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (m_currentRequest.sortMode != mode) {
        m_currentRequest.page = -1;
    }
    m_currentRequest.sortMode = mode;
    reloadEntries();
}

Provider::SortMode KNSCore::Engine::sortMode() const
{
    return m_currentRequest.sortMode;
}

void KNSCore::Engine::setFilter(Provider::Filter filter)
{
    if (m_currentRequest.filter != filter) {
        m_currentRequest.page = -1;
    }
    m_currentRequest.filter = filter;
    reloadEntries();
}

Provider::Filter KNSCore::Engine::filter() const
{
    return m_currentRequest.filter;
}

void KNSCore::Engine::fetchEntryById(const QString &id)
{
    m_searchTimer->stop();
    m_currentRequest = KNSCore::Provider::SearchRequest(KNSCore::Provider::Newest, KNSCore::Provider::ExactEntryId, id);
    m_currentRequest.pageSize = m_pageSize;

    EntryInternal::List cache = m_cache->requestFromCache(m_currentRequest);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        m_searchTimer->start();
    }
}

void Engine::setSearchTerm(const QString &searchString)
{
    m_searchTimer->stop();
    m_currentRequest.searchTerm = searchString;
    EntryInternal::List cache = m_cache->requestFromCache(m_currentRequest);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        m_searchTimer->start();
    }
}

QString KNSCore::Engine::searchTerm() const
{
    return m_currentRequest.searchTerm;
}

void Engine::setTagFilter(const QStringList &filter)
{
    d->tagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        p->setTagFilter(d->tagFilter);
    }
}

QStringList Engine::tagFilter() const
{
    return d->tagFilter;
}

void KNSCore::Engine::addTagFilter(const QString &filter)
{
    d->tagFilter << filter;
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        p->setTagFilter(d->tagFilter);
    }
}

void Engine::setDownloadTagFilter(const QStringList &filter)
{
    d->downloadTagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        p->setDownloadTagFilter(d->downloadTagFilter);
    }
}

QStringList Engine::downloadTagFilter() const
{
    return d->downloadTagFilter;
}

void Engine::addDownloadTagFilter(const QString &filter)
{
    d->downloadTagFilter << filter;
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        p->setDownloadTagFilter(d->downloadTagFilter);
    }
}

void Engine::slotSearchTimerExpired()
{
    reloadEntries();
}

void Engine::requestMoreData()
{
    qCDebug(KNEWSTUFFCORE) << "Get more data! current page: " << m_currentPage  << " requested: " << m_currentRequest.page;

    if (m_currentPage < m_currentRequest.page) {
        return;
    }

    m_currentRequest.page++;
    doRequest();
}

void Engine::requestData(int page, int pageSize)
{
    m_currentRequest.page = page;
    m_currentRequest.pageSize = pageSize;
    doRequest();
}

void Engine::doRequest()
{
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        if (p->isInitialized()) {
            p->loadEntries(m_currentRequest);
            ++m_numDataJobs;
            updateStatus();
        }
    }
}

void Engine::install(KNSCore::EntryInternal entry, int linkId)
{
    if (entry.status() == KNS3::Entry::Updateable) {
        entry.setStatus(KNS3::Entry::Updating);
    } else  {
        entry.setStatus(KNS3::Entry::Installing);
    }
    Q_EMIT signalEntryEvent(entry, EntryInternal::StatusChangedEvent);

    qCDebug(KNEWSTUFFCORE) << "Install " << entry.name()
       << " from: " << entry.providerId();
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    if (p) {
        // If linkId is -1, assume that it's an update and that we don't know what to update
        if (entry.status() == KNS3::Entry::Updating && linkId == -1) {
            if (entry.downloadLinkCount() == 1) {
                // If there is only one downloadable item, then we can fairly safely assume that's what we're wanting
                // to update, meaning we can bypass some of the more expensive operations in downloadLinkLoaded
                qCDebug(KNEWSTUFFCORE) << "Just the one download link, so let's use that";
                d->payloadToIdentify[entry] = QString{};
                linkId = 1;
            } else {
                qCDebug(KNEWSTUFFCORE) << "Try and identify a download link to use from a total of" << entry.downloadLinkCount();
                // While this seems silly, the payload gets reset when fetching the new download link information
                d->payloadToIdentify[entry] = entry.payload();
                // Drop a fresh list in place so we've got something to work with when we get the links
                d->payloads[entry] = QStringList{};
                linkId = 1;
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Link ID already known" << linkId;
            // If there is no payload to identify, we will assume the payload is already known and just use that
            d->payloadToIdentify[entry] = QString{};
        }

        p->loadPayloadLink(entry, linkId);

        ++m_numInstallJobs;
        updateStatus();
    }
}

void Engine::slotInstallationFinished()
{
    --m_numInstallJobs;
    updateStatus();
}

void Engine::slotInstallationFailed(const QString &message)
{
    --m_numInstallJobs;
    Q_EMIT signalErrorCode(KNSCore::InstallationError, message, QVariant());
}

void Engine::slotEntryDetailsLoaded(const KNSCore::EntryInternal &entry)
{
    Q_EMIT signalEntryEvent(entry, EntryInternal::DetailsLoadedEvent);
}

void Engine::downloadLinkLoaded(const KNSCore::EntryInternal &entry)
{
    if (entry.status() == KNS3::Entry::Updating) {
        if (d->payloadToIdentify.isEmpty()) {
            // If there's nothing to identify, and we've arrived here, then we know what the payload is
            qCDebug(KNEWSTUFFCORE) << "If there's nothing to identify, and we've arrived here, then we know what the payload is";
            m_installation->install(entry);
        } else if (d->payloads[entry].count() < entry.downloadLinkCount()) {
            // We've got more to get before we can attempt to identify anything, so fetch the next one...
            qCDebug(KNEWSTUFFCORE) << "We've got more to get before we can attempt to identify anything, so fetch the next one...";
            QStringList payloads = d->payloads[entry];
            payloads << entry.payload();
            d->payloads[entry] = payloads;
            QSharedPointer<Provider> p = m_providers.value(entry.providerId());
            if (p) {
                // ok, so this should definitely always work, but... safety first, kids!
                p->loadPayloadLink(entry, payloads.count());
            }
        } else {
            // We now have all the links, so let's try and identify the correct one...
            qCDebug(KNEWSTUFFCORE) << "We now have all the links, so let's try and identify the correct one...";
            QString identifiedLink;
            const QString payloadToIdentify = d->payloadToIdentify[entry];
            const QList<EntryInternal::DownloadLinkInformation> downloadLinks = entry.downloadLinkInformationList();
            const QStringList &payloads = d->payloads[entry];

            if (payloads.contains(payloadToIdentify)) {
                // Simplest option, the link hasn't changed at all
                qCDebug(KNEWSTUFFCORE) << "Simplest option, the link hasn't changed at all";
                identifiedLink = payloadToIdentify;
            } else {
                // Next simplest option, filename is the same but in a different folder
                qCDebug(KNEWSTUFFCORE) << "Next simplest option, filename is the same but in a different folder";
                const QStringRef fileName = payloadToIdentify.splitRef(QChar::fromLatin1('/')).last();
                for (const QString &payload : payloads) {
                    if (payload.endsWith(fileName)) {
                        identifiedLink = payload;
                        break;
                    }
                }

                // Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...
                qCDebug(KNEWSTUFFCORE) << "Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...";
                QStringList payloadNames;
                for (const EntryInternal::DownloadLinkInformation &downloadLink : downloadLinks) {
                    qCDebug(KNEWSTUFFCORE) << "Download link" << downloadLink.name << downloadLink.id << downloadLink.size << downloadLink.descriptionLink;
                    payloadNames << downloadLink.name;
                    if (downloadLink.name == fileName) {
                        identifiedLink = payloads[payloadNames.count() - 1];
                        qCDebug(KNEWSTUFFCORE) << "Found a suitable download link for" << fileName << "which should match" << identifiedLink;
                    }
                }

                if (identifiedLink.isEmpty()) {
                    // Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)
                    qCDebug(KNEWSTUFFCORE) << "Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)";
                    auto question = std::make_unique<Question>(Question::SelectFromListQuestion);
                    question->setTitle(i18n("Pick Update Item"));
                    question->setQuestion(i18n("Please pick the item from the list below which should be used to apply this update. We were unable to identify which item to select, based on the original item, which was named %1", fileName.toString()));
                    question->setList(payloadNames);
                    if(question->ask() == Question::OKResponse) {
                        identifiedLink = payloads.value(payloadNames.indexOf(question->response()));
                    }
                }
            }
            if (!identifiedLink.isEmpty()) {
                KNSCore::EntryInternal theEntry(entry);
                theEntry.setPayload(identifiedLink);
                m_installation->install(theEntry);
            } else {
                qCWarning(KNEWSTUFFCORE) << "We failed to identify a good link for updating" << entry.name() << "and are unable to perform the update";
            }
            // As the serverside data may change before next time this is called, even in the same session,
            // let's not make assumptions, and just get rid of this
            d->payloads.remove(entry);
            d->payloadToIdentify.remove(entry);
        }
    } else {
        m_installation->install(entry);
    }
}

void Engine::uninstall(KNSCore::EntryInternal entry)
{
    const KNSCore::EntryInternal::List list = m_cache->registryForProvider(entry.providerId());
    //we have to use the cached entry here, not the entry from the provider
    //since that does not contain the list of installed files
    KNSCore::EntryInternal actualEntryForUninstall;
    for (const KNSCore::EntryInternal &eInt : list) {
        if (eInt.uniqueId() == entry.uniqueId()) {
            actualEntryForUninstall = eInt;
            break;
        }
    }
    if (!actualEntryForUninstall.isValid()) {
        qCDebug(KNEWSTUFFCORE) << "could not find a cached entry with following id:" << entry.uniqueId() <<
                 " ->  using the non-cached version";
        actualEntryForUninstall = entry;
    }

    entry.setStatus(KNS3::Entry::Installing);
    actualEntryForUninstall.setStatus(KNS3::Entry::Installing);
    Q_EMIT signalEntryEvent(entry, EntryInternal::StatusChangedEvent);

    qCDebug(KNEWSTUFFCORE) << "about to uninstall entry " << entry.uniqueId();
    m_installation->uninstall(actualEntryForUninstall);

    entry.setStatus(actualEntryForUninstall.status());
    Q_EMIT signalEntryEvent(entry, EntryInternal::StatusChangedEvent);
}

void Engine::loadDetails(const KNSCore::EntryInternal &entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->loadEntryDetails(entry);
}

void Engine::loadPreview(const KNSCore::EntryInternal &entry, EntryInternal::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "START  preview: " << entry.name() << type;
    ImageLoader *l = new ImageLoader(entry, type, this);
    connect(l, &ImageLoader::signalPreviewLoaded, this, &Engine::slotPreviewLoaded);
    connect(l, &ImageLoader::signalError, this, [this](const KNSCore::EntryInternal &entry,
                                                       EntryInternal::PreviewType type,
                                                       const QString &errorText) {
        Q_EMIT signalErrorCode(KNSCore::ImageError, errorText, QVariantList() << entry.name() << type);
        qCDebug(KNEWSTUFFCORE) << "ERROR preview: " << errorText << entry.name() << type;
        --m_numPictureJobs;
        updateStatus();
    });
    l->start();
    ++m_numPictureJobs;
    updateStatus();
}

void Engine::slotPreviewLoaded(const KNSCore::EntryInternal &entry, EntryInternal::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "FINISH preview: " << entry.name() << type;
    Q_EMIT signalEntryPreviewLoaded(entry, type);
    --m_numPictureJobs;
    updateStatus();
}

void Engine::contactAuthor(const EntryInternal &entry)
{
    if (!entry.author().email().isEmpty()) {
        // invoke mail with the address of the author
        QUrl mailUrl;
        mailUrl.setScheme(QStringLiteral("mailto"));
        mailUrl.setPath(entry.author().email());
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("subject"), i18n("Re: %1", entry.name()));
        mailUrl.setQuery(query);
        QDesktopServices::openUrl(mailUrl);
    } else if (!entry.author().homepage().isEmpty()) {
        QDesktopServices::openUrl(QUrl(entry.author().homepage()));
    }
}

void Engine::slotEntryChanged(const KNSCore::EntryInternal &entry)
{
    Q_EMIT signalEntryEvent(entry, EntryInternal::StatusChangedEvent);
}

bool Engine::userCanVote(const EntryInternal &entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    return p->userCanVote();
}

void Engine::vote(const EntryInternal &entry, uint rating)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->vote(entry, rating);
}

bool Engine::userCanBecomeFan(const EntryInternal &entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    return p->userCanBecomeFan();
}

void Engine::becomeFan(const EntryInternal &entry)
{
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    p->becomeFan(entry);
}

void Engine::updateStatus()
{
    BusyState state;
    QString busyMessage;
    if (m_numInstallJobs > 0) {
        busyMessage = i18n("Installing");
        state |= BusyOperation::InstallingEntry;
    }
    if (m_numPictureJobs > 0) {
        busyMessage = i18np("Loading one preview", "Loading %1 previews", m_numPictureJobs);
        state |= BusyOperation::LoadingPreview;
    }
    if (m_numDataJobs > 0) {
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingPreview;
    }
    setBusy(state, busyMessage);
}

void Engine::checkForUpdates()
{
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Updates);
        p->loadEntries(request);
    }
}

void KNSCore::Engine::checkForInstalled()
{
    for (const QSharedPointer<KNSCore::Provider> &p : qAsConst(m_providers)) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Installed);
        request.page = 0;
        request.pageSize = m_pageSize;
        p->loadEntries(request);
    }
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 77)
QString Engine::adoptionCommand(const KNSCore::EntryInternal& entry) const
{
    return d->getAdoptionCommand(m_adoptionCommand, entry, m_installation);
}
#endif

bool KNSCore::Engine::hasAdoptionCommand() const
{
    return !m_adoptionCommand.isEmpty();
}

void KNSCore::Engine::setPageSize(int pageSize)
{
    m_pageSize = pageSize;
}

QStringList KNSCore::Engine::configSearchLocations(bool includeFallbackLocations)
{
    QStringList ret;
    if(includeFallbackLocations) {
        ret += QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    }
    const QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for( const QString& path : paths) {
        ret << QString::fromLocal8Bit("%1/knsrcfiles").arg(path);
    }
    return ret;
}

void KNSCore::Engine::setConfigLocationFallback(bool enableFallback)
{
    d->configLocationFallback = enableFallback;
}

QSharedPointer<KNSCore::Provider> KNSCore::Engine::provider(const QString &providerId) const
{
    return m_providers.value(providerId);
}

QSharedPointer<KNSCore::Provider> KNSCore::Engine::defaultProvider() const
{
    if (m_providers.count() > 0)
        return m_providers.constBegin().value();
    return nullptr;
}

KNSCore::CommentsModel *KNSCore::Engine::commentsForEntry(const KNSCore::EntryInternal &entry)
{
    CommentsModel *model = d->commentsModels[entry];
    if (!model) {
        model = new CommentsModel(this);
        model->setEntry(entry);
        connect(model, &QObject::destroyed, this, [=](){
            d->commentsModels.remove(entry);
        });
        d->commentsModels[entry] = model;
    }
    return model;
}

QString Engine::busyMessage() const
{
    return d->busyMessage;
}

void Engine::setBusyMessage(const QString &busyMessage)
{
    if (busyMessage != d->busyMessage) {
        d->busyMessage = busyMessage;
        Q_EMIT busyMessageChanged();
    }
#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 74)
    // Emit old signals for compatibility
    if (busyMessage.isEmpty()) {
        Q_EMIT signalIdle({});
    } else {
        Q_EMIT signalBusy(busyMessage);
    }
#endif
}

Engine::BusyState Engine::busyState() const
{
    return d->busyState;
}

void Engine::setBusyState(Engine::BusyState state)
{
    if (d->busyState != state) {
        d->busyState = state;
        Q_EMIT busyStateChanged();
    }
}

void Engine::setBusy(Engine::BusyState state, const QString &busyMessage) {
    setBusyState(state);
    setBusyMessage(busyMessage);
}

QSharedPointer<KNSCore::Cache> KNSCore::Engine::cache() const
{
    return m_cache;
}

void KNSCore::Engine::revalidateCacheEntries()
{
    // This gets called from QML, because in QtQuick we reuse the engine, BUG: 417985
    // We can't handle this in the cache, because it can't access the configuration of the engine
    if (m_cache && d->shouldRemoveDeletedEntries) {
        for (const auto &provider : qAsConst(m_providers)) {
            if (provider && provider->isInitialized()) {
                const EntryInternal::List cacheBefore = m_cache->registryForProvider(provider->id());
                m_cache->removeDeletedEntries();
                const EntryInternal::List cacheAfter = m_cache->registryForProvider(provider->id());
                // If the user has deleted them in the background we have to update the state to deleted
                for (const auto &oldCachedEntry : cacheBefore){
                    if (!cacheAfter.contains(oldCachedEntry)) {
                        EntryInternal removedEntry = oldCachedEntry;
                        removedEntry.setStatus(KNS3::Entry::Deleted);
                        Q_EMIT signalEntryEvent(removedEntry, EntryInternal::StatusChangedEvent);
                    }
                }
            }
        }
    }
}

void Engine::adoptEntry(const EntryInternal &entry)
{
    if (!hasAdoptionCommand()) {
        qCWarning(KNEWSTUFFCORE) << "no adoption command specified";
        return;
    }
    const QString command = d->getAdoptionCommand(m_adoptionCommand, entry, m_installation);
    QStringList split = KShell::splitArgs(command);
    QProcess *process = new QProcess(this);
    process->setProgram(split.takeFirst());
    process->setArguments(split);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // The debug output is too talkative to be useful
    env.insert(QStringLiteral("QT_LOGGING_RULES"), QStringLiteral("*.debug=false"));
    process->setProcessEnvironment(env);

    process->start();

    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process ,entry, command](int exitCode, QProcess::ExitStatus) {
                if (exitCode == 0) {
                    Q_EMIT signalEntryEvent(entry, EntryInternal::EntryEvent::AdoptedEvent);

                    // Handle error output as warnings if the process hasn't crashed
                    const QString stdErr = QString::fromLocal8Bit(process->readAllStandardError());
                    if (!stdErr.isEmpty()) {
                        Q_EMIT signalMessage(stdErr);
                    }
                } else {
                    const QString errorMsg = i18n("Failed to adopt '%1'\n%2",
                                                  entry.name(), QString::fromLocal8Bit(process->readAllStandardError()));
                    Q_EMIT signalErrorCode(KNSCore::AdoptionError, errorMsg, QVariantList{command});
                }
            });
}

QString Engine::useLabel() const
{
    return d->useLabel;
}
