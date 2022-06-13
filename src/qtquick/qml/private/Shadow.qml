/*
    SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtGraphicalEffects 1.15

import org.kde.kirigami 2.20 as Kirigami

LinearGradient {
    id: shadow

    property int edge: Qt.LeftEdge

    width: Kirigami.Units.gridUnit/2
    height: Kirigami.Units.gridUnit/2

    start: Qt.point((edge !== Qt.RightEdge ? 0 : width), (edge !== Qt.BottomEdge ? 0 : height))
    end: Qt.point((edge !== Qt.LeftEdge ? 0 : width), (edge !== Qt.TopEdge ? 0 : height))
    gradient: Gradient {
        GradientStop {
            position: 0.0
            color: Kirigami.Theme.backgroundColor
        }
        GradientStop {
            position: 0.3
            color: Qt.rgba(0, 0, 0, 0.1)
        }
        GradientStop {
            position: 1.0
            color:  "transparent"
        }
    }
}

