// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include "provider.h"
#include "searchrequest.h"

namespace KNSCore
{

struct SearchRequestPrivate {
    KNSCore::SortMode sortMode;
    KNSCore::Filter filter;
    QString searchTerm;
    QStringList categories;
    int page;
    int pageSize;
    quint64 id;

    [[nodiscard]] QString hashForRequest() const
    {
        return QString::number((int)sortMode) + QLatin1Char(',') + searchTerm + QLatin1Char(',') + categories.join(QLatin1Char('-')) + QLatin1Char(',')
            + QString::number(page) + QLatin1Char(',') + QString::number(pageSize);
    }

    bool operator==(const SearchRequestPrivate &other) const
    {
        return id == other.id;
    }

    [[nodiscard]] static quint64 searchRequestId()
    {
        static quint64 id = 0;
        return id++;
    }
};

KNSCore::SearchRequest searchRequestFromLegacy(const KNSCore::Provider::SearchRequest &request);

} // namespace KNSCore
