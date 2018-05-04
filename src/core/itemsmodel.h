/*
    knewstuff3/ui/itemsmodel.h.
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>

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

#ifndef KNEWSTUFF3_ITEMSMODEL_P_H
#define KNEWSTUFF3_ITEMSMODEL_P_H

#include <QAbstractListModel>
#include <QImage>

#include "entryinternal.h"
#include "knewstuffcore_export.h"

class KJob;

namespace KNSCore
{
class Engine;

class KNEWSTUFFCORE_EXPORT ItemsModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemsModel(Engine *engine, QObject *parent = nullptr);
    ~ItemsModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addEntry(const EntryInternal &entry);
    void removeEntry(const EntryInternal &entry);

    bool hasPreviewImages() const;
    bool hasWebService() const;

Q_SIGNALS:
    void jobStarted(KJob *, const QString &label);

public Q_SLOTS:
    void slotEntryChanged(const KNSCore::EntryInternal &entry);
    void slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void clearEntries();
    void slotEntryPreviewLoaded(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::PreviewType type);

private:
    Engine *m_engine;
    // the list of entries
    QList<EntryInternal> m_entries;
    bool m_hasPreviewImages;
};

} // end KNS namespace

Q_DECLARE_METATYPE(KNSCore::EntryInternal)

#endif
