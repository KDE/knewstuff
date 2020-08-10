/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.7
import org.kde.newstuff 1.62 as NewStuff

NewStuff.Dialog {
    id: component
    configFile: knsrcfile
    Component.onCompleted: open()
    onVisibleChanged: {
        if (visible === false) {
            Qt.quit();
        }
    }
}
