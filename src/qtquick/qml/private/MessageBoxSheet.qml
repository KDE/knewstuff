/*
 *   Copyright 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as QtControls
import QtQuick.Layouts 1.12 as QtLayouts

import org.kde.kirigami 2.7 as Kirigami

Kirigami.OverlaySheet {
    id: component

    property alias title: headerLabel.text
    property alias text: messageLabel.text
    property list<QtObject> actions

    showCloseButton: true
    header: Kirigami.Heading {
        id: headerLabel
        QtLayouts.Layout.fillWidth: true
        elide: Text.ElideRight
    }
    contentItem: TextEdit {
        id: messageLabel
        QtLayouts.Layout.preferredWidth: Kirigami.Units.gridUnit * 10
        QtLayouts.Layout.margins: Kirigami.Units.largeSpacing
        wrapMode: Text.Wrap
        readOnly: true
        selectByMouse: true
    }
    footer: QtLayouts.RowLayout {
        Item { QtLayouts.Layout.fillWidth: true }
        Repeater {
            model: component.actions;
            QtControls.Button {
                action: modelData
                Connections {
                    target: action
                    onTriggered: component.close()
                }
            }
        }
    }
}
