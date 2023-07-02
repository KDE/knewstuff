/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "categoriesmodel.h"

#include "engine.h"

#include <KLocalizedString>

class CategoriesModelPrivate
{
public:
    CategoriesModelPrivate()
    {
    }
    KNSCore::Engine *engine;
};

CategoriesModel::CategoriesModel(Engine *parent)
    : QAbstractListModel(parent)
    , d(new CategoriesModelPrivate)
{
    d->engine = qobject_cast<KNSCore::Engine *>(parent->engine());
    connect(d->engine, &KNSCore::Engine::signalCategoriesMetadataLoded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

CategoriesModel::~CategoriesModel() = default;

QHash<int, QByteArray> CategoriesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{NameRole, "name"}, {IdRole, "id"}, {DisplayNameRole, "displayName"}};
    return roles;
}

int CategoriesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
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
                result.setValue(QString());
                break;
            case IdRole:
                result.setValue(0);
                break;
            case DisplayNameRole:
                result.setValue(i18nc("The first entry in the category selection list (also the default value)", "All Categories"));
                break;
            default:
                result.setValue(QStringLiteral("Unknown role"));
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
                result.setValue(QStringLiteral("Unknown role"));
                break;
            }
        }
    }
    return result;
}

QString CategoriesModel::idToDisplayName(const QString &id) const
{
    QString dispName = i18nc("The string passed back in the case the requested category is not known", "Unknown Category");
    const auto metaData = d->engine->categoriesMetadata();
    for (const KNSCore::Provider::CategoryMetadata &cat : metaData) {
        if (cat.id == id) {
            dispName = cat.displayName;
            break;
        }
    }
    return dispName;
}

#include "moc_categoriesmodel.cpp"
