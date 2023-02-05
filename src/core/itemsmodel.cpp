/*
    knewstuff3/ui/itemsmodel.cpp.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "itemsmodel.h"

#include <KLocalizedString>
#include <knewstuffcore_debug.h>

#include "engine.h"
#include "imageloader_p.h"

namespace KNSCore
{
class ItemsModelPrivate
{
public:
    ItemsModelPrivate(Engine *e)
        : engine(e)
    {
    }
    Engine *const engine;
    // the list of entries
    QList<EntryInternal> entries;
    bool hasPreviewImages = false;
};
ItemsModel::ItemsModel(Engine *engine, QObject *parent)
    : QAbstractListModel(parent)
    , d(new ItemsModelPrivate(engine))
{
}

ItemsModel::~ItemsModel() = default;

int ItemsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return d->entries.count();
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::UserRole) {
        return QVariant();
    }
    EntryInternal entry = d->entries[index.row()];
    return QVariant::fromValue(entry);
}

int ItemsModel::row(const EntryInternal &entry) const
{
    return d->entries.indexOf(entry);
}

void ItemsModel::slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    for (const KNSCore::EntryInternal &entry : entries) {
        addEntry(entry);
    }
}

void ItemsModel::addEntry(const EntryInternal &entry)
{
    // This might be expensive, but it avoids duplicates, which is not awesome for the user
    if (!d->entries.contains(entry)) {
        QString preview = entry.previewUrl(EntryInternal::PreviewSmall1);
        if (!d->hasPreviewImages && !preview.isEmpty()) {
            d->hasPreviewImages = true;
            if (rowCount() > 0) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
            }
        }

        qCDebug(KNEWSTUFFCORE) << "adding entry " << entry.name() << " to the model";
        beginInsertRows(QModelIndex(), d->entries.count(), d->entries.count());
        d->entries.append(entry);
        endInsertRows();

        if (!preview.isEmpty() && entry.previewImage(EntryInternal::PreviewSmall1).isNull()) {
            d->engine->loadPreview(entry, EntryInternal::PreviewSmall1);
        }
    }
}

void ItemsModel::removeEntry(const EntryInternal &entry)
{
    qCDebug(KNEWSTUFFCORE) << "removing entry " << entry.name() << " from the model";
    int index = d->entries.indexOf(entry);
    if (index > -1) {
        beginRemoveRows(QModelIndex(), index, index);
        d->entries.removeAt(index);
        endRemoveRows();
    }
}

void ItemsModel::slotEntryChanged(const EntryInternal &entry)
{
    int i = d->entries.indexOf(entry);
    QModelIndex entryIndex = index(i, 0);
    Q_EMIT dataChanged(entryIndex, entryIndex);
}

void ItemsModel::clearEntries()
{
    beginResetModel();
    d->entries.clear();
    endResetModel();
}

void ItemsModel::slotEntryPreviewLoaded(const EntryInternal &entry, EntryInternal::PreviewType type)
{
    // we only care about the first small preview in the list
    if (type != EntryInternal::PreviewSmall1) {
        return;
    }
    slotEntryChanged(entry);
}

bool ItemsModel::hasPreviewImages() const
{
    return d->hasPreviewImages;
}

} // end KNS namespace
