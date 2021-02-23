/*
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSVIEWBASEDELEGATE_P_H
#define KNEWSTUFF3_ITEMSVIEWBASEDELEGATE_P_H

#include <QList>
#include <QModelIndex>
#include <QObject>

#include "core/engine.h"
#include "core/entryinternal.h"

#include <KWidgetItemDelegate>

namespace KNS3
{
class ItemsViewBaseDelegate : public KWidgetItemDelegate
{
    Q_OBJECT
public:
    explicit ItemsViewBaseDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent = nullptr);
    virtual ~ItemsViewBaseDelegate();
    // paint the item at index with all its attributes shown
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override = 0;

    // get the list of widgets
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override = 0;

    // update the widgets
    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override = 0;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override = 0;

Q_SIGNALS:
    void signalShowDetails(const KNSCore::EntryInternal &entry);

protected Q_SLOTS:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void slotInstallClicked();
    void slotInstallActionTriggered(QAction *action);
    void slotLinkClicked(const QString &url);
    void slotDetailsClicked(const QModelIndex &index);
    void slotDetailsClicked();

protected:
    KNSCore::Engine *const m_engine;
    QAbstractItemView *const m_itemView;
    QIcon m_iconInvalid;
    QIcon m_iconDownloadable;
    QIcon m_iconInstall;
    QIcon m_iconUpdate;
    QIcon m_iconDelete;
    QPixmap m_frameImage;
    QSize m_buttonSize;
};
}
#endif
