/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts

import org.kde.kcmutils as KCM
import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff as NewStuff

import ".." as Private

KCM.GridDelegate {
    id: component
    property string useLabel
    property string uninstallLabel
    text: model.name
    property var entry: model.entry
    actions: [
        Kirigami.Action {
            text: component.useLabel
            icon.name: "dialog-ok-apply"
            onTriggered: { newStuffModel.engine.adoptEntry(entry); }
            enabled: (entry.status == NewStuff.Entry.Installed || entry.status == NewStuff.Entry.Updateable) && newStuffEngine.hasAdoptionCommand
            visible: enabled
        },
        Kirigami.Action {
            text: model.downloadLinks.length === 1 ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install") : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…");
            icon.name: "install"
            onTriggered: {
                if (model.downloadLinks.length === 1) {
                    newStuffModel.engine.install(entry, NewStuff.Entry.FirstLinkId);
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
            text: i18ndc("knewstuff6", "Request updating of this item", "Update");
            icon.name: "update-none"
            onTriggered: { newStuffModel.engine.install(entry, NewStuff.ItemsModel.AutoDetectLinkId); }
            enabled: entry.status == NewStuff.Entry.Updateable
            visible: enabled
        },
        Kirigami.Action {
            text: component.uninstallLabel
            icon.name: "edit-delete"
            onTriggered: { newStuffModel.engine.uninstall(entry); }
            enabled: entry.status == NewStuff.Entry.Installed || entry.status == NewStuff.Entry.Updateable
            visible: enabled
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
            opacity: (entry.status == NewStuff.Entry.Updateable) ? 1 : 0;
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
            opacity: (entry.status == NewStuff.Entry.Installed) ? 1 : 0;
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
                entry: entry,
            });

        }
    }
}
