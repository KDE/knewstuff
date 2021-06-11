/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "searchpresetmodel.h"
#include "engine.h"

#include "knewstuffquick_debug.h"

#include <KLocalizedString>

class SearchPresetModel::Private
{
public:
    Private()
    {
    }
    KNSCore::Engine *engine;
};

SearchPresetModel::SearchPresetModel(Engine *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    d->engine = qobject_cast<KNSCore::Engine *>(parent->engine());
    connect(d->engine, &KNSCore::Engine::signalSearchPresetsLoaded, this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

SearchPresetModel::~SearchPresetModel()
{
    delete d;
}

QHash<int, QByteArray> SearchPresetModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{DisplayNameRole, "displayName"}, {IconRole, "iconName"}};
    return roles;
}

QVariant SearchPresetModel::data(const QModelIndex &index, int role) const
{
    QVariant result;
    if (index.isValid() && checkIndex(index)) {
        const QList<KNSCore::Provider::SearchPreset> presets = d->engine->searchPresets();
        const KNSCore::Provider::SearchPreset preset = presets[index.row()];

        if (role == DisplayNameRole) {
            QString name = preset.displayName;

            if (name.isEmpty()) {
                switch (preset.type) {
                case KNSCore::Provider::SearchPresetTypes::GoBack:
                    name = i18nc("Knewstuff5", "Back");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Popular:
                    name = i18nc("Knewstuff5", "Popular");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Featured:
                    name = i18nc("Knewstuff5", "Featured");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Start:
                    name = i18nc("Knewstuff5", "Restart");
                    break;
                case KNSCore::Provider::SearchPresetTypes::New:
                    name = i18nc("Knewstuff5", "New");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Root:
                    name = i18nc("Knewstuff5", "Home");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Shelf:
                    name = i18nc("Knewstuff5", "Shelf");
                    break;
                case KNSCore::Provider::SearchPresetTypes::FolderUp:
                    name = i18nc("Knewstuff5", "Up");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Recommended:
                    name = i18nc("Knewstuff5", "Recommended");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Subscription:
                    name = i18nc("Knewstuff5", "Subscriptions");
                    break;
                case KNSCore::Provider::SearchPresetTypes::AllEntries:
                    name = i18nc("Knewstuff5", "All Entries");
                    break;
                default:
                    name = i18nc("Knewstuff5", "Search Preset: %1", preset.request.searchTerm);
                }
            }

            result.setValue(name);
        } else if (role == IconRole) {
            QString name = preset.iconName;

            if (name.isEmpty()) {
                switch (preset.type) {
                case KNSCore::Provider::SearchPresetTypes::GoBack:
                    name = QStringLiteral("arrow-left");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Popular:
                case KNSCore::Provider::SearchPresetTypes::Featured:
                case KNSCore::Provider::SearchPresetTypes::Recommended:
                    name = QStringLiteral("rating");
                    break;
                case KNSCore::Provider::SearchPresetTypes::New:
                    name = QStringLiteral("change-date-symbolic");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Start:
                    name = QStringLiteral("start-over");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Root:
                    name = QStringLiteral("go-home");
                    break;
                case KNSCore::Provider::SearchPresetTypes::Shelf:
                case KNSCore::Provider::SearchPresetTypes::Subscription:
                    name = QStringLiteral("bookmark");
                    break;
                case KNSCore::Provider::SearchPresetTypes::FolderUp:
                    name = QStringLiteral("arrow-up");
                    break;
                default:
                    name = QStringLiteral("search");
                }
            }

            result.setValue(name);
        }
    }
    return result;
}

int SearchPresetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->engine->searchPresets().count();
}

void SearchPresetModel::loadSearch(const QModelIndex &index)
{
    if (index.row() >= rowCount() || !index.isValid()) {
        qCWarning(KNEWSTUFFQUICK) << "index SearchPresetModel::loadSearch invalid" << index;
        return;
    }
    const KNSCore::Provider::SearchPreset preset = d->engine->searchPresets().at(index.row());
    d->engine->setSearchTerm(preset.request.searchTerm);
}
