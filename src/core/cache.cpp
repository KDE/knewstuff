/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>
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

#include "cache.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <qstandardpaths.h>
#include <knewstuffcore_debug.h>

using namespace KNSCore;

typedef QHash<QString, QWeakPointer<Cache> > CacheHash;
Q_GLOBAL_STATIC(CacheHash, s_caches)

Cache::Cache(const QString &appName): QObject(nullptr)
{
    m_kns2ComponentName = appName;

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/");
    QDir().mkpath(path);
    registryFile = path + appName + QStringLiteral(".knsregistry");
    qCDebug(KNEWSTUFFCORE) << "Using registry file: " << registryFile;
    setProperty("dirty", false); //KF6 make normal variable
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
    // read KNS2 registry first to migrate it
    readKns2MetaFiles();

    QFile f(registryFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (QFileInfo::exists(registryFile)) {
            qWarning() << "The file " << registryFile << " could not be opened.";
        }
        return;
    }

    QXmlStreamReader reader(&f);
    if (reader.hasError() || !reader.readNextStartElement()) {
        qWarning() << "The file could not be parsed.";
        return;
    }

    if (reader.name() != QLatin1String("hotnewstuffregistry")) {
        qWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    for (auto token = reader.readNext(); !reader.atEnd(); token = reader.readNext()) {
        if (token != QXmlStreamReader::StartElement)
            continue;
        EntryInternal e;
        e.setEntryXML(reader);
        e.setSource(EntryInternal::Cache);
        cache.insert(e);
        Q_ASSERT(reader.tokenType() == QXmlStreamReader::EndElement);
    }

    qCDebug(KNEWSTUFFCORE) << "Cache read... entries: " << cache.size();
}

void Cache::readKns2MetaFiles()
{
    qCDebug(KNEWSTUFFCORE) << "Loading KNS2 registry of files for the component: " << m_kns2ComponentName;

    const auto realAppName = m_kns2ComponentName.splitRef(':')[0];

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("knewstuff2-entries.registry"), QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        qCDebug(KNEWSTUFFCORE) << " + Load from directory '" + (*it) + "'.";
        QDir dir((*it));
        const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QStringList::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
            QString filepath = (*it) + QLatin1Char('/') + (*fit);

            qCDebug(KNEWSTUFFCORE) << " Load from file '" + filepath + "'.";

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

EntryInternal::List Cache::registryForProvider(const QString &providerId)
{
    EntryInternal::List entries;
    foreach (const EntryInternal &e, cache) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}

void Cache::writeRegistry()
{
    if (!property("dirty").toBool())
        return;

    qCDebug(KNEWSTUFFCORE) << "Write registry";

    QFile f(registryFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write meta information to '" << registryFile << "'." << endl;
        return;
    }

    QDomDocument doc(QStringLiteral("khotnewstuff3"));
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));
    QDomElement root = doc.createElement(QStringLiteral("hotnewstuffregistry"));
    doc.appendChild(root);

    foreach (const EntryInternal &entry, cache) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (entry.status() == KNS3::Entry::Installed || entry.status() == KNS3::Entry::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    setProperty("dirty", false);
}

void Cache::registerChangedEntry(const KNSCore::EntryInternal &entry)
{
    setProperty("dirty", true);
    cache.insert(entry);
}

void Cache::insertRequest(const KNSCore::Provider::SearchRequest &request, const KNSCore::EntryInternal::List &entries)
{
    // append new entries
    auto &cacheList = requestCache[request.hashForRequest()];
    for (const auto &entry : entries) {
        if (!cacheList.contains(entry)) {
            cacheList.append(entry);
        }
    }
    qCDebug(KNEWSTUFFCORE) << request.hashForRequest() << " add: " << entries.size() << " keys: " << requestCache.keys();
}

EntryInternal::List Cache::requestFromCache(const KNSCore::Provider::SearchRequest &request)
{
    qDebug() << request.hashForRequest();
    return requestCache.value(request.hashForRequest());
}

