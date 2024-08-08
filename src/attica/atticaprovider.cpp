/*
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "atticaprovider_p.h"

#include "commentsmodel.h"
#include "entry_p.h"
#include "question.h"
#include "tagsfilterchecker.h"

#include <KFormat>
#include <KLocalizedString>
#include <QCollator>
#include <QDomDocument>
#include <QTimer>
#include <knewstuffcore_debug.h>

#include <attica/accountbalance.h>
#include <attica/config.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <attica/listjob.h>
#include <attica/person.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "atticarequester_p.h"
#include "categorymetadata.h"
#include "categorymetadata_p.h"

using namespace Attica;

namespace KNSCore
{
AtticaProvider::AtticaProvider(const QStringList &categories, const QString &additionalAgentInformation)
    : mInitialized(false)
{
    // init categories map with invalid categories
    for (const QString &category : categories) {
        mCategoryMap.insert(category, Attica::Category());
    }

    connect(&m_providerManager, &ProviderManager::providerAdded, this, [this, additionalAgentInformation](const Attica::Provider &provider) {
        providerLoaded(provider);
        m_provider.setAdditionalAgentInformation(additionalAgentInformation);
    });
    connect(&m_providerManager, &ProviderManager::authenticationCredentialsMissing, this, &AtticaProvider::onAuthenticationCredentialsMissing);
}

AtticaProvider::AtticaProvider(const Attica::Provider &provider, const QStringList &categories, const QString &additionalAgentInformation)
    : mInitialized(false)
{
    // init categories map with invalid categories
    for (const QString &category : categories) {
        mCategoryMap.insert(category, Attica::Category());
    }
    providerLoaded(provider);
    m_provider.setAdditionalAgentInformation(additionalAgentInformation);
}

QString AtticaProvider::id() const
{
    return m_providerId;
}

void AtticaProvider::onAuthenticationCredentialsMissing(const Attica::Provider &)
{
    qCDebug(KNEWSTUFFCORE) << "Authentication missing!";
    // FIXME Show authentication dialog
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
        qCDebug(KNEWSTUFFCORE) << "base url of attica provider:" << m_providerManager.providers().constLast().baseUrl().toString();
    } else {
        qCCritical(KNEWSTUFFCORE) << "Could not load provider.";
        return false;
    }
    return true;
}

void AtticaProvider::setCachedEntries(const KNSCore::Entry::List &cachedEntries)
{
    mCachedEntries = cachedEntries;
}

void AtticaProvider::providerLoaded(const Attica::Provider &provider)
{
    m_name = provider.name();
    m_icon = provider.icon();
    qCDebug(KNEWSTUFFCORE) << "Added provider: " << provider.name();

    m_provider = provider;
    m_provider.setAdditionalAgentInformation(name());
    m_providerId = provider.baseUrl().host();

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

    auto *job = static_cast<Attica::ListJob<Attica::Category> *>(listJob);
    const Category::List categoryList = job->itemList();

    QList<CategoryMetadata> categoryMetadataList;
    for (const Category &category : categoryList) {
        if (mCategoryMap.contains(category.name())) {
            qCDebug(KNEWSTUFFCORE) << "Adding category: " << category.name() << category.displayName();
            // If there is only the placeholder category, replace it
            if (mCategoryMap.contains(category.name()) && !mCategoryMap.value(category.name()).isValid()) {
                mCategoryMap.replace(category.name(), category);
            } else {
                mCategoryMap.insert(category.name(), category);
            }

            categoryMetadataList << CategoryMetadata(new CategoryMetadataPrivate{
                .id = category.id(),
                .name = category.name(),
                .displayName = category.displayName(),
            });
        }
    }
    std::sort(categoryMetadataList.begin(), categoryMetadataList.end(), [](const auto &i, const auto &j) -> bool {
        const QString a(i.displayName().isEmpty() ? i.name() : i.displayName());
        const QString b(j.displayName().isEmpty() ? j.name() : j.displayName());

        return (QCollator().compare(a, b) < 0);
    });

    bool correct = false;
    for (auto it = mCategoryMap.cbegin(), itEnd = mCategoryMap.cend(); it != itEnd; ++it) {
        if (!it.value().isValid()) {
            qCWarning(KNEWSTUFFCORE) << "Could not find category" << it.key();
        } else {
            correct = true;
        }
    }

    if (correct) {
        mInitialized = true;
        Q_EMIT providerInitialized(this);
        Q_EMIT categoriesMetadataLoaded(categoryMetadataList);
    } else {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("All categories are missing"), QVariant());
    }
}

bool AtticaProvider::isInitialized() const
{
    return mInitialized;
}

void AtticaProvider::loadEntries(const KNSCore::SearchRequest &request)
{
    auto requester = new AtticaRequester(request, this, this);
    connect(requester, &AtticaRequester::entryDetailsLoaded, this, &AtticaProvider::entryDetailsLoaded);
    connect(requester, &AtticaRequester::loadingFinished, this, [this, requester](const KNSCore::Entry::List &list) {
        Q_EMIT loadingFinished(requester->request(), list);
    });
    connect(requester, &AtticaRequester::loadingFailed, this, [this, requester] {
        Q_EMIT loadingFailed(requester->request());
    });
    requester->start();
}

void AtticaProvider::loadEntryDetails(const KNSCore::Entry &entry)
{
    ItemJob<Content> *job = m_provider.requestContent(entry.uniqueId());
    connect(job, &BaseJob::finished, this, [this, entry] {
        Q_EMIT entryDetailsLoaded(entry);
    });
    job->start();
}

void AtticaProvider::loadPayloadLink(const KNSCore::Entry &entry, int linkId)
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

void AtticaProvider::loadComments(const Entry &entry, int commentsPerPage, int page)
{
    ListJob<Attica::Comment> *job = m_provider.requestComments(Attica::Comment::ContentComment, entry.uniqueId(), QStringLiteral("0"), page, commentsPerPage);
    connect(job, &BaseJob::finished, this, &AtticaProvider::loadedComments);
    job->start();
}

QList<std::shared_ptr<KNSCore::Comment>> getCommentsList(const Attica::Comment::List &comments, std::shared_ptr<KNSCore::Comment> parent)
{
    QList<std::shared_ptr<KNSCore::Comment>> knsComments;
    for (const Attica::Comment &comment : comments) {
        qCDebug(KNEWSTUFFCORE) << "Appending comment with id" << comment.id() << ", which has" << comment.childCount() << "children";
        auto knsComment = std::make_shared<KNSCore::Comment>();
        knsComment->id = comment.id();
        knsComment->subject = comment.subject();
        knsComment->text = comment.text();
        knsComment->childCount = comment.childCount();
        knsComment->username = comment.user();
        knsComment->date = comment.date();
        knsComment->score = comment.score();
        knsComment->parent = parent;
        knsComments << knsComment;
        if (comment.childCount() > 0) {
            qCDebug(KNEWSTUFFCORE) << "Getting more comments, as this one has children, and we currently have this number of comments:" << knsComments.count();
            knsComments << getCommentsList(comment.children(), knsComment);
            qCDebug(KNEWSTUFFCORE) << "After getting the children, we now have the following number of comments:" << knsComments.count();
        }
    }
    return knsComments;
}

void AtticaProvider::loadedComments(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ListJob<Attica::Comment> *>(baseJob);
    Attica::Comment::List comments = job->itemList();

    QList<std::shared_ptr<KNSCore::Comment>> receivedComments = getCommentsList(comments, nullptr);
    Q_EMIT commentsLoaded(receivedComments);
}

void AtticaProvider::loadPerson(const QString &username)
{
    if (m_provider.hasPersonService()) {
        ItemJob<Attica::Person> *job = m_provider.requestPerson(username);
        job->setProperty("username", username);
        connect(job, &BaseJob::finished, this, &AtticaProvider::loadedPerson);
        job->start();
    }
}

void AtticaProvider::loadedPerson(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<Attica::Person> *>(baseJob);
    Attica::Person person = job->result();

    auto author = std::make_shared<KNSCore::Author>();
    // This is a touch hack-like, but it ensures we actually have the data in case it is not returned by the server
    author->setId(job->property("username").toString());
    author->setName(QStringLiteral("%1 %2").arg(person.firstName(), person.lastName()).trimmed());
    author->setHomepage(person.homepage());
    author->setProfilepage(person.extendedAttribute(QStringLiteral("profilepage")));
    author->setAvatarUrl(person.avatarUrl());
    author->setDescription(person.extendedAttribute(QStringLiteral("description")));
    Q_EMIT personLoaded(author);
}

void AtticaProvider::loadedConfig(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = dynamic_cast<ItemJob<Attica::Config> *>(baseJob);
    Attica::Config config = job->result();
    m_version = config.version();
    m_supportsSsl = config.ssl();
    m_contactEmail = config.contact();
    const auto protocol = [&config] {
        QString protocol{QStringLiteral("http")};
        if (config.ssl()) {
            protocol = QStringLiteral("https");
        }
        return protocol;
    }();
    m_website = [&config, &protocol] {
        // There is usually no protocol in the website and host, but in case
        // there is, trust what's there
        if (config.website().contains(QLatin1String("://"))) {
            return QUrl(config.website());
        }
        return QUrl(QLatin1String("%1://%2").arg(protocol).arg(config.website()));
    }();
    m_host = [&config, &protocol] {
        if (config.host().contains(QLatin1String("://"))) {
            return QUrl(config.host());
        }
        return QUrl(QLatin1String("%1://%2").arg(protocol).arg(config.host()));
    }();

    Q_EMIT basicsLoaded();
}

void AtticaProvider::accountBalanceLoaded(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<AccountBalance> *>(baseJob);
    AccountBalance item = job->result();

    QPair<Entry, int> pair = mDownloadLinkJobs.take(job);
    Entry entry(pair.first);
    Content content = mCachedContent.value(entry.uniqueId());
    if (content.downloadUrlDescription(pair.second).priceAmount() < item.balance()) {
        qCDebug(KNEWSTUFFCORE) << "Your balance is greater than the price." << content.downloadUrlDescription(pair.second).priceAmount()
                               << " balance: " << item.balance();
        Question question;
        question.setEntry(entry);
        question.setQuestion(i18nc("the price of a download item, parameter 1 is the currency, 2 is the price",
                                   "This item costs %1 %2.\nDo you want to buy it?",
                                   item.currency(),
                                   content.downloadUrlDescription(pair.second).priceAmount()));
        if (question.ask() == Question::YesResponse) {
            ItemJob<DownloadItem> *job = m_provider.downloadLink(entry.uniqueId(), QString::number(pair.second));
            connect(job, &BaseJob::finished, this, &AtticaProvider::downloadItemLoaded);
            mDownloadLinkJobs[job] = qMakePair(entry, pair.second);
            job->start();
        } else {
            return;
        }
    } else {
        qCDebug(KNEWSTUFFCORE) << "You don't have enough money on your account!" << content.downloadUrlDescription(0).priceAmount()
                               << " balance: " << item.balance();
        Q_EMIT signalInformation(i18n("Your account balance is too low:\nYour balance: %1\nPrice: %2", //
                                      item.balance(),
                                      content.downloadUrlDescription(0).priceAmount()));
    }
}

void AtticaProvider::downloadItemLoaded(BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<DownloadItem> *>(baseJob);
    DownloadItem item = job->result();

    Entry entry = mDownloadLinkJobs.take(job).first;
    entry.setPayload(QString(item.url().toString()));
    Q_EMIT payloadLinkLoaded(entry);
}

void AtticaProvider::vote(const Entry &entry, uint rating)
{
    PostJob *job = m_provider.voteForContent(entry.uniqueId(), rating);
    connect(job, &BaseJob::finished, this, &AtticaProvider::votingFinished);
    job->start();
}

void AtticaProvider::votingFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    Q_EMIT signalInformation(i18nc("voting for an item (good/bad)", "Your vote was recorded."));
}

void AtticaProvider::becomeFan(const Entry &entry)
{
    PostJob *job = m_provider.becomeFan(entry.uniqueId());
    connect(job, &BaseJob::finished, this, &AtticaProvider::becomeFanFinished);
    job->start();
}

void AtticaProvider::becomeFanFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    Q_EMIT signalInformation(i18n("You are now a fan."));
}

bool AtticaProvider::jobSuccess(Attica::BaseJob *job)
{
    if (job->metadata().error() == Attica::Metadata::NoError) {
        return true;
    }
    qCDebug(KNEWSTUFFCORE) << "job error: " << job->metadata().error() << " status code: " << job->metadata().statusCode() << job->metadata().message();

    if (job->metadata().error() == Attica::Metadata::NetworkError) {
        if (job->metadata().statusCode() == 503) {
            QDateTime retryAfter;
            static const QByteArray retryAfterKey{"Retry-After"};
            for (const QNetworkReply::RawHeaderPair &headerPair : job->metadata().headers()) {
                if (headerPair.first == retryAfterKey) {
                    // Retry-After is not a known header, so we need to do a bit of running around to make that work
                    // Also, the fromHttpDate function is in the private qnetworkrequest header, so we can't use that
                    // So, simple workaround, just pass it through a dummy request and get a formatted date out (the
                    // cost is sufficiently low here, given we've just done a bunch of i/o heavy things, so...)
                    QNetworkRequest dummyRequest;
                    dummyRequest.setRawHeader(QByteArray{"Last-Modified"}, headerPair.second);
                    retryAfter = dummyRequest.header(QNetworkRequest::LastModifiedHeader).toDateTime();
                    break;
                }
            }
            static const KFormat formatter;
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::TryAgainLaterError,
                                   i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                        formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                   {retryAfter});
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::NetworkError,
                                   i18n("Network error %1: %2", job->metadata().statusCode(), job->metadata().statusString()),
                                   job->metadata().statusCode());
        }
    }
    if (job->metadata().error() == Attica::Metadata::OcsError) {
        if (job->metadata().statusCode() == 200) {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("Too many requests to server. Please try again in a few minutes."),
                                   job->metadata().statusCode());
        } else if (job->metadata().statusCode() == 405) {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("The Open Collaboration Services instance %1 does not support the attempted function.", name()),
                                   job->metadata().statusCode());
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("Unknown Open Collaboration Service API error. (%1)", job->metadata().statusCode()),
                                   job->metadata().statusCode());
        }
    }

    if (auto searchRequestVar = job->property("searchRequest"); searchRequestVar.isValid()) {
        auto req = searchRequestVar.value<SearchRequest>();
        Q_EMIT loadingFailed(req);
    }
    return false;
}

void AtticaProvider::updateOnFirstBasicsGet()
{
    if (!m_basicsGot) {
        m_basicsGot = true;
        QTimer::singleShot(0, this, [this] {
            Attica::ItemJob<Attica::Config> *configJob = m_provider.requestConfig();
            connect(configJob, &BaseJob::finished, this, &AtticaProvider::loadedConfig);
            configJob->start();
        });
    }
};

QString AtticaProvider::name() const
{
    return m_name;
}

QUrl AtticaProvider::icon() const
{
    return m_icon;
}

QString AtticaProvider::version()
{
    updateOnFirstBasicsGet();
    return m_version;
}

QUrl AtticaProvider::website()
{
    updateOnFirstBasicsGet();
    return m_website;
}

QUrl AtticaProvider::host()
{
    updateOnFirstBasicsGet();
    return m_host;
}

QString AtticaProvider::contactEmail()
{
    updateOnFirstBasicsGet();
    return m_contactEmail;
}

bool AtticaProvider::supportsSsl()
{
    updateOnFirstBasicsGet();
    return m_supportsSsl;
}

} // namespace KNSCore

#include "moc_atticaprovider_p.cpp"
