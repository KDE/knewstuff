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
#ifndef KNEWSTUFF3_AUTHOR_P_H
#define KNEWSTUFF3_AUTHOR_P_H

#include <QString>
#include <QUrl>

#include "knewstuffcore_export.h"

namespace KNSCore
{
struct AuthorPrivate;

/**
 * @short KNewStuff author information.
 *
 * This class provides accessor methods to the author data
 * as used by KNewStuff.
 * It should probably not be used directly by the application.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class KNEWSTUFFCORE_EXPORT Author
{
public:
    explicit Author();
    Author(const Author &other);
    Author& operator=(const Author &other) = default;
    Author& operator=(Author&&) = default;
    ~Author();

    /**
     * Sets the user ID of the author.
     */
    void setId(const QString &id);

    /**
     * Retrieve the author's user ID
     * @return the author's user ID
     */
    QString id() const;

    /**
     * Sets the full name of the author.
     */
    void setName(const QString &name);

    /**
     * Retrieve the author's name.
     *
     * @return author name
     */
    QString name() const;

    /**
     * Sets the email address of the author.
     */
    void setEmail(const QString &email);

    /**
     * Retrieve the author's email address.
     *
     * @return author email address
     */
    QString email() const;

    /**
     * Sets the jabber address of the author.
     */
    void setJabber(const QString &jabber);

    /**
     * Retrieve the author's jabber address.
     *
     * @return author jabber address
     */
    QString jabber() const;

    /**
     * Sets the homepage of the author.
     */
    void setHomepage(const QString &homepage);

    /**
     * Retrieve the author's homepage.
     *
     * @return author homepage
     */
    QString homepage() const;

    /**
     * Sets the profile page of the author, usually located on the server hosting the content.
     */
    void setProfilepage(const QString &profilepage);

    /**
     * Retrieve the author's profile page.
     *
     * @return author profile page
     */
    QString profilepage() const;

    /**
     * Sets the url for the user's avatar image
     */
    void setAvatarUrl(const QUrl &avatarUrl);

    /**
     * Retrieve the url of the user's avatar image
     *
     * @return a url for the user's avatar (may be empty)
     */
    QUrl avatarUrl() const;

    /**
     * Retrieve the user's description text
     *
     * @return A long(ish)-form text describing this user, usually self-entered
     */
    QString description() const;
    /**
     * Set the user's description
     */
    void setDescription(const QString &description);
private:
    QString mName;
    QString mEmail;
    QString mJabber;
    QString mHomepage;
};

}

#endif
