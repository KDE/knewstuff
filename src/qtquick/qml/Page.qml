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
import org.kde.kirigami 2.12 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

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
        newStuffEngine.engine.fetchEntryById(entryId);
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
            if (newStuffModel.isLoadingData === false && root.view.count > 0) {
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

    titleDelegate: QtLayouts.RowLayout {
        QtLayouts.Layout.fillWidth: true
        Kirigami.Heading {
            id: title
            level: 1

            QtLayouts.Layout.fillWidth: true;
            opacity: root.isCurrentPage ? 1 : 0.4
            maximumLineCount: 1
            elide: Text.ElideRight
            text: root.title
        }
        QtControls.ButtonGroup {
            id: displayModeGroup
            buttons: [displayModeTiles, displayModeIcons]
        }
        QtControls.ToolButton {
            id: displayModeTiles
            icon.name: "view-list-details"
            onClicked: { root.viewMode = Page.ViewMode.Tiles; }
            checked: root.viewMode == Page.ViewMode.Tiles
            QtControls.ToolTip {
                text: i18nd("knewstuff5", "Tiles view mode")
            }
        }
        QtControls.ToolButton {
            id: displayModeIcons
            icon.name: "view-list-icons"
            onClicked: { root.viewMode = Page.ViewMode.Icons; }
            checked: root.viewMode == Page.ViewMode.Icons
            QtControls.ToolTip {
                text: i18nd("knewstuff5", "Icons view mode")
            }
        }
        QtControls.ToolButton {
            id: displayPreview
            icon.name: "view-preview"
            onClicked: { root.viewMode = Page.ViewMode.Preview; }
            checked: root.viewMode == Page.ViewMode.Preview
            QtControls.ToolTip {
                text: i18nd("knewstuff5", "Preview view mode")
            }
        }
        Kirigami.ActionTextField {
            id: searchField
            placeholderText: i18nd("knewstuff5", "Search...")
            focusSequence: "Ctrl+F"
            rightActions: [
                Kirigami.Action {
                    iconName: "edit-clear"
                    visible: searchField.text !== ""
                    onTriggered: {
                        searchField.text = "";
                        searchField.accepted();
                    }
                }
            ]
            onAccepted: {
                newStuffEngine.searchTerm = searchField.text;
            }
            enabled: filterCombo.currentIndex === 0
        }
    }

    footer: QtLayouts.RowLayout {
        QtControls.Label {
            text: i18n("Show:")
        }
        QtControls.ComboBox {
            id: categoriesCombo
            QtLayouts.Layout.fillWidth: true
            // Only show this combobox when there are multiple categories
            visible: count > 2
            model: newStuffEngine.categories
            textRole: "displayName"
            onCurrentIndexChanged: {
                newStuffEngine.categoriesFilter = model.data(model.index(currentIndex, 0), NewStuff.CategoriesModel.NameRole);
            }
        }
        QtControls.ComboBox {
            id: filterCombo
            QtLayouts.Layout.fillWidth: true
            model: ListModel {}
            Component.onCompleted: {
                filterCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the filter to show everything", "Everything") });
                filterCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the filter so only installed items are shown", "Installed Only") });
                filterCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the filter so only installed items with updates available are shown", "Updateable Only") });
                filterCombo.currentIndex = newStuffEngine.filter;
            }
            onCurrentIndexChanged: {
                newStuffEngine.filter = currentIndex;
            }
        }
        QtControls.ComboBox {
            id: sortCombo
            QtLayouts.Layout.fillWidth: true
            model: ListModel { }
            Component.onCompleted: {
                sortCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the sort order to based on when items were most recently updated", "Most recent first") });
                sortCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the sort order to be alphabetical based on the name", "A-Z") });
                sortCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the sort order to based on user ratings", "Highest rated first") });
                sortCombo.model.append({ text: i18ndc("knewstuff5", "List option which will set the sort order to based on number of downloads", "Most downloaded first") });
                sortCombo.currentIndex = newStuffEngine.sortOrder;
            }
            onCurrentIndexChanged: {
                newStuffEngine.sortOrder = currentIndex;
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

    Item {
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
            running: newStuffEngine.isLoading || newStuffModel.isLoadingData
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
