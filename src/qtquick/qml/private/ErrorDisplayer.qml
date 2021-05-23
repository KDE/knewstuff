/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12

MessageBoxSheet {
    id: component
    title: i18ndc("knewstuff5", "Title for a dialog box which shows error messages", "An Error Occurred");
    property bool active: true;
    property QtObject engine;
    property QtObject connection: Connections {
        target: engine
        function onErrorMessage(message) { component.showError(message); }
    }
    property var errorsToShow: []
    function showError(errorMessage) {
        if (active === true) {
            errorsToShow.push(errorMessage);
            if (sheetOpen === false) {
                text = errorsToShow.shift();
                open();
            }
        }
    }
    onSheetOpenChanged: {
        if (sheetOpen === false && errorsToShow.length > 0) {
            text = errorsToShow.shift();
            open();
        }
    }
}
