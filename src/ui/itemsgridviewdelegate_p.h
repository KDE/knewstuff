/*
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSGRIDVIEWDELEGATE_P_H
#define KNEWSTUFF3_ITEMSGRIDVIEWDELEGATE_P_H

#include "itemsviewbasedelegate_p.h"
class QToolButton;
namespace KNS3
{
static const int ItemGridHeight = 202;
static const int ItemGridWidth = 158;

static const int FrameThickness = 5;
static const int ItemMargin = 2;
class ItemsGridViewDelegate : public ItemsViewBaseDelegate
{
    Q_OBJECT
public:
    explicit ItemsGridViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent = nullptr);
    ~ItemsGridViewDelegate() override;

    // paint the item at index with all its attributes shown
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // get the list of widgets
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;

    // update the widgets
    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void createOperationBar();
    void displayOperationBar(const QRect &rect, const QModelIndex &index);

private:
    QWidget *m_operationBar;
    QToolButton *m_detailsButton;
    QToolButton *m_installButton;

    QModelIndex m_oldIndex;
    mutable int m_elementYPos;
};
}

#endif
