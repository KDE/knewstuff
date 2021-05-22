/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OPDSPROVIDER_H
#define OPDSPROVIDER_H

#include "provider.h"
#include <xmlloader.h>
#include <QMap>

/**
 * OPDS provider.
 *
 * The OPDS provider loads OPDS feeds:
 * https://specs.opds.io/opds-1.2
 *
 * These feeds are most common with online book providers, but the format itself is agnostic.
 *
 * Supports:
 * - Loads a given feed, it's images, and loads it's download links.
 * - Opensearch for the search, if available.
 * - Should load full entries, if possible.
 *
 * TODO:
 * - Navigation links don't work.
 * - Navigation feed entries cannot be selected.
 * - We need a better way to indicate the non-free items.
 * - entry navigation links are now mixed in with the download links.
 * - pagination support (together with the navigation links)
 * - lots of duplication between parseExtraDetails and parseFeed.
 * - Caching is half-implemented.
 * - hairyness regarding the way categories work???
 *
 * Would-be-nice, but requires a lot of rewiring in knewstuff:
 * - We could get authenticated feeds going by using basic http authentiation(in spec), or have bearer token uris (oauth bearcaps).
 * - Autodiscovery or protocol based discovery of opds catalogs, this does not gel with the provider xml system used by knewstuff.
 */

namespace KNSCore
{
class XmlLoader;

class OPDSProvider : public Provider
{
    Q_OBJECT
public:
    typedef QList<Provider *> List;

    OPDSProvider();
    ~OPDSProvider();

    // Unique ID, url of the feed.
    QString id() const override;

    // Name of the feed.
    QString name() const override;

    // Feed icon
    QUrl icon() const override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadEntryDetails(const KNSCore::EntryInternal &entry) override;
    void loadPayloadLink(const KNSCore::EntryInternal &entry, int linkNumber) override;

    bool setProviderXML(const QDomElement &xmldata) override;
    bool isInitialized() const override;
    void setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries) override;

private Q_SLOTS:
    void parseFeedData(const QDomDocument &doc);
    void parseExtraDetails(const QDomDocument &doc);
    void slotEmitProviderInitialized();

    // Parse the opensearch configuration document.
    // https://github.com/dewitt/opensearch
    void parserOpenSearchDocument(const QDomDocument &doc);

private:
    // Generate an opensearch string.
    QUrl getOpenSearchString(const KNSCore::Provider::SearchRequest &request);
    QUrl fixRelativeUrl(QString urlPart);

    QString m_providerId;
    QString m_providerName;
    QUrl m_iconUrl;
    bool m_initialized;

    /***
     * OPDS catalogs consist of many small atom feeds. This variable
     * tracks which atom feed to load.
     */
    QUrl m_currentUrl;

    XmlLoader *m_xmlLoader;

    EntryInternal::List m_cachedEntries;
    Provider::SearchRequest m_currentRequest;

    QString m_openSearchTemplate;

    Q_DISABLE_COPY(OPDSProvider)

};

}

#endif // OPDSPROVIDER_H
