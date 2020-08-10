/*
    knewstuff3/ui/itemsmodel.h.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSMODEL_P_H
#define KNEWSTUFF3_ITEMSMODEL_P_H

#include <QAbstractListModel>

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
    /**
     * The row of the entry passed to the function, or -1 if the entry is not contained
     * within the model.
     * @since 5.63
     */
    int row(const EntryInternal &entry) const;

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
