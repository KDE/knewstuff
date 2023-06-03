/*
    SPDX-FileCopyrightText: 2015 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami 2 as Kirigami
import org.kde.newstuff as NewStuff

Kirigami.SwipeListItem {
    id: listItem

    property QtObject listModel

    height: Math.max(Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing * 2,
                     nameText.height + descriptionText.height + Kirigami.Units.smallSpacing * 5)
    enabled: true

    actions: [
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request installation of this item", "Install")
            icon.name: "list-add"
            onTriggered: { listModel.installItem(model.index, NewStuff.ItemsModel.FirstLinkId); }
            enabled: model.status == NewStuff.ItemsModel.DownloadableStatus || model.status == NewStuff.ItemsModel.DeletedStatus
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "refresh"
            onTriggered: listModel.updateItem(model.index)
            enabled: model.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request uninstallation of this item", "Uninstall")
            icon.name: "list-remove"
            onTriggered: listModel.uninstallItem(model.index)
            enabled: model.status == NewStuff.ItemsModel.InstalledStatus || model.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled
        }
    ]

    RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Item {
            id: previewContainer

            Layout.preferredHeight: listItem.height - Kirigami.Units.smallSpacing * 2
            Layout.minimumWidth: Kirigami.Units.iconSizes.huge
            Layout.maximumWidth: Kirigami.Units.iconSizes.huge

            Image {
                id: previewImage

                anchors {
                    fill: parent
                    margins: Kirigami.Units.smallSpacing
                    leftMargin: -Kirigami.Units.smallSpacing
                }
                asynchronous: true
                fillMode: Image.PreserveAspectFit
                source: model.previewsSmall.length > 0 ? model.previewsSmall[0] : ""

                Kirigami.Icon {
                    id: updateAvailableBadge

                    opacity: (model.status == NewStuff.ItemsModel.UpdateableStatus) ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.shortDuration
                        }
                    }

                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                        margins: -Kirigami.Units.smallSpacing
                    }
                    height: Kirigami.Units.iconSizes.smallMedium
                    width: height
                    source: "vcs-update-required"
                }

                Kirigami.Icon {
                    id: installedBadge

                    opacity: (model.status == NewStuff.ItemsModel.InstalledStatus) ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: Kirigami.Units.shortDuration
                        }
                    }

                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                        margins: -Kirigami.Units.smallSpacing
                    }
                    height: Kirigami.Units.iconSizes.smallMedium
                    width: height
                    source: "vcs-normal"
                }
            }

            Rectangle {
                anchors.fill: parent

                opacity: installIndicator.opacity > 0 ? 0.7 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }

                visible: opacity > 0
            }

            QQC2.BusyIndicator {
                id: installIndicator

                anchors.centerIn: parent

                opacity: (model.status == NewStuff.ItemsModel.InstallingStatus || model.status == NewStuff.ItemsModel.UpdatingStatus) ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: Kirigami.Units.shortDuration
                    }
                }

                running: opacity > 0

                QQC2.Label {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        bottom: parent.bottom
                        margins: Kirigami.Units.smallSpacing
                    }

                    // There is no distinction between installing and uninstalling as a status, so we have to word things accordingly
                    text: (model.status == NewStuff.ItemsModel.InstallingStatus) ? "Working" : ((model.status == NewStuff.ItemsModel.UpdatingStatus) ? "Updating" : "")
                    width: paintedWidth
                }
            }
        }

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            opacity: 1 - installIndicator.opacity

            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.Heading {
                id: nameText

                Layout.fillWidth: true

                level: 3
                text: model.name
            }

            QQC2.Label {
                id: descriptionText

                Layout.fillWidth: true

                text: model.summary.split("\n")[0]
                elide: Text.ElideRight
                maximumLineCount: 2
                wrapMode: Text.Wrap
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
