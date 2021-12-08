/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "downloadlinkinfo.h"
#include <KFormat>
#include <QMimeDatabase>

static const KFormat formatter;

class DownloadLinkInfoPrivate
{
public:
    DownloadLinkInfoPrivate()
        : id(0)
        , isDownloadtypeLink(true)
        , size(0)
    {
    }

    QString name;
    QString priceAmount;
    QString distributionType;
    QString descriptionLink;
    int id;
    bool isDownloadtypeLink;
    quint64 size;
    QString mimeType;
    QString icon;
};

DownloadLinkInfo::DownloadLinkInfo(QObject *parent)
    : QObject(parent)
    , d(new DownloadLinkInfoPrivate)
{
}

DownloadLinkInfo::~DownloadLinkInfo() = default;

void DownloadLinkInfo::setData(const KNSCore::EntryInternal::DownloadLinkInformation &data)
{
    d->name = data.name;
    d->priceAmount = data.priceAmount;
    d->distributionType = data.distributionType;
    d->descriptionLink = data.descriptionLink;
    d->id = data.id;
    d->isDownloadtypeLink = data.isDownloadtypeLink;
    d->size = data.size;
    QMimeDatabase db;
    for (QString string : data.tags) {
        if (string.startsWith(QStringLiteral("data##mimetype="))) {
            d->mimeType = string.split(QStringLiteral("=")).last();
        }
    }
    d->icon = db.mimeTypeForName(d->mimeType).iconName();
    if (d->icon.isEmpty()) {
        d->icon = db.mimeTypeForName(d->mimeType).genericIconName();
    }
    if (d->icon.isEmpty()) {
        d->icon = QStringLiteral("download");
    }
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

QString DownloadLinkInfo::formattedSize() const
{
    if (d->size == 0) {
        return QString();
    }
    return formatter.formatByteSize(d->size * 1000);
}

QString DownloadLinkInfo::icon() const
{
    return d->icon;
}
