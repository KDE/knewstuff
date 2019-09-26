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

#ifndef KNSQUICK_COMMENTSMODEL_H
#define KNSQUICK_COMMENTSMODEL_H

#include <QIdentityProxyModel>
#include <QQmlParserStatus>
#include <entryinternal.h>

namespace KNewStuffQuick
{
/**
 * @short Encapsulates a KNSCore::CommentsModel for use in Qt Quick
 *
 * This class takes care of initialisation of a KNSCore::CommentsModel when assigned an engine,
 * providerId and entryId. If the data is not yet cached, it will be requested from the provider,
 * and updated for display
 * @since 5.63
 */
class CommentsModel : public QIdentityProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The KNewStufQuick::ItemsModel to interact with servers through
     */
    Q_PROPERTY(QObject *itemsModel READ itemsModel WRITE setItemsModel NOTIFY itemsModelChanged)
    /**
     * The index in the model of the entry to fetch comments for
     */
    Q_PROPERTY(int entryIndex READ entryIndex WRITE setEntryIndex NOTIFY entryIndexChanged)
public:
    explicit CommentsModel(QObject *parent = nullptr);
    ~CommentsModel() override;
    void classBegin() override;
    void componentComplete() override;

    QObject *itemsModel() const;
    void setItemsModel(QObject *newItemsModel);
    Q_SIGNAL void itemsModelChanged();

    int entryIndex() const;
    void setEntryIndex(int entryIndex);
    Q_SIGNAL void entryIndexChanged();


private:
    class Private;
    Private *d;
};
}

#endif//KNSQUICK_COMMENTSMODEL_H
