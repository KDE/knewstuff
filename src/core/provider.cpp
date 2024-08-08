/*
    knewstuff3/provider.cpp
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "provider.h"

#include "provider_p.h"
#include "xmlloader_p.h"

#include <KLocalizedString>

#include <QTimer>

#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(6, 9)

namespace KNSCore
{

QString Provider::SearchRequest::hashForRequest() const
{
    return QString::number((int)sortMode) + QLatin1Char(',') + searchTerm + QLatin1Char(',') + categories.join(QLatin1Char('-')) + QLatin1Char(',')
        + QString::number(page) + QLatin1Char(',') + QString::number(pageSize);
}

Provider::Provider()
    : d(new ProviderPrivate(this))
{
}

Provider::~Provider() = default;

QString Provider::name() const
{
    return d->name;
}

QUrl Provider::icon() const
{
    return d->icon;
}

void Provider::setTagFilter(const QStringList &tagFilter)
{
    d->tagFilter = tagFilter;
    Q_EMIT tagFilterChanged();
}

QStringList Provider::tagFilter() const
{
    return d->tagFilter;
}

void Provider::setDownloadTagFilter(const QStringList &downloadTagFilter)
{
    d->downloadTagFilter = downloadTagFilter;
    Q_EMIT downloadTagFilterChanged();
}

QStringList Provider::downloadTagFilter() const
{
    return d->downloadTagFilter;
}

QDebug operator<<(QDebug dbg, const Provider::SearchRequest &search)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "Provider::SearchRequest(";
    dbg << "searchTerm: " << search.searchTerm << ',';
    dbg << "categories: " << search.categories << ',';
    dbg << "filter: " << search.filter << ',';
    dbg << "page: " << search.page << ',';
    dbg << "pageSize: " << search.pageSize;
    dbg << ')';
    return dbg;
}

QString Provider::version() const
{
    d->updateOnFirstBasicsGet();
    return d->version;
}

void Provider::setVersion(const QString &version)
{
    if (d->version != version) {
        d->version = version;
        d->throttleBasics();
    }
}

QUrl Provider::website() const
{
    d->updateOnFirstBasicsGet();
    return d->website;
}

void Provider::setWebsite(const QUrl &website)
{
    if (d->website != website) {
        d->website = website;
        d->throttleBasics();
    }
}

QUrl Provider::host() const
{
    d->updateOnFirstBasicsGet();
    return d->host;
}

void Provider::setHost(const QUrl &host)
{
    if (d->host != host) {
        d->host = host;
        d->throttleBasics();
    }
}

QString Provider::contactEmail() const
{
    d->updateOnFirstBasicsGet();
    return d->contactEmail;
}

void Provider::setContactEmail(const QString &contactEmail)
{
    if (d->contactEmail != contactEmail) {
        d->contactEmail = contactEmail;
        d->throttleBasics();
    }
}

bool Provider::supportsSsl() const
{
    d->updateOnFirstBasicsGet();
    return d->supportsSsl;
}

void Provider::setSupportsSsl(bool supportsSsl)
{
    if (d->supportsSsl != supportsSsl) {
        d->supportsSsl = supportsSsl;
        d->throttleBasics();
    }
}

void Provider::setName(const QString &name)
{
    d->name = name;
}

void Provider::setIcon(const QUrl &icon)
{
    d->icon = icon;
}
}

#include "moc_provider.cpp"

#endif
