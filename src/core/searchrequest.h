// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QStringList>

#include "knewstuffcore_export.h"

namespace KNSCore
{
Q_NAMESPACE_EXPORT(KNEWSTUFFCORE_EXPORT)

struct SearchRequestPrivate;

/**
 * @since 6.5
 */
enum class SortMode {
    Newest,
    Alphabetical,
    Rating,
    Downloads,
};
Q_ENUM_NS(SortMode)

/**
 * @since 6.5
 */
enum class Filter {
    None,
    Installed,
    Updates,
    ExactEntryId,
};
Q_ENUM_NS(Filter)

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug, const class SearchRequest &);

/**
 * @brief A search request
 * @since 6.5
 */
class KNEWSTUFFCORE_EXPORT SearchRequest
{
public:
    SearchRequest(SortMode sortMode_ = KNSCore::SortMode::Downloads,
                  Filter filter_ = KNSCore::Filter::None,
                  const QString &searchTerm_ = {},
                  const QStringList &categories_ = {},
                  int page_ = -1,
                  int pageSize_ = 20);

    [[nodiscard]] SortMode sortMode() const;
    [[nodiscard]] Filter filter() const;
    [[nodiscard]] QString searchTerm() const;
    [[nodiscard]] QStringList categories() const;
    [[nodiscard]] int page() const;
    [[nodiscard]] int pageSize() const;
    [[nodiscard]] SearchRequest nextPage() const;

private:
    friend class ResultsStream;
    friend class AtticaProvider;
    friend class AtticaRequester;
    friend class StaticXmlProvider;
    friend class OPDSProvider;
    friend class Cache2;
    friend QDebug KNSCore::operator<<(QDebug, const SearchRequest &);
    std::shared_ptr<SearchRequestPrivate> d;
};

} // namespace KNSCore
