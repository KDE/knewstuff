/*
 * Copyright (C) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.7
import QtQuick.Dialogs 1.3 as QtDialogs
import QtQuick.Layouts 1.12 as QtLayouts
import org.kde.kirigami 2.5 as Kirigami
import org.kde.newstuff 1.62 as NewStuff

Kirigami.ApplicationWindow {
    id: root;
    title: "KNewStuff Dialog"

    globalDrawer: Kirigami.GlobalDrawer {
        title: "KNewStuff Dialog"
        titleIcon: "get-hot-new-stuff"
        drawerOpen: true;
        modal: false;
        topContent: NewStuff.Button {
            id: newStuffButton
            QtLayouts.Layout.fillWidth: true
            configFile: knsrcfile
        }

        actions: [
            Kirigami.Action {
                text: "Find Different Hot New Stuff..."
                icon.name: "document-open"
                onTriggered: {
                    fileDialog.open();
                }
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.defaultColumnWidth: pageStack.width
    Component {
        id: mainPageComponent
        NewStuff.Page { }
    }
    Component.onCompleted: {
        pageStack.push(mainPageComponent, { configFile: knsrcfile } );
    }

    QtDialogs.FileDialog {
        id: fileDialog
        title: "Open KNewStuff configuration file"
        folder: knsrcFilesLocation
        nameFilters: [ "KNewStuff Configuration Files (*.knsrc)", "All Files (*)" ]
        onAccepted: {
            newStuffButton.configFile = fileDialog.fileUrl.toString().substring(7)
            pageStack.clear();
            pageStack.push(mainPageComponent, { configFile: newStuffButton.configFile });
        }
    }
}
