/*
    Copyright (C) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_ITEMSVIEWBASEDELEGATE_P_H
#define KNEWSTUFF3_ITEMSVIEWBASEDELEGATE_P_H

#include <QList>
#include <QMap>
#include <QModelIndex>
#include <QObject>
#include <QImage>

#include "core/engine.h"
#include "core/entryinternal.h"

#include <kwidgetitemdelegate.h>

namespace KNS3
{
class ItemsViewBaseDelegate: public KWidgetItemDelegate
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
    void updateItemWidgets(const QList<QWidget *> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const override = 0;

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
    KNSCore::Engine *m_engine;
    QAbstractItemView *m_itemView;
    QIcon m_iconInvalid;
    QIcon m_iconDownloadable;
    QIcon m_iconInstall;
    QIcon m_iconUpdate;
    QIcon m_iconDelete;
    QPixmap m_frameImage;
    QPixmap m_noImage;
    QSize m_buttonSize;
};
}
#endif
