/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickitemsmodel.h"
#include "quickengine.h"
#include "knewstuffquick_debug.h"

#include "itemsmodel.h"
#include "engine.h"
#include "downloadlinkinfo.h"
#include "core/commentsmodel.h"

#include <KLocalizedString>
#include <KShell>
#include <QProcess>

class ItemsModel::Private {
public:
    Private(ItemsModel *qq)
        : q(qq)
        , model(nullptr)
        , engine(nullptr)
        , coreEngine(nullptr)
    {}
    ~Private()
    {
    }
    ItemsModel *q;
    KNSCore::ItemsModel *model;
    Engine *engine;
    KNSCore::Engine *coreEngine;

    QHash<QString, KNSCore::CommentsModel*> commentsModels;

    bool isLoadingData{false};

    bool initModel()
    {
        if (model) {
            return true;
        }
        if (!coreEngine) {
            return false;
        }
        model = new KNSCore::ItemsModel(coreEngine, q);

        q->connect(coreEngine, &KNSCore::Engine::signalBusy, q, [=](){ isLoadingData = true; emit q->isLoadingDataChanged(); });
        q->connect(coreEngine, &KNSCore::Engine::signalIdle, q, [=](){ isLoadingData = false; emit q->isLoadingDataChanged(); });

        q->connect(coreEngine, &KNSCore::Engine::signalProvidersLoaded, coreEngine, &KNSCore::Engine::reloadEntries);
        // Entries have been fetched and should be shown:
        q->connect(coreEngine, &KNSCore::Engine::signalEntriesLoaded, model, [this](const KNSCore::EntryInternal::List& entries){
            if (coreEngine->filter() != KNSCore::Provider::Updates) {
                model->slotEntriesLoaded(entries);
            }
        });
        q->connect(coreEngine, &KNSCore::Engine::signalUpdateableEntriesLoaded, model, [this](const KNSCore::EntryInternal::List& entries){
            if (coreEngine->filter() == KNSCore::Provider::Updates) {
                model->slotEntriesLoaded(entries);
            }
        });

        // An entry has changes - eg because it was installed
        q->connect(coreEngine, &KNSCore::Engine::signalEntryChanged, model, &KNSCore::ItemsModel::slotEntryChanged);
        q->connect(coreEngine, &KNSCore::Engine::signalEntryChanged, q, [=](const KNSCore::EntryInternal &entry){
            emit q->entryChanged(model->row(entry));
        });

        q->connect(coreEngine, &KNSCore::Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);
        q->connect(coreEngine, &KNSCore::Engine::signalEntryPreviewLoaded, model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

        q->connect(model, &KNSCore::ItemsModel::rowsInserted, q, &ItemsModel::rowsInserted);
        q->connect(model, &KNSCore::ItemsModel::rowsRemoved, q, &ItemsModel::rowsRemoved);
        q->connect(model, &KNSCore::ItemsModel::dataChanged, q, &ItemsModel::dataChanged);
        q->connect(model, &KNSCore::ItemsModel::modelReset, q, &ItemsModel::modelReset);
        return true;
    }
};

ItemsModel::ItemsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

ItemsModel::~ItemsModel()
{
    delete d;
}

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = QHash<int, QByteArray>{
        {Qt::DisplayRole, "display"},
        {NameRole, "name"},
        {UniqueIdRole, "uniqueId"},
        {CategoryRole, "category"},
        {HomepageRole, "homepage"},
        {AuthorRole, "author"},
        {LicenseRole, "license"},
        {ShortSummaryRole, "shortSummary"},
        {SummaryRole, "summary"},
        {ChangelogRole, "changelog"},
        {VersionRole, "version"},
        {ReleaseDateRole, "releaseDate"},
        {UpdateVersionRole, "updateVersion"},
        {UpdateReleaseDateRole, "updateReleaseDate"},
        {PayloadRole, "payload"},
        {Qt::DecorationRole, "decoration"},
        {PreviewsSmallRole, "previewsSmall"},
        {PreviewsRole, "previews"},
        {InstalledFilesRole, "installedFiles"},
        {UnInstalledFilesRole, "uninstalledFiles"},
        {RatingRole, "rating"},
        {NumberOfCommentsRole, "numberOfComments"},
        {DownloadCountRole, "downloadCount"},
        {NumberFansRole, "numberFans"},
        {NumberKnowledgebaseEntriesRole, "numberKnowledgebaseEntries"},
        {KnowledgebaseLinkRole, "knowledgebaseLink"},
        {DownloadLinksRole, "downloadLinks"},
        {DonationLinkRole, "donationLink"},
        {ProviderIdRole, "providerId"},
        {SourceRole, "source"},
        {StatusRole, "status"}
    };
    return roles;
}

int ItemsModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    if (d->initModel())
        return d->model->rowCount(QModelIndex());
    return 0;
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if (index.isValid() && d->initModel())
    {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index.row()), Qt::UserRole).value<KNSCore::EntryInternal>();
        switch (role)
        {
            case NameRole:
            case Qt::DisplayRole:
                data.setValue<QString>(entry.name());
                break;
            case UniqueIdRole:
                data.setValue<QString>(entry.uniqueId());
                break;
            case CategoryRole:
                data.setValue<QString>(entry.category());
                break;
            case HomepageRole:
                data.setValue<QUrl>(entry.homepage());
                break;
            case AuthorRole:
                {
                    KNSCore::Author author = entry.author();
                    QVariantMap returnAuthor;
                    returnAuthor[QStringLiteral("id")] = author.id();
                    returnAuthor[QStringLiteral("name")] = author.name();
                    returnAuthor[QStringLiteral("email")] = author.email();
                    returnAuthor[QStringLiteral("homepage")] = author.homepage();
                    returnAuthor[QStringLiteral("jabber")] = author.jabber();
                    returnAuthor[QStringLiteral("avatarUrl")] = author.avatarUrl();
                    returnAuthor[QStringLiteral("description")] = author.description();
                    data.setValue<>(returnAuthor);
                }
                break;
            case LicenseRole:
                data.setValue<QString>(entry.license());
                break;
            case ShortSummaryRole:
                data.setValue<QString>(entry.shortSummary());
                break;
            case SummaryRole:
                data.setValue<QString>(entry.summary());
                break;
            case ChangelogRole:
                data.setValue<QString>(entry.changelog());
                break;
            case VersionRole:
                data.setValue<QString>(entry.version());
                break;
            case ReleaseDateRole:
                data.setValue<QDate>(entry.releaseDate());
                break;
            case UpdateVersionRole:
                data.setValue<QString>(entry.updateVersion());
                break;
            case UpdateReleaseDateRole:
                data.setValue<QDate>(entry.updateReleaseDate());
                break;
            case PayloadRole:
                data.setValue<QString>(entry.payload());
                break;
            case Qt::DecorationRole:
                data.setValue<QString>(entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1));
                break;
            case PreviewsSmallRole:
                {
                    QStringList previews;
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3);
                    while(!previews.isEmpty() && previews.last().isEmpty()) {
                        previews.takeLast();
                    }
                    data.setValue<QStringList>(previews);
                }
                break;
            case PreviewsRole:
                {
                    QStringList previews;
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig1);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig2);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig3);
                    while(!previews.isEmpty() && previews.last().isEmpty()) {
                        previews.takeLast();
                    }
                    data.setValue<QStringList>(previews);
                }
                break;
            case InstalledFilesRole:
                data.setValue<QStringList>(entry.installedFiles());
                break;
            case UnInstalledFilesRole:
                data.setValue<QStringList>(entry.uninstalledFiles());
                break;
            case RatingRole:
                data.setValue<int>(entry.rating());
                break;
            case NumberOfCommentsRole:
                data.setValue<int>(entry.numberOfComments());
                break;
            case DownloadCountRole:
                data.setValue<int>(entry.downloadCount());
                break;
            case NumberFansRole:
                data.setValue<int>(entry.numberFans());
                break;
            case NumberKnowledgebaseEntriesRole:
                data.setValue<int>(entry.numberKnowledgebaseEntries());
                break;
            case KnowledgebaseLinkRole:
                data.setValue<QString>(entry.knowledgebaseLink());
                break;
            case DownloadLinksRole:
                {
                    // This would be good to cache... but it also needs marking as dirty, somehow...
                    const QList<KNSCore::EntryInternal::DownloadLinkInformation> dllinks = entry.downloadLinkInformationList();
                    QObjectList list;
                    for(const KNSCore::EntryInternal::DownloadLinkInformation &link : dllinks)
                    {
                        DownloadLinkInfo *info = new DownloadLinkInfo();
                        info->setData(link);
                        list.append(info);
                    }
                    data.setValue<QObjectList>(list);
                }
                break;
            case DonationLinkRole:
                data.setValue<QString>(entry.donationLink());
                break;
            case ProviderIdRole:
                data.setValue<QString>(entry.providerId());
                break;
            case SourceRole:
                {
                    KNSCore::EntryInternal::Source src = entry.source();
                    switch(src)
                    {
                        case KNSCore::EntryInternal::Cache:
                            data.setValue<QString>(QStringLiteral("Cache"));
                            break;
                        case KNSCore::EntryInternal::Online:
                            data.setValue<QString>(QStringLiteral("Online"));
                            break;
                        case KNSCore::EntryInternal::Registry:
                            data.setValue<QString>(QStringLiteral("Registry"));
                            break;
                        default:
                            data.setValue<QString>(QStringLiteral("Unknown source - shouldn't be possible"));
                            break;
                    }
                }
                break;
            case StatusRole:
                {
                    KNS3::Entry::Status status = entry.status();
                    switch(status)
                    {
                        case KNS3::Entry::Downloadable:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::DownloadableStatus);
                            break;
                        case KNS3::Entry::Installed:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::InstalledStatus);
                            break;
                        case KNS3::Entry::Updateable:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::UpdateableStatus);
                            break;
                        case KNS3::Entry::Deleted:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::DeletedStatus);
                            break;
                        case KNS3::Entry::Installing:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::InstallingStatus);
                            break;
                        case KNS3::Entry::Updating:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::UpdatingStatus);
                            break;
                        case KNS3::Entry::Invalid:
                        default:
                            data.setValue<ItemsModel::ItemStatus>(ItemsModel::InvalidStatus);
                            break;
                    }
                }
                break;
            case CommentsModelRole:
                {
                    KNSCore::CommentsModel *commentsModel{nullptr};
                    if (!d->commentsModels.contains(entry.uniqueId())) {
                        commentsModel = d->coreEngine->commentsForEntry(entry);
                        d->commentsModels[entry.uniqueId()] = commentsModel;
                    } else {
                        commentsModel = d->commentsModels[entry.uniqueId()];
                    }
                    data.setValue<QObject*>(commentsModel);
                }
                break;
            default:
                data.setValue<QString>(QStringLiteral("Unknown role"));
                break;
        }
    }
    return data;
}

bool ItemsModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid() && d->coreEngine && d->coreEngine->categoriesMetadata().count() > 0) {
        return true;
    }
    return false;
}

void ItemsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() || !d->coreEngine) {
        return;
    }
    d->coreEngine->requestMoreData();
}

QObject *ItemsModel::engine() const
{
    return d->engine;
}

void ItemsModel::setEngine(QObject *newEngine)
{
    if (d->engine != newEngine) {
        beginResetModel();
        d->engine = qobject_cast<Engine*>(newEngine);
        d->model->deleteLater();
        d->model = nullptr;
        d->coreEngine = nullptr;
        if (d->engine) {
            d->coreEngine = qobject_cast<KNSCore::Engine*>(d->engine->engine());
        }
        connect(d->engine, &Engine::engineChanged, this, [this](){
            beginResetModel();
            d->model->deleteLater();
            d->model = nullptr;
            d->coreEngine = qobject_cast<KNSCore::Engine*>(d->engine->engine());
            endResetModel();
        });
        emit engineChanged();
        endResetModel();
    }
}

bool ItemsModel::isLoadingData() const
{
    return d->isLoadingData;
}

void ItemsModel::installItem(int index, int linkId)
{
    if (d->coreEngine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if(entry.isValid()) {
            d->coreEngine->install(entry, linkId);
        }
    }
}

void ItemsModel::updateItem(int index)
{
    installItem(index, AutoDetectLinkId);
}

void ItemsModel::uninstallItem(int index)
{
    if (d->coreEngine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if(entry.isValid()) {
            d->coreEngine->uninstall(entry);
        }
    }
}

void ItemsModel::adoptItem(int index)
{
    if (d->coreEngine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.isValid()) {
            QStringList args = KShell::splitArgs(d->coreEngine->adoptionCommand(entry));
            qCDebug(KNEWSTUFFQUICK) << "executing AdoptionCommand" << args;
            QProcess::startDetached(args.takeFirst(), args);
            d->engine->idleMessage(i18n("Using %1", entry.name()));
        }
    }
}
