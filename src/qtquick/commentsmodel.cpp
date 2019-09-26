/*
 * Copyright (C) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "commentsmodel.h"

#include "quickitemsmodel.h"

#include "core/commentsmodel.h"

#include "knewstuffquick_debug.h"

#include <memory>

namespace KNewStuffQuick {

class CommentsModel::Private {
public:
    Private(CommentsModel *qq)
        : q(qq)
    {}
    CommentsModel *q;
    ItemsModel *itemsModel{nullptr};
    int entryIndex{-1};
    bool componentCompleted{false};

    QSharedPointer<KNSCore::Provider> provider;
    void resetConnections() {
        if (componentCompleted && itemsModel) {
            q->setSourceModel(qobject_cast<QAbstractListModel*>(itemsModel->data(itemsModel->index(entryIndex), ItemsModel::CommentsModelRole).value<QObject*>()));
        }
    }
};
}

using namespace KNewStuffQuick;

CommentsModel::CommentsModel(QObject *parent)
    : QIdentityProxyModel(parent)
    , d(new Private(this))
{
}

CommentsModel::~CommentsModel()
{
    delete d;
}

void KNewStuffQuick::CommentsModel::classBegin()
{}

void KNewStuffQuick::CommentsModel::componentComplete()
{
    d->componentCompleted = true;
    d->resetConnections();
}

QObject *CommentsModel::itemsModel() const
{
    return d->itemsModel;
}

void CommentsModel::setItemsModel(QObject *newItemsModel)
{
    if (d->itemsModel != newItemsModel) {
        d->itemsModel = qobject_cast<ItemsModel*>(newItemsModel);
        d->resetConnections();
        emit itemsModelChanged();
    }
}

int CommentsModel::entryIndex() const
{
    return d->entryIndex;
}

void CommentsModel::setEntryIndex(int entryIndex)
{
    if (d->entryIndex != entryIndex) {
        d->entryIndex = entryIndex;
        d->resetConnections();
        emit entryIndexChanged();
    }
}
