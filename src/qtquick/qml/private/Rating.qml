/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.11 as QtControls

import org.kde.kirigami 2.0 as Kirigami

RowLayout
{
    id: view
    property bool editable: false
    property int max: 100
    property int rating: 0
    property real starSize: Kirigami.Units.gridUnit
    property bool reverseLayout: false

    clip: true
    spacing: 0

    readonly property var ratingIndex: Math.floor((theRepeater.count*view.rating)/view.max)
    readonly property var ratingHalf: (theRepeater.count*view.rating)%view.max >= view.max / 2

    QtControls.Label {
        Layout.minimumWidth: view.starSize
        Layout.minimumHeight: view.starSize
        visible: view.reverseLayout
        text: ratingAsText.text
    }
    Item {
        visible: view.reverseLayout
        Layout.minimumHeight: view.starSize;
        Layout.minimumWidth: Kirigami.Units.smallSpacing;
        Layout.maximumWidth: Kirigami.Units.smallSpacing;
    }
    Repeater {
        id: theRepeater
        model: 5
        delegate: Kirigami.Icon {
            Layout.minimumWidth: view.starSize
            Layout.minimumHeight: view.starSize
            Layout.preferredWidth: view.starSize
            Layout.preferredHeight: view.starSize

            source: index < view.ratingIndex
                ? "rating"
                : (view.ratingHalf && index == view.ratingIndex
                    ? (view.LayoutMirroring.enabled ? "rating-half-rtl" : "rating-half")
                    : "rating-unrated")
            opacity: (view.editable && mouse.item.containsMouse) ? 0.7 : 1

            ConditionalLoader {
                id: mouse

                anchors.fill: parent
                condition: view.editable
                componentTrue: MouseArea {
                    hoverEnabled: true
                    onClicked: rating = (max/theRepeater.model*(index+1))
                }
                componentFalse: null
            }
        }
    }
    Item {
        visible: !view.reverseLayout
        Layout.minimumHeight: view.starSize;
        Layout.minimumWidth: Kirigami.Units.smallSpacing;
        Layout.maximumWidth: Kirigami.Units.smallSpacing;
    }
    QtControls.Label {
        id: ratingAsText
        Layout.minimumWidth: view.starSize
        Layout.minimumHeight: view.starSize
        visible: !view.reverseLayout
        text: i18ndc("knewstuff5", "A text representation of the rating, shown as a fraction of the max value", "(%1/%2)", view.rating / 10, view.max / 10)
    }
}
