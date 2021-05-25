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
#include <QIcon>

#include <knewstuffcore_debug.h>

#include "tagsfilterchecker.h"

namespace KNSCore
{
static const QLatin1String OPDS_REL_ACQUISITION {"http://opds-spec.org/acquisition"};
static const QLatin1String OPDS_REL_AC_OPEN_ACCESS {"http://opds-spec.org/acquisition/open-access"};
static const QLatin1String OPDS_REL_AC_BORROW {"http://opds-spec.org/acquisition/borrow"};
static const QLatin1String OPDS_REL_AC_BUY {"http://opds-spec.org/acquisition/buy"};
static const QLatin1String OPDS_REL_AC_SUBSCRIBE {"http://opds-spec.org/acquisition/subscribe"};
static const QLatin1String OPDS_REL_AC_SAMPLE {"http://opds-spec.org/acquisition/sample"};
static const QLatin1String OPDS_REL_IMAGE {"http://opds-spec.org/image"};
static const QLatin1String OPDS_REL_THUMBNAIL {"http://opds-spec.org/image/thumbnail"};
static const QLatin1String OPDS_REL_CRAWL {"http://opds-spec.org/crawlable"};
static const QLatin1String OPDS_REL_FACET {"http://opds-spec.org/facet"};
static const QLatin1String OPDS_REL_SHELF {"http://opds-spec.org/shelf"};
static const QLatin1String OPDS_REL_SORT_NEW {"http://opds-spec.org/sort/new"};
static const QLatin1String OPDS_REL_SORT_POPULAR {"http://opds-spec.org/sort/popular"};
static const QLatin1String OPDS_REL_FEATURED {"http://opds-spec.org/featured"};
static const QLatin1String OPDS_REL_RECOMMENDED {"http://opds-spec.org/recommended"};
static const QLatin1String OPDS_REL_SUBSCRIPTIONS {"http://opds-spec.org/subscriptions"};
static const QLatin1String OPDS_EL_PRICE {"opds:price"};
static const QLatin1String OPDS_EL_INDIRECT {"opds:indirectAcquisition"};
static const QLatin1String OPDS_ATTR_FACET_GROUP {"opds:facetGroup"};
static const QLatin1String OPDS_ATTR_ACTIVE_FACET {"opds:activeFacet"};

static const QLatin1String OPDS_ATOM_MT {"application/atom+xml"};
static const QLatin1String OPDS_PROFILE {"profile=opds-catalog"};
static const QLatin1String OPDS_TYPE_ENTRY {"type=entry"};
static const QLatin1String OPDS_KIND_NAVIGATION {"kind=navigation"};
static const QLatin1String OPDS_KIND_ACQUISITION {"kind=acquisition"};

static const QLatin1String REL_START {"start"};
static const QLatin1String REL_SUBSECTION {"subsection"};
static const QLatin1String REL_COLLECTION {"collection"};
static const QLatin1String REL_PREVIEW {"preview"};
static const QLatin1String REL_REPLIES {"replies"};
static const QLatin1String REL_RELATED {"related"};
static const QLatin1String REL_PREVIOUS {"previous"};
static const QLatin1String REL_NEXT {"next"};
static const QLatin1String REL_FIRST {"first"};
static const QLatin1String REL_LAST {"last"};
static const QLatin1String REL_UP {"up"};
static const QLatin1String REL_SELF {"self"};
static const QLatin1String REL_ALTERNATE {"alternate"};
static const QLatin1String ATTR_CURRENCY_CODE {"currencycode"};
static const QLatin1String FEED_COMPLETE {"fh:complete"};
static const QLatin1String THREAD_COUNT {"count"};

static const QLatin1String OPENSEARCH_NS {"http://a9.com/-/spec/opensearch/1.1/"};
static const QLatin1String OPENSEARCH_MT {"application/opensearchdescription+xml"};
static const QLatin1String REL_SEARCH {"search"};

static const QLatin1String OPENSEARCH_SEARCH_TERMS {"searchTerms"};
static const QLatin1String OPENSEARCH_COUNT {"count"};
static const QLatin1String OPENSEARCH_START_INDEX {"startIndex"};
static const QLatin1String OPENSEARCH_START_PAGE {"startPage"};

static const QLatin1String HTML_MT {"text/html"};

static const QLatin1String KEY_MIME_TYPE {"data##mimetype="};
static const QLatin1String KEY_URL {"data##url="};
static const QLatin1String KEY_LANGUAGE {"data##language="};

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

    if (request.filter == Installed) {
        Q_EMIT loadingFinished(request, installedEntries());
        return;
    } else if (request.filter == Provider::ExactEntryId) {
        for (EntryInternal entry: m_cachedEntries) {
            if (entry.uniqueId() == request.searchTerm) {
                loadEntryDetails(entry);
            }
        }
    } else {
        if (request.searchTerm.startsWith(QStringLiteral("http"))) {
            m_currentUrl = fixRelativeUrl(request.searchTerm);
        } else if (!m_openSearchTemplate.isEmpty() && !request.searchTerm.isEmpty()) {
            // We should check if there's an opensearch implementation, and see if we can funnel search
            // requests to that.
            m_currentUrl = openSearchStringForRequest(request);
        }

        //TODO request: check if entries is above pagesize*index, otherwise load next page.

        QUrl url = m_currentUrl;
        if (!url.isEmpty()) {
            qCDebug(KNEWSTUFFCORE) << "requesting url" << url;
            m_xmlLoader = new XmlLoader(this);
            m_currentTime = QDateTime::currentDateTime();
            m_loadingExtraDetails = false;
            connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parseFeedData);
            connect(m_xmlLoader, &XmlLoader::signalFailed, this, &OPDSProvider::slotLoadingFailed);
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
        m_xmlLoader = new XmlLoader(this);
        m_currentTime = QDateTime::currentDateTime();
        m_loadingExtraDetails = true;
        connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parseFeedData);
        connect(m_xmlLoader, &XmlLoader::signalFailed, this, &OPDSProvider::slotLoadingFailed);
        m_xmlLoader->load(url);
    }
}

void OPDSProvider::loadPayloadLink(const KNSCore::EntryInternal &entry, int linkNumber)
{
    KNSCore::EntryInternal copy = entry;
    for (auto downloadInfo: entry.downloadLinkInformationList()) {
        if (downloadInfo.id == linkNumber) {
            for (QString string: downloadInfo.tags) {
                if (string.startsWith(KEY_URL)) {
                    copy.setPayload(string.split(QStringLiteral("=")).last());
                }
            }

        }
    }
    Q_EMIT payloadLinkLoaded(copy);
}

bool OPDSProvider::setProviderXML(const QDomElement &xmldata)
{
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

    if (!feedDoc->isValid()) {
        qCWarning(KNEWSTUFFCORE) << "OPDS Feed not valid";
        Q_EMIT loadingFailed(m_currentRequest);
        return;
    }
    if (!feedDoc->title().isEmpty()) {
        m_providerName = feedDoc->title();
    }
    if (!feedDoc->icon().isEmpty()) {
        m_iconUrl = QUrl(fixRelativeUrl(feedDoc->icon()));
    }

    EntryInternal::List entries;
    QList<SearchPreset> presets;

    {
        SearchPreset preset;
        preset.providerId = m_providerId;
        SearchRequest request;
        request.searchTerm = m_providerId;
        preset.request = request;
        preset.type = Provider::SearchPresetTypes::Start;
        presets.append(preset);
    }

    QList<KNSCore::Provider::CategoryMetadata> categories;

    // find the self link first!
    m_selfUrl.clear();
    for (auto link: feedDoc->links()) {
        if (link.rel().contains(REL_SELF)) {
            m_selfUrl = link.href();
        }
    }

    for (auto link: feedDoc->links()) {
        // There will be a number of links toplevel, amongst which probably a lot of sortorder and navigation links.
        if (link.rel() == REL_SEARCH && link.type() == OPENSEARCH_MT) {
            m_openSearchDocumentURL = fixRelativeUrl(link.href());
            m_xmlLoader = new XmlLoader(this);
            connect(m_xmlLoader, &XmlLoader::signalLoaded, this, &OPDSProvider::parseOpenSearchDocument);
            connect(m_xmlLoader, &XmlLoader::signalFailed, this, [this]() {
                qCWarning(KNEWSTUFFCORE) << "OpenSearch XML Document Loading failed" << m_openSearchDocumentURL;
            });
            m_xmlLoader->load(m_openSearchDocumentURL);
        } else if (link.type().contains(OPDS_PROFILE) && link.rel() != REL_SELF) {
            SearchPreset preset;
            preset.providerId = m_providerId;
            preset.displayName = link.title();
            SearchRequest request;
            request.searchTerm = fixRelativeUrl(link.href()).toString();
            preset.request = request;
            if (link.rel() == REL_START) {
                preset.type = Provider::SearchPresetTypes::Root;
            } else if (link.rel() == OPDS_REL_FEATURED) {
                preset.type = Provider::SearchPresetTypes::Featured;
            } else if (link.rel() == OPDS_REL_SHELF) {
                preset.type = Provider::SearchPresetTypes::Shelf;
            } else if (link.rel() == OPDS_REL_SORT_NEW) {
                preset.type = Provider::SearchPresetTypes::New;
            } else if (link.rel() == OPDS_REL_SORT_POPULAR) {
                preset.type = Provider::SearchPresetTypes::Popular;
            } else if (link.rel() == REL_UP) {
                preset.type = Provider::SearchPresetTypes::FolderUp;
            } else if (link.rel() == OPDS_REL_CRAWL) {
                preset.type = Provider::SearchPresetTypes::AllEntries;
            } else if (link.rel() == OPDS_REL_RECOMMENDED) {
                preset.type = Provider::SearchPresetTypes::Recommended;
            } else if (link.rel() == OPDS_REL_SUBSCRIPTIONS) {
                preset.type = Provider::SearchPresetTypes::Subscription;
            } else {
                preset.type = Provider::SearchPresetTypes::NoPresetType;
                if (preset.displayName.isEmpty()) {
                    preset.displayName = link.rel();
                }
            }
            presets.append(preset);
        }
    }
    TagsFilterChecker downloadTagChecker(downloadTagFilter());
    TagsFilterChecker entryTagChecker(tagFilter());

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
        QStringList entryTags;
        for(int j=0; j<feedEntry.categories().size(); j++) {
            QString tag = feedEntry.categories().at(j).label();
            if (tag.isEmpty()) {
                tag = feedEntry.categories().at(j).term();
            }
            entryTags.append(tag);
        }
        if (entryTagChecker.filterAccepts(entryTags)) {
            entry.setTags(entryTags);
        } else {
            continue;
        }
        // Same issue with author...
        for(int j=0; j<feedEntry.authors().size(); j++) {
            Author author;
            Syndication::Atom::Person person = feedEntry.authors().at(j);
            author.setId(person.uri());
            author.setName(person.name());
            author.setEmail(person.email());
            author.setHomepage(person.uri());
            entry.setAuthor(author);
        }
        entry.setLicense(feedEntry.rights());
        if (feedEntry.content().isEscapedHTML()) {
            entry.setSummary(feedEntry.content().childNodesAsXML());
        } else {
            entry.setSummary(feedEntry.content().asString());
        }
        entry.setShortSummary(feedEntry.summary());

        int counterThumbnails = 0;
        int counterImages = 0;
        QString groupEntryUrl;
        for(int j=0; j<feedEntry.links().size(); j++) {
            Syndication::Atom::Link link = feedEntry.links().at(j);

            KNSCore::EntryInternal::DownloadLinkInformation download;
            download.id = entry.downloadLinkCount()+1;
            // Linkrelations can have multiple values, expressed as something like... rel="me nofollow alternate".
            QStringList linkRelation = link.rel().split(QStringLiteral(" "));

            QStringList tags;
            tags.append(KEY_MIME_TYPE+link.type());
            if (!link.hrefLanguage().isEmpty()) { tags.append(KEY_LANGUAGE+link.hrefLanguage()); }
            QString linkUrl = fixRelativeUrl(link.href()).toString();
            tags.append(KEY_URL + linkUrl);
            download.name = link.title();
            download.size = link.length() / 1000;
            download.tags = tags;
            download.isDownloadtypeLink = false;


            if (link.rel().startsWith(OPDS_REL_ACQUISITION)) {
                if (link.title().isEmpty()) {
                    QStringList l;
                    l.append(link.type());
                    l.append(QStringLiteral("(")+link.rel().split(QStringLiteral("/")).last()+ QStringLiteral(")"));
                    download.name = l.join(QStringLiteral(" "));
                }

                if (!downloadTagChecker.filterAccepts(download.tags)) {

                    continue;
                }

                if (linkRelation.contains(OPDS_REL_AC_BORROW) || linkRelation.contains(OPDS_REL_AC_SUBSCRIBE)
                        || linkRelation.contains(OPDS_REL_AC_BUY)) {

                    // TODO we don't support borrow, buy and subscribe right now, requires authentication.
                    continue;

                } else if (linkRelation.contains(OPDS_REL_ACQUISITION) || linkRelation.contains(OPDS_REL_AC_OPEN_ACCESS)) {

                    download.isDownloadtypeLink = true;

                    if (entry.status() != KNS3::Entry::Installed &&
                            entry.status() != KNS3::Entry::Updateable) {
                        entry.setStatus(KNS3::Entry::Downloadable);
                    }

                    entry.setEntryType(EntryInternal::CatalogEntry);

                }
                //TODO, support preview relation, but this requires we show that an entry is otherwise paid for in the UI.

                for (QDomElement el:feedEntry.elementsByTagName(OPDS_EL_PRICE)) {
                    QLocale locale;
                    download.priceAmount = locale.toCurrencyString(el.text().toFloat(), el.attribute(ATTR_CURRENCY_CODE));
                }
                // There's an 'opds:indirectaquistition' element that gives extra metadata about bundles.
                entry.appendDownloadLinkInformation(download);

            } else if (link.rel().startsWith(OPDS_REL_IMAGE)) {
                if (link.rel() == OPDS_REL_THUMBNAIL) {
                    entry.setPreviewUrl( linkUrl, KNSCore::EntryInternal::PreviewType(counterThumbnails));
                    counterThumbnails +=1;
                } else {
                    entry.setPreviewUrl( linkUrl, KNSCore::EntryInternal::PreviewType(counterImages+3));
                    counterImages +=1;
                }

            } else {
                // This could be anything from a more info link, to navigation links, to links to the outside world.
                // Todo: think of using link rel's 'replies', 'payment'(donation) and 'version-history'.

                if (link.type().startsWith(OPDS_ATOM_MT) ) {
                    groupEntryUrl = linkUrl;

                } else  if (link.type() == HTML_MT && linkRelation.contains(REL_ALTERNATE)) {
                    entry.setHomepage( QUrl(linkUrl) );

                } else if (downloadTagChecker.filterAccepts(download.tags)) {
                    entry.appendDownloadLinkInformation( download );
                }
            }
        }

        //Todo:
        // feedEntry.elementsByTagName( dc:terms:issued ) is the official initial release date.
        // published is the released date of the opds catalog item, updated for the opds catalog item update.
        // maybe we should make sure to also check dc:terms:modified?
        //QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.published());

        QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.updated());

        if (entry.releaseDate().isNull()) {
            entry.setReleaseDate(date.date());
        }

        if (entry.status() != KNS3::Entry::Invalid) {
            entry.setPayload(QString());
            // Gutenberg doesn't do versioning in the opds, so it's update value is unreliable,
            // even though openlib and standard do use it properly. We'll instead doublecheck that
            // the new time is larger than 6min since we requested the feed.
            if (date.secsTo(m_currentTime) > 360) {
                if (entry.releaseDate() < date.date()) {
                    entry.setUpdateReleaseDate(date.date());
                    if (entry.status() == KNS3::Entry::Installed) {
                        entry.setStatus(KNS3::Entry::Updateable);
                    }
                }
            }
        }
        if (counterThumbnails == 0) {
            //fallback.
            if (!feedDoc->icon().isEmpty()) {
                entry.setPreviewUrl(fixRelativeUrl(feedDoc->icon()).toString());
            }
        }

        if (entry.downloadLinkCount() == 0) {
            if (groupEntryUrl.isEmpty()) {
                continue;
            } else {
                entry.setEntryType(EntryInternal::GroupEntry);
                entry.setPayload(groupEntryUrl);

            }
        }

        entries.append(entry);
    }

    if (m_loadingExtraDetails) {
        Q_EMIT entryDetailsLoaded(entries.first());
        m_loadingExtraDetails = false;
    } else {
        Q_EMIT loadingFinished(m_currentRequest, entries);
    }
    Q_EMIT categoriesMetadataLoded(categories);
    Q_EMIT searchPresetsLoaded(presets);
}

void OPDSProvider::slotLoadingFailed()
{
    qCWarning(KNEWSTUFFCORE) << "OPDS Loading failed" << m_currentUrl;
    Q_EMIT loadingFailed(m_currentRequest);
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

void OPDSProvider::parseOpenSearchDocument(const QDomDocument &doc)
{
    m_openSearchTemplate = QString();
    if (doc.documentElement().attribute(QStringLiteral("xmlns")) != OPENSEARCH_NS) {
        qCWarning(KNEWSTUFFCORE) << "Opensearch link does not point at document with opensearch namespace" << m_openSearchDocumentURL;
        return;
    }
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

QUrl OPDSProvider::openSearchStringForRequest(const Provider::SearchRequest &request)
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
        if (m_selfUrl.isEmpty() || !QUrl(m_selfUrl).isRelative()) {
            QUrl host = m_currentUrl;
            host.setPath(query.path());
            host.setQuery(query.query());
            return host;
        } else {
            int length = m_selfUrl.size();
            int index = m_currentUrl.toString().size()-length;
            QString base  = m_currentUrl.toString().remove(index, length);
            base += urlPart;
            return QUrl(base);
        }
    }
    return query;
}
}

