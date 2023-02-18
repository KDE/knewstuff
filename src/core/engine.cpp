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
#include "imageloader_p.h"
#include "question.h"

#include "installation_p.h"
#include "xmlloader_p.h"

#include <KConfig>
#include <KConfigGroup>
#include <KFileUtils>
#include <KFormat>
#include <KLocalizedString>
#include <KShell>
#include <QDesktopServices>
#include <knewstuffcore_debug.h>

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QThreadStorage>
#include <QTimer>
#include <QUrlQuery>
#include <qdom.h>

#if defined(Q_OS_WIN)
#include <shlobj.h>
#include <windows.h>
#endif

// libattica
#include <attica/providermanager.h>
#include <qstandardpaths.h>

// own
#include "../attica/atticaprovider_p.h"
#include "../staticxml/staticxmlprovider_p.h"
#ifdef SYNDICATION_FOUND
#include "../opds/opdsprovider_p.h"
#endif
#include "cache.h"

using namespace KNSCore;

typedef QHash<QUrl, XmlLoader *> EngineProviderLoaderHash;
Q_GLOBAL_STATIC(QThreadStorage<EngineProviderLoaderHash>, s_engineProviderLoaders)

class EnginePrivate
{
public:
    QString getAdoptionCommand(const QString &command, const KNSCore::Entry &entry, Installation *inst)
    {
        auto adoption = command;
        if (adoption.isEmpty()) {
            return {};
        }

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
        while (!dirs.isEmpty()) {
            QString thisDir(dirs.takeLast());
            if (thisDir.endsWith(QStringLiteral("*"))) {
                qCInfo(KNEWSTUFFCORE) << "Directory entry" << thisDir
                                      << "ends in a *, indicating this was installed from an archive - see Installation::archiveEntries";
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
            if (dir.path() == (rootPath + dir.dirName())) {
                qCDebug(KNEWSTUFFCORE) << "Found directory" << dir;
                return dir;
            }
        }
        qCWarning(KNEWSTUFFCORE) << "Failed to locate any shared installed directory in" << dirs << "and this is almost certainly very bad.";
        return {};
    }

    QList<Provider::CategoryMetadata> categoriesMetadata;
    QList<Provider::SearchPreset> searchPresets;
    Attica::ProviderManager *atticaProviderManager = nullptr;
    QStringList tagFilter;
    QStringList downloadTagFilter;
    bool configLocationFallback = true; // TODO KF6 remove old location
    QString name;
    QMap<Entry, CommentsModel *> commentsModels;
    bool shouldRemoveDeletedEntries = false;
    KNSCore::Provider::SearchRequest storedRequest;

    // Used for updating purposes - we ought to be saving this information, but we also have to deal with old stuff, and so... this will have to do for now
    // TODO KF6: Installed state needs to move onto a per-downloadlink basis rather than per-entry
    QMap<Entry, QStringList> payloads;
    QMap<Entry, QString> payloadToIdentify;
    Engine::BusyState busyState;
    QString busyMessage;
    QString useLabel;
    bool uploadEnabled = false;
    QString configFileName;

    // handle installation of entries
    Installation *installation = new Installation();
    // read/write cache of entries
    QSharedPointer<Cache> cache;
    QTimer *searchTimer = new QTimer();
    // The url of the file containing information about content providers
    QUrl providerFileUrl;
    // Categories from knsrc file
    QStringList categories;

    QHash<QString, QSharedPointer<KNSCore::Provider>> providers;

    QString adoptionCommand;

    // the current request from providers
    Provider::SearchRequest currentRequest;

    // the page that is currently displayed, so it is not requested repeatedly
    int currentPage = -1;

    // when requesting entries from a provider, how many to ask for
    int pageSize = 20;

    int numDataJobs = 0;
    int numPictureJobs = 0;
    int numInstallJobs = 0;
    // If the provider is ready to be used
    bool initialized = false;
};

Engine::Engine(QObject *parent)
    : QObject(parent)
    , d(new EnginePrivate)
{
    d->searchTimer->setSingleShot(true);
    d->searchTimer->setInterval(1000);
    connect(d->searchTimer, &QTimer::timeout, this, &Engine::slotSearchTimerExpired);
    connect(d->installation, &Installation::signalInstallationFinished, this, &Engine::slotInstallationFinished);
    connect(d->installation, &Installation::signalInstallationFailed, this, &Engine::slotInstallationFailed);
    connect(d->installation, &Installation::signalInstallationError, this, [this](const QString &message) {
        Q_EMIT signalErrorCode(ErrorCode::InstallationError, i18n("An error occurred during the installation process:\n%1", message), QVariant());
    });
}

Engine::~Engine()
{
    if (d->cache) {
        d->cache->writeRegistry();
    }
    delete d->atticaProviderManager;
    delete d->searchTimer;
    delete d->installation;
}

bool Engine::init(const QString &configfile)
{
    qCDebug(KNEWSTUFFCORE) << "Initializing KNSCore::Engine from '" << configfile << "'";

    setBusy(BusyOperation::Initializing, i18n("Initializing"));

    QScopedPointer<KConfig> conf;
    QFileInfo configFileInfo(configfile);
    // TODO KF6: This is fallback logic for an old location for the knsrc files. This is deprecated in KF5 and should be removed in KF6
    bool isRelativeConfig = configFileInfo.isRelative();
    QString actualConfig;
    if (isRelativeConfig) {
        if (configfile.contains(QStringLiteral("/"))) {
            // If this is the case, then we've been given an /actual/ relative path, not just the name of a knsrc file
            actualConfig = configFileInfo.canonicalFilePath();
        } else {
            // Don't do the expensive search unless the config is relative
            actualConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile));
        }
    }

    if (!QFileInfo::exists(actualConfig)) {
        Q_EMIT signalErrorCode(KNSCore::ConfigFileError, i18n("Configuration file does not exist: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file" << configfile << "does not exist";
        return false;
    }

    // We need to always have a full path in some cases, and the given config name in others, so let's just
    // store this in a variable with a useful name
    QString configFullPath = actualConfig.isEmpty() ? configfile : actualConfig;
    QString configFileName{configfile};
    if (isRelativeConfig && d->configLocationFallback && actualConfig.isEmpty()) {
        conf.reset(new KConfig(configfile));
        qCWarning(KNEWSTUFFCORE) << "Using a deprecated location for the knsrc file" << configfile
                                 << " - please contact the author of the software which provides this file to get it updated to use the new location";
        configFileName = QFileInfo(configfile).baseName();
    } else if (isRelativeConfig && actualConfig.isEmpty()) {
        configFileName = QFileInfo(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile))).baseName();
        conf.reset(new KConfig(QStringLiteral("knsrcfiles/%1").arg(configfile), KConfig::FullConfig, QStandardPaths::GenericDataLocation));
    } else if (isRelativeConfig) {
        configFileName = configFileInfo.baseName();
        conf.reset(new KConfig(actualConfig));
    } else {
        configFileName = configFileInfo.baseName();
        conf.reset(new KConfig(configfile));
    }
    d->configFileName = configFileName;

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
    d->categories = group.readEntry("Categories", QStringList());
    qCDebug(KNEWSTUFFCORE) << "Categories: " << d->categories;
    d->adoptionCommand = group.readEntry("AdoptionCommand");
    d->useLabel = group.readEntry("UseLabel", i18n("Use"));
    Q_EMIT useLabelChanged();
    d->uploadEnabled = group.readEntry("UploadEnabled", true);
    Q_EMIT uploadEnabledChanged();

    d->providerFileUrl = group.readEntry("ProvidersUrl", QUrl(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    if (d->providerFileUrl.toString() == QLatin1String("https://download.kde.org/ocs/providers.xml")) {
        d->providerFileUrl = QUrl(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml"));
        qCWarning(KNEWSTUFFCORE) << "Please make sure" << configfile << "has ProvidersUrl=https://autoconfig.kde.org/ocs/providers.xml";
    }
    if (group.readEntry("UseLocalProvidersFile", "false").toLower() == QLatin1String{"true"}) {
        // The local providers file is called "appname.providers", to match "appname.knsrc"
        d->providerFileUrl = QUrl::fromLocalFile(QLatin1String("%1.providers").arg(configFullPath.left(configFullPath.length() - 6)));
    }

    d->tagFilter = group.readEntry("TagFilter", QStringList(QStringLiteral("ghns_excluded!=1")));
    d->downloadTagFilter = group.readEntry("DownloadTagFilter", QStringList());

    // Make sure that config is valid
    QString error;
    if (!d->installation->readConfig(group, error)) {
        Q_EMIT signalErrorCode(ErrorCode::ConfigFileError,
                               i18n("Could not initialise the installation handler for %1:\n%2\n"
                                    "This is a critical error and should be reported to the application author",
                                    configfile,
                                    error),
                               configfile);
        return false;
    }

    connect(d->installation, &Installation::signalEntryChanged, this, &Engine::slotEntryChanged);

    d->cache = Cache::getCache(configFileName);
    qCDebug(KNEWSTUFFCORE) << "Cache is" << d->cache << "for" << configFileName;
    connect(this, &Engine::signalEntryEvent, d->cache.data(), [this](const Entry &entry, Entry::EntryEvent event) {
        if (event == Entry::StatusChangedEvent) {
            d->cache->registerChangedEntry(entry);
        }
    });
    connect(d->cache.data(), &Cache::entryChanged, this, &Engine::slotEntryChanged);
    d->cache->readRegistry();

    // Cache cleanup option, to help work around people deleting files from underneath KNewStuff (this
    // happens a lot with e.g. wallpapers and icons)
    if (d->installation->uncompressionSetting() == Installation::UseKPackageUncompression) {
        d->shouldRemoveDeletedEntries = true;
    }

    d->shouldRemoveDeletedEntries = group.readEntry("RemoveDeadEntries", d->shouldRemoveDeletedEntries);
    if (d->shouldRemoveDeletedEntries) {
        d->cache->removeDeletedEntries();
    }

    d->initialized = true;

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
    return d->categories;
}

QStringList Engine::categoriesFilter() const
{
    return d->currentRequest.categories;
}

QList<Provider::CategoryMetadata> Engine::categoriesMetadata()
{
    return d->categoriesMetadata;
}

QList<Provider::SearchPreset> Engine::searchPresets()
{
    return d->searchPresets;
}

void Engine::loadProviders()
{
    if (d->providerFileUrl.isEmpty()) {
        // it would be nicer to move the attica stuff into its own class
        qCDebug(KNEWSTUFFCORE) << "Using OCS default providers";
        delete d->atticaProviderManager;
        d->atticaProviderManager = new Attica::ProviderManager;
        connect(d->atticaProviderManager, &Attica::ProviderManager::providerAdded, this, &Engine::atticaProviderLoaded);
        connect(d->atticaProviderManager, &Attica::ProviderManager::failedToLoad, this, &Engine::slotProvidersFailed);
        d->atticaProviderManager->loadDefaultProviders();
    } else {
        qCDebug(KNEWSTUFFCORE) << "loading providers from " << d->providerFileUrl;
        setBusy(BusyOperation::LoadingData, i18n("Loading provider information"));

        XmlLoader *loader = s_engineProviderLoaders()->localData().value(d->providerFileUrl);
        if (!loader) {
            qCDebug(KNEWSTUFFCORE) << "No xml loader for this url yet, so create one and temporarily store that" << d->providerFileUrl;
            loader = new XmlLoader(this);
            s_engineProviderLoaders()->localData().insert(d->providerFileUrl, loader);
            connect(loader, &XmlLoader::signalLoaded, this, [this]() {
                s_engineProviderLoaders()->localData().remove(d->providerFileUrl);
            });
            connect(loader, &XmlLoader::signalFailed, this, [this]() {
                s_engineProviderLoaders()->localData().remove(d->providerFileUrl);
            });
            connect(loader, &XmlLoader::signalHttpError, this, [this](int status, QList<QNetworkReply::RawHeaderPair> rawHeaders) {
                if (status == 503) { // Temporarily Unavailable
                    QDateTime retryAfter;
                    static const QByteArray retryAfterKey{"Retry-After"};
                    for (const QNetworkReply::RawHeaderPair &headerPair : rawHeaders) {
                        if (headerPair.first == retryAfterKey) {
                            // Retry-After is not a known header, so we need to do a bit of running around to make that work
                            // Also, the fromHttpDate function is in the private qnetworkrequest header, so we can't use that
                            // So, simple workaround, just pass it through a dummy request and get a formatted date out (the
                            // cost is sufficiently low here, given we've just done a bunch of i/o heavy things, so...)
                            QNetworkRequest dummyRequest;
                            dummyRequest.setRawHeader(QByteArray{"Last-Modified"}, headerPair.second);
                            retryAfter = dummyRequest.header(QNetworkRequest::LastModifiedHeader).toDateTime();
                            break;
                        }
                    }
                    QTimer::singleShot(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), this, &Engine::loadProviders);
                    // if it's a matter of a human moment's worth of seconds, just reload
                    if (retryAfter.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch() > 2) {
                        // more than that, spit out TryAgainLaterError to let the user know what we're doing with their time
                        static const KFormat formatter;
                        Q_EMIT signalErrorCode(KNSCore::TryAgainLaterError,
                                               i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                                    formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                               {retryAfter});
                    }
                }
            });
            loader->load(d->providerFileUrl);
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
        Q_EMIT signalErrorCode(KNSCore::ProviderError,
                               i18n("Could not load get hot new stuff providers from file: %1", d->providerFileUrl.toString()),
                               d->providerFileUrl);
        return;
    }

    QDomElement n = providers.firstChildElement(QStringLiteral("provider"));
    while (!n.isNull()) {
        qCDebug(KNEWSTUFFCORE) << "Provider attributes: " << n.attribute(QStringLiteral("type"));

        QSharedPointer<KNSCore::Provider> provider;
        if (isAtticaProviderFile || n.attribute(QStringLiteral("type")).toLower() == QLatin1String("rest")) {
            provider.reset(new AtticaProvider(d->categories, d->configFileName));
            connect(provider.data(), &Provider::categoriesMetadataLoded, this, [this](const QList<Provider::CategoryMetadata> &categories) {
                d->categoriesMetadata = categories;
                Q_EMIT signalCategoriesMetadataLoded(categories);
            });
#ifdef SYNDICATION_FOUND
        } else if (n.attribute(QStringLiteral("type")).toLower() == QLatin1String("opds")) {
            provider.reset(new OPDSProvider);
            connect(provider.data(), &Provider::searchPresetsLoaded, this, [this](const QList<Provider::SearchPreset> &presets) {
                d->searchPresets = presets;
                Q_EMIT signalSearchPresetsLoaded(presets);
            });
#endif
        } else {
            provider.reset(new StaticXmlProvider);
        }

        if (provider->setProviderXML(n)) {
            addProvider(provider);
        } else {
            Q_EMIT signalErrorCode(KNSCore::ProviderError, i18n("Error initializing provider."), d->providerFileUrl);
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
    QSharedPointer<KNSCore::Provider> provider = QSharedPointer<KNSCore::Provider>(new AtticaProvider(atticaProvider, d->categories, d->configFileName));
    connect(provider.data(), &Provider::categoriesMetadataLoded, this, [this](const QList<Provider::CategoryMetadata> &categories) {
        d->categoriesMetadata = categories;
        Q_EMIT signalCategoriesMetadataLoded(categories);
    });
    addProvider(provider);
}

void Engine::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    qCDebug(KNEWSTUFFCORE) << "Engine addProvider called with provider with id " << provider->id();
    d->providers.insert(provider->id(), provider);
    provider->setTagFilter(d->tagFilter);
    provider->setDownloadTagFilter(d->downloadTagFilter);
    connect(provider.data(), &Provider::providerInitialized, this, &Engine::providerInitialized);
    connect(provider.data(), &Provider::loadingFinished, this, &Engine::slotEntriesLoaded);
    connect(provider.data(), &Provider::entryDetailsLoaded, this, &Engine::slotEntryDetailsLoaded);
    connect(provider.data(), &Provider::payloadLinkLoaded, this, &Engine::downloadLinkLoaded);

    connect(provider.data(), &Provider::signalError, this, [this, provider](const QString &msg) {
        Q_EMIT signalErrorCode(ErrorCode::ProviderError, msg, d->providerFileUrl);
    });
    connect(provider.data(), &Provider::signalErrorCode, this, &Engine::signalErrorCode);
    connect(provider.data(), &Provider::signalInformation, this, [this](const QString &message) {
        Q_EMIT signalMessage(message);
    });
    connect(provider.data(), &Provider::basicsLoaded, this, &Engine::providersChanged);
    Q_EMIT providersChanged();
}

void Engine::providerJobStarted(KJob *job)
{
    Q_EMIT jobStarted(job, i18n("Loading data from provider"));
}

void Engine::slotProvidersFailed()
{
    Q_EMIT signalErrorCode(KNSCore::ProviderError, i18n("Loading of providers from file: %1 failed", d->providerFileUrl.toString()), d->providerFileUrl);
}

void Engine::providerInitialized(Provider *p)
{
    qCDebug(KNEWSTUFFCORE) << "providerInitialized" << p->name();
    p->setCachedEntries(d->cache->registryForProvider(p->id()));
    updateStatus();

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (!p->isInitialized()) {
            return;
        }
    }
    Q_EMIT signalProvidersLoaded();
}

void Engine::slotEntriesLoaded(const KNSCore::Provider::SearchRequest &request, KNSCore::Entry::List entries)
{
    d->currentPage = qMax<int>(request.page, d->currentPage);
    qCDebug(KNEWSTUFFCORE) << "loaded page " << request.page << "current page" << d->currentPage << "count:" << entries.count();

    if (request.filter == Provider::Updates) {
        Q_EMIT signalUpdateableEntriesLoaded(entries);
    } else {
        d->cache->insertRequest(request, entries);
        Q_EMIT signalEntriesLoaded(entries);
    }

    --d->numDataJobs;
    updateStatus();
}

void Engine::reloadEntries()
{
    Q_EMIT signalResetView();
    d->currentPage = -1;
    d->currentRequest.pageSize = d->pageSize;
    d->currentRequest.page = 0;
    d->numDataJobs = 0;

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (p->isInitialized()) {
            if (d->currentRequest.filter == Provider::Installed) {
                // when asking for installed entries, never use the cache
                p->loadEntries(d->currentRequest);
            } else {
                // take entries from cache until there are no more
                Entry::List cache;
                Entry::List lastCache = d->cache->requestFromCache(d->currentRequest);
                while (!lastCache.isEmpty()) {
                    qCDebug(KNEWSTUFFCORE) << "From cache";
                    cache << lastCache;

                    d->currentPage = d->currentRequest.page;
                    ++d->currentRequest.page;
                    lastCache = d->cache->requestFromCache(d->currentRequest);
                }

                // Since the cache has no more pages, reset the request's page
                if (d->currentPage >= 0) {
                    d->currentRequest.page = d->currentPage;
                }

                if (!cache.isEmpty()) {
                    Q_EMIT signalEntriesLoaded(cache);
                } else {
                    qCDebug(KNEWSTUFFCORE) << "From provider";
                    p->loadEntries(d->currentRequest);

                    ++d->numDataJobs;
                    updateStatus();
                }
            }
        }
    }
}

void Engine::setCategoriesFilter(const QStringList &categories)
{
    d->currentRequest.categories = categories;
    reloadEntries();
}

void Engine::setSortMode(Provider::SortMode mode)
{
    if (d->currentRequest.sortMode != mode) {
        d->currentRequest.page = -1;
    }
    d->currentRequest.sortMode = mode;
    reloadEntries();
}

Provider::SortMode KNSCore::Engine::sortMode() const
{
    return d->currentRequest.sortMode;
}

void KNSCore::Engine::setFilter(Provider::Filter filter)
{
    if (d->currentRequest.filter != filter) {
        d->currentRequest.page = -1;
    }
    d->currentRequest.filter = filter;
    reloadEntries();
}

Provider::Filter KNSCore::Engine::filter() const
{
    return d->currentRequest.filter;
}

void KNSCore::Engine::fetchEntryById(const QString &id)
{
    d->searchTimer->stop();
    d->currentRequest = KNSCore::Provider::SearchRequest(KNSCore::Provider::Newest, KNSCore::Provider::ExactEntryId, id);
    d->currentRequest.pageSize = d->pageSize;

    Entry::List cache = d->cache->requestFromCache(d->currentRequest);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer->start();
    }
}

void KNSCore::Engine::restoreSearch()
{
    d->searchTimer->stop();
    d->currentRequest = d->storedRequest;
    if (d->cache) {
        Entry::List cache = d->cache->requestFromCache(d->currentRequest);
        if (!cache.isEmpty()) {
            reloadEntries();
        } else {
            d->searchTimer->start();
        }
    } else {
        qCWarning(KNEWSTUFFCORE) << "Attempted to call restoreSearch() without a correctly initialized engine. You will likely get unexpected behaviour.";
    }
}

void KNSCore::Engine::storeSearch()
{
    d->storedRequest = d->currentRequest;
}

void Engine::setSearchTerm(const QString &searchString)
{
    d->searchTimer->stop();
    d->currentRequest.searchTerm = searchString;
    Entry::List cache = d->cache->requestFromCache(d->currentRequest);
    if (!cache.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer->start();
    }
}

QString KNSCore::Engine::searchTerm() const
{
    return d->currentRequest.searchTerm;
}

void Engine::setTagFilter(const QStringList &filter)
{
    d->tagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
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
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setTagFilter(d->tagFilter);
    }
}

void Engine::setDownloadTagFilter(const QStringList &filter)
{
    d->downloadTagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
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
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setDownloadTagFilter(d->downloadTagFilter);
    }
}

void Engine::slotSearchTimerExpired()
{
    reloadEntries();
}

void Engine::requestMoreData()
{
    qCDebug(KNEWSTUFFCORE) << "Get more data! current page: " << d->currentPage << " requested: " << d->currentRequest.page;

    if (d->currentPage < d->currentRequest.page) {
        return;
    }

    d->currentRequest.page++;
    doRequest();
}

void Engine::requestData(int page, int pageSize)
{
    d->currentRequest.page = page;
    d->currentRequest.pageSize = pageSize;
    doRequest();
}

void Engine::doRequest()
{
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (p->isInitialized()) {
            p->loadEntries(d->currentRequest);
            ++d->numDataJobs;
            updateStatus();
        }
    }
}

void Engine::install(KNSCore::Entry entry, int linkId)
{
    if (entry.downloadLinkCount() == 0 && entry.payload().isEmpty()) {
        // Turns out this happens sometimes, so we should deal with that and spit out an error
        qCDebug(KNEWSTUFFCORE) << "There were no downloadlinks defined in the entry we were just asked to update: " << entry.uniqueId() << "on provider"
                               << entry.providerId();
        Q_EMIT signalErrorCode(KNSCore::InstallationError,
                               i18n("Could not perform an installation of the entry %1 as it does not have any downloadable items defined. Please contact the "
                                    "author so they can fix this.",
                                    entry.name()),
                               entry.uniqueId());
    } else {
        if (entry.status() == KNSCore::Entry::Updateable) {
            entry.setStatus(KNSCore::Entry::Updating);
        } else {
            entry.setStatus(KNSCore::Entry::Installing);
        }
        Q_EMIT signalEntryEvent(entry, Entry::StatusChangedEvent);

        qCDebug(KNEWSTUFFCORE) << "Install " << entry.name() << " from: " << entry.providerId();
        QSharedPointer<Provider> p = d->providers.value(entry.providerId());
        if (p) {
            // If linkId is -1, assume that it's an update and that we don't know what to update
            if (entry.status() == KNSCore::Entry::Updating && linkId == -1) {
                if (entry.downloadLinkCount() == 1 || !entry.payload().isEmpty()) {
                    // If there is only one downloadable item (which also includes a predefined payload name), then we can fairly safely assume that's what
                    // we're wanting to update, meaning we can bypass some of the more expensive operations in downloadLinkLoaded
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

            ++d->numInstallJobs;
            updateStatus();
        }
    }
}

void Engine::slotInstallationFinished()
{
    --d->numInstallJobs;
    updateStatus();
}

void Engine::slotInstallationFailed(const QString &message)
{
    --d->numInstallJobs;
    Q_EMIT signalErrorCode(KNSCore::InstallationError, message, QVariant());
}

void Engine::slotEntryDetailsLoaded(const KNSCore::Entry &entry)
{
    --d->numDataJobs;
    updateStatus();
    Q_EMIT signalEntryEvent(entry, Entry::DetailsLoadedEvent);
}

void Engine::downloadLinkLoaded(const KNSCore::Entry &entry)
{
    if (entry.status() == KNSCore::Entry::Updating) {
        if (d->payloadToIdentify[entry].isEmpty()) {
            // If there's nothing to identify, and we've arrived here, then we know what the payload is
            qCDebug(KNEWSTUFFCORE) << "If there's nothing to identify, and we've arrived here, then we know what the payload is";
            d->installation->install(entry);
            d->payloadToIdentify.remove(entry);
        } else if (d->payloads[entry].count() < entry.downloadLinkCount()) {
            // We've got more to get before we can attempt to identify anything, so fetch the next one...
            qCDebug(KNEWSTUFFCORE) << "We've got more to get before we can attempt to identify anything, so fetch the next one...";
            QStringList payloads = d->payloads[entry];
            payloads << entry.payload();
            d->payloads[entry] = payloads;
            QSharedPointer<Provider> p = d->providers.value(entry.providerId());
            if (p) {
                // ok, so this should definitely always work, but... safety first, kids!
                p->loadPayloadLink(entry, payloads.count());
            }
        } else {
            // We now have all the links, so let's try and identify the correct one...
            qCDebug(KNEWSTUFFCORE) << "We now have all the links, so let's try and identify the correct one...";
            QString identifiedLink;
            const QString payloadToIdentify = d->payloadToIdentify[entry];
            const QList<Entry::DownloadLinkInformation> downloadLinks = entry.downloadLinkInformationList();
            const QStringList &payloads = d->payloads[entry];

            if (payloads.contains(payloadToIdentify)) {
                // Simplest option, the link hasn't changed at all
                qCDebug(KNEWSTUFFCORE) << "Simplest option, the link hasn't changed at all";
                identifiedLink = payloadToIdentify;
            } else {
                // Next simplest option, filename is the same but in a different folder
                qCDebug(KNEWSTUFFCORE) << "Next simplest option, filename is the same but in a different folder";
                const QString fileName = payloadToIdentify.split(QChar::fromLatin1('/')).last();
                for (const QString &payload : payloads) {
                    if (payload.endsWith(fileName)) {
                        identifiedLink = payload;
                        break;
                    }
                }

                // Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...
                qCDebug(KNEWSTUFFCORE) << "Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...";
                QStringList payloadNames;
                for (const Entry::DownloadLinkInformation &downloadLink : downloadLinks) {
                    qCDebug(KNEWSTUFFCORE) << "Download link" << downloadLink.name << downloadLink.id << downloadLink.size << downloadLink.descriptionLink;
                    payloadNames << downloadLink.name;
                    if (downloadLink.name == fileName) {
                        identifiedLink = payloads[payloadNames.count() - 1];
                        qCDebug(KNEWSTUFFCORE) << "Found a suitable download link for" << fileName << "which should match" << identifiedLink;
                    }
                }

                if (identifiedLink.isEmpty()) {
                    // Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)
                    qCDebug(KNEWSTUFFCORE)
                        << "Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)";
                    auto question = std::make_unique<Question>(Question::SelectFromListQuestion);
                    question->setTitle(i18n("Pick Update Item"));
                    question->setQuestion(
                        i18n("Please pick the item from the list below which should be used to apply this update. We were unable to identify which item to "
                             "select, based on the original item, which was named %1",
                             fileName));
                    question->setList(payloadNames);
                    if (question->ask() == Question::OKResponse) {
                        identifiedLink = payloads.value(payloadNames.indexOf(question->response()));
                    }
                }
            }
            if (!identifiedLink.isEmpty()) {
                KNSCore::Entry theEntry(entry);
                theEntry.setPayload(identifiedLink);
                d->installation->install(theEntry);
            } else {
                qCWarning(KNEWSTUFFCORE) << "We failed to identify a good link for updating" << entry.name() << "and are unable to perform the update";
                KNSCore::Entry theEntry(entry);
                theEntry.setStatus(KNSCore::Entry::Updateable);
                Q_EMIT signalEntryEvent(theEntry, Entry::StatusChangedEvent);
                Q_EMIT signalErrorCode(ErrorCode::InstallationError,
                                       i18n("We failed to identify a good link for updating %1, and are unable to perform the update", entry.name()),
                                       {entry.uniqueId()});
            }
            // As the serverside data may change before next time this is called, even in the same session,
            // let's not make assumptions, and just get rid of this
            d->payloads.remove(entry);
            d->payloadToIdentify.remove(entry);
        }
    } else {
        d->installation->install(entry);
    }
}

void Engine::uninstall(KNSCore::Entry entry)
{
    const KNSCore::Entry::List list = d->cache->registryForProvider(entry.providerId());
    // we have to use the cached entry here, not the entry from the provider
    // since that does not contain the list of installed files
    KNSCore::Entry actualEntryForUninstall;
    for (const KNSCore::Entry &eInt : list) {
        if (eInt.uniqueId() == entry.uniqueId()) {
            actualEntryForUninstall = eInt;
            break;
        }
    }
    if (!actualEntryForUninstall.isValid()) {
        qCDebug(KNEWSTUFFCORE) << "could not find a cached entry with following id:" << entry.uniqueId() << " ->  using the non-cached version";
        actualEntryForUninstall = entry;
    }

    entry.setStatus(KNSCore::Entry::Installing);
    actualEntryForUninstall.setStatus(KNSCore::Entry::Installing);
    Q_EMIT signalEntryEvent(entry, Entry::StatusChangedEvent);

    qCDebug(KNEWSTUFFCORE) << "about to uninstall entry " << entry.uniqueId();
    d->installation->uninstall(actualEntryForUninstall);

    entry.setStatus(actualEntryForUninstall.status());
    Q_EMIT signalEntryEvent(entry, Entry::StatusChangedEvent);
}

void Engine::loadDetails(const KNSCore::Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    p->loadEntryDetails(entry);
}

void Engine::loadPreview(const KNSCore::Entry &entry, Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "START  preview: " << entry.name() << type;
    ImageLoader *l = new ImageLoader(entry, type, this);
    connect(l, &ImageLoader::signalPreviewLoaded, this, &Engine::slotPreviewLoaded);
    connect(l, &ImageLoader::signalError, this, [this](const KNSCore::Entry &entry, Entry::PreviewType type, const QString &errorText) {
        Q_EMIT signalErrorCode(KNSCore::ImageError, errorText, QVariantList() << entry.name() << type);
        qCDebug(KNEWSTUFFCORE) << "ERROR preview: " << errorText << entry.name() << type;
        --d->numPictureJobs;
        updateStatus();
    });
    l->start();
    ++d->numPictureJobs;
    updateStatus();
}

void Engine::slotPreviewLoaded(const KNSCore::Entry &entry, Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFCORE) << "FINISH preview: " << entry.name() << type;
    Q_EMIT signalEntryPreviewLoaded(entry, type);
    --d->numPictureJobs;
    updateStatus();
}

void Engine::contactAuthor(const Entry &entry)
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

void Engine::slotEntryChanged(const KNSCore::Entry &entry)
{
    Q_EMIT signalEntryEvent(entry, Entry::StatusChangedEvent);
}

bool Engine::userCanVote(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    return p->userCanVote();
}

void Engine::vote(const Entry &entry, uint rating)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    p->vote(entry, rating);
}

bool Engine::userCanBecomeFan(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    return p->userCanBecomeFan();
}

void Engine::becomeFan(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    p->becomeFan(entry);
}

void Engine::updateStatus()
{
    BusyState state;
    QString busyMessage;
    if (d->numInstallJobs > 0) {
        busyMessage = i18n("Installing");
        state |= BusyOperation::InstallingEntry;
    }
    if (d->numPictureJobs > 0) {
        busyMessage = i18np("Loading one preview", "Loading %1 previews", d->numPictureJobs);
        state |= BusyOperation::LoadingPreview;
    }
    if (d->numDataJobs > 0) {
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingPreview;
    }
    setBusy(state, busyMessage);
}

void Engine::checkForUpdates()
{
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::Updates);
        p->loadEntries(request);
    }
}

void KNSCore::Engine::checkForInstalled()
{
    Entry::List entries = d->cache->registry();
    std::remove_if(entries.begin(), entries.end(), [](const auto &entry) {
        return entry.status() != KNSCore::Entry::Installed && entry.status() != KNSCore::Entry::Updateable;
    });
    Q_EMIT signalEntriesLoaded(entries);
}

bool KNSCore::Engine::hasAdoptionCommand() const
{
    return !d->adoptionCommand.isEmpty();
}

void KNSCore::Engine::setPageSize(int pageSize)
{
    d->pageSize = pageSize;
}

int KNSCore::Engine::pageSize() const
{
    return d->pageSize;
}

QStringList KNSCore::Engine::availableConfigFiles()
{
    QStringList configSearchLocations;
    configSearchLocations << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, //
                                                       QStringLiteral("knsrcfiles"),
                                                       QStandardPaths::LocateDirectory);
    configSearchLocations << QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    return KFileUtils::findAllUniqueFiles(configSearchLocations, {QStringLiteral("*.knsrc")});
}

QSharedPointer<KNSCore::Provider> KNSCore::Engine::provider(const QString &providerId) const
{
    return d->providers.value(providerId);
}

QSharedPointer<KNSCore::Provider> KNSCore::Engine::defaultProvider() const
{
    if (d->providers.count() > 0) {
        return d->providers.constBegin().value();
    }
    return nullptr;
}

QStringList Engine::providerIDs() const
{
    return d->providers.keys();
}

KNSCore::CommentsModel *KNSCore::Engine::commentsForEntry(const KNSCore::Entry &entry)
{
    CommentsModel *model = d->commentsModels[entry];
    if (!model) {
        model = new CommentsModel(this);
        model->setEntry(entry);
        connect(model, &QObject::destroyed, this, [=]() {
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

void Engine::setBusy(Engine::BusyState state, const QString &busyMessage)
{
    setBusyState(state);
    setBusyMessage(busyMessage);
}

QSharedPointer<KNSCore::Cache> KNSCore::Engine::cache() const
{
    return d->cache;
}

void KNSCore::Engine::revalidateCacheEntries()
{
    // This gets called from QML, because in QtQuick we reuse the engine, BUG: 417985
    // We can't handle this in the cache, because it can't access the configuration of the engine
    if (d->cache && d->shouldRemoveDeletedEntries) {
        for (const auto &provider : std::as_const(d->providers)) {
            if (provider && provider->isInitialized()) {
                const Entry::List cacheBefore = d->cache->registryForProvider(provider->id());
                d->cache->removeDeletedEntries();
                const Entry::List cacheAfter = d->cache->registryForProvider(provider->id());
                // If the user has deleted them in the background we have to update the state to deleted
                for (const auto &oldCachedEntry : cacheBefore) {
                    if (!cacheAfter.contains(oldCachedEntry)) {
                        Entry removedEntry = oldCachedEntry;
                        removedEntry.setStatus(KNSCore::Entry::Deleted);
                        Q_EMIT signalEntryEvent(removedEntry, Entry::StatusChangedEvent);
                    }
                }
            }
        }
    }
}

void Engine::adoptEntry(const Entry &entry)
{
    if (!hasAdoptionCommand()) {
        qCWarning(KNEWSTUFFCORE) << "no adoption command specified";
        return;
    }
    const QString command = d->getAdoptionCommand(d->adoptionCommand, entry, d->installation);
    QStringList split = KShell::splitArgs(command);
    QProcess *process = new QProcess(this);
    process->setProgram(split.takeFirst());
    process->setArguments(split);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    // The debug output is too talkative to be useful
    env.insert(QStringLiteral("QT_LOGGING_RULES"), QStringLiteral("*.debug=false"));
    process->setProcessEnvironment(env);

    process->start();

    connect(process, &QProcess::finished, this, [this, process, entry, command](int exitCode) {
        if (exitCode == 0) {
            Q_EMIT signalEntryEvent(entry, Entry::EntryEvent::AdoptedEvent);

            // Handle error output as warnings if the process hasn't crashed
            const QString stdErr = QString::fromLocal8Bit(process->readAllStandardError());
            if (!stdErr.isEmpty()) {
                Q_EMIT signalMessage(stdErr);
            }
        } else {
            const QString errorMsg = i18n("Failed to adopt '%1'\n%2", entry.name(), QString::fromLocal8Bit(process->readAllStandardError()));
            Q_EMIT signalErrorCode(KNSCore::AdoptionError, errorMsg, QVariantList{command});
        }
    });
}

QString Engine::useLabel() const
{
    return d->useLabel;
}

bool KNSCore::Engine::uploadEnabled() const
{
    return d->uploadEnabled;
}

QVector<Attica::Provider *> Engine::atticaProviders() const
{
    QVector<Attica::Provider *> ret;
    ret.reserve(d->providers.size());
    for (const auto &p : d->providers) {
        const auto atticaProvider = p.dynamicCast<AtticaProvider>();
        if (atticaProvider) {
            ret += atticaProvider->provider();
        }
    }
    return ret;
}
