/*
    This file is part of KNewStuff2.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 - 2007 Josef Spillner <spillner@kde.org>
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

#include "entryinternal.h"

#include <QStringList>
#include <QXmlStreamReader>
#include <QImage>
#include <knewstuffcore_debug.h>

#include "xmlloader.h"
#include "../entry_p.h" // For Entry::Status ONLY!


using namespace KNSCore;

class EntryInternal::Private : public QSharedData
{
public:
    Private()
        : mReleaseDate(QDate::currentDate())
        , mRating(0)
        , mNumberOfComments(0)
        , mDownloadCount(0)
        , mNumberFans(0)
        , mNumberKnowledgebaseEntries(0)
        , mStatus(KNS3::Entry::Invalid)
        , mSource(EntryInternal::Online)
    {
        qRegisterMetaType<KNSCore::EntryInternal::List>();
    }

    bool operator==(const Private &other) const
    {
        return mUniqueId == other.mUniqueId && mProviderId == other.mProviderId;
    }

    QString mUniqueId;
    QString mName;
    QUrl mHomepage;
    QString mCategory;
    QString mLicense;
    QString mVersion;
    QDate mReleaseDate;

    // Version and date if a newer version is found (updateable)
    QString mUpdateVersion;
    QDate mUpdateReleaseDate;

    Author mAuthor;
    int mRating;
    int mNumberOfComments;
    int mDownloadCount;
    int mNumberFans;
    int mNumberKnowledgebaseEntries;
    QString mKnowledgebaseLink;
    QString mSummary;
    QString mShortSummary;
    QString mChangelog;
    QString mPayload;
    QStringList mInstalledFiles;
    QString mProviderId;
    QStringList mUnInstalledFiles;
    QString mDonationLink;

    QString mChecksum;
    QString mSignature;
    KNS3::Entry::Status mStatus;
    EntryInternal::Source mSource;

    QString mPreviewUrl[6];
    QImage mPreviewImage[6];

    QList<EntryInternal::DownloadLinkInformation> mDownloadLinkInformationList;
};

EntryInternal::EntryInternal()
    : d(new Private)
{
}

EntryInternal::EntryInternal(const EntryInternal &other)
    : d(other.d)
{
}

EntryInternal &EntryInternal::operator=(const EntryInternal &other)
{
    d = other.d;
    return *this;
}

bool EntryInternal::operator<(const KNSCore::EntryInternal &other) const
{
    return d->mUniqueId < other.d->mUniqueId;
}

bool EntryInternal::operator==(const KNSCore::EntryInternal &other) const
{
    return d->mUniqueId == other.d->mUniqueId && d->mProviderId == other.d->mProviderId;
}

EntryInternal::~EntryInternal()
{
}

bool EntryInternal::isValid() const
{
    return !d->mUniqueId.isEmpty();
}

QString EntryInternal::name() const
{
    return d->mName;
}

void EntryInternal::setName(const QString &name)
{
    d->mName = name;
}

QString EntryInternal::uniqueId() const
{
    return d->mUniqueId;
}

void EntryInternal::setUniqueId(const QString &id)
{
    d->mUniqueId = id;
}

QString EntryInternal::providerId() const
{
    return d->mProviderId;
}

void EntryInternal::setProviderId(const QString &id)
{
    d->mProviderId = id;
}

QString EntryInternal::category() const
{
    return d->mCategory;
}

void EntryInternal::setCategory(const QString &category)
{
    d->mCategory = category;
}

QUrl EntryInternal::homepage() const
{
    return d->mHomepage;
}

void EntryInternal::setHomepage(const QUrl &page)
{
    d->mHomepage = page;
}

Author EntryInternal::author() const
{
    return d->mAuthor;
}

void EntryInternal::setAuthor(const KNSCore::Author &author)
{
    d->mAuthor = author;
}

QString EntryInternal::license() const
{
    return d->mLicense;
}

void EntryInternal::setLicense(const QString &license)
{
    d->mLicense = license;
}

QString EntryInternal::summary() const
{
    return d->mSummary;
}

void EntryInternal::setSummary(const QString &summary)
{
    d->mSummary = summary;
}

QString EntryInternal::shortSummary() const
{
    return d->mShortSummary;
}

void EntryInternal::setShortSummary(const QString &summary)
{
    d->mShortSummary = summary;
}

void EntryInternal::setChangelog(const QString &changelog)
{
    d->mChangelog = changelog;
}

QString EntryInternal::changelog() const
{
    return d->mChangelog;
}

QString EntryInternal::version() const
{
    return d->mVersion;
}

void EntryInternal::setVersion(const QString &version)
{
    d->mVersion = version;
}

QDate EntryInternal::releaseDate() const
{
    return d->mReleaseDate;
}

void EntryInternal::setReleaseDate(const QDate &releasedate)
{
    d->mReleaseDate = releasedate;
}

QString EntryInternal::payload() const
{
    return d->mPayload;
}

void EntryInternal::setPayload(const QString &url)
{
    d->mPayload = url;
}

QDate EntryInternal::updateReleaseDate() const
{
    return d->mUpdateReleaseDate;
}

void EntryInternal::setUpdateReleaseDate(const QDate &releasedate)
{
    d->mUpdateReleaseDate = releasedate;
}

QString EntryInternal::updateVersion() const
{
    return d->mUpdateVersion;
}

void EntryInternal::setUpdateVersion(const QString &version)
{
    d->mUpdateVersion = version;
}

QString EntryInternal::previewUrl(PreviewType type) const
{
    return d->mPreviewUrl[type];
}

void EntryInternal::setPreviewUrl(const QString &url, PreviewType type)
{
    d->mPreviewUrl[type] = url;
}

QImage EntryInternal::previewImage(PreviewType type) const
{
    return d->mPreviewImage[type];
}

void EntryInternal::setPreviewImage(const QImage &image, PreviewType type)
{
    d->mPreviewImage[type] = image;
}

int EntryInternal::rating() const
{
    return d->mRating;
}

void EntryInternal::setRating(int rating)
{
    d->mRating = rating;
}

int EntryInternal::numberOfComments() const
{
    return d->mRating;
}

void EntryInternal::setNumberOfComments (int comments)
{
    d->mNumberOfComments = comments;
}

int EntryInternal::downloadCount() const
{
    return d->mDownloadCount;
}

void EntryInternal::setDownloadCount(int downloads)
{
    d->mDownloadCount = downloads;
}

int EntryInternal::numberFans() const
{
    return d->mNumberFans;
}

void EntryInternal::setNumberFans(int fans)
{
    d->mNumberFans = fans;
}

QString EntryInternal::donationLink() const
{
    return d->mDonationLink;
}

void EntryInternal::setDonationLink(const QString &link)
{
    d->mDonationLink = link;
}

int EntryInternal::numberKnowledgebaseEntries() const
{
    return d->mNumberKnowledgebaseEntries;
}
void EntryInternal::setNumberKnowledgebaseEntries(int num)
{
    d->mNumberKnowledgebaseEntries = num;
}

QString EntryInternal::knowledgebaseLink() const
{
    return d->mKnowledgebaseLink;
}
void EntryInternal::setKnowledgebaseLink(const QString &link)
{
    d->mKnowledgebaseLink = link;
}

/*
QString EntryInternal::checksum() const
{

    return d->mChecksum;
}

QString EntryInternal::signature() const
{

    return d->mSignature;
}
*/

EntryInternal::Source EntryInternal::source() const
{
    return d->mSource;
}

void EntryInternal::setSource(Source source)
{
    d->mSource = source;
}

KNS3::Entry::Status EntryInternal::status() const
{
    return d->mStatus;
}

void EntryInternal::setStatus(KNS3::Entry::Status status)
{
    d->mStatus = status;
}

void KNSCore::EntryInternal::setInstalledFiles(const QStringList &files)
{
    d->mInstalledFiles = files;
}

QStringList KNSCore::EntryInternal::installedFiles() const
{
    return d->mInstalledFiles;
}

void KNSCore::EntryInternal::setUnInstalledFiles(const QStringList &files)
{
    d->mUnInstalledFiles = files;
}

QStringList KNSCore::EntryInternal::uninstalledFiles() const
{
    return d->mUnInstalledFiles;
}

int KNSCore::EntryInternal::downloadLinkCount() const
{
    return d->mDownloadLinkInformationList.size();
}

QList<KNSCore::EntryInternal::DownloadLinkInformation> KNSCore::EntryInternal::downloadLinkInformationList() const
{
    return d->mDownloadLinkInformationList;
}

void KNSCore::EntryInternal::appendDownloadLinkInformation(const KNSCore::EntryInternal::DownloadLinkInformation &info)
{
    d->mDownloadLinkInformationList.append(info);
}

void EntryInternal::clearDownloadLinkInformation()
{
    d->mDownloadLinkInformationList.clear();
}

static QXmlStreamReader::TokenType readNextSkipComments(QXmlStreamReader* xml)
{
    do {
        xml->readNext();
    } while(xml->tokenType() == QXmlStreamReader::Comment || (xml->tokenType() == QXmlStreamReader::Characters && xml->text().trimmed().isEmpty()));
    return xml->tokenType();
}

static QStringRef readText(QXmlStreamReader* xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    QStringRef ret;
    const auto token = readNextSkipComments(xml);
    if (token == QXmlStreamReader::Characters) {
        ret = xml->text();
    }
    return ret;
}

static QString readStringTrimmed(QXmlStreamReader* xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    QString ret = readText(xml).trimmed().toString();

    if (xml->tokenType() == QXmlStreamReader::Characters)
        readNextSkipComments(xml);
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::EndElement);
    return ret;
}

static int readInt(QXmlStreamReader* xml)
{
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::StartElement);
    int ret = readText(xml).toInt();

    xml->readNext();
    Q_ASSERT(xml->tokenType() == QXmlStreamReader::EndElement);
    return ret;
}

bool KNSCore::EntryInternal::setEntryXML(QXmlStreamReader& reader)
{
    if (reader.name() != QLatin1String("stuff")) {
        qWarning() << "Parsing Entry from invalid XML";
        return false;
    }

    d->mCategory = reader.attributes().value(QStringLiteral("category")).toString();

    while (!reader.atEnd()) {
        const auto token = readNextSkipComments(&reader);
        if (token == QXmlStreamReader::EndElement)
            break;
        else if (token != QXmlStreamReader::StartElement)
            continue;

        if (reader.name() == QLatin1String("name")) {
            // TODO maybe do something with the language attribute? QString lang = e.attribute("lang");
            d->mName = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("author")) {
            const auto email = reader.attributes().value(QStringLiteral("email"));
            const auto jabber = reader.attributes().value(QStringLiteral("im"));
            const auto homepage = reader.attributes().value(QStringLiteral("homepage"));
            d->mAuthor.setName(readStringTrimmed(&reader));
            d->mAuthor.setEmail(email.toString());
            d->mAuthor.setJabber(jabber.toString());
            d->mAuthor.setHomepage(homepage.toString());
        } else if (reader.name() == QLatin1String("providerid")) {
            d->mProviderId = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("homepage")) {
            d->mHomepage = QUrl(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else if (reader.name() == QLatin1String("licence")) { // krazy:exclude=spelling
            d->mLicense = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("summary")) {
            d->mSummary = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("changelog")) {
            d->mChangelog = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("version")) {
            d->mVersion = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("releasedate")) {
            d->mReleaseDate = QDate::fromString(readStringTrimmed(&reader), Qt::ISODate);
        } else if (reader.name() == QLatin1String("preview")) {
            // TODO support for all 6 image links
            d->mPreviewUrl[PreviewSmall1] = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("previewBig")) {
            d->mPreviewUrl[PreviewBig1] = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("payload")) {
            d->mPayload = readStringTrimmed(&reader);
        } else if (reader.name() == QLatin1String("rating")) {
            d->mRating = readInt(&reader);
        } else if (reader.name() == QLatin1String("downloads")) {
            d->mDownloadCount = readInt(&reader);
        } else if (reader.name() == QLatin1String("category")) {
            d->mCategory = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("signature")) {
            d->mSignature = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("checksum")) {
            d->mChecksum = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("installedfile")) {
            d->mInstalledFiles.append(reader.readElementText(QXmlStreamReader::SkipChildElements));
        } else if (reader.name() == QLatin1String("id")) {
            d->mUniqueId = reader.readElementText(QXmlStreamReader::SkipChildElements);
        } else if (reader.name() == QLatin1String("status")) {
            const auto statusText = readText(&reader);
            if (statusText == QLatin1String("installed")) {
                qCDebug(KNEWSTUFFCORE) << "Found an installed entry in registry";
                d->mStatus = KNS3::Entry::Installed;
            } else if (statusText == QLatin1String("updateable")) {
                d->mStatus = KNS3::Entry::Updateable;
            }
            if (reader.tokenType() == QXmlStreamReader::Characters)
                readNextSkipComments(&reader);
        }
        Q_ASSERT(reader.tokenType() == QXmlStreamReader::EndElement);
    }

    // Validation
    if (d->mName.isEmpty()) {
        qWarning() << "Entry: no name given";
        return false;
    }

    if (d->mUniqueId.isEmpty()) {
        if (!d->mPayload.isEmpty()) {
            d->mUniqueId = d->mPayload;
        } else {
            d->mUniqueId = d->mName;
        }
    }

    if (d->mPayload.isEmpty()) {
        qWarning() << "Entry: no payload URL given for: " << d->mName << " - " << d->mUniqueId;
        return false;
    }
    return true;
}

bool KNSCore::EntryInternal::setEntryXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("stuff")) {
        qWarning() << "Parsing Entry from invalid XML";
        return false;
    }

    d->mCategory = xmldata.attribute(QStringLiteral("category"));

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == QLatin1String("name")) {
            // TODO maybe do something with the language attribute? QString lang = e.attribute("lang");
            d->mName = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("author")) {
            QString email = e.attribute(QStringLiteral("email"));
            QString jabber = e.attribute(QStringLiteral("im"));
            QString homepage = e.attribute(QStringLiteral("homepage"));
            d->mAuthor.setName(e.text().trimmed());
            d->mAuthor.setEmail(email);
            d->mAuthor.setJabber(jabber);
            d->mAuthor.setHomepage(homepage);
        } else if (e.tagName() == QLatin1String("providerid")) {
            d->mProviderId = e.text();
        } else if (e.tagName() == QLatin1String("homepage")) {
            d->mHomepage = QUrl(e.text());
        } else if (e.tagName() == QLatin1String("licence")) { // krazy:exclude=spelling
            d->mLicense = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("summary")) {
            d->mSummary = e.text();
        } else if (e.tagName() == QLatin1String("changelog")) {
            d->mChangelog = e.text();
        } else if (e.tagName() == QLatin1String("version")) {
            d->mVersion = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("releasedate")) {
            d->mReleaseDate = QDate::fromString(e.text().trimmed(), Qt::ISODate);
        } else if (e.tagName() == QLatin1String("preview")) {
            // TODO support for all 6 image links
            d->mPreviewUrl[PreviewSmall1] = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("previewBig")) {
            d->mPreviewUrl[PreviewBig1] = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("payload")) {
            d->mPayload = e.text().trimmed();
        } else if (e.tagName() == QLatin1String("rating")) {
            d->mRating = e.text().toInt();
        } else if (e.tagName() == QLatin1String("downloads")) {
            d->mDownloadCount = e.text().toInt();
        } else if (e.tagName() == QLatin1String("category")) {
            d->mCategory = e.text();
        } else if (e.tagName() == QLatin1String("signature")) {
            d->mSignature = e.text();
        } else if (e.tagName() == QLatin1String("checksum")) {
            d->mChecksum = e.text();
        } else if (e.tagName() == QLatin1String("installedfile")) {
            d->mInstalledFiles.append(e.text());
        } else if (e.tagName() == QLatin1String("id")) {
            d->mUniqueId = e.text();
        } else if (e.tagName() == QLatin1String("status")) {
            QString statusText = e.text();
            if (statusText == QLatin1String("installed")) {
                qCDebug(KNEWSTUFFCORE) << "Found an installed entry in registry";
                d->mStatus = KNS3::Entry::Installed;
            } else if (statusText == QLatin1String("updateable")) {
                d->mStatus = KNS3::Entry::Updateable;
            }
        }
    }

    // Validation
    if (d->mName.isEmpty()) {
        qWarning() << "Entry: no name given";
        return false;
    }

    if (d->mUniqueId.isEmpty()) {
        if (!d->mPayload.isEmpty()) {
            d->mUniqueId = d->mPayload;
        } else {
            d->mUniqueId = d->mName;
        }
    }

    if (d->mPayload.isEmpty()) {
        qWarning() << "Entry: no payload URL given for: " << d->mName << " - " << d->mUniqueId;
        return false;
    }
    return true;
}

/**
 * get the xml string for the entry
 */
QDomElement KNSCore::EntryInternal::entryXML() const
{
    Q_ASSERT(!d->mUniqueId.isEmpty());
    Q_ASSERT(!d->mProviderId.isEmpty());

    QDomDocument doc;

    QDomElement el = doc.createElement(QStringLiteral("stuff"));
    el.setAttribute(QStringLiteral("category"), d->mCategory);

    QString name = d->mName;

    QDomElement e;
    e = addElement(doc, el, QStringLiteral("name"), name);
    // todo: add language attribute
    (void)addElement(doc, el, QStringLiteral("providerid"), d->mProviderId);

    QDomElement author = addElement(doc, el, QStringLiteral("author"), d->mAuthor.name());
    if (!d->mAuthor.email().isEmpty()) {
        author.setAttribute(QStringLiteral("email"), d->mAuthor.email());
    }
    if (!d->mAuthor.homepage().isEmpty()) {
        author.setAttribute(QStringLiteral("homepage"), d->mAuthor.homepage());
    }
    if (!d->mAuthor.jabber().isEmpty()) {
        author.setAttribute(QStringLiteral("im"), d->mAuthor.jabber());
    }
    // FIXME: 'jabber' or 'im'? consult with kopete guys...
    addElement(doc, el, QStringLiteral("homepage"), d->mHomepage.url());
    (void)addElement(doc, el, QStringLiteral("licence"), d->mLicense); // krazy:exclude=spelling
    (void)addElement(doc, el, QStringLiteral("version"), d->mVersion);
    if ((d->mRating > 0) || (d->mDownloadCount > 0)) {
        (void)addElement(doc, el, QStringLiteral("rating"), QString::number(d->mRating));
        (void)addElement(doc, el, QStringLiteral("downloads"), QString::number(d->mDownloadCount));
    }
    if (!d->mSignature.isEmpty()) {
        (void)addElement(doc, el, QStringLiteral("signature"), d->mSignature);
    }
    if (!d->mChecksum.isEmpty()) {
        (void)addElement(doc, el, QStringLiteral("checksum"), d->mChecksum);
    }
    foreach (const QString &file, d->mInstalledFiles) {
        (void)addElement(doc, el, QStringLiteral("installedfile"), file);
    }
    if (!d->mUniqueId.isEmpty()) {
        addElement(doc, el, QStringLiteral("id"), d->mUniqueId);
    }

    (void)addElement(doc, el, QStringLiteral("releasedate"),
                     d->mReleaseDate.toString(Qt::ISODate));

    e = addElement(doc, el, QStringLiteral("summary"), d->mSummary);
    e = addElement(doc, el, QStringLiteral("changelog"), d->mChangelog);
    e = addElement(doc, el, QStringLiteral("preview"), d->mPreviewUrl[PreviewSmall1]);
    e = addElement(doc, el, QStringLiteral("previewBig"), d->mPreviewUrl[PreviewBig1]);
    e = addElement(doc, el, QStringLiteral("payload"), d->mPayload);

    if (d->mStatus == KNS3::Entry::Installed) {
        (void)addElement(doc, el, QStringLiteral("status"), QStringLiteral("installed"));
    }
    if (d->mStatus == KNS3::Entry::Updateable) {
        (void)addElement(doc, el, QStringLiteral("status"), QStringLiteral("updateable"));
    }

    return el;
}

KNSCore::EntryInternal EntryInternal::fromEntry(const KNS3::Entry &entry)
{
    return entry.d->e;
}

QString KNSCore::replaceBBCode(const QString &unformattedText)
{
    QString text(unformattedText);
    text.replace(QLatin1String("[b]"), QLatin1String("<b>"));
    text.replace(QLatin1String("[/b]"), QLatin1String("</b>"));
    text.replace(QLatin1String("[i]"), QLatin1String("<i>"));
    text.replace(QLatin1String("[/i]"), QLatin1String("</i>"));
    text.replace(QLatin1String("[u]"), QLatin1String("<i>"));
    text.replace(QLatin1String("[/u]"), QLatin1String("</i>"));
    text.replace(QLatin1String("\\\""), QLatin1String("\""));
    text.replace(QLatin1String("\\\'"), QLatin1String("\'"));
    text.replace(QLatin1String("[li]"), QLatin1String("* ")); // TODO: better replacement for list elements?
    text.remove(QStringLiteral("[/li]"));
    text.remove(QStringLiteral("[url]"));
    text.remove(QStringLiteral("[/url]"));
    return text;
}
