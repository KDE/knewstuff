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

#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include <QAbstractListModel>

#include "provider.h"
#include "quickengine.h"

/**
 * @short A model which shows the categories found in an Engine
 * @since 5.63
 */
class CategoriesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CategoriesModel(Engine *parent = nullptr);
    virtual ~CategoriesModel();

    enum Roles {
        NameRole = Qt::UserRole + 1,
        IdRole,
        DisplayNameRole
    };
    Q_ENUMS(Roles)

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Get the display name for the category with the id passed to the function
     *
     * @param id The ID of the category you want to get the display name for
     * @return The display name (or the translated string "Unknown Category" for the requested category
     */
    Q_INVOKABLE QString idToDisplayName(const QString &id) const;
private:
    class Private;
    // TODO KF6: Switch all the pimpls to const std::unique_ptr<Private> d;
    Private *d;
};

#endif//CATEGORIESMODEL_H
