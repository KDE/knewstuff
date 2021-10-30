/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DOWNLOADLINKINFO_H
#define DOWNLOADLINKINFO_H

#include <QObject>

#include "entryinternal.h"

/**
 * @short One downloadable item as contained within one content item
 *
 * A simple data container which wraps a KNSCore::EntryInternal::DownloadLinkInformation
 * instance and provides property accessors for each of the pieces of information stored
 * in it.
 */
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
    Q_PROPERTY(QString formattedSize READ formattedSize NOTIFY dataChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY dataChanged)

public:
    explicit DownloadLinkInfo(QObject *parent = nullptr);
    ~DownloadLinkInfo() override;

    void setData(const KNSCore::EntryInternal::DownloadLinkInformation &data);
    Q_SIGNAL void dataChanged();

    QString name() const;
    QString priceAmount() const;
    QString distributionType() const;
    QString descriptionLink() const;
    int id() const;
    bool isDownloadtypeLink() const;
    quint64 size() const;
    QString formattedSize() const;
    QString icon() const;

private:
    class Private;
    Private *d;
};

#endif // DOWNLOADLINKINFO_H
