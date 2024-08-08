/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_STATICXMLPROVIDER_P_H
#define KNEWSTUFF3_STATICXMLPROVIDER_P_H

#include "providerbase_p.h"
#include <QDomDocument>
#include <QMap>

namespace KNSCore
{
class XmlLoader;

/**
 * @short KNewStuff Base Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * static website providers.
 *
 * @author Jeremy Whiting <jpwhiting@kde.org>
 *
 * @internal
 */
class StaticXmlProvider : public ProviderBase
{
    Q_OBJECT
public:
    typedef QList<Provider *> List;
    /**
     * Constructor.
     */
    StaticXmlProvider();

    QString id() const override;

    /**
     * set the provider data xml, to initialize the provider
     */
    bool setProviderXML(const QDomElement &xmldata) override;

    bool isInitialized() const override;

    void setCachedEntries(const KNSCore::Entry::List &cachedEntries) override;

    void loadEntries(const KNSCore::SearchRequest &request) override;
    void loadPayloadLink(const KNSCore::Entry &entry, int) override;

    [[nodiscard]] QString name() const override;
    [[nodiscard]] QUrl icon() const override;
    [[nodiscard]] QString version() override;
    [[nodiscard]] QUrl website() override;
    [[nodiscard]] QUrl host() override;
    [[nodiscard]] QString contactEmail() override;
    [[nodiscard]] bool supportsSsl() override;

private Q_SLOTS:
    void slotFeedFileLoaded(const KNSCore::SearchRequest &request, const QDomDocument &);

private:
    bool searchIncludesEntry(const KNSCore::SearchRequest &request, const Entry &entry) const;
    QUrl downloadUrl(SortMode mode) const;
    Entry::List installedEntries() const;

    // map of download urls to their feed name
    QMap<QString, QUrl> mDownloadUrls;
    QUrl mUploadUrl;
    QUrl mNoUploadUrl;

    // cache of all entries known from this provider so far, mapped by their id
    Entry::List mCachedEntries;
    QMap<SortMode, XmlLoader *> mFeedLoaders;
    QString mId;
    bool mInitialized;
    QUrl m_iconUrl;
    QString m_name;

    Q_DISABLE_COPY(StaticXmlProvider)
};

}

#endif
