// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QString>

#include "knewstuffcore_export.h"
#include "searchrequest.h"

namespace KNSCore
{

class SearchPresetPrivate;

/**
 * Describes a search request that may come from the provider.
 * This is used by the OPDS provider to handle the different urls.
 * @since 6.5
 */
class KNEWSTUFFCORE_EXPORT SearchPreset
{
public:
    /**
     * @brief The SearchPresetTypes enum
     * the preset type enum is a helper to identify the kind of label and icon
     * the search preset should have if none are found.
     * @since 6.5
     */
    enum class Type {
        NoPresetType = 0,
        GoBack, ///< preset representing the previous search.
        Root, ///< preset indicating a root directory.
        Start, ///< preset indicating the first entry.
        Popular, ///< preset indicating popular items.
        Featured, ///< preset for featured items.
        Recommended, ///< preset for recommended. This may be customized by the server per user.
        Shelf, ///< preset indicating previously acquired items.
        Subscription, ///< preset indicating items that the user is subscribed to.
        New, ///< preset indicating new items.
        FolderUp, ///< preset indicating going up in the search result hierarchy.
        AllEntries, ///< preset indicating all possible entries, such as a crawlable list. Might be intense to load.
    };

    [[nodiscard]] SearchRequest request() const;
    [[nodiscard]] QString displayName() const;
    [[nodiscard]] QString iconName() const;
    [[nodiscard]] Type type() const;
    [[nodiscard]] QString providerId() const; // not all providers can handle all search requests.

private:
    friend class OPDSProviderPrivate;
    SearchPreset(SearchPresetPrivate *dptr);
    std::shared_ptr<SearchPresetPrivate> d;
};

} // namespace KNSCore
