/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSVIEW_P_H
#define KNEWSTUFF3_ITEMSVIEW_P_H

#include <QListView>

namespace KNS3
{
class ItemsView: public QListView
{
public:
    explicit ItemsView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};

} // end KNS namespace

#endif
