// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include "categorymetadata.h"
#include "provider.h"
#include "searchpreset.h"
#include "searchrequest.h"

namespace KNSCompat
{

inline KNSCore::Provider::SearchRequest searchRequestToLegacy(const KNSCore::SearchRequest &request)
{
    return {
        [mode = request.sortMode()] {
            switch (mode) {
            case KNSCore::SortMode::Alphabetical:
                return KNSCore::Provider::Alphabetical;
            case KNSCore::SortMode::Downloads:
                return KNSCore::Provider::Downloads;
            case KNSCore::SortMode::Newest:
                return KNSCore::Provider::Newest;
            case KNSCore::SortMode::Rating:
                return KNSCore::Provider::Rating;
            }
            return KNSCore::Provider::Rating;
        }(),
        [filter = request.filter()] {
            switch (filter) {
            case KNSCore::Filter::ExactEntryId:
                return KNSCore::Provider::ExactEntryId;
            case KNSCore::Filter::Installed:
                return KNSCore::Provider::Installed;
            case KNSCore::Filter::Updates:
                return KNSCore::Provider::Updates;
            case KNSCore::Filter::None:
                return KNSCore::Provider::None;
            }
            return KNSCore::Provider::None;
        }(),
        request.searchTerm(),
        request.categories(),
        request.page(),
        request.pageSize(),
        // Note that this loses the id but there's nothing we can do about it. It's why we deprecated it.
    };
}

inline KNSCore::SearchRequest searchRequestFromLegacy(const KNSCore::Provider::SearchRequest &request)
{
    return {[request] {
                switch (request.sortMode) {
                case KNSCore::Provider::SortMode::Alphabetical:
                    return KNSCore::SortMode::Alphabetical;
                case KNSCore::Provider::SortMode::Downloads:
                    return KNSCore::SortMode::Downloads;
                case KNSCore::Provider::SortMode::Newest:
                    return KNSCore::SortMode::Newest;
                case KNSCore::Provider::SortMode::Rating:
                    return KNSCore::SortMode::Rating;
                }
                Q_ASSERT(false);
                return KNSCore::SortMode::Rating;
            }(),
            [request] {
                switch (request.filter) {
                case KNSCore::Provider::Filter::None:
                    return KNSCore::Filter::None;
                case KNSCore::Provider::Filter::Installed:
                    return KNSCore::Filter::Installed;
                case KNSCore::Provider::Filter::Updates:
                    return KNSCore::Filter::Updates;
                case KNSCore::Provider::Filter::ExactEntryId:
                    return KNSCore::Filter::ExactEntryId;
                }
                Q_ASSERT(false);
                return KNSCore::Filter::None;
            }(),
            request.searchTerm,
            request.categories,
            request.page,
            request.pageSize};
}

inline KNSCore::Provider::SearchPreset searchPresetToLegacy(const KNSCore::SearchPreset &preset)
{
    return {
        .request = searchRequestToLegacy(preset.request()),
        .displayName = preset.displayName(),
        .iconName = preset.iconName(),
        .type =
            [type = preset.type()] {
                switch (type) {
                case KNSCore::SearchPreset::Type::GoBack:
                    return KNSCore::Provider::SearchPresetTypes::GoBack;
                case KNSCore::SearchPreset::Type::Popular:
                    return KNSCore::Provider::SearchPresetTypes::Popular;
                case KNSCore::SearchPreset::Type::Featured:
                    return KNSCore::Provider::SearchPresetTypes::Featured;
                case KNSCore::SearchPreset::Type::Start:
                    return KNSCore::Provider::SearchPresetTypes::Start;
                case KNSCore::SearchPreset::Type::New:
                    return KNSCore::Provider::SearchPresetTypes::New;
                case KNSCore::SearchPreset::Type::Root:
                    return KNSCore::Provider::SearchPresetTypes::Root;
                case KNSCore::SearchPreset::Type::Shelf:
                    return KNSCore::Provider::SearchPresetTypes::Shelf;
                case KNSCore::SearchPreset::Type::FolderUp:
                    return KNSCore::Provider::SearchPresetTypes::FolderUp;
                case KNSCore::SearchPreset::Type::Recommended:
                    return KNSCore::Provider::SearchPresetTypes::Recommended;
                case KNSCore::SearchPreset::Type::Subscription:
                    return KNSCore::Provider::SearchPresetTypes::Subscription;
                case KNSCore::SearchPreset::Type::AllEntries:
                    return KNSCore::Provider::SearchPresetTypes::AllEntries;
                case KNSCore::SearchPreset::Type::NoPresetType:
                    return KNSCore::Provider::SearchPresetTypes::NoPresetType;
                }
                return KNSCore::Provider::SearchPresetTypes::NoPresetType;
            }(),
        .providerId = preset.providerId(),
    };
}

inline KNSCore::Provider::CategoryMetadata categoryMetadataToLegacy(const KNSCore::CategoryMetadata &metadata)
{
    return {
        .id = metadata.id(),
        .name = metadata.name(),
        .displayName = metadata.displayName(),
    };
}

} // namespace KNSCompat
