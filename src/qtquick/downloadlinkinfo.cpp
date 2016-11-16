/*
 * Copyright (C) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "downloadlinkinfo.h"

class DownloadLinkInfo::Private
{
public:
    Private()
        : id(0)
        , isDownloadtypeLink(true)
        , size(0)
    {}

    QString name;
    QString priceAmount;
    QString distributionType;
    QString descriptionLink;
    int id;
    bool isDownloadtypeLink;
    quint64 size;
};

DownloadLinkInfo::DownloadLinkInfo(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

DownloadLinkInfo::~DownloadLinkInfo()
{
    delete d;
}

void DownloadLinkInfo::setData(const KNSCore::EntryInternal::DownloadLinkInformation& data)
{
    d->name = data.name;
    d->priceAmount = data.priceAmount;
    d->distributionType = data.distributionType;
    d->descriptionLink = data.descriptionLink;
    d->id = data.id;
    d->isDownloadtypeLink = data.isDownloadtypeLink;
    d->size = data.size;
    Q_EMIT dataChanged();
}

QString DownloadLinkInfo::name() const
{
    return d->name;
}

QString DownloadLinkInfo::priceAmount() const
{
    return d->priceAmount;
}

QString DownloadLinkInfo::distributionType() const
{
    return d->distributionType;
}

QString DownloadLinkInfo::descriptionLink() const
{
    return d->descriptionLink;
}

int DownloadLinkInfo::id() const
{
    return d->id;
}

bool DownloadLinkInfo::isDownloadtypeLink() const
{
    return d->isDownloadtypeLink;
}

quint64 DownloadLinkInfo::size() const
{
    return d->size;
}
