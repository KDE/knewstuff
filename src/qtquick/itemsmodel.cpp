/*
 * Copyright (C) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "itemsmodel.h"
#include "engine.h"

#include "core/itemsmodel_p.h"
#include "core/engine_p.h"

class ItemsModel::Private {
public:
    Private(ItemsModel* qq)
        : q(qq)
        , model(0)
        , engine(0)
    {}
    ItemsModel* q;
    KNS3::ItemsModel* model;
    KNS3::Engine* engine;

    bool initModel()
    {
        if(model) {
            return true;
        }
        if(!engine) {
            return false;
        }
        model = new KNS3::ItemsModel(engine, q);

        q->connect(engine, &KNS3::Engine::signalProvidersLoaded, engine, &KNS3::Engine::reloadEntries);
        // Entries have been fetched and should be shown:
        q->connect(engine, &KNS3::Engine::signalEntriesLoaded, model, &KNS3::ItemsModel::slotEntriesLoaded);

        // An entry has changes - eg because it was installed
        q->connect(engine, &KNS3::Engine::signalEntryChanged, model, &KNS3::ItemsModel::slotEntryChanged);

        q->connect(engine, &KNS3::Engine::signalResetView, model, &KNS3::ItemsModel::clearEntries);
        q->connect(engine, &KNS3::Engine::signalEntryPreviewLoaded, model, &KNS3::ItemsModel::slotEntryPreviewLoaded);

        q->connect(model, &KNS3::ItemsModel::rowsInserted, q, &ItemsModel::rowsInserted);
        q->connect(model, &KNS3::ItemsModel::rowsRemoved, q, &ItemsModel::rowsRemoved);
        q->connect(model, &KNS3::ItemsModel::dataChanged, q, &ItemsModel::dataChanged);
        q->connect(model, &KNS3::ItemsModel::modelReset, q, &ItemsModel::modelReset);
        return true;
    }
};

ItemsModel::ItemsModel(QObject* parent)
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
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[UniqueIdRole] = "uniqueId";
    roles[CategoryRole] = "category";
    roles[HomepageRole] = "homepage";
    roles[AuthorRole] = "author";
    roles[LicenseRole] = "license";
    roles[ShortSummaryRole] = "shortSummary";
    roles[SummaryRole] = "summary";
    roles[ChangelogRole] = "changelog";
    roles[VersionRole] = "version";
    roles[ReleaseDateRole] = "releaseDate";
    roles[UpdateVersionRole] = "updateVersion";
    roles[UpdateReleaseDateRole] = "updateReleaseDate";
    roles[PayloadRole] = "payload";
    roles[PreviewsSmallRole] = "previewsSmall";
    roles[PreviewsRole] = "previews";
    roles[InstalledFilesRole] = "installedFiles";
    roles[UnInstalledFilesRole] = "uninstalledFiles";
    roles[RatingRole] = "rating";
    roles[NumberOfCommentsRole] = "numberOfComments";
    roles[DownloadCountRole] = "downloadCount";
    roles[NumberFansRole] = "numberFans";
    roles[NumberKnowledgebaseEntriesRole] = "numberKnowledgebaseEntries";
    roles[KnowledgebaseLinkRole] = "knowledgebaseLink";
    roles[DownloadLinksRole] = "downloadLinks";
    roles[DonationLinkRole] = "donationLink";
    roles[ProviderIdRole] = "providerId";
    roles[SourceRole] = "source";
    roles[StatusRole] = "status";
    return roles;
}

int ItemsModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid())
        return 0;
    if(d->initModel())
        return d->model->rowCount(QModelIndex());
    return 0;
}

QVariant ItemsModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if(index.isValid() && d->initModel())
    {
        KNS3::EntryInternal entry = d->model->data(d->model->index(index.row()), Qt::UserRole).value<KNS3::EntryInternal>();
        switch(role)
        {
            case NameRole:
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
                    KNS3::Author author = entry.author();
                    QVariantMap returnAuthor;
                    returnAuthor["name"] = author.name();
                    returnAuthor["email"] = author.email();
                    returnAuthor["homepage"] = author.homepage();
                    returnAuthor["jabber"] = author.jabber();
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
            case PreviewsSmallRole:
                {
                    QStringList previews;
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewSmall1);
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewSmall2);
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewSmall3);
                    while(previews.last().isEmpty()) {
                        previews.takeLast();
                    }
                    data.setValue<QStringList>(previews);
                }
                break;
            case PreviewsRole:
                {
                    QStringList previews;
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewBig1);
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewBig2);
                    previews << entry.previewUrl(KNS3::EntryInternal::PreviewBig3);
                    while(previews.last().isEmpty()) {
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
//                 {
//                     QList<KNS3::EntryInternal::DownloadLinkInformation> dllinks = entry.downloadLinkInformationList();
//                     QStringList links;
//                     data.setValue<QString>(entry.());
//                 }
                data.setValue<QString>("download links need more things");
                break;
            case DonationLinkRole:
                data.setValue<QString>(entry.donationLink());
                break;
            case ProviderIdRole:
                data.setValue<QString>(entry.providerId());
                break;
            case SourceRole:
                {
                    KNS3::EntryInternal::Source src = entry.source();
                    switch(src)
                    {
                        case KNS3::EntryInternal::Cache:
                            data.setValue<QString>(QLatin1String("Cache"));
                            break;
                        case KNS3::EntryInternal::Online:
                            data.setValue<QString>(QLatin1String("Online"));
                            break;
                        case KNS3::EntryInternal::Registry:
                            data.setValue<QString>(QLatin1String("Registry"));
                            break;
                        default:
                            data.setValue<QString>(QLatin1String("Unknown source - shouldn't be possible"));
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
            default:
                data.setValue<QString>(QLatin1String("Unknown role"));
                break;
        }
    }
    return data;
}

bool ItemsModel::canFetchMore(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return false;
    }
    return true;
}

void ItemsModel::fetchMore(const QModelIndex& parent)
{
    if(parent.isValid()) {
        return;
    }
    d->engine->requestMoreData();
}

QObject * ItemsModel::engine() const
{
    return d->engine;
}

void ItemsModel::setEngine(QObject* newEngine)
{
    Engine* test = qobject_cast<Engine*>(newEngine);
    if(test) {
        d->engine = qobject_cast<KNS3::Engine*>(test->engine());
    }
    else {
        d->engine = qobject_cast<KNS3::Engine*>(newEngine);
    }
    emit engineChanged();
    reset();
}

void ItemsModel::installItem(int index)
{
    if(d->engine) {
        KNS3::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNS3::EntryInternal>();
        if(entry.isValid()) {
            d->engine->install(entry);
        }
    }
}

void ItemsModel::uninstallItem(int index)
{
    if(d->engine) {
        KNS3::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNS3::EntryInternal>();
        if(entry.isValid()) {
            d->engine->uninstall(entry);
        }
    }
}