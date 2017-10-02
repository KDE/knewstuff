/*
 * Copyright (C) 2015 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.2
import QtQuick.Controls 1.4 as QtControls

import org.kde.kirigami 2.1 as Kirigami
import org.kde.newstuff 1.0 as NewStuff

Kirigami.SwipeListItem {
    id: listItem;
    height: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing * 2;
    property QtObject listModel;
    enabled: true;
    actions: [
        Kirigami.Action {
            text: i18nc("Request installation of this item", "Install");
            iconName: "list-add"
            onTriggered: { listModel.installItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.DownloadableStatus || model.status == NewStuff.ItemsModel.DeletedStatus;
            visible: enabled;
        },
        Kirigami.Action {
            text: i18nc("Request updating of this item", "Update");
            iconName: "refresh"
            onTriggered: { listModel.installItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.UpdateableStatus;
            visible: enabled;
        },
        Kirigami.Action {
            text: i18nc("Request uninstallation of this item", "Uninstall");
            iconName: "list-remove"
            onTriggered: { listModel.uninstallItem(model.index); }
            enabled: model.status == NewStuff.ItemsModel.InstalledStatus
            visible: enabled;
        }
    ]
    Item {
        anchors.fill: parent;
        Item {
            id: previewContainer;
            anchors {
                top: parent.top;
                left: parent.left;
                bottom: parent.bottom;
                margins: Kirigami.Units.smallSpacing;
            }
            width: height;
            Image {
                id: previewImage;
                anchors {
                    fill: parent;
                    margins: Kirigami.Units.smallSpacing;
                }
                asynchronous: true;
                fillMode: Image.PreserveAspectFit;
                source: model.previewsSmall.length > 0 ? model.previewsSmall[0] : "";
                Kirigami.Icon {
                    id: updateAvailableBadge;
                    opacity: (model.status == NewStuff.ItemsModel.UpdateableStatus) ? 1 : 0;
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
                    anchors {
                        bottom: parent.bottom;
                        right: parent.right;
                        margins: -Kirigami.Units.smallSpacing;
                    }
                    height: Kirigami.Units.iconSizes.smallMedium;
                    width: height;
                    source: "vcs-update-required";
                }
                Kirigami.Icon {
                    id: installedBadge;
                    opacity: (model.status == NewStuff.ItemsModel.InstalledStatus) ? 1 : 0;
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
                    anchors {
                        bottom: parent.bottom;
                        right: parent.right;
                        margins: -Kirigami.Units.smallSpacing;
                    }
                    height: Kirigami.Units.iconSizes.smallMedium;
                    width: height;
                    source: "vcs-normal";
                }
            }
        }
        Kirigami.Label {
            anchors {
                verticalCenter: parent.verticalCenter;
                left: previewContainer.right;
                leftMargin: Kirigami.Units.largeSpacing;
            }
            text: model.name;
        }
        QtControls.BusyIndicator {
            anchors {
                verticalCenter: parent.verticalCenter;
                right: parent.right;
                rightMargin: Kirigami.Units.largeSpacing + Kirigami.Units.iconSizes.large;
            }
            opacity: (model.status == NewStuff.ItemsModel.InstallingStatus || model.status == NewStuff.ItemsModel.UpdatingStatus) ? 1 : 0;
            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration; } }
            running: opacity > 0;
            Kirigami.Label {
                anchors {
                    verticalCenter: parent.verticalCenter;
                    right: parent.left;
                    rightMargin: Kirigami.Units.smallSpacing;
                }
                text: (model.status == NewStuff.ItemsModel.InstallingStatus) ? "Installing" : ((model.status == NewStuff.ItemsModel.UpdatingStatus) ? "Updating" : "");
                width: paintedWidth;
            }
        }
    }
}
