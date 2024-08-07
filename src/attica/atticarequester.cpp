// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "atticarequester_p.h"

#include "commentsmodel.h"
#include "entry_p.h"
#include "question.h"
#include "tagsfilterchecker.h"

#include <KFormat>
#include <KLocalizedString>
#include <QCollator>
#include <QDomDocument>
#include <knewstuffcore_debug.h>

#include <attica/accountbalance.h>
#include <attica/config.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <attica/listjob.h>
#include <attica/person.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "atticaprovider_p.h"

using namespace Attica;

namespace
{
Attica::Provider::SortMode atticaSortMode(KNSCore::Provider::SortMode sortMode)
{
    switch (sortMode) {
    case KNSCore::Provider::SortMode::Newest:
        return Attica::Provider::Newest;
    case KNSCore::Provider::SortMode::Alphabetical:
        return Attica::Provider::Alphabetical;
    case KNSCore::Provider::SortMode::Downloads:
        return Attica::Provider::Downloads;
    case KNSCore::Provider::SortMode::Rating:
        return Attica::Provider::Rating;
    }
    qWarning() << "Unmapped sortMode" << sortMode;
    return Attica::Provider::Rating;
}
} // namespace

namespace KNSCore
{

AtticaRequester::AtticaRequester(const KNSCore::Provider::SearchRequest &request, AtticaProvider *provider, QObject *parent)
    : QObject(parent)
    , m_request(request)
    , m_provider(provider)
{
}

void AtticaRequester::detailsLoaded(BaseJob *job)
{
    if (m_provider->jobSuccess(job)) {
        auto *contentJob = dynamic_cast<ItemJob<Content> *>(job);
        Content content = contentJob->result();
        auto entry = entryFromAtticaContent(content);
        entry.setEntryRequestedId(job->property("providedEntryId").toString()); // The ResultsStream should still known that this entry was for its query
        Q_EMIT entryDetailsLoaded(entry);
        qCDebug(KNEWSTUFFCORE) << "check update finished: " << entry.name();
    }

    if (m_updateJobs.remove(job) && m_updateJobs.isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "check update finished.";
        QList<Entry> updatable;
        for (const Entry &entry : std::as_const(m_provider->mCachedEntries)) {
            if (entry.status() == KNSCore::Entry::Updateable) {
                updatable.append(entry);
            }
        }
        qDebug() << "UPDATABLE" << updatable;
        Q_EMIT loadingFinished(updatable);
    }
}

void AtticaRequester::checkForUpdates()
{
    if (m_provider->mCachedEntries.isEmpty()) {
        Q_EMIT loadingFinished({});
        return;
    }

    for (const Entry &entry : std::as_const(m_provider->mCachedEntries)) {
        ItemJob<Content> *job = m_provider->m_provider.requestContent(entry.uniqueId());
        connect(job, &BaseJob::finished, this, &AtticaRequester::detailsLoaded);
        m_updateJobs.insert(job);
        job->start();
        qCDebug(KNEWSTUFFCORE) << "Checking for update: " << entry.name();
    }
}

Entry::List AtticaRequester::installedEntries() const
{
    Entry::List entries;
    for (const Entry &entry : std::as_const(m_provider->mCachedEntries)) {
        if (entry.status() == KNSCore::Entry::Installed || entry.status() == KNSCore::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

void AtticaRequester::start()
{
    QMetaObject::invokeMethod(this, &AtticaRequester::startInternal, Qt::QueuedConnection);
}

void AtticaRequester::categoryContentsLoaded(BaseJob *job)
{
    if (!m_provider->jobSuccess(job)) {
        return;
    }

    auto *listJob = dynamic_cast<ListJob<Content> *>(job);
    const Content::List contents = listJob->itemList();

    Entry::List entries;
    TagsFilterChecker checker(m_provider->tagFilter());
    TagsFilterChecker downloadschecker(m_provider->downloadTagFilter());
    for (const Content &content : contents) {
        if (!content.isValid()) {
            qCDebug(KNEWSTUFFCORE)
                << "Filtered out an invalid entry. This suggests something is not right on the originating server. Please contact the administrators of"
                << m_provider->name() << "and inform them there is an issue with content in the category or categories" << m_request.categories;
            continue;
        }
        if (checker.filterAccepts(content.tags())) {
            bool filterAcceptsDownloads = true;
            if (content.downloads() > 0) {
                filterAcceptsDownloads = false;
                const QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
                for (const Attica::DownloadDescription &dli : descs) {
                    if (downloadschecker.filterAccepts(dli.tags())) {
                        filterAcceptsDownloads = true;
                        break;
                    }
                }
            }
            if (filterAcceptsDownloads) {
                m_provider->mCachedContent.insert(content.id(), content);
                entries.append(entryFromAtticaContent(content));
            } else {
                qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << content.name() << "on download filter" << m_provider->downloadTagFilter();
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << content.name() << "on entry filter" << m_provider->tagFilter();
        }
    }

    qCDebug(KNEWSTUFFCORE) << "loaded: " << m_request.hashForRequest() << " count: " << entries.size();
    Q_EMIT loadingFinished(entries);
}

void AtticaRequester::startInternal()
{
    switch (m_request.filter) {
    case KNSCore::Provider::None:
        break;
    case KNSCore::Provider::ExactEntryId: {
        ItemJob<Content> *job = m_provider->m_provider.requestContent(m_request.searchTerm);
        job->setProperty("providedEntryId", m_request.searchTerm);
        connect(job, &BaseJob::finished, this, &AtticaRequester::detailsLoaded);
        job->start();
        return;
    }
    case KNSCore::Provider::Installed:
        if (m_request.page == 0) {
            Q_EMIT loadingFinished(installedEntries());
        } else {
            Q_EMIT loadingFinished({});
        }
        return;
    case KNSCore::Provider::Updates:
        checkForUpdates();
        return;
    }

    Attica::Provider::SortMode sorting = atticaSortMode(m_request.sortMode);
    Attica::Category::List categoriesToSearch;

    if (m_request.categories.isEmpty()) {
        // search in all categories
        categoriesToSearch = m_provider->mCategoryMap.values();
    } else {
        categoriesToSearch.reserve(m_request.categories.size());
        for (const QString &categoryName : std::as_const(m_request.categories)) {
            categoriesToSearch.append(m_provider->mCategoryMap.values(categoryName));
        }
    }

    ListJob<Content> *job = m_provider->m_provider.searchContents(categoriesToSearch, m_request.searchTerm, sorting, m_request.page, m_request.pageSize);
    job->setProperty("searchRequest", QVariant::fromValue(m_request));
    connect(job, &BaseJob::finished, this, &AtticaRequester::categoryContentsLoaded);
    job->start();
}

Entry AtticaRequester::entryFromAtticaContent(const Attica::Content &content)
{
    Entry entry;

    entry.setProviderId(m_provider->id());
    entry.setUniqueId(content.id());
    entry.setStatus(KNSCore::Entry::Downloadable);
    entry.setVersion(content.version());
    entry.setReleaseDate(content.updated().date());
    entry.setCategory(content.attribute(QStringLiteral("typeid")));

    qDebug() << "looking for cache entry";
    auto index = m_provider->mCachedEntries.indexOf(entry);
    qDebug() << "looking for cache entry" << index;
    if (index >= 0) {
        Entry &cacheEntry = m_provider->mCachedEntries[index];
        qDebug() << "cache entry" << cacheEntry << cacheEntry.version() << entry.version();
        // check if updateable
        if (((cacheEntry.status() == KNSCore::Entry::Installed) || (cacheEntry.status() == KNSCore::Entry::Updateable))
            && ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
            cacheEntry.setStatus(KNSCore::Entry::Updateable);
            cacheEntry.setUpdateVersion(entry.version());
            cacheEntry.setUpdateReleaseDate(entry.releaseDate());
        }
        entry = cacheEntry;
    } else {
        m_provider->mCachedEntries.append(entry);
    }

    entry.setName(content.name());
    entry.setHomepage(content.detailpage());
    entry.setRating(content.rating());
    entry.setNumberOfComments(content.numberOfComments());
    entry.setDownloadCount(content.downloads());
    entry.setNumberFans(content.attribute(QStringLiteral("fans")).toInt());
    entry.setDonationLink(content.attribute(QStringLiteral("donationpage")));
    entry.setKnowledgebaseLink(content.attribute(QStringLiteral("knowledgebasepage")));
    entry.setNumberKnowledgebaseEntries(content.attribute(QStringLiteral("knowledgebaseentries")).toInt());
    entry.setHomepage(content.detailpage());

    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("1")), Entry::PreviewSmall1);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("2")), Entry::PreviewSmall2);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("3")), Entry::PreviewSmall3);

    entry.setPreviewUrl(content.previewPicture(QStringLiteral("1")), Entry::PreviewBig1);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("2")), Entry::PreviewBig2);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("3")), Entry::PreviewBig3);

    entry.setLicense(content.license());
    Author author;
    author.setId(content.author());
    author.setName(content.author());
    author.setHomepage(content.attribute(QStringLiteral("profilepage")));
    entry.setAuthor(author);

    entry.setSource(Entry::Online);
    entry.setSummary(content.description());
    entry.setShortSummary(content.summary());
    entry.setChangelog(content.changelog());
    entry.setTags(content.tags());

    const QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
    entry.d->mDownloadLinkInformationList.clear();
    entry.d->mDownloadLinkInformationList.reserve(descs.size());
    for (const Attica::DownloadDescription &desc : descs) {
        entry.d->mDownloadLinkInformationList.append({.name = desc.name(),
                                                      .priceAmount = desc.priceAmount(),
                                                      .distributionType = desc.distributionType(),
                                                      .descriptionLink = desc.link(),
                                                      .id = desc.id(),
                                                      .isDownloadtypeLink = desc.type() == Attica::DownloadDescription::LinkDownload,
                                                      .size = desc.size(),
                                                      .tags = desc.tags(),
                                                      .version = desc.version()});
    }

    return entry;
}

[[nodiscard]] KNSCore::Provider::SearchRequest AtticaRequester::request() const
{
    return m_request;
}

} // namespace KNSCore
