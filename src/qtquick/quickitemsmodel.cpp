/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickitemsmodel.h"
#include "core/commentsmodel.h"
#include "downloadlinkinfo.h"
#include "engine.h"
#include "itemsmodel.h"
#include "quickengine.h"

#include <KShell>
#include <QProcess>

class ItemsModelPrivate
{
public:
    ItemsModelPrivate(ItemsModel *qq)
        : q(qq)
        , model(nullptr)
        , engine(nullptr)
        , coreEngine(nullptr)
    {
    }
    ItemsModel *q;
    KNSCore::ItemsModel *model;
    Engine *engine;
    KNSCore::Engine *coreEngine;

    QHash<QString, KNSCore::CommentsModel *> commentsModels;

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
        q->connect(coreEngine, &KNSCore::Engine::busyStateChanged, q, [=]() {
            // If we install/update an entry the spinner should be hidden, BUG: 422047
            const KNSCore::Engine::BusyState state = coreEngine->busyState();
            const bool busy = state && !state.testFlag(KNSCore::Engine::BusyOperation::InstallingEntry);
            if (isLoadingData != busy) {
                isLoadingData = busy;
                Q_EMIT q->isLoadingDataChanged();
            }
        });

        q->connect(coreEngine, &KNSCore::Engine::signalProvidersLoaded, coreEngine, &KNSCore::Engine::reloadEntries);
        // Entries have been fetched and should be shown:
        q->connect(coreEngine, &KNSCore::Engine::signalEntriesLoaded, model, [this](const KNSCore::EntryInternal::List &entries) {
            if (coreEngine->filter() != KNSCore::Provider::Updates) {
                model->slotEntriesLoaded(entries);
            }
        });
        q->connect(coreEngine,
                   &KNSCore::Engine::signalEntryEvent,
                   model,
                   [this](const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::EntryEvent event) {
                       if (event == KNSCore::EntryInternal::DetailsLoadedEvent && coreEngine->filter() != KNSCore::Provider::Updates) {
                           model->slotEntriesLoaded(KNSCore::EntryInternal::List{entry});
                       }
                   });
        q->connect(coreEngine, &KNSCore::Engine::signalUpdateableEntriesLoaded, model, [this](const KNSCore::EntryInternal::List &entries) {
            if (coreEngine->filter() == KNSCore::Provider::Updates) {
                model->slotEntriesLoaded(entries);
            }
        });

        q->connect(coreEngine, &KNSCore::Engine::signalEntryEvent, q, [this](const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::EntryEvent event) {
            onEntryEvent(entry, event);
        });
        q->connect(coreEngine, &KNSCore::Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);
        q->connect(coreEngine, &KNSCore::Engine::signalEntryPreviewLoaded, model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

        q->connect(model, &KNSCore::ItemsModel::rowsInserted, q, &ItemsModel::rowsInserted);
        q->connect(model, &KNSCore::ItemsModel::rowsRemoved, q, &ItemsModel::rowsRemoved);
        q->connect(model, &KNSCore::ItemsModel::dataChanged, q, &ItemsModel::dataChanged);
        q->connect(model, &KNSCore::ItemsModel::modelReset, q, &ItemsModel::modelReset);
        return true;
    }

    void onEntryEvent(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::EntryEvent event)
    {
        if (event == KNSCore::EntryInternal::StatusChangedEvent) {
            model->slotEntryChanged(entry);
            Q_EMIT q->entryChanged(model->row(entry));

            // If we update/uninstall an entry we have to update the UI, see BUG: 425135
            if (coreEngine->filter() == KNSCore::Provider::Updates && entry.status() != KNS3::Entry::Updateable && entry.status() != KNS3::Entry::Updating) {
                model->removeEntry(entry);
            } else if (coreEngine->filter() == KNSCore::Provider::Installed && entry.status() == KNS3::Entry::Deleted) {
                model->removeEntry(entry);
            }
        }
    }
};

ItemsModel::ItemsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ItemsModelPrivate(this))
{
}

ItemsModel::~ItemsModel() = default;

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    // clang-format off
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
        {StatusRole, "status"},
        {EntryTypeRole, "entryType"},
    };
    // clang-format on
    return roles;
}

int ItemsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    if (d->initModel()) {
        return d->model->rowCount(QModelIndex());
    }
    return 0;
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if (index.isValid() && d->initModel()) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index.row()), Qt::UserRole).value<KNSCore::EntryInternal>();
        switch (role) {
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
        case AuthorRole: {
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
        } break;
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
        case PreviewsSmallRole: {
            QStringList previews;
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2);
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3);
            while (!previews.isEmpty() && previews.last().isEmpty()) {
                previews.takeLast();
            }
            data.setValue(previews);
        } break;
        case PreviewsRole: {
            QStringList previews;
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig1);
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig2);
            previews << entry.previewUrl(KNSCore::EntryInternal::PreviewBig3);
            while (!previews.isEmpty() && previews.last().isEmpty()) {
                previews.takeLast();
            }
            data.setValue(previews);
        } break;
        case InstalledFilesRole:
            data.setValue(entry.installedFiles());
            break;
        case UnInstalledFilesRole:
            data.setValue(entry.uninstalledFiles());
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
        case DownloadLinksRole: {
            // This would be good to cache... but it also needs marking as dirty, somehow...
            const QList<KNSCore::EntryInternal::DownloadLinkInformation> dllinks = entry.downloadLinkInformationList();
            QObjectList list;
            for (const KNSCore::EntryInternal::DownloadLinkInformation &link : dllinks) {
                DownloadLinkInfo *info = new DownloadLinkInfo();
                info->setData(link);
                list.append(info);
            }
            if (list.isEmpty() && !entry.payload().isEmpty()) {
                DownloadLinkInfo *info = new DownloadLinkInfo();
                KNSCore::EntryInternal::DownloadLinkInformation data;
                data.descriptionLink = entry.payload();
                info->setData(data);
                list.append(info);
            }
            data.setValue(list);
        } break;
        case DonationLinkRole:
            data.setValue<QString>(entry.donationLink());
            break;
        case ProviderIdRole:
            data.setValue<QString>(entry.providerId());
            break;
        case SourceRole: {
            KNSCore::EntryInternal::Source src = entry.source();
            switch (src) {
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
        } break;
        case StatusRole: {
            KNS3::Entry::Status status = entry.status();
            switch (status) {
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
        } break;
        case CommentsModelRole: {
            KNSCore::CommentsModel *commentsModel{nullptr};
            if (!d->commentsModels.contains(entry.uniqueId())) {
                commentsModel = d->coreEngine->commentsForEntry(entry);
                d->commentsModels[entry.uniqueId()] = commentsModel;
            } else {
                commentsModel = d->commentsModels[entry.uniqueId()];
            }
            data.setValue<QObject *>(commentsModel);
        } break;
        case EntryTypeRole: {
            KNSCore::EntryInternal::EntryType type = entry.entryType();
            if (type == KNSCore::EntryInternal::GroupEntry) {
                data.setValue<ItemsModel::EntryType>(ItemsModel::GroupEntry);
            } else {
                data.setValue<ItemsModel::EntryType>(ItemsModel::CatalogEntry);
            }
        } break;
        default:
            data.setValue<QString>(QStringLiteral("Unknown role"));
            break;
        }
    }
    return data;
}

bool ItemsModel::canFetchMore(const QModelIndex &parent) const
{
    return !parent.isValid() && d->coreEngine && d->coreEngine->categoriesMetadata().count() > 0;
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
        d->engine = qobject_cast<Engine *>(newEngine);
        d->model->deleteLater();
        d->model = nullptr;
        d->coreEngine = nullptr;
        if (d->engine) {
            d->coreEngine = qobject_cast<KNSCore::Engine *>(d->engine->engine());
        }
        connect(d->engine, &Engine::engineChanged, this, [this]() {
            beginResetModel();
            d->model->deleteLater();
            d->model = nullptr;
            d->coreEngine = qobject_cast<KNSCore::Engine *>(d->engine->engine());
            endResetModel();
        });
        Q_EMIT engineChanged();
        endResetModel();
    }
}

int ItemsModel::indexOfEntryId(const QString &providerId, const QString &entryId)
{
    int idx{-1};
    if (d->coreEngine && d->model) {
        for (int i = 0; i < rowCount(); ++i) {
            KNSCore::EntryInternal testEntry = d->model->data(d->model->index(i), Qt::UserRole).value<KNSCore::EntryInternal>();
            if (providerId == QUrl(testEntry.providerId()).host() && entryId == testEntry.uniqueId()) {
                idx = i;
                break;
            }
        }
    }
    return idx;
}

bool ItemsModel::isLoadingData() const
{
    return d->isLoadingData;
}

void ItemsModel::installItem(int index, int linkId)
{
    if (d->coreEngine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.isValid()) {
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
        if (entry.isValid()) {
            d->coreEngine->uninstall(entry);
        }
    }
}

void ItemsModel::adoptItem(int index)
{
    if (d->coreEngine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.isValid()) {
            d->coreEngine->adoptEntry(entry);
        }
    }
}
