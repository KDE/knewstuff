/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ITEMSMODEL_H
#define ITEMSMODEL_H

#include <QAbstractListModel>

#include <memory>

class ItemsModelPrivate;

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
            engine: newStuffEngine
        }
        NewStuff.Engine {
            id: newStuffEngine
            configFile: "/some/filesystem/location/wallpaper.knsrc"
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
    Q_PROPERTY(QObject *engine READ engine WRITE setEngine NOTIFY engineChanged REQUIRED)
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
        EntryTypeRole,
        EntryRole,
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
     * @see KNSCore::Entry::uniqueId()
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
     * @brief Fired when an entry's data changes
     *
     * @param index The index of the item which has changed
     */
    Q_SIGNAL void entryChanged(int index);

private:
    const std::unique_ptr<ItemsModelPrivate> d;
};

#endif // ITEMSMODEL_H
