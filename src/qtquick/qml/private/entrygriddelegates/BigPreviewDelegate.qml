/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts
import QtGraphicalEffects 1.11 as QtEffects

import org.kde.kirigami 2.12 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

import ".." as Private

Private.GridTileDelegate {
    id: component
    actionsAnchors.topMargin: bigPreview.height + Kirigami.Units.smallSpacing * 2
    function showDetails() {
        pageStack.push(detailsPage, {
            newStuffModel: GridView.view.model,
            index: model.index,
            name: model.name,
            author: model.author,
            previews: model.previews,
            shortSummary: model.shortSummary,
            summary: model.summary,
            homepage: model.homepage,
            donationLink: model.donationLink,
            status: model.status,
            commentsCount: model.numberOfComments,
            rating: model.rating,
            downloadCount: model.downloadCount,
            downloadLinks: model.downloadLinks,
            providerId: model.providerId
        });
    }
    actions: [
        Kirigami.Action {
            text: root.useLabel
            iconName: "dialog-ok-apply"
            onTriggered: { newStuffModel.adoptItem(model.index); }
            enabled: (model.status == NewStuff.ItemsModel.InstalledStatus || model.status == NewStuff.ItemsModel.UpdateableStatus) && newStuffEngine.hasAdoptionCommand
            visible: enabled
        },
        Kirigami.Action {
            text: model.downloadLinks.length === 1 ? i18ndc("knewstuff5", "Request installation of this item, available when there is exactly one downloadable item", "Install") : i18ndc("knewstuff5", "Show installation options, where there is more than one downloadable item", "Install...");
            iconName: "install"
            onTriggered: {
                if (model.downloadLinks.length === 1) {
                    newStuffModel.installItem(model.index, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = model.downloadLinks;
                    downloadItemsSheet.entryId = model.index;
                    downloadItemsSheet.open();
                }
            }
            enabled: model.status == NewStuff.ItemsModel.DownloadableStatus || model.status == NewStuff.ItemsModel.DeletedStatus;
            visible: enabled;
        },
        Kirigami.Action {
            text: i18ndc("knewstuff5", "Request updating of this item", "Update");
            iconName: "update-none"
            onTriggered: { newStuffModel.updateItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.UpdateableStatus;
            visible: enabled;
        },
        Kirigami.Action {
            text: root.uninstallLabel
            iconName: "edit-delete"
            onTriggered: { newStuffModel.uninstallItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.InstalledStatus || model.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled;
        }
    ]
    thumbnailAvailable: model.previewsSmall.length > 0
    tile: Item {
        anchors {
            fill: parent
            margins: Kirigami.Units.smallSpacing
        }
        QtLayouts.ColumnLayout {
            anchors.fill: parent;
            Item {
                QtLayouts.Layout.fillWidth: true
                QtLayouts.Layout.fillHeight: true
                QtLayouts.Layout.minimumHeight: width / 5
                QtLayouts.Layout.maximumHeight: width / 1.8
                Kirigami.ShadowedRectangle {
                    visible: bigPreview.status == Image.Ready
                    anchors.centerIn: bigPreview;
                    width: Math.min(bigPreview.paintedWidth, bigPreview.width);
                    height: Math.min(bigPreview.paintedHeight, bigPreview.height);
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    shadow.size: 10
                    shadow.color: Qt.rgba(0, 0, 0, 0.3)
                }
                Image {
                    id: bigPreview
                    asynchronous: true;
                    fillMode: Image.PreserveAspectCrop;
                    source: thumbnailAvailable ? model.previews[0] : "";
                    anchors.fill: parent
                }
                Kirigami.Icon {
                    id: updateAvailableBadge;
                    opacity: (model.status == NewStuff.ItemsModel.UpdateableStatus) ? 1 : 0;
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
                    anchors {
                        top: parent.top;
                        left: parent.left;
                    }
                    height: Kirigami.Units.iconSizes.medium;
                    width: height;
                    source: "package-installed-outdated";
                }
                Kirigami.Icon {
                    id: installedBadge;
                    opacity: (model.status == NewStuff.ItemsModel.InstalledStatus) ? 1 : 0;
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
                    anchors {
                        top: parent.top;
                        left: parent.left;
                    }
                    height: Kirigami.Units.iconSizes.medium;
                    width: height;
                    source: "package-installed-updated";
                }
            }
            Private.Rating {
                QtLayouts.Layout.fillWidth: true
                rating: model.rating
            }
            Kirigami.Heading {
                QtLayouts.Layout.fillWidth: true
                level: 5
                elide: Text.ElideRight
                text: i18ndc("knewstuff5", "The number of times the item has been downloaded", "%1 downloads", model.downloadCount)
            }
            Kirigami.Heading {
                QtLayouts.Layout.fillWidth: true
                elide: Text.ElideRight
                level: 3
                text: model.name
            }
            Kirigami.Heading {
                QtLayouts.Layout.fillWidth: true
                elide: Text.ElideRight
                level: 4
                textFormat: Text.StyledText
                text: i18ndc("knewstuff5", "Subheading for the tile view, located immediately underneath the name of the item", "By <i>%1</i>", model.author.name)
            }
            QtControls.Label {
                QtLayouts.Layout.fillWidth: true
                QtLayouts.Layout.fillHeight: true
                QtLayouts.Layout.minimumHeight: Kirigami.Units.gridUnit
                QtLayouts.Layout.maximumHeight: Kirigami.Units.gridUnit * 3
                wrapMode: Text.Wrap
                text: model.shortSummary.length > 0 ? model.shortSummary : model.summary
                elide: Text.ElideRight
            }
            clip: true // We are dealing with content over which we have very little control. Sometimes that means being a bit abrupt.
        }
        FeedbackOverlay {
            anchors.fill: parent
            newStuffModel: component.GridView.view.model
        }
        MouseArea {
            anchors.fill: parent;
            cursorShape: Qt.PointingHandCursor;
            onClicked: component.showDetails();
        }
    }
}
