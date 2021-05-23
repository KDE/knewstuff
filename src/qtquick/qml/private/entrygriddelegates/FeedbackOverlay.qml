/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls

import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

Item {
    property QtObject newStuffModel
    visible: opacity > 0
    opacity: (model.status == NewStuff.ItemsModel.InstallingStatus || model.status == NewStuff.ItemsModel.UpdatingStatus) ? 1 : 0
    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.longDuration; } }
    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
        opacity: 0.9;
    }
    QtControls.BusyIndicator {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.verticalCenter
            bottomMargin: Kirigami.Units.smallSpacing
        }
        running: parent.visible
    }
    QtControls.Label {
        id: statusLabel
        Connections {
            target: newStuffModel
            function onEntryChanged(index) {
                var status = newStuffModel.data(newStuffModel.index(index, 0), NewStuff.ItemsModel.StatusRole);
                if (status == NewStuff.ItemsModel.DownloadableStatus
                || status == NewStuff.ItemsModel.InstalledStatus
                || status == NewStuff.ItemsModel.UpdateableStatus
                || status == NewStuff.ItemsModel.DeletedStatus) {
                    statusLabel.text = "";
                } else if (status == NewStuff.ItemsModel.InstallingStatus) {
                    statusLabel.text = i18ndc("knewstuff5", "Label for the busy indicator showing an item is being installed OR uninstalled", "Working...");
                } else if (status == NewStuff.ItemsModel.UpdatingStatus) {
                    statusLabel.text = i18ndc("knewstuff5", "Label for the busy indicator showing an item is in the process of being updated", "Updating...");
                } else {
                    statusLabel.text = i18ndc("knewstuff5", "Label for the busy indicator which should only be shown when the entry has been given some unknown or invalid status.", "Invalid or unknown state. <a href=\"https://bugs.kde.org/enter_bug.cgi?product=frameworks-knewstuff\">Please report this to the KDE Community in a bug report</a>.");
                }
            }
        }
        onLinkActivated: Qt.openUrlExternally(link);
        anchors {
            top: parent.verticalCenter
            left: parent.left
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }
        horizontalAlignment: Text.AlignHCenter
        // TODO: This is where we'd want to put the download progress and cancel button as well
        text: i18ndc("knewstuff5", "Label for the busy indicator showing an item is installing", "Installing...");
    }
}
