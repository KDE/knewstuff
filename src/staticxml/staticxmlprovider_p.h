/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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
#ifndef KNEWSTUFF3_STATICXMLPROVIDER_P_H
#define KNEWSTUFF3_STATICXMLPROVIDER_P_H

#include "provider.h"
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
class StaticXmlProvider: public Provider
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

    void setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries) override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadPayloadLink(const KNSCore::EntryInternal &entry, int) override;

private Q_SLOTS:
    void slotEmitProviderInitialized();
    void slotFeedFileLoaded(const QDomDocument &);
    void slotFeedFailed();

private:
    bool searchIncludesEntry(const EntryInternal &entry) const;
    QUrl downloadUrl(SortMode mode) const;
    EntryInternal::List installedEntries() const;

    // map of download urls to their feed name
    QMap<QString, QUrl> mDownloadUrls;
    QUrl mUploadUrl;
    QUrl mNoUploadUrl;

    // cache of all entries known from this provider so far, mapped by their id
    EntryInternal::List mCachedEntries;
    QMap<Provider::SortMode, XmlLoader *> mFeedLoaders;
    Provider::SearchRequest mCurrentRequest;
    QString mId;
    bool mInitialized;

    Q_DISABLE_COPY(StaticXmlProvider)
};

}

#endif
