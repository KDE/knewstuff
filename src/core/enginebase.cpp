/*
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "enginebase.h"
#include "enginebase_p.h"
#include <knewstuffcore_debug.h>

#include <KConfig>
#include <KConfigGroup>
#include <KFileUtils>
#include <KFormat>
#include <KLocalizedString>

#include <QFileInfo>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QThreadStorage>
#include <QTimer>

#include "attica/atticaprovider_p.h"
#include "categorymetadata.h"
#include "compat_p.h"
#include "opds/opdsprovider_p.h"
#include "providerbubblewrap_p.h"
#include "providercore.h"
#include "providercore_p.h"
#include "resultsstream.h"
#include "searchrequest_p.h"
#include "staticxml/staticxmlprovider_p.h"
#include "transaction.h"
#include "xmlloader_p.h"

using namespace KNSCore;

typedef QHash<QUrl, QPointer<XmlLoader>> EngineProviderLoaderHash;
Q_GLOBAL_STATIC(QThreadStorage<EngineProviderLoaderHash>, s_engineProviderLoaders)

namespace
{

}

EngineBasePrivate::EngineBasePrivate(EngineBase *qptr)
    : q(qptr)
{
}

void EngineBasePrivate::addProvider(const QSharedPointer<KNSCore::ProviderCore> &provider)
{
    { // ProviderCore
        qCDebug(KNEWSTUFFCORE) << "Engine addProvider called with provider with id " << provider->d->base->id();
        providerCores.insert(provider->d->base->id(), provider);
        provider->d->base->setTagFilter(tagFilter);
        provider->d->base->setDownloadTagFilter(downloadTagFilter);
        QObject::connect(provider->d->base, &ProviderBase::providerInitialized, q, [this, providerBase = provider->d->base] {
            qCDebug(KNEWSTUFFCORE) << "providerInitialized" << providerBase->name();
            providerBase->setCachedEntries(cache->registryForProvider(providerBase->id()));

            for (const auto &core : std::as_const(providerCores)) {
                if (!core->d->base->isInitialized()) {
                    return;
                }
            }
            Q_EMIT q->signalProvidersLoaded();
        });

        QObject::connect(provider->d->base, &ProviderBase::signalError, q, [this, provider](const QString &msg) {
            Q_EMIT q->signalErrorCode(ErrorCode::ProviderError, msg, providerFileUrl);
        });
        QObject::connect(provider->d->base, &ProviderBase::signalErrorCode, q, &EngineBase::signalErrorCode);
        QObject::connect(provider->d->base, &ProviderBase::signalInformation, q, &EngineBase::signalMessage);
        QObject::connect(provider->d->base, &ProviderBase::basicsLoaded, q, &EngineBase::providersChanged);
        Q_EMIT q->providerAdded(provider.get());
    }

    { // ProviderBubbleWrap for legacy compatibility
        QSharedPointer<ProviderBubbleWrap> wrappedProvider(new ProviderBubbleWrap(provider));
        legacyProviders.insert(wrappedProvider->id(), wrappedProvider);
        wrappedProvider->setTagFilter(tagFilter);
        wrappedProvider->setDownloadTagFilter(downloadTagFilter);
        q->addProvider(wrappedProvider);
    }

    Q_EMIT q->providersChanged();
}

EngineBase::EngineBase(QObject *parent)
    : QObject(parent)
    , d(new EngineBasePrivate(this))
{
    connect(d->installation, &Installation::signalInstallationError, this, [this](const QString &message) {
        Q_EMIT signalErrorCode(ErrorCode::InstallationError, i18n("An error occurred during the installation process:\n%1", message), QVariant());
    });
}

QStringList EngineBase::availableConfigFiles()
{
    QStringList configSearchLocations;
    configSearchLocations << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, //
                                                       QStringLiteral("knsrcfiles"),
                                                       QStandardPaths::LocateDirectory);
    configSearchLocations << QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    return KFileUtils::findAllUniqueFiles(configSearchLocations, {QStringLiteral("*.knsrc")});
}

EngineBase::~EngineBase()
{
    if (d->cache) {
        d->cache->writeRegistry();
    }
    delete d->atticaProviderManager;
    delete d->installation;
}

bool EngineBase::init(const QString &configfile)
{
    qCDebug(KNEWSTUFFCORE) << "Initializing KNSCore::EngineBase from" << configfile;

    QString resolvedConfigFilePath;
    if (QFileInfo(configfile).isAbsolute()) {
        resolvedConfigFilePath = configfile; // It is an absolute path
    } else {
        // Don't do the expensive search unless the config is relative
        resolvedConfigFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile));
    }

    if (!QFileInfo::exists(resolvedConfigFilePath)) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file does not exist: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file" << configfile << "does not exist";
        return false;
    }

    const KConfig conf(resolvedConfigFilePath);

    if (conf.accessMode() == KConfig::NoAccess) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file exists, but cannot be opened: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file" << configfile << "was found but could not be opened.";
        return false;
    }

    const KConfigGroup group = conf.hasGroup(QStringLiteral("KNewStuff")) ? conf.group(QStringLiteral("KNewStuff")) : conf.group(QStringLiteral("KNewStuff3"));
    if (!group.exists()) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file is invalid: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << configfile << "doesn't contain a KNewStuff or KNewStuff3 section.";
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
    if (group.readEntry("UseLocalProvidersFile", false)) {
        // The local providers file is called "appname.providers", to match "appname.knsrc"
        d->providerFileUrl = QUrl::fromLocalFile(QLatin1String("%1.providers").arg(configfile.left(configfile.length() - 6)));
    }

    d->tagFilter = group.readEntry("TagFilter", QStringList(QStringLiteral("ghns_excluded!=1")));
    d->downloadTagFilter = group.readEntry("DownloadTagFilter", QStringList());

    QByteArray rawContentWarningType = group.readEntry("ContentWarning", QByteArrayLiteral("Static"));
    bool ok = false;
    int value = QMetaEnum::fromType<ContentWarningType>().keyToValue(rawContentWarningType.constData(), &ok);
    if (ok) {
        d->contentWarningType = static_cast<ContentWarningType>(value);
    } else {
        qCWarning(KNEWSTUFFCORE) << "Could not parse ContentWarning, invalid entry" << rawContentWarningType;
    }

    Q_EMIT contentWarningTypeChanged();

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

    const QString configFileBasename = QFileInfo(resolvedConfigFilePath).completeBaseName();

    d->legacyCache = Cache::getCache(configFileBasename);
    qCDebug(KNEWSTUFFCORE) << "Legacy cache is" << d->legacyCache << "for" << configFileBasename;
    d->legacyCache->readRegistry();

    d->cache = Cache2::getCache(configFileBasename);
    qCDebug(KNEWSTUFFCORE) << "Cache is" << d->cache << "for" << configFileBasename;
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

    loadProviders();

    return true;
}

void EngineBase::loadProviders()
{
    if (d->providerFileUrl.isEmpty()) {
        // it would be nicer to move the attica stuff into its own class
        qCDebug(KNEWSTUFFCORE) << "Using OCS default providers";
        delete d->atticaProviderManager;
        d->atticaProviderManager = new Attica::ProviderManager;
        connect(d->atticaProviderManager, &Attica::ProviderManager::providerAdded, this, &EngineBase::atticaProviderLoaded);
        connect(d->atticaProviderManager, &Attica::ProviderManager::failedToLoad, this, &EngineBase::slotProvidersFailed);
        d->atticaProviderManager->loadDefaultProviders();
    } else {
        qCDebug(KNEWSTUFFCORE) << "loading providers from " << d->providerFileUrl;
        Q_EMIT loadingProvider();

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
                    QTimer::singleShot(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), this, &EngineBase::loadProviders);
                    // if it's a matter of a human moment's worth of seconds, just reload
                    if (retryAfter.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch() > 2) {
                        // more than that, spit out TryAgainLaterError to let the user know what we're doing with their time
                        static const KFormat formatter;
                        Q_EMIT signalErrorCode(KNSCore::ErrorCode::TryAgainLaterError,
                                               i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                                    formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                               {retryAfter});
                    }
                }
            });
            loader->load(d->providerFileUrl);
        }
        connect(loader, &XmlLoader::signalLoaded, this, &EngineBase::slotProviderFileLoaded);
        connect(loader, &XmlLoader::signalFailed, this, &EngineBase::slotProvidersFailed);
    }
}

QString KNSCore::EngineBase::name() const
{
    return d->name;
}

QStringList EngineBase::categories() const
{
    return d->categories;
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QList<Provider::CategoryMetadata> EngineBase::categoriesMetadata()
{
    QList<Provider::CategoryMetadata> list;
    for (const auto &data : d->categoriesMetadata) {
        list.append(Provider::CategoryMetadata{.id = data.id(), .name = data.name(), .displayName = data.displayName()});
    }
    return list;
}
#endif

QList<CategoryMetadata> EngineBase::categoriesMetadata2()
{
    return d->categoriesMetadata;
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QList<Provider::SearchPreset> EngineBase::searchPresets()
{
    QList<Provider::SearchPreset> list;
    for (const auto &preset : d->searchPresets) {
        // This is slightly mad backwards compat. We back-convert a SearchPreset which requires a convert of
        // SearchRequest and all the involved enums.
        // Since this is the only place we need it this has been implemented thusly.
        // Should someone find it offensive feel free to tear it apart into functions, but understand they are only
        // used here.
        list.append(KNSCompat::searchPresetToLegacy(preset));
    }
    return list;
}
#endif

QList<SearchPreset> EngineBase::searchPresets2()
{
    return d->searchPresets;
}

QString EngineBase::useLabel() const
{
    return d->useLabel;
}

bool EngineBase::uploadEnabled() const
{
    return d->uploadEnabled;
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
void EngineBase::addProvider(QSharedPointer<KNSCore::Provider> /*provider*/)
{
    // Connections are established in the modern variant of this function. No need to do anything.
}
#endif

void EngineBase::providerInitialized([[maybe_unused]] Provider *p)
{
    // Unused. Replaced by lambda. Here for ABI stability.
}

void EngineBase::slotProvidersFailed()
{
    Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError,
                           i18n("Loading of providers from file: %1 failed", d->providerFileUrl.toString()),
                           d->providerFileUrl);
}

void EngineBase::slotProviderFileLoaded(const QDomDocument &doc)
{
    qCDebug(KNEWSTUFFCORE) << "slotProvidersLoaded";

    bool isAtticaProviderFile = false;

    // get each provider element, and create a provider object from it
    QDomElement providers = doc.documentElement();

    if (providers.tagName() == QLatin1String("providers")) {
        isAtticaProviderFile = true;
    } else if (providers.tagName() != QLatin1String("ghnsproviders") && providers.tagName() != QLatin1String("knewstuffproviders")) {
        qWarning() << "No document in providers.xml.";
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError,
                               i18n("Could not load get hot new stuff providers from file: %1", d->providerFileUrl.toString()),
                               d->providerFileUrl);
        return;
    }

    QDomElement n = providers.firstChildElement(QStringLiteral("provider"));
    while (!n.isNull()) {
        qCDebug(KNEWSTUFFCORE) << "Provider attributes: " << n.attribute(QStringLiteral("type"));

        QSharedPointer<KNSCore::ProviderCore> provider;
        if (isAtticaProviderFile || n.attribute(QStringLiteral("type")).toLower() == QLatin1String("rest")) {
            provider.reset(new ProviderCore(new AtticaProvider(d->categories, {})));
            connect(provider->d->base, &ProviderBase::categoriesMetadataLoaded, this, [this](const QList<CategoryMetadata> &categories) {
                d->categoriesMetadata = categories;
                Q_EMIT signalCategoriesMetadataLoaded(categories);
            });
#ifdef SYNDICATION_FOUND
        } else if (n.attribute(QStringLiteral("type")).toLower() == QLatin1String("opds")) {
            provider.reset(new ProviderCore(new OPDSProvider));
            connect(provider->d->base, &ProviderBase::searchPresetsLoaded, this, [this](const QList<SearchPreset> &presets) {
                d->searchPresets = presets;
                Q_EMIT signalSearchPresetsLoaded(presets);
            });
#endif
        } else {
            provider.reset(new ProviderCore(new StaticXmlProvider));
        }

        if (provider->d->base->setProviderXML(n)) {
            d->addProvider(provider);
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError, i18n("Error initializing provider."), d->providerFileUrl);
        }
        n = n.nextSiblingElement();
    }
    Q_EMIT loadingProvider();
}

void EngineBase::atticaProviderLoaded(const Attica::Provider &atticaProvider)
{
    qCDebug(KNEWSTUFFCORE) << "atticaProviderLoaded called";
    if (!atticaProvider.hasContentService()) {
        qCDebug(KNEWSTUFFCORE) << "Found provider: " << atticaProvider.baseUrl() << " but it does not support content";
        return;
    }
    auto provider = QSharedPointer<KNSCore::ProviderCore>(new KNSCore::ProviderCore(new AtticaProvider(atticaProvider, d->categories, {})));
    d->addProvider(provider);
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QSharedPointer<Cache> EngineBase::cache() const
{
    return d->legacyCache;
}
#endif

void EngineBase::setTagFilter(const QStringList &filter)
{
    d->tagFilter = filter;
    for (const auto &core : std::as_const(d->providerCores)) {
        core->d->base->setTagFilter(d->tagFilter);
    }
}

QStringList EngineBase::tagFilter() const
{
    return d->tagFilter;
}

void KNSCore::EngineBase::addTagFilter(const QString &filter)
{
    d->tagFilter << filter;
    for (const auto &core : std::as_const(d->providerCores)) {
        core->d->base->setTagFilter(d->tagFilter);
    }
}

void EngineBase::setDownloadTagFilter(const QStringList &filter)
{
    d->downloadTagFilter = filter;
    for (const auto &core : std::as_const(d->providerCores)) {
        core->d->base->setDownloadTagFilter(d->downloadTagFilter);
    }
}

QStringList EngineBase::downloadTagFilter() const
{
    return d->downloadTagFilter;
}

void EngineBase::addDownloadTagFilter(const QString &filter)
{
    d->downloadTagFilter << filter;
    for (const auto &core : std::as_const(d->providerCores)) {
        core->d->base->setDownloadTagFilter(d->downloadTagFilter);
    }
}

QList<Attica::Provider *> EngineBase::atticaProviders() const
{
    // This function is absolutely horrific. Unfortunately used in discover.
    QList<Attica::Provider *> ret;
    ret.reserve(d->providerCores.size());
    for (const auto &core : d->providerCores) {
        if (const auto &provider = qobject_cast<AtticaProvider *>(core->d->base)) {
            ret.append(provider->provider());
        }
    }
    return ret;
}

bool EngineBase::userCanVote(const Entry &entry)
{
    const auto &core = d->providerCores.value(entry.providerId());
    return core->d->base->userCanVote();
}

void EngineBase::vote(const Entry &entry, uint rating)
{
    const auto &core = d->providerCores.value(entry.providerId());
    core->d->base->vote(entry, rating);
}

bool EngineBase::userCanBecomeFan(const Entry &entry)
{
    const auto &core = d->providerCores.value(entry.providerId());
    return core->d->base->userCanBecomeFan();
}

void EngineBase::becomeFan(const Entry &entry)
{
    const auto &core = d->providerCores.value(entry.providerId());
    core->d->base->becomeFan(entry);
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QSharedPointer<Provider> EngineBase::provider(const QString &providerId) const
{
    return d->legacyProviders.value(providerId);
}
#endif

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QSharedPointer<Provider> EngineBase::defaultProvider() const
{
    if (d->legacyProviders.count() > 0) {
        return d->legacyProviders.constBegin().value();
    }
    return nullptr;
}
#endif

QStringList EngineBase::providerIDs() const
{
    return d->legacyProviders.keys();
}

bool EngineBase::hasAdoptionCommand() const
{
    return !d->adoptionCommand.isEmpty();
}

void EngineBase::updateStatus()
{
}

Installation *EngineBase::installation() const
{
    return d->installation;
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
ResultsStream *EngineBase::search(const Provider::SearchRequest &request)
{
    return new ResultsStream(searchRequestFromLegacy(request), this);
}
#endif

EngineBase::ContentWarningType EngineBase::contentWarningType() const
{
    return d->contentWarningType;
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)
QList<QSharedPointer<Provider>> EngineBase::providers() const
{
    return d->legacyProviders.values();
}
#endif

KNSCore::ResultsStream *KNSCore::EngineBase::search(const KNSCore::SearchRequest &request)
{
    return new ResultsStream(request, this);
}
