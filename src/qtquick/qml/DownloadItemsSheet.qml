/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts

import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

/**
 * @brief An overlay sheet for showing a list of download options for one entry
 *
 * This is used by the NewStuff.Page component
 * @since 5.63
 */

Kirigami.OverlaySheet {
    id: component

    property string entryId
    property alias downloadLinks: itemsView.model
    signal itemPicked(string entryId, int downloadItemId, string downloadName)

    showCloseButton: true
    title: i18nd("knewstuff5", "Pick Your Installation Option")

    contentItem: QtLayouts.ColumnLayout {
        QtControls.Label {
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            QtLayouts.Layout.fillWidth: true
            QtLayouts.Layout.maximumWidth: Math.round(component.parent.width * 0.75)

            text: i18nd("knewstuff5", "Please select the option you wish to install from the list of downloadable items below. If it is unclear which you should chose out of the available options, please contact the author of this item and ask that they clarify this through the naming of the items.")
            wrapMode: Text.Wrap
        }

        QtControls.ScrollView {
            QtLayouts.Layout.fillWidth: true
            QtLayouts.Layout.fillHeight: true

            ListView {
                id: itemsView

                delegate: Kirigami.BasicListItem {
                    implicitHeight: installButton.implicitHeight + Kirigami.Units.smallSpacing * 2

                    text: modelData.name
                    icon: modelData.icon

                    // Don't need a highlight or hover effects
                    hoverEnabled: false
                    activeBackgroundColor: "transparent"
                    activeTextColor: Kirigami.Theme.textColor

                    trailing: QtLayouts.RowLayout {
                        QtControls.Label {
                            text: modelData.formattedSize
                        }

                        QtControls.ToolButton {
                            id: installButton
                            text: i18nd("knewstuff5", "Install")
                            icon.name: "install"
                            onClicked: {
                                component.close();
                                component.itemPicked(component.entryId, modelData.id, modelData.name);
                            }
                        }
                    }
                }
            }
        }
    }
}
