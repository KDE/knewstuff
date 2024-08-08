// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include "searchrequest.h"
#include "searchrequest_p.h"

#include <QDebug>

using namespace KNSCore;

SearchRequest::SearchRequest(SortMode sortMode_, Filter filter_, const QString &searchTerm_, const QStringList &categories_, int page_, int pageSize_)
    : d(new SearchRequestPrivate{.sortMode = sortMode_,
                                 .filter = filter_,
                                 .searchTerm = searchTerm_,
                                 .categories = categories_,
                                 .page = page_,
                                 .pageSize = pageSize_,
                                 .id = SearchRequestPrivate::searchRequestId()})
{
}

SortMode SearchRequest::sortMode() const
{
    return d->sortMode;
}

Filter SearchRequest::filter() const
{
    return d->filter;
}

QString SearchRequest::searchTerm() const
{
    return d->searchTerm;
}

QStringList SearchRequest::categories() const
{
    return d->categories;
}

int SearchRequest::page() const
{
    return d->page;
}

int SearchRequest::pageSize() const
{
    return d->page;
}

SearchRequest SearchRequest::nextPage() const
{
    return {sortMode(), filter(), searchTerm(), categories(), page() + 1, pageSize()};
}

KNEWSTUFFCORE_EXPORT QDebug KNSCore::operator<<(QDebug dbg, const SearchRequest &search)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace();
    dbg << "SearchRequest(";
    dbg << "id: " << search.d->id << ',';
    dbg << "searchTerm: " << search.d->searchTerm << ',';
    dbg << "categories: " << search.d->categories << ',';
    dbg << "filter: " << search.d->filter << ',';
    dbg << "page: " << search.d->page << ',';
    dbg << "pageSize: " << search.d->pageSize;
    dbg << ')';
    return dbg;
}
