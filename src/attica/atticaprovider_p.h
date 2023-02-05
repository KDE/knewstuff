/*
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ATTICAPROVIDER_P_H
#define KNEWSTUFF3_ATTICAPROVIDER_P_H

#include <QPointer>
#include <QSet>

#include <attica/content.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "provider.h"

namespace Attica
{
class BaseJob;
}

namespace KNSCore
{
/**
 * @short KNewStuff Attica Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * websites that implement the Open Collaboration Services.
 *
 * @author Frederik Gladhorn <gladhorn@kde.org>
 *
 * @internal
 */
class AtticaProvider : public Provider
{
    Q_OBJECT
public:
    explicit AtticaProvider(const QStringList &categories, const QString &additionalAgentInformation);
    AtticaProvider(const Attica::Provider &provider, const QStringList &categories, const QString &additionalAgentInformation);

    QString id() const override;

    /**
     * set the provider data xml, to initialize the provider
     */
    bool setProviderXML(const QDomElement &xmldata) override;

    bool isInitialized() const override;
    void setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries) override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadEntryDetails(const KNSCore::EntryInternal &entry) override;
    void loadPayloadLink(const EntryInternal &entry, int linkId) override;
    /**
     * The slot which causes loading of comments for the Attica provider
     * @see Provider::loadComments(const EntryInternal &entry, int commentsPerPage, int page)
     */
    Q_SLOT void loadComments(const EntryInternal &entry, int commentsPerPage, int page);
    /**
     * The slot which causes loading of a person's details
     * @see Provider::loadPerson(const QString &username)
     */
    Q_SLOT void loadPerson(const QString &username);
    /**
     * The slot which causes the provider's basic information to be fetched.
     * For the Attica provider, this translates to an OCS Config call
     * @see Provider::loadBasics()
     */
    Q_SLOT void loadBasics();

    bool userCanVote() override
    {
        return true;
    }
    void vote(const EntryInternal &entry, uint rating) override;

    bool userCanBecomeFan() override
    {
        return true;
    }
    void becomeFan(const EntryInternal &entry) override;

    Attica::Provider *provider()
    {
        return &m_provider;
    }

private Q_SLOTS:
    void providerLoaded(const Attica::Provider &provider);
    void listOfCategoriesLoaded(Attica::BaseJob *);
    void categoryContentsLoaded(Attica::BaseJob *job);
    void downloadItemLoaded(Attica::BaseJob *job);
    void accountBalanceLoaded(Attica::BaseJob *job);
    void onAuthenticationCredentialsMissing(const Attica::Provider &);
    void votingFinished(Attica::BaseJob *);
    void becomeFanFinished(Attica::BaseJob *job);
    void detailsLoaded(Attica::BaseJob *job);
    void loadedComments(Attica::BaseJob *job);
    void loadedPerson(Attica::BaseJob *job);
    void loadedConfig(Attica::BaseJob *job);

private:
    void checkForUpdates();
    EntryInternal::List installedEntries() const;
    bool jobSuccess(Attica::BaseJob *job) const;

    Attica::Provider::SortMode atticaSortMode(SortMode sortMode);

    EntryInternal entryFromAtticaContent(const Attica::Content &);

    // the attica categories we are interested in (e.g. Wallpaper, Application, Vocabulary File...)
    QMultiHash<QString, Attica::Category> mCategoryMap;

    Attica::ProviderManager m_providerManager;
    Attica::Provider m_provider;

    KNSCore::EntryInternal::List mCachedEntries;
    QHash<QString, Attica::Content> mCachedContent;

    // Associate job and entry, this is needed when fetching
    // download links or the account balance in order to continue
    // when the result is there.
    QHash<Attica::BaseJob *, QPair<EntryInternal, int>> mDownloadLinkJobs;

    // keep track of the current request
    QPointer<Attica::BaseJob> mEntryJob;
    Provider::SearchRequest mCurrentRequest;

    QSet<Attica::BaseJob *> m_updateJobs;

    bool mInitialized;
    QString m_providerId;

    Q_DISABLE_COPY(AtticaProvider)
};

}

#endif
