/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>

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

#include "author.h"

#include <QHash>

// BCI: Add a real d-pointer
namespace KNSCore {
struct AuthorPrivate {
public:
    QString id;
    QString profilepage;
    QUrl avatarUrl;
    QString description;
};
}

using namespace KNSCore;

typedef QHash<const Author *, AuthorPrivate*> AuthorPrivateHash;
Q_GLOBAL_STATIC(AuthorPrivateHash, d_func)

static AuthorPrivate *d(const Author *author)
{
    AuthorPrivate *ret = d_func()->value(author);
    if (!ret) {
        ret = new AuthorPrivate;
        d_func()->insert(author, ret);
    }
    return ret;
}

static void delete_d(const Author *author)
{
    if (auto d = d_func()) {
        delete d->take(author);
    }
}

Author::Author()
{
}

KNSCore::Author::Author(const KNSCore::Author &other)
{
    this->setAvatarUrl(other.avatarUrl());
    this->setDescription(other.description());
    this->setEmail(other.email());
    this->setHomepage(other.homepage());
    this->setId(other.id());
    this->setJabber(other.jabber());
    this->setName(other.name());
    this->setProfilepage(other.profilepage());
}

Author::~Author()
{
    delete_d(this);
}

void KNSCore::Author::setId(const QString &id)
{
    d(this)->id = id;
}

QString KNSCore::Author::id() const
{
    return d(this)->id;
}

void Author::setName(const QString &_name)
{
    mName = _name;
}

QString Author::name() const
{
    return mName;
}

void Author::setEmail(const QString &_email)
{
    mEmail = _email;
}

QString Author::email() const
{
    return mEmail;
}

void Author::setJabber(const QString &_jabber)
{
    mJabber = _jabber;
}

QString Author::jabber() const
{
    return mJabber;
}

void Author::setHomepage(const QString &_homepage)
{
    mHomepage = _homepage;
}

QString Author::homepage() const
{
    return mHomepage;
}

void Author::setProfilepage(const QString &profilepage)
{
    d(this)->profilepage = profilepage;
}

QString Author::profilepage() const
{
    return d(this)->profilepage;
}

void Author::setAvatarUrl(const QUrl &avatarUrl)
{
    d(this)->avatarUrl = avatarUrl;
}

QUrl Author::avatarUrl() const
{
    return d(this)->avatarUrl;
}

void Author::setDescription(const QString &description)
{
    d(this)->description = description;
}

QString Author::description() const
{
    return d(this)->description;
}
