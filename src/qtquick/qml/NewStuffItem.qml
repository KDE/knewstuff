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

import org.kde.kirigami 1.0 as Kirigami

Kirigami.SwipeListItem {
    id: listItem;
    width: ListView.width;
    height: Kirigami.Units.iconSizes.huge + Kirigami.Units.smallSpacing * 2;
    actions: []
    Item {
        anchors.fill: parent;
        Item {
            id: previewContainer;
            anchors {
                top: parent.top;
                left: parent.left;
                bottom: parent.bottom;
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
    }
}
