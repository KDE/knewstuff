/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import org.kde.kirigami 2.7 as Kirigami
import org.kde.newstuff 1.83 as NewStuff

MessageBoxSheet {
    id: component
    title: i18ndc("knewstuff5", "Title for a dialog box which shows error messages", "An Error Occurred");
    property bool active: true;
    property QtObject engine;
    property QtObject connection: Connections {
        target: engine
        function onErrorCode(errorCode, message, metadata) {
            component.showError(errorCode, message, metadata);
        }
    }

    property var errorsToShow: []
    function showError(errorCode, errorMessage, errorMetadata) {
        if (active === true) {
            errorsToShow.push({
                code: errorCode,
                message: errorMessage,
                metadata: errorMetadata
            });
            showNextError();
        }
    }
    onSheetOpenChanged: displayThrottle.start()
    property QtObject displayThrottle: Timer {
        interval: Kirigami.Units.shortDuration
        onTriggered: showNextError();
    }
    function showNextError() {
        if (sheetOpen === false && errorsToShow.length > 0) {
            currentError = errorsToShow.shift();
            open();
        }
    }

    property var currentError: null
    text: currentError === null ? "" : currentError.message
    icon: {
        if (currentError === null) {
            return "";
        } else if (currentError.code == NewStuff.Engine.TryAgainLaterError) {
            return "accept_time_event";
        } else {
            return "dialog-warning";
        }
    }
}
