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
    header: Kirigami.Heading {
        text: i18nd("knewstuff5", "Pick Your Installation Option")
        elide: Text.ElideRight
    }
    contentItem: ListView {
        id: itemsView

        header: QtControls.Label {
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            width: parent.width
            text: i18nd("knewstuff5", "Please select the option you wish to install from the list of downloadable items below. If it is unclear which you should chose out of the available options, please contact the author of this item and ask that they clarify this through the naming of the items.")
            wrapMode: Text.Wrap
        }

        delegate: Kirigami.BasicListItem {
            text: modelData.name
            icon: modelData.icon
            QtControls.Label {
                QtLayouts.Layout.alignment: Qt.AlignRight
                text: modelData.formattedSize
            }

            QtControls.ToolButton {
                text: i18nd("knewstuff5", "Install")
                icon.name: "install"
                QtLayouts.Layout.alignment: Qt.AlignRight
                onClicked: {
                    component.close();
                    component.itemPicked(component.entryId, modelData.id, modelData.name);
                }
            }
        }
    }
}
