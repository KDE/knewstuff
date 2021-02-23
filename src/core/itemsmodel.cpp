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
ItemsModel::ItemsModel(Engine *engine, QObject *parent)
    : QAbstractListModel(parent)
    , m_engine(engine)
{
}

ItemsModel::~ItemsModel()
{
}

int ItemsModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_entries.count();
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::UserRole) {
        return QVariant();
    }
    EntryInternal entry = m_entries[index.row()];
    return QVariant::fromValue(entry);
}

int ItemsModel::row(const EntryInternal &entry) const
{
    return m_entries.indexOf(entry);
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
    if (!m_entries.contains(entry)) {
        QString preview = entry.previewUrl(EntryInternal::PreviewSmall1);
        if (!m_hasPreviewImages && !preview.isEmpty()) {
            m_hasPreviewImages = true;
            if (rowCount() > 0) {
                Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0));
            }
        }

        qCDebug(KNEWSTUFFCORE) << "adding entry " << entry.name() << " to the model";
        beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count());
        m_entries.append(entry);
        endInsertRows();

        if (!preview.isEmpty() && entry.previewImage(EntryInternal::PreviewSmall1).isNull()) {
            m_engine->loadPreview(entry, EntryInternal::PreviewSmall1);
        }
    }
}

void ItemsModel::removeEntry(const EntryInternal &entry)
{
    qCDebug(KNEWSTUFFCORE) << "removing entry " << entry.name() << " from the model";
    int index = m_entries.indexOf(entry);
    if (index > -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_entries.removeAt(index);
        endRemoveRows();
    }
}

void ItemsModel::slotEntryChanged(const EntryInternal &entry)
{
    int i = m_entries.indexOf(entry);
    QModelIndex entryIndex = index(i, 0);
    Q_EMIT dataChanged(entryIndex, entryIndex);
}

void ItemsModel::clearEntries()
{
    beginResetModel();
    m_entries.clear();
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
    return m_hasPreviewImages;
}

} // end KNS namespace
