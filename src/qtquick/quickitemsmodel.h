/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
 * You might also look at NewStuffList, NewStuffItem, and the other items, to see some more
 * detail on what can be done with the data.
 *
 * @see NewStuffList
 * @see NewStuffItem
 * @see NewStuffPage
 * @see NewStuffEntryDetails
 * @see NewStuffEntryComments
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
    /**
     * The NewStuffQuickEngine to show items from
     */
    Q_PROPERTY(QObject *engine READ engine WRITE setEngine NOTIFY engineChanged)
    /**
     * Whether or not the model is fetching information from a remote location
     * @since 5.65
     */
    Q_PROPERTY(bool isLoadingData READ isLoadingData NOTIFY isLoadingDataChanged)
public:
    explicit ItemsModel(QObject *parent = nullptr);
    ~ItemsModel() override;

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
        StatusRole,
        CommentsModelRole,
        EntryTypeRole
    };
    Q_ENUM(Roles)
    enum ItemStatus {
        InvalidStatus,
        DownloadableStatus,
        InstalledStatus,
        UpdateableStatus,
        DeletedStatus,
        InstallingStatus,
        UpdatingStatus,
    };
    Q_ENUM(ItemStatus)

    /**
     * Represents whether the current entry is an actual catalog entry,
     * or an entry that represents a set of entries.
     * @since 5.83
     */
    enum EntryType { CatalogEntry = 0, GroupEntry };

    Q_ENUM(EntryType)
    // The lists in OCS are one-indexed, and that isn't how one usually does things in C++.
    // Consequently, this enum removes what would seem like magic numbers from the code, and
    // makes their meaning more explicit.
    enum LinkId {
        AutoDetectLinkId = -1,
        FirstLinkId = 1,
    };
    Q_ENUM(LinkId)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QObject *engine() const;
    void setEngine(QObject *newEngine);
    Q_SIGNAL void engineChanged();

    /**
     * Get the index of an entry based on that entry's unique ID
     * @param providerId The provider inside of which you wish to search for an entry
     * @param entryId The unique ID within the given provider of the entry you want to know the index of
     * @return The index of the entry. In case the entry is not found, -1 is returned
     * @see KNSCore::EntryInternal::uniqueId()
     * @since 5.79
     */
    Q_INVOKABLE int indexOfEntryId(const QString &providerId, const QString &entryId);

    /**
     * Whether or not the model is fetching information from a remote location
     * @since 5.65
     */
    bool isLoadingData() const;
    /**
     * Fired when the isLoadingData value changes
     * @since 5.65
     */
    Q_SIGNAL void isLoadingDataChanged();

    /**
     * @brief This will install (or update, if already installed) the item at the given index
     *
     * There are no side effects of this function if it is called on an item which cannot be
     * installed or updated (that is, if the status is not one such that these are possible,
     * the function will simply return without performing any actions)
     *
     * @param index The index of the item to install or update
     * @param linkId The download item to install. If this is -1, it is assumed to be an update with an unknown payload, and a number of heuristics will be
     * applied by the engine
     * @see Engine::downloadLinkLoaded implementation for details
     * @see LinkId
     */
    Q_INVOKABLE void installItem(int index, int linkId);
    /**
     * @brief This will request an update of the given item
     *
     * There are no side effects of this function if it is called on an item which is not
     * in an updateable state (that is, nothing will happen if this is called on an item
     * which is not already installed, or on an installed item which does not have updates
     * available).
     *
     * @param index The index of the item you wish to update
     * @since 5.69
     */
    Q_INVOKABLE void updateItem(int index);
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

    /**
     * @brief Run the adoption command on an already installed item
     *
     * @note This will simply fail quietly if the item is not installed
     *
     * @param index The intex of the item to be adopted
     */
    Q_INVOKABLE void adoptItem(int index);

    /**
     * @brief Fired when an entry's data changes
     *
     * @param index The index of the item which has changed
     */
    Q_SIGNAL void entryChanged(int index);

private:
    class Private;
    Private *d;
};
Q_DECLARE_METATYPE(ItemsModel::ItemStatus)

#endif // ITEMSMODEL_H
