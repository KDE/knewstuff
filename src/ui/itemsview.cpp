/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "itemsview_p.h"

#include <QScrollBar>

namespace KNS3
{

ItemsView::ItemsView(QWidget *parent)
    : QListView(parent)
{
}

void ItemsView::wheelEvent(QWheelEvent *event)
{
    // this is a workaround because scrolling by mouse wheel is broken in Qt list views for big items
    verticalScrollBar()->setSingleStep(10);
    QListView::wheelEvent(event);
}

} // end KNS namespace

