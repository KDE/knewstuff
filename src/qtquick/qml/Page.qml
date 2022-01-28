/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A Kirigami.Page component used for managing KNS entries
 *
 * This component is functionally equivalent to the old DownloadDialog
 * @see KNewStuff::DownloadDialog
 * @since 5.63
 */

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts
import QtGraphicalEffects 1.11 as QtEffects

import org.kde.kcm 1.2 as KCM
import org.kde.kirigami 2.19 as Kirigami

import org.kde.newstuff 1.85 as NewStuff

import "private" as Private
import "private/entrygriddelegates" as EntryGridDelegates

KCM.GridViewKCM {
    id: root;
    /**
     * @brief The configuration file which describes the application (knsrc)
     *
     * The format and location of this file is found in the documentation for
     * KNS3::DownloadDialog
     */
    property alias configFile: newStuffEngine.configFile;
    readonly property alias engine: newStuffEngine;

    /**
     * Any generic message from the NewStuff.Engine
     * @param message The message to be shown to the user
     */
    signal message(string message);
    /**
     * A message posted usually describing that whatever action a recent busy
     * message said was happening has been completed
     * @param message The message to be shown to the user
     */
    signal idleMessage(string message);
    /**
     * A message posted when the engine is busy doing something long duration
     * (usually this will be when fetching installation data)
     * @param message The message to be shown to the user
     */
    signal busyMessage(string message);
    /**
     * A message posted when something has gone wrong
     * @param message The message to be shown to the user
     */
    signal errorMessage(string message);

    /**
     * Whether or not to show the Upload... context action
     * Usually this will be bound to the engine's property which usually defines
     * this, but you can override it programmatically by setting it here.
     * @since 5.85
     * @see KNSCore::Engine::uploadEnabled
     */
    property alias showUploadAction: uploadAction.visible


    /**
     * Show the details page for a specific entry.
     * If you call this function before the engine initialisation has been completed,
     * the action itself will be postponed until that has happened.
     * @param providerId The provider ID for the entry you wish to show details for
     * @param entryId The unique ID for the entry you wish to show details for
     * @since 5.79
     */
    function showEntryDetails(providerId, entryId) {
        _showEntryDetailsThrottle.providerId = providerId;
        _showEntryDetailsThrottle.entryId = entryId;
        newStuffEngine.engine.storeSearch();

        //check if entry in question is perhaps a group, if so, load the new details.
        var theIndex = newStuffModel.indexOfEntryId(providerId, entryId);
        var type = newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.EntryTypeRole);

        if (type === NewStuff.ItemsModel.GroupEntry) {
            newStuffEngine.searchTerm = newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.PayloadRole);
        } else {
            newStuffEngine.engine.fetchEntryById(entryId);
        }

        if (newStuffEngine.isLoading) {
            _showEntryDetailsThrottle.enabled = true;
        } else {
            _showEntryDetailsThrottle.onIsLoadingDataChanged();
        }

    }
    Connections {
        id: _showEntryDetailsThrottle;
        target: newStuffModel;
        enabled: false;
        property var entryId;
        property var providerId;
        function onIsLoadingDataChanged() {
            if (newStuffModel.isLoadingData === false && root.view.count == 1) {
                _showEntryDetailsThrottle.enabled = false;
                var theIndex = newStuffModel.indexOfEntryId(_showEntryDetailsThrottle.providerId, _showEntryDetailsThrottle.entryId);
                if (theIndex > -1) {
                    pageStack.push(detailsPage, {
                        newStuffModel: newStuffModel,
                        index: theIndex,
                        name: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.NameRole),
                        author: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.AuthorRole),
                        previews: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.PreviewsRole),
                        shortSummary: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.ShortSummaryRole),
                        summary: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.SummaryRole),
                        homepage: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.HomepageRole),
                        donationLink: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.DonationLinkRole),
                        status: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.StatusRole),
                        commentsCount: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.NumberOfCommentsRole),
                        rating: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.RatingRole),
                        downloadCount: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.DownloadCountRole),
                        downloadLinks: newStuffModel.data(newStuffModel.index(theIndex, 0), NewStuff.ItemsModel.DownloadLinksRole),
                        providerId: _showEntryDetailsThrottle.providerId
                    });
                    _restoreSearchState.enabled = true;
                } else {
                    root.message(i18ndc("knewstuff5", "A message which is shown when the user attempts to display a specific entry from a specific provider, but that entry isn't found", "The entry you attempted to display, identified by the unique ID %1, could not be found.", _showEntryDetailsThrottle.entryId));
                    newStuffEngine.engine.restoreSearch();
                }
            } else if (newStuffModel.isLoadingData === false && root.view.count > 1) {
                // right now, this is only one level deep...
                _showEntryDetailsThrottle.enabled = false;
                _restoreSearchState.enabled = true;
            }
        }
    }
    Connections {
        id: _restoreSearchState;
        target: pageStack;
        enabled: false;
        function onCurrentIndexChanged() {
            if (pageStack.currentIndex === 0) {
                newStuffEngine.engine.restoreSearch();
                _restoreSearchState.enabled = false;
            }
        }
    }

    property string uninstallLabel: i18ndc("knewstuff5", "Request uninstallation of this item", "Uninstall");
    property string useLabel: engine.engine.useLabel

    property int viewMode: Page.ViewMode.Tiles
    enum ViewMode {
        Tiles,
        Icons,
        Preview
    }

    // Otherwise the first item will be focused, see BUG: 424894
    Component.onCompleted: {
        view.currentIndex = -1
    }

    title: newStuffEngine.name

    view.header: Item {
        implicitWidth: view.width - Kirigami.Units.gridUnit
        implicitHeight: Kirigami.Units.gridUnit * 3
        visible: !loadingOverlay.visible
        Kirigami.InlineMessage {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            visible: true
            text: i18n("The content available here has been uploaded by users like you, and has not been reviewed by your distributor for functionality or stability.")
        }
    }

    NewStuff.Engine {
        id: newStuffEngine;
        property string statusMessage;
        onMessage: {
            root.message(message);
            statusMessage = message;
        }
        onIdleMessage: {
            root.idleMessage(message);
            statusMessage = message;
        }
        onBusyMessage: {
            root.busyMessage(message);
            statusMessage = message;
        }
        onErrorMessage: {
            root.errorMessage(message);
            statusMessage = message;
        }
    }
    NewStuff.QuestionAsker {}
    Private.ErrorDisplayer { engine: newStuffEngine; active: root.isCurrentPage; }

    QtControls.ActionGroup { id: viewModeActionGroup }
    QtControls.ActionGroup { id: viewFilterActionGroup }
    QtControls.ActionGroup { id: viewSortingActionGroup }
    actions {
        contextualActions: [
            Kirigami.Action {
                text: {
                    if (root.viewMode == Page.ViewMode.Tiles) {
                        return i18nd("knewstuff5", "Tiles");
                    } else if (root.viewMode == Page.ViewMode.Icons) {
                        return i18nd("knewstuff5", "Icons");
                    } else {
                        return i18nd("knewstuff5", "Preview");
                    }
                }
                checkable: false
                icon.name: {
                    if (root.viewMode == Page.ViewMode.Tiles) {
                        return "view-list-details";
                    } else if (root.viewMode == Page.ViewMode.Icons) {
                        return "view-list-icons";
                    } else {
                        return "view-preview";
                    }
                }
                Kirigami.Action {
                    icon.name: "view-list-details"
                    text: i18nd("knewstuff5", "Detailed Tiles View Mode")
                    onTriggered: { root.viewMode = Page.ViewMode.Tiles; }
                    checked: root.viewMode == Page.ViewMode.Tiles
                    checkable: true
                    QtControls.ActionGroup.group: viewModeActionGroup
                }
                Kirigami.Action {
                    icon.name: "view-list-icons"
                    text: i18nd("knewstuff5", "Icons Only View Mode")
                    onTriggered: { root.viewMode = Page.ViewMode.Icons; }
                    checked: root.viewMode == Page.ViewMode.Icons
                    checkable: true
                    QtControls.ActionGroup.group: viewModeActionGroup
                }
                Kirigami.Action {
                    icon.name: "view-preview"
                    text: i18nd("knewstuff5", "Large Preview View Mode")
                    onTriggered: { root.viewMode = Page.ViewMode.Preview; }
                    checked: root.viewMode == Page.ViewMode.Preview
                    checkable: true
                    QtControls.ActionGroup.group: viewModeActionGroup
                }
            },
            Kirigami.Action {
                text: {
                    if (newStuffEngine.filter === 0) {
                        return i18nd("knewstuff5", "Everything");
                    } else if (newStuffEngine.filter === 1) {
                        return i18nd("knewstuff5", "Installed");
                    } else if (newStuffEngine.filter === 2) {
                        return i18nd("knewstuff5", "Updateable");
                    } else {
                        // then it's ExactEntryId and we want to probably just ignore that
                    }
                }
                checkable: false
                icon.name: {
                    if (newStuffEngine.filter === 0) {
                        return "package-available"
                    } else if (newStuffEngine.filter === 1) {
                        return "package-installed-updated"
                    } else if (newStuffEngine.filter === 2) {
                        return "package-installed-outdated"
                    } else {
                        // then it's ExactEntryId and we want to probably just ignore that
                    }
                }
                Kirigami.Action {
                    icon.name: "package-available"
                    text: i18ndc("knewstuff5", "List option which will set the filter to show everything", "Show All Entries")
                    checkable: true
                    checked: newStuffEngine.filter === 0
                    onTriggered: { newStuffEngine.filter = 0; }
                    QtControls.ActionGroup.group: viewFilterActionGroup
                }
                Kirigami.Action {
                    icon.name: "package-installed-updated"
                    text: i18ndc("knewstuff5", "List option which will set the filter so only installed items are shown", "Show Only Installed Entries")
                    checkable: true
                    checked: newStuffEngine.filter === 1
                    onTriggered: { newStuffEngine.filter = 1; }
                    QtControls.ActionGroup.group: viewFilterActionGroup
                }
                Kirigami.Action {
                    icon.name: "package-installed-outdated"
                    text: i18ndc("knewstuff5", "List option which will set the filter so only installed items with updates available are shown", "Show Only Updateable Entries")
                    checkable: true
                    checked: newStuffEngine.filter === 2
                    onTriggered: { newStuffEngine.filter = 2; }
                    QtControls.ActionGroup.group: viewFilterActionGroup
                }
            },
            Kirigami.Action {
                text: {
                    if (newStuffEngine.sortOrder === 0) {
                        return i18nd("knewstuff5", "Recent");
                    } else if (newStuffEngine.sortOrder === 1) {
                        return i18nd("knewstuff5", "Alphabetical");
                    } else if (newStuffEngine.sortOrder === 2) {
                        return i18nd("knewstuff5", "Rating");
                    } else if (newStuffEngine.sortOrder === 3) {
                        return i18nd("knewstuff5", "Downloads");
                    } else {
                    }
                }
                checkable: false
                icon.name: {
                    if (newStuffEngine.sortOrder === 0) {
                        return "change-date-symbolic";
                    } else if (newStuffEngine.sortOrder === 1) {
                        return "sort-name";
                    } else if (newStuffEngine.sortOrder === 2) {
                        return "rating";
                    } else if (newStuffEngine.sortOrder === 3) {
                        return "download";
                    } else {
                    }
                }
                Kirigami.Action {
                    icon.name: "change-date-symbolic"
                    text: i18ndc("knewstuff5", "List option which will set the sort order to based on when items were most recently updated", "Show Most Recent First")
                    checkable: true
                    checked: newStuffEngine.sortOrder === 0
                    onTriggered: { newStuffEngine.sortOrder = 0; }
                    QtControls.ActionGroup.group: viewSortingActionGroup
                }
                Kirigami.Action {
                    icon.name: "sort-name"
                    text: i18ndc("knewstuff5", "List option which will set the sort order to be alphabetical based on the name", "Sort Alphabetically By Name")
                    checkable: true
                    checked: newStuffEngine.sortOrder === 1
                    onTriggered: { newStuffEngine.sortOrder = 1; }
                    QtControls.ActionGroup.group: viewSortingActionGroup
                }
                Kirigami.Action {
                    icon.name: "rating"
                    text: i18ndc("knewstuff5", "List option which will set the sort order to based on user ratings", "Show Highest Rated First")
                    checkable: true
                    checked: newStuffEngine.sortOrder === 2
                    onTriggered: { newStuffEngine.sortOrder = 2; }
                    QtControls.ActionGroup.group: viewSortingActionGroup
                }
                Kirigami.Action {
                    icon.name: "download"
                    text: i18ndc("knewstuff5", "List option which will set the sort order to based on number of downloads", "Show Most Downloaded First")
                    checkable: true
                    checked: newStuffEngine.sortOrder === 3
                    onTriggered: { newStuffEngine.sortOrder = 3; }
                    QtControls.ActionGroup.group: viewSortingActionGroup
                }
            },
            Kirigami.Action {
                id: uploadAction
                text: i18nd("knewstuff5", "Upload...")
                tooltip: i18nd("knewstuff5", "Learn how to add your own hot new stuff to this list")
                iconName: "upload-media"
                visible: root.showUploadAction && newStuffEngine.engine.uploadEnabled
                onTriggered: {
                    pageStack.push(uploadPage);
                }
            },
            Kirigami.Action {
                text: i18nd("knewstuff5", "Go to...")
                iconName: "go-next";
                id: searchModelActions;
                visible: children.length > 0;
            },
            Kirigami.Action {
                text: i18nd("knewstuff5", "Search...")
                iconName: "system-search";
                displayHint: Kirigami.DisplayHint.KeepVisible
                displayComponent: Kirigami.SearchField {
                    enabled: engine.isValid
                    id: searchField
                    focusSequence: "Ctrl+F"
                    placeholderText: i18nd("knewstuff5", "Search...")
                    onAccepted: { newStuffEngine.searchTerm = searchField.text; }
                    Component.onCompleted: if (!Kirigami.InputMethod.willShowOnActive) {
                        forceActiveFocus();
                    }
                }
            }
        ]
    }

    Instantiator {
        id: searchPresetInstatiator
        model: newStuffEngine.searchPresetModel
        Kirigami.Action {
            text: model.displayName
            iconName: model.iconName
            property int indexEntry: index;
            onTriggered: {
                var curIndex = newStuffEngine.searchPresetModel.index(indexEntry, 0);
                newStuffEngine.searchPresetModel.loadSearch(curIndex);
            }
        }
        onObjectAdded: { searchModelActions.children.push(object); }
    }

    Connections {
        target: newStuffEngine.searchPresetModel
        function onModelReset() { searchModelActions.children = []; }
    }


    // Only show this footer when there are multiple categories
    // nb: This would be more sensibly done by just setting the footer item visibility,
    // but that causes holes in the layout. This works, however, so we'll just deal with that.
    footer: categoriesCombo.count > 2 ? categoriesFooter : null
    readonly property Item categoriesFooter: QtLayouts.RowLayout {
        width: parent ? parent.width - Kirigami.Units.smallSpacing * 2 : 0
        x: Kirigami.Units.smallSpacing // Not super keen on this, but it's what the Page does for moving stuff around internally, so let's be consistent...
        QtControls.Label {
            text: i18nd("knewstuff5", "Category:")
        }
        QtControls.ComboBox {
            id: categoriesCombo
            QtLayouts.Layout.fillWidth: true
            model: newStuffEngine.categories
            textRole: "displayName"
            onCurrentIndexChanged: {
                newStuffEngine.categoriesFilter = model.data(model.index(currentIndex, 0), NewStuff.CategoriesModel.NameRole);
            }
        }
    }

    view.model: NewStuff.ItemsModel {
        id: newStuffModel;
        engine: newStuffEngine;
    }
    NewStuff.DownloadItemsSheet {
        id: downloadItemsSheet
        onItemPicked: {
            newStuffModel.installItem(entryId, downloadItemId);
        }
    }

    view.implicitCellWidth: root.viewMode == Page.ViewMode.Tiles ? Kirigami.Units.gridUnit * 30 : (root.viewMode == Page.ViewMode.Preview ? Kirigami.Units.gridUnit * 25 : Kirigami.Units.gridUnit * 10)
    view.implicitCellHeight: root.viewMode == Page.ViewMode.Tiles ? Math.round(view.implicitCellWidth / 3) : (root.viewMode == Page.ViewMode.Preview ? Kirigami.Units.gridUnit * 25 : Math.round(view.implicitCellWidth / 1.6) + Kirigami.Units.gridUnit*2)
    view.delegate: root.viewMode == Page.ViewMode.Tiles ? tileDelegate : (root.viewMode == Page.ViewMode.Preview ? bigPreviewDelegate : thumbDelegate)

    Component {
        id: bigPreviewDelegate
        EntryGridDelegates.BigPreviewDelegate { }
    }
    Component {
        id: tileDelegate
        EntryGridDelegates.TileDelegate  {
            useLabel: root.useLabel
            uninstallLabel: root.uninstallLabel
        }
    }
    Component {
        id: thumbDelegate
        EntryGridDelegates.ThumbDelegate {
            useLabel: root.useLabel
            uninstallLabel: root.uninstallLabel
        }
    }

    Component {
        id: detailsPage;
        NewStuff.EntryDetails { }
    }
    Component {
        id: uploadPage
        NewStuff.UploadPage {
            engine: newStuffEngine
        }
    }

    Item {
        id: loadingOverlay
        anchors.fill: parent
        opacity: (newStuffEngine.isLoading || newStuffModel.isLoadingData) ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
        visible: opacity > 0
        Rectangle {
            anchors.fill: parent
            color: Kirigami.Theme.backgroundColor
            opacity: 0.7
        }
        QtControls.BusyIndicator {
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.verticalCenter
                bottomMargin: Kirigami.Units.largeSpacing
            }
            running: (newStuffEngine.isLoading || newStuffModel.isLoadingData)  && newStuffEngine.isValid
        }
        QtControls.Label {
            anchors {
                top: parent.verticalCenter
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.largeSpacing
            }
            horizontalAlignment: Text.AlignHCenter
            text: newStuffEngine.isLoading ? newStuffEngine.statusMessage :
            i18ndc("knewstuff5", "A text shown beside a busy indicator suggesting that data is being fetched", "Loading more...")
        }
    }
    Kirigami.PlaceholderMessage {
         anchors.centerIn: parent
         anchors.left: parent.left
         anchors.right: parent.right
         anchors.margins: Kirigami.Units.largeSpacing

         visible: newStuffEngine.isLoading === false && newStuffModel.isLoadingData === false && view.count === 0

         text: i18ndc("knewstuff5", "A message shown when there are no entries in the list, and when it is not trying to load anything", "There is no hot new stuff to get here")
     }
}
