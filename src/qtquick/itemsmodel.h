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

#ifndef ITEMSMODEL_H
#define ITEMSMODEL_H

#include <QAbstractListModel>

class ItemsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* engine READ engine WRITE setEngine NOTIFY engineChanged)
public:
    explicit ItemsModel(QObject* parent = 0);
    virtual ~ItemsModel();

    enum Roles {
        NameRole = Qt::UserRole + 1,
        UniqueIdRole,
        CategoryRole,
        HomepageRole,
        AuthorRole,
        LicenseRole,
        ShortSummaryRole,
        SummaryRole,
        ChangelogRole,
        VersionRole,
        ReleaseDateRole,
        UpdateVersionRole,
        UpdateReleaseDateRole,
        PayloadRole,
        PreviewsSmallRole, ///@< this will return a list here, rather than be tied so tightly to the remote api
        PreviewsRole, ///@< this will return a list here, rather than be tied so tightly to the remote api
        InstalledFilesRole,
        UnInstalledFilesRole,
        RatingRole,
        NumberOfCommentsRole,
        DownloadCountRole,
        NumberFansRole,
        NumberKnowledgebaseEntriesRole,
        KnowledgebaseLinkRole,
        DownloadLinksRole,
        DonationLinkRole,
        ProviderIdRole,
        SourceRole
    };

    virtual QHash<int, QByteArray> roleNames() const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QObject* engine() const;
    void setEngine(QObject* newEngine);
    Q_SIGNAL void engineChanged();
private:
    class Private;
    Private* d;
};

#endif//ITEMSMODEL_H
