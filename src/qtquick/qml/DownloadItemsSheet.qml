/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami 2 as Kirigami
import org.kde.kirigami.delegates as KirigamiDelegates
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

        delegate: QQC2.ItemDelegate {
            id: delegate

            width: itemsView.width

            icon.name: modelData.icon
            text: modelData.name

            // Don't need a highlight, hover, or pressed effects
            highlighted: false
            hoverEnabled: false
            down: false

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                // TODO: switch to just IconTitle once it exists, since we don't need
                // the subtitle here and are only using a Kirigami delegate for the visual
                // consistency it offers
                Kirigami.IconTitleSubtitle {
                    Layout.fillWidth: true
                    icon.name: delegate.icon.name
                    title: delegate.text
                    selected: delegate.highlighted
                }
                QQC2.Label {
                    text: modelData.formattedSize
                    color: delegate.highlighted
                        ? Kirigami.Theme.highlightedTextColor
                        : Kirigami.Theme.textColor
                }
                QQC2.ToolButton {
                    id: installButton

                    text: i18nd("knewstuff6", "Install")
                    icon.name: "install-symbolic"

                    onClicked: {
                        component.close();
                        component.itemPicked(component.entry, modelData.id, modelData.name);
                    }
                }
            }
        }
    }
}
