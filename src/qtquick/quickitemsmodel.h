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

/**
 * @short A model which shows the contents found in an Engine
 *
 * Use an instance of this model to show the content items represented by the configuration
 * file passed to an engine. The following sample assumes you are using the Engine component,
 * however it is also possible to pass a KNSCore::Engine instance created from C++ to this
 * property, if you have specific requirements not covered by the convenience component.
 *
 * Most data in the model is simple, but the DownloadLinks role will return a list of
 * DownloadLinkInfo entries, which you will need to manage in some way.
 *
 * You might also look at NewStuffList and NewStuffItem to see some more detail on what can be
 * done with the data.
 *
 * @see NewStuffList
 * @see NewStuffItem
 *
 * \code
    import org.kde.newstuff 1.0 as NewStuff
    Item {
        NewStuff.ItemsModel {
            id: newStuffModel;
            engine: newStuffEngine.engine;
        }
        NewStuff.Engine {
            id: newStuffEngine;
            configFile: "/some/filesystem/location/wallpaper.knsrc";
            onMessage: console.log("KNS Message: " + message);
            onIdleMessage: console.log("KNS Idle: " + message);
            onBusyMessage: console.log("KNS Busy: " + message);
            onErrorMessage: console.log("KNS Error: " + message);
        }
    }
    \endcode
 */
class ItemsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* engine READ engine WRITE setEngine NOTIFY engineChanged)
public:
    explicit ItemsModel(QObject* parent = nullptr);
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
        SourceRole,
        StatusRole
    };
    enum ItemStatus {
        InvalidStatus,
        DownloadableStatus,
        InstalledStatus,
        UpdateableStatus,
        DeletedStatus,
        InstallingStatus,
        UpdatingStatus
    };
    Q_ENUM(ItemStatus)

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex & parent) const override;
    void fetchMore(const QModelIndex & parent) override;

    QObject* engine() const;
    void setEngine(QObject* newEngine);
    Q_SIGNAL void engineChanged();

    /**
     * @brief This will install (or update, if already installed) the item at the given index
     *
     * There are no side effects of this function if it is called on an item which cannot be
     * installed or updated (that is, if the status is not one such that these are possible,
     * the function will simply return without performing any actions)
     *
     * @param index The index of the item to install or update
     */
    Q_INVOKABLE void installItem(int index);
    /**
     * @brief Uninstall an already installed item
     *
     * There are no side effects of this function if it is called on an item which cannot be
     * uninstalled (that is, if the status is not one such that this is possible,
     * the function will simply return without performing any actions)
     *
     * @param index The index of the item to be uninstalled
     */
    Q_INVOKABLE void uninstallItem(int index);
private:
    class Private;
    Private* d;
};
Q_DECLARE_METATYPE(ItemsModel::ItemStatus)

#endif//ITEMSMODEL_H
