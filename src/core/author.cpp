/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
