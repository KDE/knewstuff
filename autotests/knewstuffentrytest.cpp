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

// unit test for entry

#include <QtTest/QtTest>
#include <QString>

#include "../src/entry.h"
#include "../src/entry_p.h"

const char *entryXML = "<stuff category=\"Category\"> "
"<name>Name</name>"
"<providerid>https://api.opendesktop.org/v1/</providerid>"
"<author homepage=\"http://testhomepage\">testauthor</author>"
"<homepage>https://testhomepage</homepage>"
"<licence>3</licence>"   // krazy:exclude=spelling
"<version>4.0</version>"
"<rating>82</rating>"
"<downloads>128891</downloads>"
"<installedfile>/some/test/path.jpg</installedfile>"
"<id>12345</id>"
"<releasedate>2008-08-12</releasedate>"
"<summary>new version 3.0</summary>"
"<changelog>Updated</changelog>"
"<preview>https://testpreview</preview>"
"<previewBig>https://testpreview</previewBig>"
"<payload>http://testpayload</payload>"
"<status>installed</status>"
"</stuff>";

const QString name = QLatin1String("Name");
const QString category = QLatin1String("Category");
const QString summary = QLatin1String("new version 3.0");
const QString version = QLatin1String("4.0");
const QString license = QLatin1String("3");

class testEntry: public QObject
{
    Q_OBJECT
private:
    KNS3::Entry createEntry();
private Q_SLOTS:
    void testProperties();
    void testCopy();
    void testAssignment();
};

KNS3::Entry testEntry::createEntry()
{
    QDomDocument document;
    document.setContent(QString::fromLatin1(entryXML));
    QDomElement node = document.documentElement();
    KNS3::EntryInternal entryInternal;
    qCDebug(KNEWSTUFF) << "Created entry from XML " << entryInternal.setEntryXML(node);
    return entryInternal.toEntry();
}

void testEntry::testProperties()
{
    KNS3::Entry entry = createEntry();

    QCOMPARE(entry.name(), name);
    QCOMPARE(entry.category(), category);
    QCOMPARE(entry.license(), license);
    QCOMPARE(entry.summary(), summary);
    QCOMPARE(entry.version(), version);
}

void testEntry::testCopy()
{
    KNS3::Entry entry = createEntry();
    KNS3::Entry entry2(entry);

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

void testEntry::testAssignment()
{
    KNS3::Entry entry = createEntry();
    KNS3::Entry entry2 = entry;

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

QTEST_MAIN(testEntry)
#include "knewstuffentrytest.moc"
