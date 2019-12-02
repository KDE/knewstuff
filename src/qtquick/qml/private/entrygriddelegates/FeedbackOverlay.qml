/*
 * Copyright (C) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
            onEntryChanged: {
                var status = newStuffModel.data(newStuffModel.index(index, 0), NewStuff.ItemsModel.StatusRole);
                if (status == NewStuff.ItemsModel.DownloadableStatus
                || status == NewStuff.ItemsModel.InstalledStatus
                || status == NewStuff.ItemsModel.UpdateableStatus
                || status == NewStuff.ItemsModel.DeletedStatus) {
                    statusLabel.text = "";
                } else if (status == NewStuff.ItemsModel.InstallingStatus) {
                    statusLabel.text = i18nc("Label for the busy indicator showing an item is being installed", "Installing...");
                } else if (status == NewStuff.ItemsModel.UpdatingStatus) {
                    statusLabel.text = i18nc("Label for the busy indicator showing an item is in the process of being updated", "Updating...");
                } else {
                    statusLabel.text = i18nc("Label for the busy indicator which should only be shown when the entry has been given some unknown or invalid status.", "Invalid or unknown state. <a href=\"https://bugs.kde.org/enter_bug.cgi?product=frameworks-knewstuff\">Please report this to the KDE Community in a bug report</a>.");
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
        text: i18nc("Label for the busy indicator showing an item is installing", "Installing...");
    }
}
