/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "opdsprovider_p.h"

#include <syndication/atom/atom.h>
#include <syndication_export.h>
#include <syndication/documentsource.h>
#include <QDate>
#include <QTimer>
#include <QUrlQuery>
#include <QLocale>

namespace KNSCore
{

const QString OPDS_REL_ACQUISITION = QStringLiteral("http://opds-spec.org/acquisition");
const QString OPDS_REL_AC_OPEN_ACCESS = QStringLiteral("http://opds-spec.org/acquisition/open-access");
const QString OPDS_REL_AC_BORROW = QStringLiteral("http://opds-spec.org/acquisition/borrow");
const QString OPDS_REL_AC_BUY = QStringLiteral("http://opds-spec.org/acquisition/buy");
const QString OPDS_REL_AC_SUBSCRIBE = QStringLiteral("http://opds-spec.org/acquisition/subscribe");
const QString OPDS_REL_AC_SAMPLE = QStringLiteral("http://opds-spec.org/acquisition/sample");
const QString OPDS_REL_IMAGE = QStringLiteral("http://opds-spec.org/image");
const QString OPDS_REL_THUMBNAIL = QStringLiteral("http://opds-spec.org/image/thumbnail");
const QString OPDS_REL_CRAWL = QStringLiteral("http://opds-spec.org/crawlable");
const QString OPDS_REL_FACET = QStringLiteral("http://opds-spec.org/facet");
const QString OPDS_REL_SHELF = QStringLiteral("http://opds-spec.org/shelf");
const QString OPDS_REL_SORT_NEW = QStringLiteral("http://opds-spec.org/sort/new");
const QString OPDS_REL_SORT_POPULAR = QStringLiteral("http://opds-spec.org/sort/popular");
const QString OPDS_REL_FEATURED = QStringLiteral("http://opds-spec.org/featured");
const QString OPDS_REL_RECOMMENDED = QStringLiteral("http://opds-spec.org/recommended");
const QString OPDS_REL_SUBSCRIPTIONS = QStringLiteral("http://opds-spec.org/subscriptions");
const QString OPDS_EL_PRICE = QStringLiteral("opds:price");
const QString OPDS_EL_INDIRECT = QStringLiteral("opds:indirectAcquisition");
const QString OPDS_ATTR_FACET_GROUP = QStringLiteral("opds:facetGroup");
const QString OPDS_ATTR_ACTIVE_FACET = QStringLiteral("opds:activeFacet");

const QString OPDS_ATOM_MT = QStringLiteral("application/atom+xml");
const QString OPDS_PROFILE = QStringLiteral("profile=opds-catalog");
const QString OPDS_TYPE_ENTRY = QStringLiteral("type=entry");
const QString OPDS_KIND_NAVIGATION = QStringLiteral("kind=navigation");
const QString OPDS_KIND_ACQUISITION = QStringLiteral("kind=acquisition");

const QString REL_START = QStringLiteral("start");
const QString REL_SUBSECTION = QStringLiteral("subsection");
const QString REL_COLLECTION = QStringLiteral("collection");
const QString REL_PREVIEW = QStringLiteral("preview");
const QString REL_REPLIES = QStringLiteral("replies");
const QString REL_RELATED = QStringLiteral("related");
const QString REL_ALTERNATE = QStringLiteral("alternate");
const QString ATTR_CURRENCY_CODE = QStringLiteral("currencycode");
const QString FEED_COMPLETE = QStringLiteral("fh:complete");
const QString THREAD_COUNT = QStringLiteral("count");

const QString OPENSEARCH_MT = QStringLiteral("application/opensearchdescription+xml");
const QString REL_SEARCH = QStringLiteral("search");

const QString OPENSEARCH_SEARCH_TERMS = QStringLiteral("searchTerms");
const QString OPENSEARCH_COUNT = QStringLiteral("count");
const QString OPENSEARCH_START_INDEX = QStringLiteral("startIndex");
const QString OPENSEARCH_START_PAGE = QStringLiteral("startPage");

const QString HTML_MT = QStringLiteral("text/html");


OPDSProvider::OPDSProvider():
    m_initialized(false)
{

}

OPDSProvider::~OPDSProvider()
{

}

QString OPDSProvider::id() const
{
    return m_providerId;
}

QString OPDSProvider::name() const
{
    return m_providerName;
}

QUrl OPDSProvider::icon() const
{
    return m_iconUrl;
}

void OPDSProvider::loadEntries(const KNSCore::Provider::SearchRequest &request)
{
    m_currentRequest = request;

    qDebug() << "Starting search";
    if (request.filter == Installed) {
        Q_EMIT loadingFinished(request, installedEntries());
        return;
    } else if (request.filter == Provider::ExactEntryId) {
        qDebug() << "Getting a single file";
        for (EntryInternal entry: m_cachedEntries) {
            if (entry.uniqueId() == request.searchTerm) {
                loadEntryDetails(entry);
            }
        }
    } else {
        if (request.searchTerm.startsWith(QStringLiteral("/"))) {
            m_currentUrl = fixRelativeUrl(request.searchTerm);
        } else if (request.searchTerm.startsWith(QStringLiteral("http"))) {
            m_currentUrl = QUrl(request.searchTerm);
        } else if (!m_openSearchTemplate.isEmpty() && !request.searchTerm.isEmpty()) {
            // We should check if there's an opensearch implementation, and see if we can funnel search
            // requests to that.
            m_currentUrl = getOpenSearchString(request);
        }

        //request: check if entries is above pagesize*index, otherwise load next page.

        qDebug() << "loading...";
        QUrl url = m_currentUrl;
        if (!url.isEmpty()) {
            qDebug() << "loading opds feed" << m_currentUrl;
            m_xmlLoader = new XmlLoader(this);
            connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parseFeedData);
            //connect(m_xmlLoader, &XmlLoader::signalFailed, this, SIGNAL(loadingFailed()));
            m_xmlLoader->load(url);
        } else {
            Q_EMIT loadingFailed(request);
        }
    }
}

void OPDSProvider::loadEntryDetails(const EntryInternal &entry)
{
    QUrl url;
    for (auto link : entry.downloadLinkInformationList()) {
        if (link.distributionType.contains(OPDS_TYPE_ENTRY)) {
            url = QUrl(link.descriptionLink);
        }
    }
    if (!url.isEmpty()) {
        qDebug() << "loading bookdetails...";
        m_xmlLoader = new XmlLoader(this);
        connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parseExtraDetails);
        m_xmlLoader->load(url);
    }
}

void OPDSProvider::loadPayloadLink(const KNSCore::EntryInternal &entry, int linkNumber)
{
    KNSCore::EntryInternal copy = entry;
    qDebug() << linkNumber << entry.downloadLinkCount();
    for (auto downloadInfo: entry.downloadLinkInformationList()) {
        if (downloadInfo.id == linkNumber) {
            copy.setPayload(downloadInfo.descriptionLink);
        }
    }
    Q_EMIT payloadLinkLoaded(copy);
}

bool OPDSProvider::setProviderXML(const QDomElement &xmldata)
{
    qDebug() << "initializing opds";
    if (xmldata.tagName() != QLatin1String("provider")) {
        return false;
    }
    m_providerId = xmldata.attribute(QStringLiteral("downloadurl"));

    QUrl iconurl(xmldata.attribute(QStringLiteral("icon")));
    if (!iconurl.isValid()) {
        iconurl = QUrl::fromLocalFile(xmldata.attribute(QStringLiteral("icon")));
    }
    m_iconUrl = iconurl;

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == QLatin1String("title")) {
            m_providerName = e.text().trimmed();
        }
    }

    m_currentUrl = QUrl(m_providerId);
    QTimer::singleShot(0, this, &OPDSProvider::slotEmitProviderInitialized);
    return true;
}

bool OPDSProvider::isInitialized() const
{
    return m_initialized;
}

void OPDSProvider::setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries)
{
    m_cachedEntries = cachedEntries;
}

void OPDSProvider::parseFeedData(const QDomDocument &doc)
{
    Syndication::DocumentSource source(doc.toByteArray(), m_currentUrl.toString());
    Syndication::Atom::Parser parser;
    Syndication::Atom::FeedDocumentPtr feedDoc = parser.parse(source).staticCast<Syndication::Atom::FeedDocument>();

    if (!feedDoc->title().isEmpty()) {
        m_providerName = feedDoc->title();
    }
    if (!feedDoc->icon().isEmpty()) {
        m_iconUrl = QUrl(feedDoc->icon());
    }

    QList<KNSCore::Provider::CategoryMetadata> categories;
    for (auto link: feedDoc->links()) {
        // There will be a number of links toplevel, amongst which probably a lot of sortorder and navigation links.
        if (link.rel() == REL_SEARCH && link.type() == OPENSEARCH_MT) {
            m_xmlLoader = new XmlLoader(this);
            connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parserOpenSearchDocument);
            m_xmlLoader->load(QUrl(link.href()));
        } else if (link.rel() == OPDS_REL_FACET || link.rel() == OPDS_REL_FEATURED) {
            KNSCore::Provider::CategoryMetadata category;
            category.id = link.href();
            category.displayName = link.title();
            category.name = link.href();
            categories.append(category);
        }
    }

    EntryInternal::List entries;
    for(int i=0; i<feedDoc->entries().size(); i++) {
        Syndication::Atom::Entry feedEntry = feedDoc->entries().at(i);

        EntryInternal entry;
        entry.setName(feedEntry.title());
        entry.setProviderId(m_providerId);
        entry.setUniqueId(feedEntry.id());

        entry.setStatus(KNS3::Entry::Invalid);
        for (const EntryInternal &cachedEntry : qAsConst(m_cachedEntries)) {
            if (entry.uniqueId() == cachedEntry.uniqueId()) {
                entry = cachedEntry;
                break;
            }
        }


        // This is a bit of a pickle: atom feeds can have multiple catagories.
        // but these catagories are not specifically tags...
        QStringList tags;
        for(int j=0; j<feedEntry.categories().size(); j++) {
            tags.append(feedEntry.categories().at(j).term());
        }
        entry.setTags(tags);
        // Same issue with author...
        for(int j=0; j<feedEntry.authors().size(); j++) {
            Author author;
            Syndication::Atom::Person person = feedEntry.authors().at(j);
            author.setId(person.uri());
            author.setName(person.name());
            author.setEmail(person.email());
            entry.setAuthor(author);
        }
        entry.setLicense(feedEntry.rights());
        entry.setSummary(feedEntry.content().asString());
        entry.setShortSummary(feedEntry.summary());

        int downloads = 0;
        int counterThumbnails = 0;
        int counterImages = 0;
        for(int j=0; j<feedEntry.links().size(); j++) {
            Syndication::Atom::Link link = feedEntry.links().at(j);
            qDebug() << link.rel() << link.type() << link.href();

            if (link.rel().startsWith(OPDS_REL_ACQUISITION)) {
                KNSCore::EntryInternal::DownloadLinkInformation download;
                download.id = entry.downloadLinkCount();
                downloads +=1;
                download.name = link.title();
                if (link.title().isEmpty()) {
                    QStringList l;
                    l.append(link.type());
                    l.append(QStringLiteral("(")+link.rel().split(QStringLiteral("/")).last()+ QStringLiteral(")"));
                    download.name = l.join(QStringLiteral(" "));
                }
                download.size = link.length();
                download.descriptionLink = fixRelativeUrl(link.href()).toString();
                download.distributionType = link.type();
                download.isDownloadtypeLink = false;

                if (link.rel() == OPDS_REL_ACQUISITION || link.rel() == OPDS_REL_AC_OPEN_ACCESS) {
                    download.isDownloadtypeLink = true;
                    if (entry.status() != KNS3::Entry::Installed &&
                            entry.status() != KNS3::Entry::Updateable) {
                        entry.setStatus(KNS3::Entry::Downloadable);
                    }
                    entry.setEntryType(EntryInternal::CatalogEntry);
                }

                for (QDomElement el:feedEntry.elementsByTagName(OPDS_EL_PRICE)) {
                    QLocale locale;
                    download.priceAmount = locale.toCurrencyString(el.text().toFloat(), el.attribute(ATTR_CURRENCY_CODE));
                }
                // There's an 'opds:indirectaquistition' element that gives extra metadata about bundles.
                entry.appendDownloadLinkInformation(download);

            } else if (link.rel().startsWith(OPDS_REL_IMAGE)) {
                if (link.rel() == OPDS_REL_THUMBNAIL) {
                    entry.setPreviewUrl(fixRelativeUrl(link.href()).toString(), KNSCore::EntryInternal::PreviewType(counterThumbnails));
                    counterThumbnails +=1;
                } else {
                    entry.setPreviewUrl(fixRelativeUrl(link.href()).toString(), KNSCore::EntryInternal::PreviewType(counterImages+3));
                    counterImages +=1;
                }

            } else {
                // This could be anything from a more info link, to navigation links, to links to the outside world.
                // Todo: think of using link rel's 'replies', 'payment'(donation) and 'version-history'.
                KNSCore::EntryInternal::DownloadLinkInformation otherLink;
                otherLink.isDownloadtypeLink = false;
                otherLink.name = link.title();
                otherLink.id = entry.downloadLinkCount();

                otherLink.distributionType = link.type();
                otherLink.descriptionLink = fixRelativeUrl(link.href()).toString();
                otherLink.size = link.length();
                QStringList tags;
                tags.append(link.rel());
                tags.append(link.hrefLanguage());
                otherLink.tags = tags;

                if (link.rel() == OPDS_REL_CRAWL || link.type().startsWith(OPDS_ATOM_MT)) {
                    entry.setEntryType(EntryInternal::GroupEntry);
                    entry.setPayload(link.href());
                    entry.appendDownloadLinkInformation(otherLink);
                } if (link.type() == HTML_MT) {
                    entry.setHomepage(fixRelativeUrl(link.href()));
                } else {
                    entry.appendDownloadLinkInformation(otherLink);
                }
            }
        }

        QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.published());
        entry.setReleaseDate(date.date());
        date = QDateTime::fromSecsSinceEpoch(feedEntry.updated());

        if (entry.status() != KNS3::Entry::Invalid) {
            // Set this back to catalog entry when we can download a thing.
            entry.setEntryType(EntryInternal::CatalogEntry);
            //Gutenberg doesn't do versioning in the opds, so it's update value is unreliable...
            /*if (entry.updateReleaseDate() < date.date()) {
                qDebug() << entry.updateReleaseDate() << date.date();
                entry.setUpdateReleaseDate(date.date());
                if (entry.status() == KNS3::Entry::Installed) {
                    entry.setStatus(KNS3::Entry::Updateable);
                }
            }*/
        }

        entries.append(entry);
    }

    Q_EMIT loadingFinished(m_currentRequest, entries);
    Q_EMIT categoriesMetadataLoded(categories);
}

void OPDSProvider::parseExtraDetails(const QDomDocument &doc)
{
    Syndication::DocumentSource source(doc.toByteArray(), m_currentUrl.toString());
    Syndication::Atom::Parser parser;
    Syndication::Atom::FeedDocumentPtr feedDoc = parser.parse(source).staticCast<Syndication::Atom::FeedDocument>();

    Syndication::Atom::Entry feedEntry = feedDoc->entries().first();

    EntryInternal entry;
    entry.setName(feedEntry.title());
    entry.setProviderId(m_providerId);
    entry.setUniqueId(feedEntry.id());

    // This is a bit of a pickle: atom feeds can have multiple catagories.
    // but these catagories are not specifically tags...
    QStringList tags;
    for(int j=0; j<feedEntry.categories().size(); j++) {
        tags.append(feedEntry.categories().at(j).term());
    }
    entry.setTags(tags);
    // Same issue with author...
    for(int j=0; j<feedEntry.authors().size(); j++) {
        Author author;
        Syndication::Atom::Person person = feedEntry.authors().at(j);
        author.setId(person.uri());
        author.setName(person.name());
        author.setEmail(person.email());
        entry.setAuthor(author);
    }
    entry.setLicense(feedEntry.rights());
    entry.setSummary(feedEntry.content().asString());
    entry.setShortSummary(feedEntry.summary());

    QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.published());
    entry.setReleaseDate(date.date());
    //Gutenberg doesn't do versioning in the opds, so it's update value is unreliable.

    int downloads = 0;
    int counterImages = 0;
    for(int j=0; j<feedEntry.links().size(); j++) {
        Syndication::Atom::Link link = feedEntry.links().at(j);

        if (link.rel().startsWith(OPDS_REL_ACQUISITION)) {
            KNSCore::EntryInternal::DownloadLinkInformation download;
            download.id = downloads;
            downloads +=1;
            download.name = link.title();
            if (link.title().isEmpty()) {
                QStringList l;
                l.append(link.type());
                l.append(QStringLiteral("(")+link.rel().split(QStringLiteral("/")).last()+ QStringLiteral(")"));
                download.name = l.join(QStringLiteral(" "));
            }
            download.size = link.length();
            download.descriptionLink = link.href();
            download.distributionType = link.type();
            entry.setStatus(KNS3::Entry::Invalid);
            download.isDownloadtypeLink = false;

            if (link.rel() == OPDS_REL_CRAWL || link.type() == OPDS_KIND_NAVIGATION) {
                entry.setEntryType(EntryInternal::GroupEntry);
            }

            if (link.rel() == OPDS_REL_ACQUISITION || link.rel() == OPDS_REL_AC_OPEN_ACCESS) {
                download.isDownloadtypeLink = true;
                entry.setStatus(KNS3::Entry::Downloadable);
                entry.setEntryType(EntryInternal::CatalogEntry);
            }

            for (QDomElement el : feedEntry.elementsByTagName(OPDS_EL_PRICE)) {
                QLocale locale;
                download.priceAmount = locale.toCurrencyString(el.text().toFloat(), el.attribute(ATTR_CURRENCY_CODE));
            }
            // There's an 'opds:indirectaquistition' element that gives extra metadata about bundles.
            entry.appendDownloadLinkInformation(download);
        } else if (link.rel().startsWith(OPDS_REL_IMAGE)) {
            if (counterImages < 6) {
                entry.setPreviewUrl(fixRelativeUrl(link.href()).toString(), KNSCore::EntryInternal::PreviewType(counterImages));
            }
            counterImages +=1;
        } else {
            // This could be anything from a more info link, to navigation links, to links to the outside world.
            KNSCore::EntryInternal::DownloadLinkInformation otherLink;
            otherLink.isDownloadtypeLink = false;
            otherLink.name = link.title();
            otherLink.id = downloads;
            downloads +=1;
            otherLink.distributionType = link.type();
            otherLink.descriptionLink = link.href();
            otherLink.size = link.length();
            QStringList tags;
            tags.append(link.rel());
            tags.append(link.hrefLanguage());
            otherLink.tags = tags;
            entry.appendDownloadLinkInformation(otherLink);
        }
    }

    Q_EMIT entryDetailsLoaded(entry);
}

void OPDSProvider::slotEmitProviderInitialized()
{
    m_initialized = true;
    Q_EMIT providerInitialized(this);
}

EntryInternal::List OPDSProvider::installedEntries() const
{
    EntryInternal::List entries;
    for (const EntryInternal &entry : qAsConst(m_cachedEntries)) {
        if (entry.status() == KNS3::Entry::Installed || entry.status() == KNS3::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

void OPDSProvider::parserOpenSearchDocument(const QDomDocument &doc)
{
    m_openSearchTemplate = QString();
    QDomElement el = doc.documentElement().firstChildElement(QStringLiteral("Url"));
    while (!el.isNull()) {
        if (el.attribute(QStringLiteral("type")).contains(OPDS_ATOM_MT)) {
            if (m_openSearchTemplate.isEmpty() || el.attribute(QStringLiteral("type")).contains(OPDS_PROFILE)) {
                m_openSearchTemplate = el.attribute(QStringLiteral("template"));
            }
        }

        el = el.nextSiblingElement(QStringLiteral("Url"));
    }
}

QUrl OPDSProvider::getOpenSearchString(const Provider::SearchRequest &request)
{
    QUrl searchUrl = QUrl(m_openSearchTemplate);

    QUrlQuery templateQuery(searchUrl);
    QUrlQuery query;

    for (QPair<QString, QString> key:templateQuery.queryItems()) {
        if (key.second.contains(OPENSEARCH_SEARCH_TERMS)) {
            query.addQueryItem(key.first, request.searchTerm);
        } else if (key.second.contains(OPENSEARCH_COUNT)) {
            query.addQueryItem(key.first, QString::number(request.pageSize));
        } else if (key.second.contains(OPENSEARCH_START_PAGE)) {
            query.addQueryItem(key.first, QString::number(request.page));
        } else if (key.second.contains(OPENSEARCH_START_INDEX)) {
            query.addQueryItem(key.first, QString::number(request.page*request.pageSize));
        }
    }
    searchUrl.setQuery(query);
    return searchUrl;
}

QUrl OPDSProvider::fixRelativeUrl(QString urlPart)
{
    QUrl query = QUrl(urlPart);
    if (query.isRelative()) {
    QUrl host = m_currentUrl;
    host.setPath(query.path());
    host.setQuery(query.query());
    return host;
    }
    return query;
}
}

