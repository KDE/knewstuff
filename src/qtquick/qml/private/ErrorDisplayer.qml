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

MessageBoxSheet {
    id: component
    title: i18ndc("knewstuff5", "Title for a dialog box which shows error messages", "An Error Occurred");
    property bool active: true;
    property QtObject engine;
    property QtObject connection: Connections {
        target: engine
        onErrorMessage: { component.showError(message); }
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
