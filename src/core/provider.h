/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>
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
#ifndef KNEWSTUFF3_PROVIDER_P_H
#define KNEWSTUFF3_PROVIDER_P_H

#include <QList>
#include <QString>
#include <QUrl>

#include "entryinternal.h"

#include "knewstuffcore_export.h"

class KJob;

namespace KNSCore
{
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
class KNEWSTUFFCORE_EXPORT Provider: public QObject
{
    Q_OBJECT
public:
    typedef QList<Provider *> List;

    enum SortMode {
        Newest,
        Alphabetical,
        Rating,
        Downloads,
    };
    Q_ENUM(SortMode)

    enum Filter {
        None,
        Installed,
        Updates,
        ExactEntryId
    };
    Q_ENUM(Filter)

    /**
     * used to keep track of a search
     */
    struct SearchRequest {
        SortMode sortMode;
        Filter filter;
        QString searchTerm;
        QStringList categories;
        int page;
        int pageSize;

        SearchRequest(SortMode sortMode_ = Newest, Filter filter_ = None, const QString &searchTerm_ = QString(), const QStringList &categories_ = QStringList(), int page_ = -1, int pageSize_ = 20)
            : sortMode(sortMode_), filter(filter_), searchTerm(searchTerm_), categories(categories_), page(page_), pageSize(pageSize_)
        {}

        QString hashForRequest() const;
    };

    /**
     * Describes a category: id/name/disaplayName
     */
    struct CategoryMetadata {
        QString id;
        QString name;
        QString displayName;
    };

    /**
     * Constructor.
     */
    Provider();

    /**
     * Destructor.
     */
    virtual ~Provider();

    /**
     * A unique Id for this provider (the url in most cases)
     */
    virtual QString id() const = 0;

    /**
     * Set the provider data xml, to initialize the provider.
     * The Provider needs to have it's ID set in this function and cannot change it from there on.
     */
    virtual bool setProviderXML(const QDomElement &xmldata) = 0;

    virtual bool isInitialized() const = 0;

    virtual void setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries) = 0;

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    virtual QString name() const;

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    virtual QUrl icon() const; // FIXME use QIcon::fromTheme or pixmap?

    /**
     * load the given search and return given page
     * @param sortMode string to select the order in which the results are presented
     * @param searchstring string to search with
     * @param page         page number to load
     *
     * Note: the engine connects to loadingFinished() signal to get the result
     */
    virtual void loadEntries(const KNSCore::Provider::SearchRequest &request) = 0;
    virtual void loadEntryDetails(const KNSCore::EntryInternal &) {}
    virtual void loadPayloadLink(const EntryInternal &entry, int linkId) = 0;

    virtual bool userCanVote()
    {
        return false;
    }
    virtual void vote(const EntryInternal &entry, uint rating)
    {
        Q_UNUSED(entry) Q_UNUSED(rating)
    }

    virtual bool userCanBecomeFan()
    {
        return false;
    }
    virtual void becomeFan(const EntryInternal &entry)
    {
        Q_UNUSED(entry)
    }

Q_SIGNALS:
    void providerInitialized(KNSCore::Provider *);

    void loadingFinished(const KNSCore::Provider::SearchRequest &, const KNSCore::EntryInternal::List &) const;
    void loadingFailed(const KNSCore::Provider::SearchRequest &);

    void entryDetailsLoaded(const KNSCore::EntryInternal &);
    void payloadLinkLoaded(const KNSCore::EntryInternal &);

    void signalInformation(const QString &) const;
    void signalError(const QString &) const;

    void categoriesMetadataLoded(const QList<CategoryMetadata> &categories);

protected:
    QString mName;
    QUrl mIcon;

private:
    Q_DISABLE_COPY(Provider)
};

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug, const Provider::SearchRequest &);
}

#endif
