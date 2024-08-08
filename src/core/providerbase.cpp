// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "providerbase_p.h"

#include <QTimer>

namespace KNSCore
{

ProviderBase::ProviderBase(QObject *parent)
    : QObject(parent)
    , d(new ProviderBasePrivate(this))
{
}

void ProviderBase::setTagFilter(const QStringList &tagFilter)
{
    d->tagFilter = tagFilter;
    Q_EMIT tagFilterChanged();
}

QStringList ProviderBase::tagFilter() const
{
    return d->tagFilter;
}

void ProviderBase::setDownloadTagFilter(const QStringList &downloadTagFilter)
{
    d->downloadTagFilter = downloadTagFilter;
    Q_EMIT downloadTagFilterChanged();
}

QStringList ProviderBase::downloadTagFilter() const
{
    return d->downloadTagFilter;
}

} // namespace KNSCore
