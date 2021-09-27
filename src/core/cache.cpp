/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "cache.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QTimer>
#include <QXmlStreamReader>
#include <knewstuffcore_debug.h>
#include <qstandardpaths.h>

class KNSCore::CachePrivate
{
public:
    CachePrivate(Cache *qq)
        : q(qq)
    {
    }
    ~CachePrivate()
    {
    }

    Cache *q;
    QHash<QString, EntryInternal::List> requestCache;

    QPointer<QTimer> throttleTimer;
    void throttleWrite()
    {
        if (!throttleTimer) {
            throttleTimer = new QTimer(q);
            QObject::connect(throttleTimer, &QTimer::timeout, q, [this]() {
                q->writeRegistry();
            });
            throttleTimer->setSingleShot(true);
            throttleTimer->setInterval(1000);
        }
        throttleTimer->start();
    }
};

using namespace KNSCore;

typedef QHash<QString, QWeakPointer<Cache>> CacheHash;
Q_GLOBAL_STATIC(CacheHash, s_caches)

Cache::Cache(const QString &appName)
    : QObject(nullptr)
    , d(new CachePrivate(this))
{
    m_kns2ComponentName = appName;

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/");
    QDir().mkpath(path);
    registryFile = path + appName + QStringLiteral(".knsregistry");
    qCDebug(KNEWSTUFFCORE) << "Using registry file: " << registryFile;
    setProperty("dirty", false); // KF6 make normal variable

    QFileSystemWatcher *watcher = new QFileSystemWatcher(QStringList{registryFile}, this);
    std::function<void()> changeChecker = [this, &changeChecker]() {
        if (property("writingRegistry").toBool()) {
            QTimer::singleShot(0, this, changeChecker);
        } else {
            setProperty("reloadingRegistry", true);
            const QSet<KNSCore::EntryInternal> oldCache = cache;
            cache.clear();
            readRegistry();
            // First run through the old cache and see if any have disappeared (at
            // which point we need to set them as available and emit that change)
            for (const EntryInternal &entry : oldCache) {
                if (!cache.contains(entry)) {
                    EntryInternal removedEntry(entry);
                    removedEntry.setStatus(KNS3::Entry::Deleted);
                    Q_EMIT entryChanged(removedEntry);
                }
            }
            // Then run through the new cache and see if there's any that were not
            // in the old cache (at which point just emit those as having changed,
            // they're already the correct status)
            for (const EntryInternal &entry : cache) {
                auto iterator = oldCache.constFind(entry);
                if (iterator == oldCache.constEnd()) {
                    Q_EMIT entryChanged(entry);
                } else if ((*iterator).status() != entry.status()) {
                    // If there are entries which are in both, but which have changed their
                    // status, we should adopt the status from the newly loaded cache in place
                    // of the one in the old cache. In reality, what this means is we just
                    // need to emit the changed signal for anything in the new cache which
                    // doesn't match the old one
                    Q_EMIT entryChanged(entry);
                }
            }
            setProperty("reloadingRegistry", false);
        }
    };
    connect(watcher, &QFileSystemWatcher::fileChanged, this, changeChecker);
}

QSharedPointer<Cache> Cache::getCache(const QString &appName)
{
    CacheHash::const_iterator it = s_caches()->constFind(appName);
    if ((it != s_caches()->constEnd()) && !(*it).isNull()) {
        return QSharedPointer<Cache>(*it);
    }

    QSharedPointer<Cache> p(new Cache(appName));
    s_caches()->insert(appName, QWeakPointer<Cache>(p));
    QObject::connect(p.data(), &QObject::destroyed, [appName] {
        if (auto cache = s_caches()) {
            cache->remove(appName);
        }
    });

    return p;
}

Cache::~Cache()
{
}

void Cache::readRegistry()
{
#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 77)
    // read KNS2 registry first to migrate it
    readKns2MetaFiles();
#endif

    QFile f(registryFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (QFileInfo::exists(registryFile)) {
            qWarning() << "The file " << registryFile << " could not be opened.";
        }
        return;
    }

    QXmlStreamReader reader(&f);
    if (reader.hasError() || !reader.readNextStartElement()) {
        qCWarning(KNEWSTUFFCORE) << "The file could not be parsed.";
        return;
    }

    if (reader.name() != QLatin1String("hotnewstuffregistry")) {
        qCWarning(KNEWSTUFFCORE) << "The file doesn't seem to be of interest.";
        return;
    }

    for (auto token = reader.readNext(); !reader.atEnd(); token = reader.readNext()) {
        if (token != QXmlStreamReader::StartElement) {
            continue;
        }
        EntryInternal e;
        e.setEntryXML(reader);
        e.setSource(EntryInternal::Cache);
        cache.insert(e);
        Q_ASSERT(reader.tokenType() == QXmlStreamReader::EndElement);
    }

    qCDebug(KNEWSTUFFCORE) << "Cache read... entries: " << cache.size();
}

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 77)
void Cache::readKns2MetaFiles()
{
    qCDebug(KNEWSTUFFCORE) << "Loading KNS2 registry of files for the component: " << m_kns2ComponentName;

    const auto realAppName = m_kns2ComponentName.split(QLatin1Char(':'))[0];

    const QStringList dirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("knewstuff2-entries.registry"), QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        qCDebug(KNEWSTUFFCORE) << QStringLiteral(" + Load from directory '") + (*it) + QStringLiteral("'.");
        QDir dir((*it));
        const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QStringList::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
            QString filepath = (*it) + QLatin1Char('/') + (*fit);

            qCDebug(KNEWSTUFFCORE) << QStringLiteral(" Load from file '") + filepath + QStringLiteral("'.");

            QFileInfo info(filepath);
            QFile f(filepath);

            // first see if this file is even for this app
            // because the registry contains entries for all apps
            // FIXMEE: should be able to do this with a filter on the entryList above probably
            QString thisAppName = QString::fromUtf8(QByteArray::fromBase64(info.baseName().toUtf8()));

            // NOTE: the ":" needs to always coincide with the separator character used in
            // the id(Entry*) method
            thisAppName = thisAppName.split(QLatin1Char(':'))[0];

            if (thisAppName != realAppName) {
                continue;
            }

            if (!f.open(QIODevice::ReadOnly)) {
                qWarning() << "The file: " << filepath << " could not be opened.";
                continue;
            }

            QDomDocument doc;
            if (!doc.setContent(&f)) {
                qWarning() << "The file could not be parsed.";
                return;
            }
            qCDebug(KNEWSTUFFCORE) << "found entry: " << doc.toString();

            QDomElement root = doc.documentElement();
            if (root.tagName() != QLatin1String("ghnsinstall")) {
                qWarning() << "The file doesn't seem to be of interest.";
                return;
            }

            // The .meta files only contain one entry
            QDomElement stuff = root.firstChildElement(QStringLiteral("stuff"));
            EntryInternal e;
            e.setEntryXML(stuff);
            e.setSource(EntryInternal::Cache);

            if (e.payload().startsWith(QLatin1String("http://download.kde.org/khotnewstuff"))) {
                // This is 99% sure a opendesktop file, make it a real one.
                e.setProviderId(QStringLiteral("https://api.opendesktop.org/v1/"));
                e.setHomepage(QUrl(QString(QLatin1String("http://opendesktop.org/content/show.php?content=") + e.uniqueId())));

            } else if (e.payload().startsWith(QLatin1String("http://edu.kde.org/contrib/kvtml/"))) {
                // kvmtl-1
                e.setProviderId(QStringLiteral("http://edu.kde.org/contrib/kvtml/kvtml.xml"));
            } else if (e.payload().startsWith(QLatin1String("http://edu.kde.org/contrib/kvtml2/"))) {
                // kvmtl-2
                e.setProviderId(QStringLiteral("http://edu.kde.org/contrib/kvtml2/provider41.xml"));
            } else {
                // we failed, skip
                qWarning() << "Could not load entry: " << filepath;
                continue;
            }

            e.setStatus(KNS3::Entry::Installed);

            cache.insert(e);
            QDomDocument tmp(QStringLiteral("yay"));
            tmp.appendChild(e.entryXML());
            qCDebug(KNEWSTUFFCORE) << "new entry: " << tmp.toString();

            f.close();

            QDir dir;
            if (!dir.remove(filepath)) {
                qWarning() << "could not delete old kns2 .meta file: " << filepath;
            } else {
                qCDebug(KNEWSTUFFCORE) << "Migrated KNS2 entry to KNS3.";
            }
        }
    }
    setProperty("dirty", false);
}
#endif

EntryInternal::List Cache::registryForProvider(const QString &providerId)
{
    EntryInternal::List entries;
    for (const EntryInternal &e : std::as_const(cache)) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}

EntryInternal::List Cache::registry() const
{
    EntryInternal::List entries;
    for (const EntryInternal &e : std::as_const(cache)) {
        entries.append(e);
    }
    return entries;
}

void Cache::writeRegistry()
{
    if (!property("dirty").toBool()) {
        return;
    }

    qCDebug(KNEWSTUFFCORE) << "Write registry";

    setProperty("writingRegistry", true);
    QFile f(registryFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write meta information to '" << registryFile << "'.";
        return;
    }

    QDomDocument doc(QStringLiteral("khotnewstuff3"));
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));
    QDomElement root = doc.createElement(QStringLiteral("hotnewstuffregistry"));
    doc.appendChild(root);

    for (const EntryInternal &entry : std::as_const(cache)) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (entry.status() == KNS3::Entry::Installed || entry.status() == KNS3::Entry::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    setProperty("dirty", false);
    setProperty("writingRegistry", false);
}

void Cache::registerChangedEntry(const KNSCore::EntryInternal &entry)
{
    // If we have intermediate states, like updating or installing we do not want to write them
    if (entry.status() == KNS3::Entry::Updating || entry.status() == KNS3::Entry::Installing) {
        return;
    }
    if (!property("reloadingRegistry").toBool()) {
        setProperty("dirty", true);
        cache.remove(entry); // If value already exists in the set, the set is left unchanged
        cache.insert(entry);
        d->throttleWrite();
    }
}

void Cache::insertRequest(const KNSCore::Provider::SearchRequest &request, const KNSCore::EntryInternal::List &entries)
{
    // append new entries
    auto &cacheList = d->requestCache[request.hashForRequest()];
    for (const auto &entry : entries) {
        if (!cacheList.contains(entry)) {
            cacheList.append(entry);
        }
    }
    qCDebug(KNEWSTUFFCORE) << request.hashForRequest() << " add: " << entries.size() << " keys: " << d->requestCache.keys();
}

EntryInternal::List Cache::requestFromCache(const KNSCore::Provider::SearchRequest &request)
{
    qCDebug(KNEWSTUFFCORE) << request.hashForRequest();
    return d->requestCache.value(request.hashForRequest());
}

void KNSCore::Cache::removeDeletedEntries()
{
    QMutableSetIterator<KNSCore::EntryInternal> i(cache);
    while (i.hasNext()) {
        const KNSCore::EntryInternal &entry = i.next();
        bool installedFileExists{false};
        const QStringList installedFiles = entry.installedFiles();
        for (const auto &installedFile : installedFiles) {
            // Handle the /* notation, BUG: 425704
            if (installedFile.endsWith(QLatin1String("/*"))) {
                if (QDir(installedFile.left(installedFile.size() - 2)).exists()) {
                    installedFileExists = true;
                    break;
                }
            } else if (QFile::exists(installedFile)) {
                installedFileExists = true;
                break;
            }
        }
        if (!installedFileExists) {
            i.remove();
            setProperty("dirty", true);
        }
    }
    writeRegistry();
}

KNSCore::EntryInternal KNSCore::Cache::entryFromInstalledFile(const QString &installedFile) const
{
    for (const EntryInternal &entry : cache) {
        if (entry.installedFiles().contains(installedFile)) {
            return entry;
        }
    }
    return EntryInternal{};
}
