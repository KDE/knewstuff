/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts
import QtGraphicalEffects 1.11 as QtEffects

import org.kde.kcm 1.2 as KCM
import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

import ".." as Private

KCM.GridDelegate {
    id: component
    property string useLabel
    property string uninstallLabel
    text: model.name
//         onClicked: pageStack.push(detailsPage, {
//             name: model.name,
//             author: model.author,
//             previews: model.previews,
//             shortSummary: model.shortSummary,
//             summary: model.summary,
//             homepage: model.homepage,
//             donationLink: model.donationLink
//         });
    actions: [
        Kirigami.Action {
            text: component.useLabel
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
            text: component.uninstallLabel
            iconName: "edit-delete"
            onTriggered: { newStuffModel.uninstallItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.InstalledStatus || model.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled;
        }
    ]
    thumbnailAvailable: model.previewsSmall.length > 0
    thumbnail: Image {
        anchors {
            fill: parent;
            margins: Kirigami.Units.smallSpacing;
        }
        asynchronous: true;
        fillMode: Image.PreserveAspectFit;
        source: thumbnailAvailable ? model.previewsSmall[0] : "";
        Kirigami.Icon {
            id: updateAvailableBadge;
            opacity: (model.status == NewStuff.ItemsModel.UpdateableStatus) ? 1 : 0;
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
            anchors {
                top: parent.top;
                right: parent.right;
                margins: -Kirigami.Units.smallSpacing;
            }
            height: Kirigami.Units.iconSizes.smallMedium;
            width: height;
            source: "package-installed-outdated";
        }
        Kirigami.Icon {
            id: installedBadge;
            opacity: (model.status == NewStuff.ItemsModel.InstalledStatus) ? 1 : 0;
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
            anchors {
                top: parent.top;
                right: parent.right;
                margins: -Kirigami.Units.smallSpacing;
            }
            height: Kirigami.Units.iconSizes.smallMedium;
            width: height;
            source: "package-installed-updated";
        }
        FeedbackOverlay {
            anchors.fill: parent
            newStuffModel: component.GridView.view.model
        }
        MouseArea {
            anchors.fill: parent;
            cursorShape: Qt.PointingHandCursor;
            onClicked: pageStack.push(detailsPage, {
                newStuffModel: component.GridView.view.model,
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
    }
}
