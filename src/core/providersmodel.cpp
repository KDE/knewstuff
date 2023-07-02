/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "providersmodel.h"

namespace KNSCore
{
class ProvidersModelPrivate
{
public:
    Engine *engine{nullptr};
    QStringList knownProviders;
};

ProvidersModel::ProvidersModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ProvidersModelPrivate)
{
}

ProvidersModel::~ProvidersModel() = default;

QHash<int, QByteArray> KNSCore::ProvidersModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {IdRole, "id"},
        {NameRole, "name"},
        {VersionRole, "version"},
        {WebsiteRole, "website"},
        {HostRole, "host"},
        {ContactEmailRole, "contactEmail"},
        {SupportsSslRole, "supportsSsl"},
        {IconRole, "icon"},
        {ObjectRole, "object"},
    };
    return roles;
}

int KNSCore::ProvidersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->knownProviders.count();
}

QVariant KNSCore::ProvidersModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (checkIndex(index) && d->engine) {
        QSharedPointer<Provider> provider = d->engine->provider(d->knownProviders.value(index.row()));
        if (provider) {
            switch (role) {
            case IdRole:
                result.setValue(provider->id());
                break;
            case NameRole:
                result.setValue(provider->name());
                break;
            case VersionRole:
                result.setValue(provider->version());
                break;
            case WebsiteRole:
                result.setValue(provider->website());
                break;
            case HostRole:
                result.setValue(provider->host());
                break;
            case ContactEmailRole:
                result.setValue(provider->contactEmail());
                break;
            case SupportsSslRole:
                result.setValue(provider->supportsSsl());
                break;
            case IconRole:
                result.setValue(provider->icon());
                break;
            case ObjectRole:
                result.setValue<QObject *>(provider.data());
                break;
            default:
                break;
            }
        }
    }
    return result;
}

QObject *KNSCore::ProvidersModel::engine() const
{
    return d->engine;
}

void KNSCore::ProvidersModel::setEngine(QObject *engine)
{
    if (d->engine != engine) {
        if (d->engine) {
            d->engine->disconnect(this);
        }
        d->engine = qobject_cast<Engine *>(engine);
        Q_EMIT engineChanged();
        if (d->engine) {
            connect(d->engine, &Engine::providersChanged, this, [this]() {
                beginResetModel();
                d->knownProviders = d->engine->providerIDs();
                endResetModel();
            });
            beginResetModel();
            d->knownProviders = d->engine->providerIDs();
            endResetModel();
        }
    }
}

}

#include "moc_providersmodel.cpp"
