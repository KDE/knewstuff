/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

// unit test for entry

#include <QString>
#include <QTest>
#include <QXmlStreamReader>

#include "../src/entry.h"
#include "../src/entry_p.h"

#include <knewstuffcore_debug.h>

const char *entryXML =
    "<stuff category=\"Category\"> "
    "<name>Name</name>"
    "<providerid>https://api.opendesktop.org/v1/</providerid>"
    "<author homepage=\"http://testhomepage\">testauthor</author>"
    "<homepage>https://testhomepage</homepage>"
    "<licence>3</licence>" // krazy:exclude=spelling
    "<version>4.0</version>"
    "<rating>82</rating>"
    "<downloads>128891</downloads>"
    "<installedfile>/some/test/path.jpg</installedfile>"
    "<id>12345</id>"
    "<releasedate>2008-08-12<!--randomcomment--></releasedate>"
    "<summary>new version 3.0</summary>"
    "<!--randomcomment-->"
    "<changelog>Updated</changelog>"
    "<preview>https://testpreview</preview>"
    "<previewBig>https://testpreview</previewBig>"
    "<payload>http://testpayload</payload>"
    "<status>"
    "<!--randomcomment-->"
    "installed"
    "<!--randomcomment-->"
    "</status>"
    "<tags>ghns_excluded=1</tags>"
    "</stuff>";

const QString name = QStringLiteral("Name");
const QString category = QStringLiteral("Category");
const QString summary = QStringLiteral("new version 3.0");
const QString version = QStringLiteral("4.0");
const QString license = QStringLiteral("3");

class testEntry : public QObject
{
    Q_OBJECT
private:
    KNS3::Entry createEntryOld();
    KNS3::Entry createEntry();
private Q_SLOTS:
    void testProperties();
    void testCopy();
    void testAssignment();
    void testDomImplementation();
};

KNS3::Entry testEntry::createEntryOld()
{
    QDomDocument document;
    document.setContent(QString::fromLatin1(entryXML));
    QDomElement node = document.documentElement();
    KNSCore::EntryInternal entryInternal;
    bool xmlResult = entryInternal.setEntryXML(node);
    qCDebug(KNEWSTUFFCORE) << "Created entry from XML " << xmlResult;
    return KNS3::EntryPrivate::fromInternal(&entryInternal);
}

KNS3::Entry testEntry::createEntry()
{
    QXmlStreamReader reader;
    reader.addData(entryXML);
    KNSCore::EntryInternal entryInternal;
    bool xmlResult = reader.readNextStartElement() && entryInternal.setEntryXML(reader);
    qCDebug(KNEWSTUFFCORE) << "Created entry from XML " << xmlResult;
    return KNS3::EntryPrivate::fromInternal(&entryInternal);
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

void testEntry::testDomImplementation()
{
    KNS3::Entry entry = createEntry();
    KNS3::Entry entry2 = createEntryOld();

    QCOMPARE(entry.name(), entry2.name());
    QCOMPARE(entry.category(), entry2.category());
    QCOMPARE(entry.license(), entry2.license());
    QCOMPARE(entry.summary(), entry2.summary());
    QCOMPARE(entry.version(), entry2.version());
}

QTEST_GUILESS_MAIN(testEntry)
#include "knewstuffentrytest.moc"
