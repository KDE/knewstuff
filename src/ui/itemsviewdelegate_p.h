/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSVIEWDELEGATE_P_H
#define KNEWSTUFF3_ITEMSVIEWDELEGATE_P_H

#include "itemsviewbasedelegate_p.h"

namespace KNS3
{

class ItemsViewDelegate: public ItemsViewBaseDelegate
{
    Q_OBJECT
public:
    explicit ItemsViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent = nullptr);
    ~ItemsViewDelegate();

    // paint the item at index with all its attributes shown
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // get the list of widgets
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;

    // update the widgets
    void updateItemWidgets(const QList<QWidget *> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
}

#endif
