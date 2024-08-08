/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "searchpresetmodel.h"

#include "knewstuffquick_debug.h"

#include <KLocalizedString>

#include "../core/enginebase_p.h"

SearchPresetModel::SearchPresetModel(KNSCore::EngineBase *engine)
    : QAbstractListModel(engine)
    , m_engine(engine)
{
    connect(m_engine, qOverload<const QList<KNSCore::SearchPreset> &>(&KNSCore::EngineBase::signalSearchPresetsLoaded), this, [this]() {
        beginResetModel();
        endResetModel();
    });
}

SearchPresetModel::~SearchPresetModel() = default;

QHash<int, QByteArray> SearchPresetModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{{DisplayNameRole, "displayName"}, {IconRole, "iconName"}};
    return roles;
}

QVariant SearchPresetModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && checkIndex(index)) {
        const QList<KNSCore::SearchPreset> presets = m_engine->d->searchPresets;
        const KNSCore::SearchPreset &preset = presets[index.row()];

        if (role == DisplayNameRole) {
            if (QString name = preset.displayName(); !name.isEmpty()) {
                return name;
            }

            switch (preset.type()) {
            case KNSCore::SearchPreset::Type::GoBack:
                return i18nc("Knewstuff5", "Back");
            case KNSCore::SearchPreset::Type::Popular:
                return i18nc("Knewstuff5", "Popular");
            case KNSCore::SearchPreset::Type::Featured:
                return i18nc("Knewstuff5", "Featured");
            case KNSCore::SearchPreset::Type::Start:
                return i18nc("Knewstuff5", "Restart");
            case KNSCore::SearchPreset::Type::New:
                return i18nc("Knewstuff5", "New");
            case KNSCore::SearchPreset::Type::Root:
                return i18nc("Knewstuff5", "Home");
            case KNSCore::SearchPreset::Type::Shelf:
                return i18nc("Knewstuff5", "Shelf");
            case KNSCore::SearchPreset::Type::FolderUp:
                return i18nc("Knewstuff5", "Up");
            case KNSCore::SearchPreset::Type::Recommended:
                return i18nc("Knewstuff5", "Recommended");
            case KNSCore::SearchPreset::Type::Subscription:
                return i18nc("Knewstuff5", "Subscriptions");
            case KNSCore::SearchPreset::Type::AllEntries:
                return i18nc("Knewstuff5", "All Entries");
            case KNSCore::SearchPreset::Type::NoPresetType:
                break;
            }
            return i18nc("Knewstuff5", "Search Preset: %1", preset.request().searchTerm());
        }
        if (role == IconRole) {
            if (QString name = preset.iconName(); !name.isEmpty()) {
                return name;
            }

            switch (preset.type()) {
            case KNSCore::SearchPreset::Type::GoBack:
                return QStringLiteral("arrow-left");
            case KNSCore::SearchPreset::Type::Popular:
            case KNSCore::SearchPreset::Type::Featured:
            case KNSCore::SearchPreset::Type::Recommended:
                return QStringLiteral("rating");
            case KNSCore::SearchPreset::Type::New:
                return QStringLiteral("change-date-symbolic");
            case KNSCore::SearchPreset::Type::Start:
                return QStringLiteral("start-over");
            case KNSCore::SearchPreset::Type::Root:
                return QStringLiteral("go-home");
            case KNSCore::SearchPreset::Type::Shelf:
            case KNSCore::SearchPreset::Type::Subscription:
                return QStringLiteral("bookmark");
            case KNSCore::SearchPreset::Type::FolderUp:
                return QStringLiteral("arrow-up");
            case KNSCore::SearchPreset::Type::AllEntries:
            case KNSCore::SearchPreset::Type::NoPresetType:
                break;
            }
            return QStringLiteral("search");
        }
    }
    return QVariant();
}

int SearchPresetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_engine->d->searchPresets.count();
}

void SearchPresetModel::loadSearch(const QModelIndex &index)
{
    if (index.row() >= rowCount() || !index.isValid()) {
        qCWarning(KNEWSTUFFQUICK) << "index SearchPresetModel::loadSearch invalid" << index;
        return;
    }
    const auto preset = m_engine->d->searchPresets.at(index.row());
    m_engine->search(preset.request());
}

#include "moc_searchpresetmodel.cpp"
