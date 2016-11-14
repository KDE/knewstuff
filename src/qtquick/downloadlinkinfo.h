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

#ifndef DOWNLOADLINKINFO_H
#define DOWNLOADLINKINFO_H

#include <QObject>

#include "entryinternal_p.h"

class DownloadLinkInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString priceAmount READ priceAmount NOTIFY dataChanged)
    Q_PROPERTY(QString distributionType READ distributionType NOTIFY dataChanged)
    Q_PROPERTY(QString descriptionLink READ descriptionLink NOTIFY dataChanged)
    Q_PROPERTY(int id READ id NOTIFY dataChanged)
    Q_PROPERTY(bool isDownloadtypeLink READ isDownloadtypeLink NOTIFY dataChanged)
    Q_PROPERTY(quint64 size READ size NOTIFY dataChanged)
public:
    explicit DownloadLinkInfo(QObject* parent = 0);
    virtual ~DownloadLinkInfo();

    void setData(const KNSCore::EntryInternal::DownloadLinkInformation& data);
    Q_SIGNAL void dataChanged();

    QString name() const;
    QString priceAmount() const;
    QString distributionType() const;
    QString descriptionLink() const;
    int id() const;
    bool isDownloadtypeLink() const;
    quint64 size() const;
private:
    class Private;
    Private* d;
};

#endif//DOWNLOADLINKINFO_H
