/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami 2 as Kirigami
import org.kde.newstuff as NewStuff

/**
 * @brief An overlay sheet for showing a list of download options for one entry
 *
 * This is used by the NewStuff.Page component
 * @since 5.63
 */
Kirigami.OverlaySheet {
    id: component

    property var entry

    property alias downloadLinks: itemsView.model

    signal itemPicked(var entry, int downloadItemId, string downloadName)

    showCloseButton: true
    title: i18nd("knewstuff6", "Pick Your Installation Option")

    ListView {
        id: itemsView

        headerPositioning: ListView.InlineHeader
        header: QQC2.Label {
            width: ListView.view.width - ListView.view.leftMargin - ListView.view.rightMargin
            padding: Kirigami.Units.largeSpacing

            text: i18nd("knewstuff6", "Please select the option you wish to install from the list of downloadable items below. If it is unclear which you should chose out of the available options, please contact the author of this item and ask that they clarify this through the naming of the items.")
            wrapMode: Text.Wrap
        }

        delegate: Kirigami.BasicListItem {
            implicitHeight: installButton.implicitHeight + Kirigami.Units.smallSpacing * 2

            text: modelData.name
            icon.name: modelData.icon

            // Don't need a highlight or hover effects
            hoverEnabled: false
            activeBackgroundColor: "transparent"
            activeTextColor: Kirigami.Theme.textColor

            trailing: RowLayout {
                QQC2.Label {
                    text: modelData.formattedSize
                }

                QQC2.ToolButton {
                    id: installButton

                    text: i18nd("knewstuff6", "Install")
                    icon.name: "install"

                    onClicked: {
                        component.close();
                        component.itemPicked(component.entry, modelData.id, modelData.name);
                    }
                }
            }
        }
    }
}
