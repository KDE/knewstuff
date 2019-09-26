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

#include "categoriesmodel.h"

#include "engine.h"

#include <KLocalizedString>

class CategoriesModel::Private {
public:
    Private() {}
    KNSCore::Engine *engine;
};

CategoriesModel::CategoriesModel(Engine *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    d->engine = qobject_cast<KNSCore::Engine*>(parent->engine());
    connect(d->engine, &KNSCore::Engine::signalCategoriesMetadataLoded, this, [this](){ beginResetModel(); endResetModel(); });
}

CategoriesModel::~CategoriesModel()
{
    delete d;
}

QHash<int, QByteArray> CategoriesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {NameRole, "name"},
        {IdRole, "id"},
        {DisplayNameRole, "displayName"}
    };
    return roles;
}

int CategoriesModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->engine->categoriesMetadata().count() + 1;
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    const QList<KNSCore::Provider::CategoryMetadata> categoriesMetadata = d->engine->categoriesMetadata();
    if (index.isValid()) {
        if (index.row() == 0) {
            switch (role) {
                case NameRole:
                    result.setValue(QString::fromLatin1(""));
                    break;
                case IdRole:
                    result.setValue(0);
                    break;
                case DisplayNameRole:
                    result.setValue(i18nc("The first entry in the category selection list (also the default value)", "Show All Categories"));
                    break;
                default:
                    result.setValue(QString::fromLatin1("Unknown role"));
                    break;
            }
        } else if (index.row() <= categoriesMetadata.count()) {
            const KNSCore::Provider::CategoryMetadata category = categoriesMetadata[index.row() - 1];
            switch (role) {
                case NameRole:
                    result.setValue(category.name);
                    break;
                case IdRole:
                    result.setValue(category.id);
                    break;
                case DisplayNameRole:
                    result.setValue(category.displayName);
                    break;
                default:
                    result.setValue(QString::fromLatin1("Unknown role"));
                    break;
            }
        }
    }
    return result;
}

QString CategoriesModel::idToDisplayName(const QString &id) const
{
    QString dispName = i18nc("The string passed back in the case the requested category is not known", "Unknown Category");
    for (const KNSCore::Provider::CategoryMetadata &cat : d->engine->categoriesMetadata()) {
        if (cat.id == id) {
            dispName = cat.displayName;
            break;
        }
    }
    return dispName;
}
