/*
    knewstuff3/provider.cpp
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#include "provider.h"

#include "xmlloader.h"

#include <KLocalizedString>

namespace KNSCore
{

// BCI: Add a real d-pointer
class ProviderPrivate {
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
    return QString(QString::number((int)sortMode) + QLatin1Char(',')
                   + searchTerm + QLatin1Char(',')
                   + categories.join(QLatin1Char('-')) + QLatin1Char(',')
                   + QString::number(page) + QLatin1Char(',')
                   + QString::number(pageSize));
}

Provider::Provider()
{}

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

QDebug operator<<(QDebug dbg, const Provider::SearchRequest & search)
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

