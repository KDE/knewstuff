/*
    Copyright (c) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

#include "atticaprovider_p.h"

#include "question.h"

#include <knewstuffcore_debug.h>
#include <klocalizedstring.h>

#include <attica/providermanager.h>
#include <attica/provider.h>
#include <attica/listjob.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <attica/accountbalance.h>
#include <attica/person.h>

using namespace Attica;

namespace KNSCore
{

AtticaProvider::AtticaProvider(const QStringList &categories)
    : mEntryJob(0)
    , mInitialized(false)
{
    // init categories map with invalid categories
    foreach (const QString &category, categories) {
        mCategoryMap.insert(category, Attica::Category());
    }

    connect(&m_providerManager, &ProviderManager::providerAdded, this, &AtticaProvider::providerLoaded);
    connect(&m_providerManager, SIGNAL(authenticationCredentialsMissing(Provider)),
            SLOT(authenticationCredentialsMissing(Provider)));
}

AtticaProvider::AtticaProvider(const Attica::Provider &provider, const QStringList &categories)
    : mEntryJob(0)
    , mInitialized(false)
{
    // init categories map with invalid categories
    foreach (const QString &category, categories) {
        mCategoryMap.insert(category, Attica::Category());
    }
    providerLoaded(provider);
}

QString AtticaProvider::id() const
{
    return m_provider.baseUrl().toString();
}

void AtticaProvider::authenticationCredentialsMissing(const KNSCore::Provider &)
{
    qCDebug(KNEWSTUFFCORE) << "Authentication missing!";
    // FIXME Show autentication dialog
}

bool AtticaProvider::setProviderXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("provider")) {
        return false;
    }

    // FIXME this is quite ugly, repackaging the xml into a string
    QDomDocument doc(QStringLiteral("temp"));
    qCDebug(KNEWSTUFFCORE) << "setting provider xml" << doc.toString();

    doc.appendChild(xmldata.cloneNode(true));
    m_providerManager.addProviderFromXml(doc.toString());

    if (!m_providerManager.providers().isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "base url of attica provider:" << m_providerManager.providers().last().baseUrl().toString();
    } else {
        qCritical() << "Could not load provider.";
        return false;
    }
    return true;
}

void AtticaProvider::setCachedEntries(const KNSCore::EntryInternal::List &cachedEntries)
{
    mCachedEntries = cachedEntries;
}

void AtticaProvider::providerLoaded(const Attica::Provider &provider)
{
    mName = provider.name();
    qCDebug(KNEWSTUFFCORE) << "Added provider: " << provider.name();

    m_provider = provider;

    Attica::ListJob<Attica::Category> *job = m_provider.requestCategories();
    connect(job, &BaseJob::finished, this, &AtticaProvider::listOfCategoriesLoaded);
    job->start();
}

void AtticaProvider::listOfCategoriesLoaded(Attica::BaseJob *listJob)
{
    if (!jobSuccess(listJob)) {
        return;
    }

    qCDebug(KNEWSTUFFCORE) << "loading categories: " << mCategoryMap.keys();

    Attica::ListJob<Attica::Category> *job = static_cast<Attica::ListJob<Attica::Category>*>(listJob);
    Category::List categoryList = job->itemList();

    foreach (const Category &category, categoryList) {
        if (mCategoryMap.contains(category.name())) {
            qCDebug(KNEWSTUFFCORE) << "Adding category: " << category.name();
            mCategoryMap[category.name()] = category;
        }
    }

    bool correct = false;
    for(auto it = mCategoryMap.cbegin(), itEnd = mCategoryMap.cend(); it!=itEnd; ++it) {
        if (!it.value().isValid()) {
            qCWarning(KNEWSTUFFCORE) << "Could not find category" << it.key();
        } else {
            correct = true;
        }
    }

    if (correct) {
        mInitialized = true;
        emit providerInitialized(this);
    } else {
        emit signalError(i18n("All categories are missing"));
    }
}

bool AtticaProvider::isInitialized() const
{
    return mInitialized;
}

void AtticaProvider::loadEntries(const KNSCore::Provider::SearchRequest &request)
{
    if (mEntryJob) {
        mEntryJob->abort();
        mEntryJob = 0;
    }

    mCurrentRequest = request;
    switch (request.filter) {
        case None:
            break;
        case ExactEntryId: {
            ItemJob<Content> *job = m_provider.requestContent(request.searchTerm);
            connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
            job->start();
            return;
        }
        case Installed:
            if (request.page == 0) {
                emit loadingFinished(request, installedEntries());
            } else {
                emit loadingFinished(request, EntryInternal::List());
            }
            return;
        case Updates:
            checkForUpdates();
            return;
    }

    Attica::Provider::SortMode sorting = atticaSortMode(request.sortMode);
    Attica::Category::List categoriesToSearch;

    if (request.categories.isEmpty()) {
        // search in all categories
        categoriesToSearch = mCategoryMap.values();
    } else {
        categoriesToSearch.reserve(request.categories.size());
        foreach (const QString &categoryName, request.categories) {
            categoriesToSearch.append(mCategoryMap.value(categoryName));
        }
    }

    ListJob<Content> *job = m_provider.searchContents(categoriesToSearch, request.searchTerm, sorting, request.page, request.pageSize);
    connect(job, &BaseJob::finished, this, &AtticaProvider::categoryContentsLoaded);

    mEntryJob = job;
    job->start();
}

void AtticaProvider::checkForUpdates()
{
    foreach (const EntryInternal &e, mCachedEntries) {
        ItemJob<Content> *job = m_provider.requestContent(e.uniqueId());
        connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
        m_updateJobs.insert(job);
        job->start();
        qCDebug(KNEWSTUFFCORE) << "Checking for update: " << e.name();
    }
}

void AtticaProvider::loadEntryDetails(const KNSCore::EntryInternal &entry)
{
    ItemJob<Content> *job = m_provider.requestContent(entry.uniqueId());
    connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
    job->start();
}

void AtticaProvider::detailsLoaded(BaseJob *job)
{
    if (jobSuccess(job)) {
        ItemJob<Content> *contentJob = static_cast<ItemJob<Content>*>(job);
        Content content = contentJob->result();
        EntryInternal entry = entryFromAtticaContent(content);
        emit entryDetailsLoaded(entry);
        qCDebug(KNEWSTUFFCORE) << "check update finished: " << entry.name();
    }

    if (m_updateJobs.remove(job) && m_updateJobs.isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "check update finished.";
        QList<EntryInternal> updatable;
        foreach (const EntryInternal &entry, mCachedEntries) {
            if (entry.status() == KNS3::Entry::Updateable) {
                updatable.append(entry);
            }
        }
        emit loadingFinished(mCurrentRequest, updatable);
    }
}

void AtticaProvider::categoryContentsLoaded(BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }

    ListJob<Content> *listJob = static_cast<ListJob<Content>*>(job);
    Content::List contents = listJob->itemList();

    EntryInternal::List entries;
    Q_FOREACH (const Content &content, contents) {
        mCachedContent.insert(content.id(), content);
        entries.append(entryFromAtticaContent(content));
    }

    qCDebug(KNEWSTUFFCORE) << "loaded: " << mCurrentRequest.hashForRequest() << " count: " << entries.size();
    emit loadingFinished(mCurrentRequest, entries);
    mEntryJob = 0;
}

Attica::Provider::SortMode AtticaProvider::atticaSortMode(const SortMode &sortMode)
{
    switch(sortMode) {
        case Newest:
            return Attica::Provider::Newest;
        case Alphabetical:
            return Attica::Provider::Alphabetical;
        case Downloads:
            return Attica::Provider::Downloads;
        default:
            return Attica::Provider::Rating;
    }
}

void AtticaProvider::loadPayloadLink(const KNSCore::EntryInternal &entry, int linkId)
{
    Attica::Content content = mCachedContent.value(entry.uniqueId());
    const DownloadDescription desc = content.downloadUrlDescription(linkId);

    if (desc.hasPrice()) {
        // Ask for balance, then show information...
        ItemJob<AccountBalance> *job = m_provider.requestAccountBalance();
        connect(job, &BaseJob::finished, this, &AtticaProvider::accountBalanceLoaded);
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        qCDebug(KNEWSTUFFCORE) << "get account balance";
    } else {
        ItemJob<DownloadItem> *job = m_provider.downloadLink(entry.uniqueId(), QString::number(linkId));
        connect(job, &BaseJob::finished, this, &AtticaProvider::downloadItemLoaded);
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        qCDebug(KNEWSTUFFCORE) << " link for " << entry.uniqueId();
    }
}

void AtticaProvider::accountBalanceLoaded(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    ItemJob<AccountBalance> *job = static_cast<ItemJob<AccountBalance>*>(baseJob);
    AccountBalance item = job->result();

    QPair<EntryInternal, int> pair = mDownloadLinkJobs.take(job);
    EntryInternal entry(pair.first);
    Content content = mCachedContent.value(entry.uniqueId());
    if (content.downloadUrlDescription(pair.second).priceAmount() < item.balance()) {
        qCDebug(KNEWSTUFFCORE) << "Your balance is greater than the price."
                   << content.downloadUrlDescription(pair.second).priceAmount() << " balance: " << item.balance();
        Question question;
        question.setQuestion(i18nc("the price of a download item, parameter 1 is the currency, 2 is the price",
                "This item costs %1 %2.\nDo you want to buy it?",
                item.currency(), content.downloadUrlDescription(pair.second).priceAmount()
            ));
        if(question.ask() == Question::YesResponse) {
            ItemJob<DownloadItem> *job = m_provider.downloadLink(entry.uniqueId(), QString::number(pair.second));
            connect(job, &BaseJob::finished, this, &AtticaProvider::downloadItemLoaded);
            connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
            mDownloadLinkJobs[job] = qMakePair(entry, pair.second);
            job->start();
        } else {
            return;
        }
    } else {
        qCDebug(KNEWSTUFFCORE) << "You don't have enough money on your account!"
               << content.downloadUrlDescription(0).priceAmount() << " balance: " << item.balance();
        emit signalInformation(i18n("Your account balance is too low:\nYour balance: %1\nPrice: %2",
                                     item.balance(), content.downloadUrlDescription(0).priceAmount()));
    }
}

void AtticaProvider::downloadItemLoaded(BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    ItemJob<DownloadItem> *job = static_cast<ItemJob<DownloadItem>*>(baseJob);
    DownloadItem item = job->result();

    EntryInternal entry = mDownloadLinkJobs.take(job).first;
    entry.setPayload(QString(item.url().toString()));
    emit payloadLinkLoaded(entry);
}

EntryInternal::List AtticaProvider::installedEntries() const
{
    EntryInternal::List entries;
    foreach (const EntryInternal &entry, mCachedEntries) {
        if (entry.status() == KNS3::Entry::Installed || entry.status() == KNS3::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

void AtticaProvider::vote(const EntryInternal &entry, uint rating)
{
    PostJob *job = m_provider.voteForContent(entry.uniqueId(), rating);
    connect(job, &BaseJob::finished, this, &AtticaProvider::votingFinished);
    connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
    job->start();
}

void AtticaProvider::votingFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    emit signalInformation(i18nc("voting for an item (good/bad)", "Your vote was recorded."));
}

void AtticaProvider::becomeFan(const EntryInternal &entry)
{
    PostJob *job = m_provider.becomeFan(entry.uniqueId());
    connect(job, &BaseJob::finished, this, &AtticaProvider::becomeFanFinished);
    connect(job, SIGNAL(jobStarted(QNetworkReply*)), SLOT(atticaJobStarted(QNetworkReply*)));
    job->start();
}

void AtticaProvider::becomeFanFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    emit signalInformation(i18n("You are now a fan."));
}

bool AtticaProvider::jobSuccess(Attica::BaseJob *job) const
{
    if (job->metadata().error() == Attica::Metadata::NoError) {
        return true;
    }
    qCDebug(KNEWSTUFFCORE) << "job error: " << job->metadata().error() << " status code: " << job->metadata().statusCode() << job->metadata().message();

    if (job->metadata().error() == Attica::Metadata::NetworkError) {
        emit signalError(i18n("Network error. (%1)", job->metadata().statusCode()));
    }
    if (job->metadata().error() == Attica::Metadata::OcsError) {
        if (job->metadata().statusCode() == 200) {
            emit signalError(i18n("Too many requests to server. Please try again in a few minutes."));
        } else {
            emit signalError(i18n("Unknown Open Collaboration Service API error. (%1)", job->metadata().statusCode()));
        }
    }
    return false;
}

EntryInternal AtticaProvider::entryFromAtticaContent(const Attica::Content &content)
{
    EntryInternal entry;

    entry.setProviderId(id());
    entry.setUniqueId(content.id());
    entry.setStatus(KNS3::Entry::Downloadable);
    entry.setVersion(content.version());
    entry.setReleaseDate(content.updated().date());

    int index = mCachedEntries.indexOf(entry);
    if (index >= 0) {
        EntryInternal &cacheEntry = mCachedEntries[index];
        // check if updateable
        if (((cacheEntry.status() == KNS3::Entry::Installed) || (cacheEntry.status() == KNS3::Entry::Updateable)) &&
                ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
            cacheEntry.setStatus(KNS3::Entry::Updateable);
            cacheEntry.setUpdateVersion(entry.version());
            cacheEntry.setUpdateReleaseDate(entry.releaseDate());
        }
        entry = cacheEntry;
    } else {
        mCachedEntries.append(entry);
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

    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("1")), EntryInternal::PreviewSmall1);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("2")), EntryInternal::PreviewSmall2);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("3")), EntryInternal::PreviewSmall3);

    entry.setPreviewUrl(content.previewPicture(QStringLiteral("1")), EntryInternal::PreviewBig1);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("2")), EntryInternal::PreviewBig2);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("3")), EntryInternal::PreviewBig3);

    entry.setLicense(content.license());
    Author author;
    author.setName(content.author());
    author.setHomepage(content.attribute(QStringLiteral("profilepage")));
    entry.setAuthor(author);

    entry.setSource(EntryInternal::Online);
    entry.setSummary(content.description());
    entry.setShortSummary(content.summary());
    entry.setChangelog(content.changelog());

    entry.clearDownloadLinkInformation();
    QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
    foreach (const Attica::DownloadDescription &desc, descs) {
        EntryInternal::DownloadLinkInformation info;
        info.name = desc.name();
        info.priceAmount = desc.priceAmount();
        info.distributionType = desc.distributionType();
        info.descriptionLink = desc.link();
        info.id = desc.id();
        info.size = desc.size();
        info.isDownloadtypeLink = desc.type() == Attica::DownloadDescription::LinkDownload;
        entry.appendDownloadLinkInformation(info);
    }

    return entry;
}

} // namespace

