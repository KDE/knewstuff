/*
    This file is part of KNewStuff2.
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>

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

// unit test for author

#include <QtTest>
#include <QString>

#include "../src/core/author.h"

const QString name = QStringLiteral("Name");
const QString email = QStringLiteral("Email@nowhere.com");
const QString jabber = QStringLiteral("something@kdetalk.net");
const QString homepage = QStringLiteral("http://www.myhomepage.com");

class testAuthor: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testProperties();
    void testCopy();
    void testAssignment();
};

void testAuthor::testProperties()
{
    KNSCore::Author author;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    QCOMPARE(author.name(), name);
    QCOMPARE(author.email(), email);
    QCOMPARE(author.jabber(), jabber);
    QCOMPARE(author.homepage(), homepage);
}

void testAuthor::testCopy()
{
    KNSCore::Author author;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    KNSCore::Author author2(author);
    QCOMPARE(author.name(), author2.name());
    QCOMPARE(author.email(), author2.email());
    QCOMPARE(author.jabber(), author2.jabber());
    QCOMPARE(author.homepage(), author2.homepage());
}

void testAuthor::testAssignment()
{
    KNSCore::Author author;
    KNSCore::Author author2;
    author.setName(name);
    author.setEmail(email);
    author.setJabber(jabber);
    author.setHomepage(homepage);
    author2 = author;
    QCOMPARE(author.name(), author2.name());
    QCOMPARE(author.email(), author2.email());
    QCOMPARE(author.jabber(), author2.jabber());
    QCOMPARE(author.homepage(), author2.homepage());
}

QTEST_GUILESS_MAIN(testAuthor)
#include "knewstuffauthortest.moc"
