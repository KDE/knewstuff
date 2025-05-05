/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_PROVIDER_P_H
#define KNEWSTUFF3_PROVIDER_P_H

#include <QDebug>
#include <QList>
#include <QString>
#include <QUrl>

#include <memory>

#include "entry.h"
#include "errorcode.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class ProviderPrivate;
struct Comment;

/*!
 * \class KNSCore::Provider
 * \inheaderfile KNewStuff/Provider
 * \inmodule KNewStuffCore
 *
 * \brief Base Provider class.
 *
 * This class provides accessors for the provider object.
 *
 * \note This class should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * static website providers.
 *
 * \deprecated[6.9] Use ProviderBase to implement providers (only in-tree supported). Use ProviderCore to manage instances of base.
 */
class KNEWSTUFFCORE_EXPORT
    KNEWSTUFFCORE_DEPRECATED_VERSION(6,
                                     9,
                                     "Use ProviderBase to implement providers (only in-tree supported). Use ProviderCore to manage instances of base.") Provider
    : public QObject
{
    Q_OBJECT

    /*!
     * \qmlproperty string Provider::version
     */
    /*!
     * \property KNSCore::Provider::version
     */
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY basicsLoaded)

    /*!
     * \qmlproperty url Provider::website
     */
    /*!
     * \property KNSCore::Provider::website
     */
    Q_PROPERTY(QUrl website READ website WRITE setWebsite NOTIFY basicsLoaded)

    /*!
     * \qmlproperty url Provider::host
     */
    /*!
     * \property KNSCore::Provider::host
     */
    Q_PROPERTY(QUrl host READ host WRITE setHost NOTIFY basicsLoaded)

    /*!
     * \qmlproperty string Provider::contactEmail
     */
    /*!
     * \property KNSCore::Provider::contactEmail
     */
    Q_PROPERTY(QString contactEmail READ contactEmail WRITE setContactEmail NOTIFY basicsLoaded)

    /*!
     * \qmlproperty bool Provider::supportsSsl
     */
    /*!
     * \property KNSCore::Provider::supportsSsl
     */
    Q_PROPERTY(bool supportsSsl READ supportsSsl WRITE setSupportsSsl NOTIFY basicsLoaded)
public:
    typedef QList<Provider *> List;

    /*!
     * \enum KNSCore::Provider::SortMode
     *
     * \value Newest
     * \value Alphabetical
     * \value Rating
     * \value Downloads
     */
    enum SortMode {
        Newest,
        Alphabetical,
        Rating,
        Downloads,
    };
    Q_ENUM(SortMode)

    /*!
     * KNSCore::Provider::Filter
     *
     * \value None
     * \value Installed
     * \value Updates
     * \value ExactEntryId
     */
    enum Filter {
        None,
        Installed,
        Updates,
        ExactEntryId,
    };
    Q_ENUM(Filter)

    /*!
     * used to keep track of a search
     * \deprecated[6.9]
     * Use KNSCore::SearchRequest
     */
    struct KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Use KNSCore::SearchRequest") SearchRequest {
        SortMode sortMode;
        Filter filter;
        QString searchTerm;
        QStringList categories;
        int page;
        int pageSize;

        SearchRequest(SortMode sortMode_ = Downloads,
                      Filter filter_ = None,
                      const QString &searchTerm_ = QString(),
                      const QStringList &categories_ = QStringList(),
                      int page_ = 0,
                      int pageSize_ = 20)
            : sortMode(sortMode_)
            , filter(filter_)
            , searchTerm(searchTerm_)
            , categories(categories_)
            , page(page_)
            , pageSize(pageSize_)
        {
        }

        QString hashForRequest() const;
        bool operator==(const SearchRequest &other) const
        {
            return sortMode == other.sortMode && filter == other.filter && searchTerm == other.searchTerm && categories == other.categories
                && page == other.page && pageSize == other.pageSize;
        }
    };

    /*!
     * Describes a category: id/name/displayName
     * \deprecated[6.9]
     * Use KNSCore::CategoryMetadata
     */
    struct KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Use KNSCore::CategoryMetadata") CategoryMetadata {
        /*!
         *
         */
        QString id;

        /*!
         *
         */
        QString name;

        /*!
         * The human-readable name displayed to the user.
         */
        QString displayName;
    };

    /*!
     * \enum KNSCore::Provider::SearchPresetTypes
     * \brief A helper to identify the kind of label and icon
     * the search preset should have if none are found.
     * \deprecated[6.9]
     * Use KNSCore::SearchPreset::SearchPresetTypes
     *
     * \since 5.83
     *
     * \value NoPresetType
     *
     * \value GoBack
     * Preset representing the previous search.
     *
     * \value Root
     * Preset indicating a root directory.
     *
     * \value Start
     * Preset indicating the first entry.
     *
     * \value Popular
     * Preset indicating popular items.
     *
     * \value Featured
     * Preset for featured items.
     *
     * \value Recommended
     * Preset for recommended. This may be customized by the server per user.
     *
     * \value Shelf
     * Preset indicating previously acquired items.
     *
     * \value Subscription
     * Preset indicating items that the user is subscribed to.
     *
     * \value New
     * Preset indicating new items.
     *
     * \value FolderUp
     * Preset indicating going up in the search result hierarchy.
     *
     * \value AllEntries
     * Preset indicating all possible entries, such as a crawlable list. Might be intense to load.
     */
    enum KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Use KNSCore::SearchPreset::SearchPresetTypes") SearchPresetTypes {
        NoPresetType = 0,
        GoBack,
        Root,
        Start,
        Popular,
        Featured,
        Recommended,
        Shelf,
        Subscription,
        New,
        FolderUp,
        AllEntries,
    };
    /*!
     * \class KNSCore::Provider::SearchPreset
     * \inmodule KNewStuffCore
     * \brief Describes a search request that may come from the provider.
     * This is used by the OPDS provider to handle the different urls.
     * \deprecated[6.9]
     * Use KNSCore::SearchPreset
     */
    struct KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Use KNSCore::SearchPreset") SearchPreset {
        SearchRequest request;
        QString displayName;
        QString iconName;
        SearchPresetTypes type;
        QString providerId; // not all providers can handle all search requests.
    };

    /*!
     * Constructor.
     */
    Provider();

    ~Provider() override;

    /*!
     * A unique Id for this provider (the url in most cases)
     */
    virtual QString id() const = 0;

    /*!
     * Sets the provider \a xmldata to initialize the provider.
     * The Provider needs to have it's ID set in this function and cannot change it from there on.
     *
     * Returns true if successful, or false if the XML data is invalid
     */
    virtual bool setProviderXML(const QDomElement &xmldata) = 0;

    /*!
     *
     */
    virtual bool isInitialized() const = 0;

    /*!
     *
     */
    virtual void setCachedEntries(const KNSCore::Entry::List &cachedEntries) = 0;

    /*!
     * Returns the common name of the provider.
     */
    virtual QString name() const;

    /*!
     * Returns the icon URL for this provider.
     */
    virtual QUrl icon() const; // FIXME use QIcon::fromTheme or pixmap?

    /*!
     * Loads the given search and return given page
     *
     * \a request defines the search parameters
     *
     * \note the engine connects to loadingFinished() signal to get the result
     */
    virtual void loadEntries(const KNSCore::Provider::SearchRequest &request) = 0;

    /*!
     *
     */
    virtual void loadEntryDetails(const KNSCore::Entry &)
    {
    }

    /*!
     *
     */
    virtual void loadPayloadLink(const Entry &entry, int linkId) = 0;

    /*!
     * Request a loading of comments from this provider. The engine listens to the
     * commentsLoaded() signal for the result
     *
     * \note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * \sa commentsLoaded
     * \since 5.63
     */
    virtual void loadComments(const KNSCore::Entry &, int /*commentsPerPage*/, int /*page*/)
    {
    }

    /*!
     * Request loading of the details for a specific person with the given username.
     * The engine listens to the personLoaded() for the result
     *
     * \note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * \since 5.63
     */
    virtual void loadPerson(const QString & /*username*/)
    {
    }

    /*!
     * Request loading of the basic information for this provider. The engine listens
     * to the basicsLoaded() signal for the result, which is also the signal the respective
     * properties listen to.
     *
     * This is fired automatically on the first attempt to read one of the properties
     * which contain this basic information, and you will not need to call it as a user
     * of the class (just listen to the properties, which will update when the information
     * has been fetched).
     *
     * \note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support fetching
     * this basic information (if the information is set during construction, you will
     * not need to worry about this).
     *
     * \sa version()
     * \sa website()
     * \sa host()
     * \sa contactEmail()
     * \sa supportsSsl()
     * \since 5.85
     */
    virtual void loadBasics()
    {
    }

    /*!
     * \since 5.85
     */
    QString version() const;

    /*!
     * Sets the \a version.
     * \since 5.85
     */
    void setVersion(const QString &version);

    /*!
     * \since 5.85
     */
    QUrl website() const;

    /*!
     * Sets the \a website URL.
     * \since 5.85
     */
    void setWebsite(const QUrl &website);

    /*!
     * \since 5.85
     */
    QUrl host() const;

    /*!
     * Sets the host used for this provider to \a host.
     *
     * \since 5.85
     */
    void setHost(const QUrl &host);

    /*!
     * Returns The general contact email for this provider.
     * \since 5.85
     */
    QString contactEmail() const;

    /*!
     * Sets the general contact email address for this provider.
     *
     * \a contactEmail The general contact email for this provider
     *
     * \since 5.85
     */
    void setContactEmail(const QString &contactEmail);

    /*!
     * Returns True if the server supports SSL connections, false if not
     * \since 5.85
     */
    bool supportsSsl() const;

    /*!
     * Sets whether or not the provider supports SSL connections.
     *
     * \a supportsSsl True if the server supports SSL connections, false if not
     *
     * \since 5.85
     */
    void setSupportsSsl(bool supportsSsl);

    /*!
     *
     */
    virtual bool userCanVote()
    {
        return false;
    }

    /*!
     *
     */
    virtual void vote(const Entry & /*entry*/, uint /*rating*/)
    {
    }

    /*!
     *
     */
    virtual bool userCanBecomeFan()
    {
        return false;
    }

    /*!
     *
     */
    virtual void becomeFan(const Entry & /*entry*/)
    {
    }

    /*!
     * Sets the tag filter used for entries by this provider.
     *
     * \a tagFilter The new list of filters
     *
     * \sa EngineBase::setTagFilter
     * \since 5.51
     */
    void setTagFilter(const QStringList &tagFilter);

    /*!
     * Returns the tag filter used for downloads by this provider.
     * \sa EngineBase::setTagFilter
     * \since 5.51
     */
    QStringList tagFilter() const;

    /*!
     * Sets the tag filter used for download items by this provider
     *
     * \a downloadTagFilter The new list of filters
     *
     * \sa EngineBase::setDownloadTagFilter
     * \since 5.51
     */
    void setDownloadTagFilter(const QStringList &downloadTagFilter);

    /*!
     * Returns the tag filter used for downloads by this provider.
     * \sa EngineBase::setDownloadTagFilter
     * \since 5.51
     */
    QStringList downloadTagFilter() const;

Q_SIGNALS:
    /*!
     *
     */
    void providerInitialized(KNSCore::Provider *);

    /*!
     *
     */
    void loadingFinished(const KNSCore::Provider::SearchRequest &, const KNSCore::Entry::List &);

    /*!
     *
     */
    void loadingFailed(const KNSCore::Provider::SearchRequest &);

    /*!
     *
     */
    void entryDetailsLoaded(const KNSCore::Entry &);

    /*!
     *
     */
    void payloadLinkLoaded(const KNSCore::Entry &);

    /*!
     * Fired when new comments have been loaded
     *
     * \a comments The list of newly loaded comments, in a depth-first order
     *
     * \since 5.63
     */
    void commentsLoaded(const QList<std::shared_ptr<KNSCore::Comment>> comments);

    /*!
     * Fired when the details of a person have been loaded
     *
     * \a author The person we've just loaded data for
     *
     * \since 5.63
     */
    void personLoaded(const std::shared_ptr<KNSCore::Author> author);
    /*!
     * Fired when the provider's basic information has been fetched and updated
     * \since 5.85
     */
    void basicsLoaded();

    /*!
     * Fires when the provider has loaded search \a presets. These represent interesting
     * searches for the user, such as recommendations.
     * \since 5.83
     */
    void searchPresetsLoaded(const QList<KNSCore::Provider::SearchPreset> &presets);

    /*!
     *
     */
    void signalInformation(const QString &);

    /*!
     *
     */
    void signalError(const QString &);

    /*!
     *
     */
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

    /*!
     *
     */
    void categoriesMetadataLoded(const QList<KNSCore::Provider::CategoryMetadata> &categories);

    /*!
     *
     */
    void tagFilterChanged();

    /*!
     *
     */
    void downloadTagFilterChanged();

protected:
    /*!
     *
     */
    void setName(const QString &name);
    /*!
     *
     */
    void setIcon(const QUrl &icon);

private:
    friend class ProviderBubbleWrap;
    const std::unique_ptr<ProviderPrivate> d;
    Q_DISABLE_COPY(Provider)
};

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug, const Provider::SearchRequest &);
}

#endif
