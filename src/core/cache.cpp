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

#include "cache_p.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QXmlStreamReader>
#include <qstandardpaths.h>
#include <QDebug>

using namespace KNS3;

typedef QHash<QString, QWeakPointer<Cache> > CacheHash;
Q_GLOBAL_STATIC(CacheHash, s_caches)

Cache::Cache(const QString &appName): QObject(0)
{
    m_kns2ComponentName = appName;

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QLatin1String("knewstuff3/");
    QDir().mkpath(path);
    registryFile = path + appName + ".knsregistry";
    qCDebug(KNEWSTUFF) << "Using registry file: " << registryFile;
}

QSharedPointer<Cache> Cache::getCache(const QString &appName)
{
    CacheHash::const_iterator it = s_caches()->constFind(appName);
    if ((it != s_caches()->constEnd()) && !(*it).isNull()) {
        return QSharedPointer<Cache>(*it);
    }

    QSharedPointer<Cache> p(new Cache(appName));
    s_caches()->insert(appName, QWeakPointer<Cache>(p));

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
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning() << "The file " << registryFile << " could not be opened.";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&f)) {
        qWarning() << "The file could not be parsed.";
        return;
    }

    QDomElement root = doc.documentElement();
    if (root.tagName() != QLatin1String("hotnewstuffregistry")) {
        qWarning() << "The file doesn't seem to be of interest.";
        return;
    }

    QDomElement stuff = root.firstChildElement(QStringLiteral("stuff"));
    while (!stuff.isNull()) {
        EntryInternal e;
        e.setEntryXML(stuff);
        e.setSource(EntryInternal::Cache);
        cache.insert(e);
        stuff = stuff.nextSiblingElement(QStringLiteral("stuff"));
    }

    qCDebug(KNEWSTUFF) << "Cache read... entries: " << cache.size();
}

void Cache::readKns2MetaFiles()
{
    qCDebug(KNEWSTUFF) << "Loading KNS2 registry of files for the component: " << m_kns2ComponentName;

    QString realAppName = m_kns2ComponentName.split(':')[0];

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("knewstuff2-entries.registry"), QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = dirs.begin(); it != dirs.end(); ++it) {
        qCDebug(KNEWSTUFF) << " + Load from directory '" + (*it) + "'.";
        QDir dir((*it));
        const QStringList files = dir.entryList(QDir::Files | QDir::Readable);
        for (QStringList::const_iterator fit = files.begin(); fit != files.end(); ++fit) {
            QString filepath = (*it) + '/' + (*fit);

            qCDebug(KNEWSTUFF) << " Load from file '" + filepath + "'.";

            QFileInfo info(filepath);
            QFile f(filepath);

            // first see if this file is even for this app
            // because the registry contains entries for all apps
            // FIXMEE: should be able to do this with a filter on the entryList above probably
            QString thisAppName = QString::fromUtf8(QByteArray::fromBase64(info.baseName().toUtf8()));

            // NOTE: the ":" needs to always coincide with the separator character used in
            // the id(Entry*) method
            thisAppName = thisAppName.split(':')[0];

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
            qCDebug(KNEWSTUFF) << "found entry: " << doc.toString();

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

            e.setStatus(Entry::Installed);

            cache.insert(e);
            QDomDocument tmp(QStringLiteral("yay"));
            tmp.appendChild(e.entryXML());
            qCDebug(KNEWSTUFF) << "new entry: " << tmp.toString();

            f.close();

            QDir dir;
            if (!dir.remove(filepath)) {
                qWarning() << "could not delete old kns2 .meta file: " << filepath;
            } else {
                qCDebug(KNEWSTUFF) << "Migrated KNS2 entry to KNS3.";
            }

        }
    }
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
    qCDebug(KNEWSTUFF) << "Write registry";

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
        if (entry.status() == Entry::Installed || entry.status() == Entry::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    f.close();
}

void Cache::registerChangedEntry(const KNS3::EntryInternal &entry)
{
    cache.insert(entry);
}

void Cache::insertRequest(const KNS3::Provider::SearchRequest &request, const KNS3::EntryInternal::List &entries)
{
    // append new entries
    requestCache[request.hashForRequest()].append(entries);
    qCDebug(KNEWSTUFF) << request.hashForRequest() << " add: " << entries.size() << " keys: " << requestCache.keys();
}

EntryInternal::List Cache::requestFromCache(const KNS3::Provider::SearchRequest &request)
{
    qCDebug(KNEWSTUFF) << request.hashForRequest();
    return requestCache.value(request.hashForRequest());
}

