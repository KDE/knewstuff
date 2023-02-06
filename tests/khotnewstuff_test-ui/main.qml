/*
    SPDX-FileCopyrightText: 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.7
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root;

    globalDrawer: Kirigami.GlobalDrawer {
        title: "KNewStuff Test"
        titleIcon: "applications-development"
        drawerOpen: true;
        modal: false;

        actions: [
            Kirigami.Action {
                text: "Run Engine test"
                onTriggered: testObject.engineTest();
                iconName: "run-build"
            },
            Kirigami.Action {
                text: "Test entry download as well"
                onTriggered: testObject.testAll = !testObject.testAll
                iconName: typeof(testObject) !== "undefined" ? (testObject.testAll ? "checkmark" : "") : ""
            },
            Kirigami.Action {},
            Kirigami.Action {
                text: "Run Entry test"
                onTriggered: testObject.entryTest();
                iconName: "run-build"
            },
            Kirigami.Action {
                text: "Run Provider test"
                onTriggered: testObject.providerTest();
                iconName: "run-build"
            }
        ]
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.initialPage: mainPageComponent

    Component {
        id: mainPageComponent
        Kirigami.ScrollablePage {
            title: "Welcome"
            ListView {
                id: messageView;
                model: testObject.messages();
                onCountChanged: {
                    messageView.currentIndex = messageView.count - 1;
                }
                delegate:  Kirigami.BasicListItem {
                    id: listItem

                    reserveSpaceForIcon: true
                    label: model.display
                    icon.name: model.whatsThis

                    Accessible.role: Accessible.MenuItem
                    onClicked: {}
                    highlighted: focus && ListView.isCurrentItem
                }
            }
        }
    }
}
