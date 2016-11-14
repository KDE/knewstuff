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

#include "itemsmodel_p.h"
#include "engine_p.h"

class ItemsModel::Private {
public:
    Private(ItemsModel* qq)
        : q(qq)
        , model(0)
        , engine(0)
    {}
    ItemsModel* q;
    KNSCore::ItemsModel* model;
    KNSCore::Engine* engine;

    bool initModel()
    {
        if(model) {
            return true;
        }
        if(!engine) {
            return false;
        }
        model = new KNSCore::ItemsModel(engine, q);

        q->connect(engine, &KNSCore::Engine::signalProvidersLoaded, engine, &KNSCore::Engine::reloadEntries);
        // Entries have been fetched and should be shown:
        q->connect(engine, &KNSCore::Engine::signalEntriesLoaded, model, &KNSCore::ItemsModel::slotEntriesLoaded);

        // An entry has changes - eg because it was installed
        q->connect(engine, &KNSCore::Engine::signalEntryChanged, model, &KNSCore::ItemsModel::slotEntryChanged);

        q->connect(engine, &KNSCore::Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);
        q->connect(engine, &KNSCore::Engine::signalEntryPreviewLoaded, model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

        q->connect(model, &KNSCore::ItemsModel::rowsInserted, q, &ItemsModel::rowsInserted);
        q->connect(model, &KNSCore::ItemsModel::rowsRemoved, q, &ItemsModel::rowsRemoved);
        q->connect(model, &KNSCore::ItemsModel::dataChanged, q, &ItemsModel::dataChanged);
        q->connect(model, &KNSCore::ItemsModel::modelReset, q, &ItemsModel::modelReset);
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
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index.row()), Qt::UserRole).value<KNSCore::EntryInternal>();
        switch(role)
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
            case Qt::DecorationRole:
                data.setValue<QString>(entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1));
                break;
            case PreviewsSmallRole:
                {
                    QStringList previews;
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2);
                    previews << entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3);
                    while(previews.last().isEmpty()) {
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
//                     QList<KNSCore::EntryInternal::DownloadLinkInformation> dllinks = entry.downloadLinkInformationList();
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
                    KNSCore::EntryInternal::Source src = entry.source();
                    switch(src)
                    {
                        case KNSCore::EntryInternal::Cache:
                            data.setValue<QString>(QLatin1String("Cache"));
                            break;
                        case KNSCore::EntryInternal::Online:
                            data.setValue<QString>(QLatin1String("Online"));
                            break;
                        case KNSCore::EntryInternal::Registry:
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
    beginResetModel();
    Engine* test = qobject_cast<Engine*>(newEngine);
    if(test) {
        d->engine = qobject_cast<KNSCore::Engine*>(test->engine());
    }
    else {
        d->engine = qobject_cast<KNSCore::Engine*>(newEngine);
    }
    emit engineChanged();
    endResetModel();
}

void ItemsModel::installItem(int index)
{
    if(d->engine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if(entry.isValid()) {
            d->engine->install(entry);
        }
    }
}

void ItemsModel::uninstallItem(int index)
{
    if(d->engine) {
        KNSCore::EntryInternal entry = d->model->data(d->model->index(index), Qt::UserRole).value<KNSCore::EntryInternal>();
        if(entry.isValid()) {
            d->engine->uninstall(entry);
        }
    }
}
