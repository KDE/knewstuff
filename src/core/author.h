/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_AUTHOR_P_H
#define KNEWSTUFF3_AUTHOR_P_H

#include <QSharedData>
#include <QString>
#include <QUrl>

#include "knewstuffcore_export.h"

namespace KNSCore
{
class AuthorPrivate;

/*!
 * \class KNSCore::Author
 * \inheaderfile KNSCore/Author
 * \inmodule KNewStuffCore
 *
 * \brief KNewStuff author information.
 *
 * This class provides accessor methods to the author data
 * as used by KNewStuff.
 *
 * \warning This class should probably not be used directly by the application.
 */
class KNEWSTUFFCORE_EXPORT Author
{
    Q_GADGET

    /*!
     * \qmlproperty string Author::name
     */
    /*!
     * \property KNSCore::Author::name
     */
    Q_PROPERTY(QString name READ name)

    /*!
     * \qmlproperty string Author::email
     */
    /*!
     * \property KNSCore::Author::email
     */
    Q_PROPERTY(QString email READ email)

public:
    /*!
     *
     */
    explicit Author();
    Author(const Author &other);
    Author &operator=(const Author &other);
    ~Author();

    /*!
     * Sets the author's user ID to \a id.
     */
    void setId(const QString &id);

    /*!
     * Returns the author's user ID.
     */
    QString id() const;

    /*!
     * Sets the author's full \a name.
     */
    void setName(const QString &name);

    /*!
     * Returns the author's full name.
     */
    QString name() const;

    /*!
     * Sets the author's \a email address.
     */
    void setEmail(const QString &email);

    /*!
     * Returns the author's email address.
     */
    QString email() const;

    /*!
     * Sets the author's \a jabber address.
     */
    void setJabber(const QString &jabber);

    /*!
     * Returns the author's jabber address.
     */
    QString jabber() const;

    /*!
     * Sets the author's \a homepage URL.
     */
    void setHomepage(const QString &homepage);

    /*!
     * Returns the author's homepage URL.
     */
    QString homepage() const;

    /*!
     * Sets the profile page of the author, usually located on the server hosting the content.
     *
     * \a profilepage is the URL of the author's profile page
     *
     */
    void setProfilepage(const QString &profilepage);

    /*!
     * Returns the author's profile page URL.
     */
    QString profilepage() const;

    /*!
     * Sets the author's avatar image URL to \a avatarUrl.
     */
    void setAvatarUrl(const QUrl &avatarUrl);

    /*!
     * Returns the author's avatar image URL (may be empty).
     */
    QUrl avatarUrl() const;

    /*!
     * Returns A long(ish)-form text describing this author, usually self-entered.
     */
    QString description() const;

    /*!
     * Sets the author's \a description.
     */
    void setDescription(const QString &description);

private:
    QSharedDataPointer<AuthorPrivate> d;
};

}

#endif
