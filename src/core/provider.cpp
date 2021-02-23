/*
    knewstuff3/provider.cpp
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "provider.h"

#include "xmlloader.h"

#include <KLocalizedString>

namespace KNSCore
{
// TODO KF6 BCI: Add a real d-pointer
class ProviderPrivate
{
public:
    QStringList tagFilter;
    QStringList downloadTagFilter;
};
typedef QHash<const Provider *, ProviderPrivate *> ProviderPrivateHash;
Q_GLOBAL_STATIC(ProviderPrivateHash, d_func)

static ProviderPrivate *d(const Provider *provider)
{
    ProviderPrivate *ret = d_func()->value(provider);
    if (!ret) {
        ret = new ProviderPrivate;
        d_func()->insert(provider, ret);
    }
    return ret;
}

static void delete_d(const Provider *provider)
{
    if (auto d = d_func()) {
        delete d->take(provider);
    }
}

QString Provider::SearchRequest::hashForRequest() const
{
    return QString(QString::number((int)sortMode) + QLatin1Char(',') + searchTerm + QLatin1Char(',') + categories.join(QLatin1Char('-')) + QLatin1Char(',')
                   + QString::number(page) + QLatin1Char(',') + QString::number(pageSize));
}

Provider::Provider()
{
}

Provider::~Provider()
{
    delete_d(this);
}

QString Provider::name() const
{
    return mName;
}

QUrl Provider::icon() const
{
    return mIcon;
}

void Provider::setTagFilter(const QStringList &tagFilter)
{
    d(this)->tagFilter = tagFilter;
}

QStringList Provider::tagFilter() const
{
    return d(this)->tagFilter;
}

void Provider::setDownloadTagFilter(const QStringList &downloadTagFilter)
{
    d(this)->downloadTagFilter = downloadTagFilter;
}

QStringList Provider::downloadTagFilter() const
{
    return d(this)->downloadTagFilter;
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

}
