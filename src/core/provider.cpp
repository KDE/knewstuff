/*
    knewstuff3/provider.cpp
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "provider.h"

#include "xmlloader_p.h"

#include <KLocalizedString>

#include <QTimer>

namespace KNSCore
{
// TODO KF6 BCI: Add a real d-pointer
class ProviderPrivate
{
public:
    Provider *q;
    QStringList tagFilter;
    QStringList downloadTagFilter;

    QTimer *basicsThrottle{nullptr};
    QString version;
    QUrl website;
    QUrl host;
    QString contactEmail;
    bool supportsSsl{false};
    bool basicsGot{false};
    void updateOnFirstBasicsGet()
    {
        if (!basicsGot) {
            basicsGot = true;
            QTimer::singleShot(0, q, &Provider::loadBasics);
        }
    };
    void throttleBasics()
    {
        if (!basicsThrottle) {
            basicsThrottle = new QTimer(q);
            basicsThrottle->setInterval(0);
            basicsThrottle->setSingleShot(true);
            QObject::connect(basicsThrottle, &QTimer::timeout, q, &Provider::basicsLoaded);
        }
        basicsThrottle->start();
    }
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
    d(this)->q = this;
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

QString Provider::version() const
{
    d(this)->updateOnFirstBasicsGet();
    return d(this)->version;
}

void Provider::setVersion(const QString &version)
{
    if (d(this)->version != version) {
        d(this)->version = version;
        d(this)->throttleBasics();
    }
}

QUrl Provider::website() const
{
    d(this)->updateOnFirstBasicsGet();
    return d(this)->website;
}

void Provider::setWebsite(const QUrl &website)
{
    if (d(this)->website != website) {
        d(this)->website = website;
        d(this)->throttleBasics();
    }
}

QUrl Provider::host() const
{
    d(this)->updateOnFirstBasicsGet();
    return d(this)->host;
}

void Provider::setHost(const QUrl &host)
{
    if (d(this)->host != host) {
        d(this)->host = host;
        d(this)->throttleBasics();
    }
}

QString Provider::contactEmail() const
{
    d(this)->updateOnFirstBasicsGet();
    return d(this)->contactEmail;
}

void Provider::setContactEmail(const QString &contactEmail)
{
    if (d(this)->contactEmail != contactEmail) {
        d(this)->contactEmail = contactEmail;
        d(this)->throttleBasics();
    }
}

bool Provider::supportsSsl() const
{
    d(this)->updateOnFirstBasicsGet();
    return d(this)->supportsSsl;
}

void Provider::setSupportsSsl(bool supportsSsl)
{
    if (d(this)->supportsSsl != supportsSsl) {
        d(this)->supportsSsl = supportsSsl;
        d(this)->throttleBasics();
    }
}
}
