// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QDebug>
#include <QList>
#include <QString>
#include <QUrl>

#include <memory>

#include "entry.h"
#include "errorcode.h"

#include "commentsmodel.h"
#include "knewstuffcore_export.h"
#include "searchrequest.h"

namespace KNSCore
{

class ProviderBase;

class ProviderBasePrivate
{
public:
    ProviderBasePrivate(ProviderBase *qq)
        : q(qq)
    {
    }
    ProviderBase *q;
    QStringList tagFilter;
    QStringList downloadTagFilter;
};

/**
 * @brief ProviderBase Interface
 * Exported for our qtquick components. Do not install the header or use from the outside!
 */
class KNEWSTUFFCORE_EXPORT ProviderBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl website READ website NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl host READ host NOTIFY basicsLoaded)
    Q_PROPERTY(QString contactEmail READ contactEmail NOTIFY basicsLoaded)
    Q_PROPERTY(bool supportsSsl READ supportsSsl NOTIFY basicsLoaded)
public:
    ProviderBase(QObject *parent = nullptr);

    /**
     * A unique Id for this provider (the url in most cases)
     */
    [[nodiscard]] virtual QString id() const = 0;

    /**
     * Set the provider data xml, to initialize the provider.
     * The Provider needs to have it's ID set in this function and cannot change it from there on.
     */
    virtual bool setProviderXML(const QDomElement &xmldata) = 0;

    [[nodiscard]] virtual bool isInitialized() const = 0;

    virtual void setCachedEntries(const KNSCore::Entry::List &cachedEntries) = 0;

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    [[nodiscard]] virtual QString name() const = 0;

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    [[nodiscard]] virtual QUrl icon() const = 0; // FIXME use QIcon::fromTheme or pixmap?

    /**
     * load the given search and return given page
     * @param sortMode string to select the order in which the results are presented
     * @param searchstring string to search with
     * @param page         page number to load
     *
     * Note: the engine connects to loadingFinished() signal to get the result
     */
    virtual void loadEntries(const KNSCore::SearchRequest &request) = 0;
    virtual void loadEntryDetails(const KNSCore::Entry &)
    {
    }
    virtual void loadPayloadLink(const Entry &entry, int linkId) = 0;
    /**
     * Request a loading of comments from this provider. The engine listens to the
     * commentsLoaded() signal for the result
     *
     * @note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * @see commentsLoaded(const QList<shared_ptr<KNSCore::Comment>> comments)
     * @since 5.63
     */
    virtual void loadComments(const KNSCore::Entry &, int /*commentsPerPage*/, int /*page*/)
    {
    }

    /**
     * Request loading of the details for a specific person with the given username.
     * The engine listens to the personLoaded() for the result
     *
     * @note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * @since 5.63
     */
    virtual void loadPerson(const QString & /*username*/)
    {
    }

    /**
     * @since 5.85
     */
    [[nodiscard]] virtual QString version() = 0;
    /**
     * @since 5.85
     */
    [[nodiscard]] virtual QUrl website() = 0;
    /**
     * @since 5.85
     */
    [[nodiscard]] virtual QUrl host() = 0;
    /**
     * The general contact email for this provider
     * @return The general contact email for this provider
     * @since 5.85
     */
    [[nodiscard]] virtual QString contactEmail() = 0;
    /**
     * Whether or not the provider supports SSL connections
     * @return True if the server supports SSL connections, false if not
     * @since 5.85
     */
    [[nodiscard]] virtual bool supportsSsl() = 0;

    virtual bool userCanVote()
    {
        return false;
    }
    virtual void vote(const Entry & /*entry*/, uint /*rating*/)
    {
    }

    virtual bool userCanBecomeFan()
    {
        return false;
    }
    virtual void becomeFan(const Entry & /*entry*/)
    {
    }

    /**
     * Set the tag filter used for entries by this provider
     * @param tagFilter The new list of filters
     * @see Engine::setTagFilter(QStringList)
     * @since 5.51
     */
    void setTagFilter(const QStringList &tagFilter);
    /**
     * The tag filter used for downloads by this provider
     * @return The list of filters
     * @see Engine::setTagFilter(QStringList)
     * @since 5.51
     */
    QStringList tagFilter() const;
    /**
     * Set the tag filter used for download items by this provider
     * @param downloadTagFilter The new list of filters
     * @see Engine::setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void setDownloadTagFilter(const QStringList &downloadTagFilter);
    /**
     * The tag filter used for downloads by this provider
     * @return The list of filters
     * @see Engine::setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    QStringList downloadTagFilter() const;

Q_SIGNALS:
    void providerInitialized(KNSCore::ProviderBase *);

    void entriesLoaded(const KNSCore::SearchRequest &, const KNSCore::Entry::List &);
    void loadingDone(const KNSCore::SearchRequest &);
    void loadingFailed(const KNSCore::SearchRequest &);

    void entryDetailsLoaded(const KNSCore::Entry &);
    void payloadLinkLoaded(const KNSCore::Entry &);
    /**
     * Fired when new comments have been loaded
     * @param comments The list of newly loaded comments, in a depth-first order
     * @since 5.63
     */
    void commentsLoaded(const QList<std::shared_ptr<KNSCore::Comment>> &comments);
    /**
     * Fired when the details of a person have been loaded
     * @param author The person we've just loaded data for
     * @since 5.63
     */
    void personLoaded(const std::shared_ptr<KNSCore::Author> &author);
    /**
     * Fired when the provider's basic information has been fetched and updated
     * @since 5.85
     */
    void basicsLoaded();

    /**
     * Fires when the provider has loaded search presets. These represent interesting
     * searches for the user, such as recommendations.
     * @since 5.83
     */
    void searchPresetsLoaded(const QList<KNSCore::SearchPreset> &presets);

    void signalInformation(const QString &);
    void signalError(const QString &);
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);
    void categoriesMetadataLoaded(const QList<KNSCore::CategoryMetadata> &categories);
    void tagFilterChanged();
    void downloadTagFilterChanged();

private:
    friend class ProviderBubbleWrap;
    std::unique_ptr<ProviderBasePrivate> d;
};

}
