/*
    knewstuff3/engine.cpp
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (c) 2010 Matthias Fuchs <mat69@gmx.net>

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

#include "engine.h"

#include "../entry.h"
#include "installation.h"
#include "xmlloader.h"
#include "imageloader_p.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <knewstuffcore_debug.h>
#include <klocalizedstring.h>
#include <QDesktopServices>

#include <QTimer>
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
    QList<Provider::CategoryMetadata> categoriesMetadata;
};

// BCI: Add a real d-pointer
typedef QHash<const Engine *, EnginePrivate *> EnginePrivateHash;
Q_GLOBAL_STATIC(EnginePrivateHash, d_func)
static EnginePrivate *d(const Engine* engine)
{
    EnginePrivate* ret = d_func()->value(engine);
    if (!ret) {
        ret = new EnginePrivate;
        d_func()->insert(engine, ret);
    }
    return ret;
}
static void delete_d(const Engine* engine)
{
    if (auto d = d_func()) {
        delete d->take(engine);
    }
}

Engine::Engine(QObject *parent)
    : QObject(parent)
    , m_installation(new Installation)
    , m_cache()
    , m_searchTimer(new QTimer)
    , m_atticaProviderManager(nullptr)
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
}

Engine::~Engine()
{
    if (m_cache) {
        m_cache->writeRegistry();
    }
    delete m_atticaProviderManager;
    delete m_searchTimer;
    delete m_installation;
    delete_d(this);
}

bool Engine::init(const QString &configfile)
{
    qCDebug(KNEWSTUFFCORE) << "Initializing KNSCore::Engine from '" << configfile << "'";

    emit signalBusy(i18n("Initializing"));

    KConfig conf(configfile);
    if (conf.accessMode() == KConfig::NoAccess) {
        emit signalError(i18n("Configuration file exists, but cannot be opened: \"%1\"", configfile));
        qCritical() << "The knsrc file '" << configfile << "' was found but could not be opened.";
        return false;
    }

    KConfigGroup group;
    if (conf.hasGroup("KNewStuff3")) {
        qCDebug(KNEWSTUFFCORE) << "Loading KNewStuff3 config: " << configfile;
        group = conf.group("KNewStuff3");
    } else if (conf.hasGroup("KNewStuff2")) {
        qCDebug(KNEWSTUFFCORE) << "Loading KNewStuff2 config: " << configfile;
        group = conf.group("KNewStuff2");
    } else {
        emit signalError(i18n("Configuration file is invalid: \"%1\"", configfile));
        qCritical() << configfile << " doesn't contain a KNewStuff3 section.";
        return false;
    }

    m_categories = group.readEntry("Categories", QStringList());
    m_adoptionCommand = group.readEntry("AdoptionCommand", QString());

    qCDebug(KNEWSTUFFCORE) << "Categories: " << m_categories;
    m_providerFileUrl = group.readEntry("ProvidersUrl", QString());

    const QString configFileName = QFileInfo(QDir::isAbsolutePath(configfile) ? configfile : QStandardPaths::locate(QStandardPaths::GenericConfigLocation, configfile)).baseName();
    // let installation read install specific config
    if (!m_installation->readConfig(group)) {
        return false;
    }

    connect(m_installation, &Installation::signalEntryChanged, this, &Engine::slotEntryChanged);

    m_cache = Cache::getCache(configFileName);
    connect(this, &Engine::signalEntryChanged, m_cache.data(), &Cache::registerChangedEntry);
    m_cache->readRegistry();

    m_initialized = true;

    // load the providers
    loadProviders();

    return true;
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
    return d(this)->categoriesMetadata;
}

void Engine::loadProviders()
{
    if (m_providerFileUrl.isEmpty()) {
        // it would be nicer to move the attica stuff into its own class
        qCDebug(KNEWSTUFFCORE) << "Using OCS default providers";
        delete m_atticaProviderManager;
        m_atticaProviderManager = new Attica::ProviderManager;
        connect(m_atticaProviderManager, &Attica::ProviderManager::providerAdded, this, &Engine::atticaProviderLoaded);
        m_atticaProviderManager->loadDefaultProviders();
    } else {
        qCDebug(KNEWSTUFFCORE) << "loading providers from " << m_providerFileUrl;
        emit signalBusy(i18n("Loading provider information"));

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
        emit signalError(i18n("Could not load get hot new stuff providers from file: %1", m_providerFileUrl));
        return;
    }

    QDomElement n = providers.firstChildElement(QStringLiteral("provider"));
    while (!n.isNull()) {
        qCDebug(KNEWSTUFFCORE) << "Provider attributes: " << n.attribute(QStringLiteral("type"));

        QSharedPointer<KNSCore::Provider> provider;
        if (isAtticaProviderFile || n.attribute(QStringLiteral("type")).toLower() == QLatin1String("rest")) {
            provider = QSharedPointer<KNSCore::Provider> (new AtticaProvider(m_categories));
            connect(provider.data(), &Provider::categoriesMetadataLoded,
                    this, [this](const QList<Provider::CategoryMetadata> &categories){
                        d(this)->categoriesMetadata = categories;
                        emit signalCategoriesMetadataLoded(categories);
                    });
        } else {
            provider = QSharedPointer<KNSCore::Provider> (new StaticXmlProvider);
        }

        if (provider->setProviderXML(n)) {
            addProvider(provider);
        } else {
            emit signalError(i18n("Error initializing provider."));
        }
        n = n.nextSiblingElement();
    }
    emit signalBusy(i18n("Loading data"));
}

void Engine::atticaProviderLoaded(const Attica::Provider &atticaProvider)
{
    qCDebug(KNEWSTUFFCORE) << "atticaProviderLoaded called";
    if (!atticaProvider.hasContentService()) {
        qCDebug(KNEWSTUFFCORE) << "Found provider: " << atticaProvider.baseUrl() << " but it does not support content";
        return;
    }
    QSharedPointer<KNSCore::Provider> provider =
        QSharedPointer<KNSCore::Provider> (new AtticaProvider(atticaProvider, m_categories));
    connect(provider.data(), &Provider::categoriesMetadataLoded,
            this, [this](const QList<Provider::CategoryMetadata> &categories){
                d(this)->categoriesMetadata = categories;
                emit signalCategoriesMetadataLoded(categories);
            });
    addProvider(provider);
}

void Engine::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    qCDebug(KNEWSTUFFCORE) << "Engine addProvider called with provider with id " << provider->id();
    m_providers.insert(provider->id(), provider);
    connect(provider.data(), &Provider::providerInitialized, this, &Engine::providerInitialized);
    connect(provider.data(), &Provider::loadingFinished, this, &Engine::slotEntriesLoaded);
    connect(provider.data(), &Provider::entryDetailsLoaded, this, &Engine::slotEntryDetailsLoaded);
    connect(provider.data(), &Provider::payloadLinkLoaded, this, &Engine::downloadLinkLoaded);
    connect(provider.data(), &Provider::signalError, this, &Engine::signalError);
    connect(provider.data(), &Provider::signalInformation, this, &Engine::signalIdle);
}

void Engine::providerJobStarted(KJob *job)
{
    emit jobStarted(job, i18n("Loading data from provider"));
}

void Engine::slotProvidersFailed()
{
    emit signalError(i18n("Loading of providers from file: %1 failed", m_providerFileUrl));
}

void Engine::providerInitialized(Provider *p)
{
    qCDebug(KNEWSTUFFCORE) << "providerInitialized" << p->name();
    p->setCachedEntries(m_cache->registryForProvider(p->id()));
    updateStatus();

    foreach (const QSharedPointer<KNSCore::Provider> &p, m_providers) {
        if (!p->isInitialized()) {
            return;
        }
    }
    emit signalProvidersLoaded();
}

void Engine::slotEntriesLoaded(const KNSCore::Provider::SearchRequest &request, KNSCore::EntryInternal::List entries)
{
    m_currentPage = qMax<int>(request.page, m_currentPage);
    qCDebug(KNEWSTUFFCORE) << "loaded page " << request.page << "current page" << m_currentPage << "count:" << entries.count();

    if (request.filter == Provider::Updates) {
        emit signalUpdateableEntriesLoaded(entries);
    } else {
        m_cache->insertRequest(request, entries);
        emit signalEntriesLoaded(entries);
    }

    --m_numDataJobs;
    updateStatus();
}

void Engine::reloadEntries()
{
    emit signalResetView();
    m_currentPage = -1;
    m_currentRequest.pageSize = m_pageSize;
    m_currentRequest.page = 0;
    m_numDataJobs = 0;

    foreach (const QSharedPointer<KNSCore::Provider> &p, m_providers) {
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
                    emit signalEntriesLoaded(cache);
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

void KNSCore::Engine::setFilter(Provider::Filter filter)
{
    if (m_currentRequest.filter != filter) {
        m_currentRequest.page = -1;
    }
    m_currentRequest.filter = filter;
    reloadEntries();
}

void KNSCore::Engine::fetchEntryById(const QString& id)
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
    foreach (const QSharedPointer<KNSCore::Provider> &p, m_providers) {
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
    emit signalEntryChanged(entry);

    qCDebug(KNEWSTUFFCORE) << "Install " << entry.name()
       << " from: " << entry.providerId();
    QSharedPointer<Provider> p = m_providers.value(entry.providerId());
    if (p) {
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
    emit signalError(message);
}

void Engine::slotEntryDetailsLoaded(const KNSCore::EntryInternal &entry)
{
    emit signalEntryDetailsLoaded(entry);
}

void Engine::downloadLinkLoaded(const KNSCore::EntryInternal &entry)
{
    m_installation->install(entry);
}

void Engine::uninstall(KNSCore::EntryInternal entry)
{
    KNSCore::EntryInternal::List list = m_cache->registryForProvider(entry.providerId());
    //we have to use the cached entry here, not the entry from the provider
    //since that does not contain the list of installed files
    KNSCore::EntryInternal actualEntryForUninstall;
    foreach (const KNSCore::EntryInternal &eInt, list) {
        if (eInt.uniqueId() == entry.uniqueId()) {
            actualEntryForUninstall = eInt;
            break;
        }
    }
    if (!actualEntryForUninstall.isValid()) {
        qCDebug(KNEWSTUFFCORE) << "could not find a cached entry with following id:" << entry.uniqueId() <<
                 " ->  using the non-cached version";
        return;
    }

    entry.setStatus(KNS3::Entry::Installing);
    actualEntryForUninstall.setStatus(KNS3::Entry::Installing);
    emit signalEntryChanged(entry);

    qCDebug(KNEWSTUFFCORE) << "about to uninstall entry " << entry.uniqueId();
    // FIXME: change the status?
    m_installation->uninstall(actualEntryForUninstall);

    entry.setStatus(KNS3::Entry::Deleted); //status for actual entry gets set in m_installation->uninstall()
    emit signalEntryChanged(entry);

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
    emit signalEntryPreviewLoaded(entry, type);
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
    emit signalEntryChanged(entry);
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
    if (m_numDataJobs > 0) {
        emit signalBusy(i18n("Loading data"));
    } else if (m_numPictureJobs > 0) {
        emit signalBusy(i18np("Loading one preview", "Loading %1 previews", m_numPictureJobs));
    } else if (m_numInstallJobs > 0) {
        emit signalBusy(i18n("Installing"));
    } else {
        emit signalIdle(QString());
    }
}

void Engine::checkForUpdates()
{
    foreach (QSharedPointer<Provider> p, m_providers) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Updates);
        p->loadEntries(request);
    }
}

void KNSCore::Engine::checkForInstalled()
{
    foreach (QSharedPointer<Provider> p, m_providers) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Installed);
        request.page = 0;
        request.pageSize = m_pageSize;
        p->loadEntries(request);
    }
}

/**
 * we look for the directory where all the resources got installed.
 * assuming it was extracted into a directory
 */
static QDir sharedDir(QStringList dirs, const QString &rootPath)
{
    while(!dirs.isEmpty()) {
        const QString currentPath = QDir::cleanPath(dirs.takeLast());
        if (!currentPath.startsWith(rootPath))
            continue;

        const QFileInfo current(currentPath);
        if (!current.isDir())
            continue;

        const QDir dir = current.dir();
        if (dir.path()==(rootPath+dir.dirName())) {
            return dir;
        }
    }
    return {};
}

QString Engine::adoptionCommand(const KNSCore::EntryInternal& entry) const
{
    auto adoption = m_adoptionCommand;
    if(adoption.isEmpty())
        return {};

    const QLatin1String dirReplace("%d");
    if (adoption.contains(dirReplace)) {
        QString installPath = sharedDir(entry.installedFiles(), m_installation->targetInstallationPath()).path();
        adoption.replace(dirReplace, installPath);
    }

    const QLatin1String fileReplace("%f");
    QStringList ret;
    if (adoption.contains(fileReplace)) {
        if (entry.installedFiles().isEmpty()) {
            qCWarning(KNEWSTUFFCORE) << "no installed files to adopt";
        } else if (entry.installedFiles().count() != 1) {
            qCWarning(KNEWSTUFFCORE) << "can only adopt one file, will be using the first" << entry.installedFiles().at(0);
        }

        adoption.replace(fileReplace, entry.installedFiles().at(0));
    }
    return adoption;
}

bool KNSCore::Engine::hasAdoptionCommand() const
{
    return !m_adoptionCommand.isEmpty();
}

void KNSCore::Engine::setPageSize(int pageSize)
{
    m_pageSize = pageSize;
}
