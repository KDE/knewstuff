/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts

import org.kde.kirigami 2.12 as Kirigami

import org.kde.newstuff as NewStuff

import ".." as Private

Private.GridTileDelegate {
    id: component
    property var entry: model.entry
    property string useLabel
    property string uninstallLabel
    function showDetails() {
        if (entry.entryType == NewStuff.Entry.GroupEntry) {
            newStuffEngine.storeSearch();
            newStuffEngine.searchTerm = model.payload;
        } else {
            pageStack.push(detailsPage, {
                newStuffModel: GridView.view.model,
                entry,
            });
        }
    }
    actions: [
        Kirigami.Action {
            text: component.useLabel
            icon.name: "dialog-ok-apply"
            onTriggered: source => {
                newStuffModel.adoptItem(model.index);
            }
            enabled: (entry.status == NewStuff.Entry.Installed || entry.status == NewStuff.Entry.Updateable) && newStuffEngine.hasAdoptionCommand
            visible: enabled
        },
        Kirigami.Action {
            text: model.downloadLinks.length === 1 ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install") : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…")
            icon.name: "install"
            onTriggered: source => {
                if (model.downloadLinks.length === 1) {
                    newStuffEngine.install(entry, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = model.downloadLinks;
                    downloadItemsSheet.entry = entry;
                    downloadItemsSheet.open();
                }
            }
            enabled: entry.status == NewStuff.Entry.Downloadable || entry.status == NewStuff.Entry.Deleted
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "update-none"
            onTriggered: source => {
                newStuffEngine.install(entry, NewStuff.ItemsModel.AutoDetectLinkId);
            }
            enabled: entry.status == NewStuff.Entry.Updateable
            visible: enabled
        },
        Kirigami.Action {
            text: component.uninstallLabel
            icon.name: "edit-delete"
            onTriggered: source => {
                newStuffEngine.uninstall(model.entry);
            }
            enabled: entry.status == NewStuff.Entry.Installed|| entry.status == NewStuff.Entry.Updateable
            visible: enabled && hovered
        }
    ]
    thumbnailArea: tilePreview
    thumbnailAvailable: model.previewsSmall.length > 0
    tile: Item {
        anchors {
            fill: parent
            margins: Kirigami.Units.smallSpacing
        }
        QtLayouts.GridLayout {
            anchors.fill: parent
            columns: 2
            QtLayouts.ColumnLayout {
                QtLayouts.Layout.minimumWidth: view.implicitCellWidth / 5
                QtLayouts.Layout.maximumWidth: view.implicitCellWidth / 5
                Item {
                    QtLayouts.Layout.fillWidth: true
                    QtLayouts.Layout.minimumHeight: width
                    QtLayouts.Layout.maximumHeight: width
                    Kirigami.ShadowedRectangle {
                        visible: tilePreview.status == Image.Ready
                        anchors.centerIn: tilePreview
                        width: Math.min(tilePreview.paintedWidth, tilePreview.width)
                        height: Math.min(tilePreview.paintedHeight, tilePreview.height)
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        shadow.size: Kirigami.Units.largeSpacing
                        shadow.color: Qt.rgba(0, 0, 0, 0.3)
                    }
                    Image {
                        id: tilePreview
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        source: thumbnailAvailable ? model.previewsSmall[0] : ""
                        anchors {
                            fill: parent
                            margins: Kirigami.Units.smallSpacing
                        }
                        verticalAlignment: Image.AlignTop
                    }
                    Kirigami.Icon {
                        id: updateAvailableBadge
                        opacity: (entry.status == NewStuff.Entry.Updateable) ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                        anchors {
                            top: parent.top
                            left: parent.left
                            margins: -Kirigami.Units.smallSpacing
                        }
                        height: Kirigami.Units.iconSizes.smallMedium
                        width: height
                        source: "package-installed-outdated"
                    }
                    Kirigami.Icon {
                        id: installedBadge
                        opacity: (entry.status == NewStuff.Entry.Installed) ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                        anchors {
                            top: parent.top
                            left: parent.left
                            margins: -Kirigami.Units.smallSpacing
                        }
                        height: Kirigami.Units.iconSizes.smallMedium
                        width: height
                        source: "package-installed-updated"
                    }
                }
                Item {
                    QtLayouts.Layout.fillHeight: true
                }
            }
            QtLayouts.ColumnLayout {
                QtLayouts.Layout.fillWidth: true
                QtLayouts.Layout.fillHeight: true
                Kirigami.Heading {
                    QtLayouts.Layout.fillWidth: true
                    elide: Text.ElideRight
                    level: 3
                    text: entry.name
                }
                Kirigami.Heading {
                    QtLayouts.Layout.fillWidth: true
                    elide: Text.ElideRight
                    level: 4
                    textFormat: Text.StyledText
                    text: i18ndc("knewstuff6", "Subheading for the tile view, located immediately underneath the name of the item", "By <i>%1</i>", entry.author.name)
                }
                QtControls.Label {
                    QtLayouts.Layout.fillWidth: true
                    QtLayouts.Layout.fillHeight: true
                    wrapMode: Text.Wrap
                    text: entry.shortSummary.length > 0 ? entry.shortSummary : entry.summary
                    elide: Text.ElideRight
                    clip: true // We are dealing with content over which we have very little control. Sometimes that means being a bit abrupt.
                }
            }
            Private.Rating {
                QtLayouts.Layout.fillWidth: true
                rating: entry.rating
                visible: entry.entryType == NewStuff.Entry.CatalogEntry
            }
            Kirigami.Heading {
                QtLayouts.Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                level: 5
                elide: Text.ElideRight
                text: i18ndc("knewstuff6", "The number of times the item has been downloaded", "%1 downloads", entry.downloadCount)
                visible: entry.entryType == NewStuff.Entry.CatalogEntry
            }
        }
        FeedbackOverlay {
            anchors.fill: parent
            newStuffModel: component.GridView.view.model
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: mouse => {
                component.showDetails();
            }
        }
    }
}
