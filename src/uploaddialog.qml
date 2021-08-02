/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.7
import QtQuick.Layouts 1.11 as QtLayouts
import QtQuick.Window 2.15
import org.kde.kirigami 2.14 as Kirigami
import org.kde.newstuff 1.85 as NewStuff

Kirigami.ApplicationItem {
    signal closed()
    anchors.fill: parent
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 44, Screen.width)
    implicitHeight: Math.min(Kirigami.Units.gridUnit * 30, Screen.height)
    pageStack.defaultColumnWidth: pageStack.width
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Auto
    pageStack.globalToolBar.canContainHandles: true
    pageStack.initialPage: NewStuff.UploadPage {
        engine: NewStuff.Engine {
            configFile: knsrcfile
        }
        onVisibleChanged: {
            if (!visible) {
                applicationWindow.closed();
            }
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
            id: contextDrawer
    }
}
