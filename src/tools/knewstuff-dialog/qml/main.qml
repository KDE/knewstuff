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
import QtQuick.Layouts 1.12 as QtLayouts
import org.kde.kirigami 2.12 as Kirigami
import org.kde.newstuff 1.62 as NewStuff
import org.kde.newstuff.tools.dialog 1.0 as Myself

Kirigami.ApplicationWindow {
    id: root;
    title: "KNewStuff Dialog"

    globalDrawer: Kirigami.GlobalDrawer {
        id: globalDrawer
        title: "KNewStuff Dialog"
        titleIcon: "get-hot-new-stuff"
        drawerOpen: true;
        modal: false;

        actions: []
        Instantiator {
            id: configsInstantiator
            model: Myself.KNSRCModel { folder: "file://"+knsrcFilesLocation }
            Kirigami.Action {
                text: model.name
                icon.name: "get-hot-new-stuff"
                onTriggered: {
                    pageStack.clear();
                    pageStack.push(mainPageComponent, { configFile: model.filePath });
                }
            }
            onObjectAdded: globalDrawer.actions.push(object);
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    pageStack.defaultColumnWidth: pageStack.width
    Component {
        id: mainPageComponent
        NewStuff.Page { }
    }
    Component {
        id: startPageComponent
        Kirigami.AboutPage {
            aboutData: {
                "displayName" : "KNewStuff Dialog",
                "productName" : "org.kde.knewstuff.tools.dialog",
                "programLogo" : "get-hot-new-stuff",
                "componentName" : "knewstuff-dialog",
                "shortDescription" : "Get All Your Hot New Stuff",
                "homepage" : "https://kde.org/",
                "bugAddress" : "https://bugs.kde.org/",
                "version" : "v1.0",
                "otherText": "",
                "authors" : [
                            {
                                "name" : "Dan Leinir Turthra Jensen\n",
                                "task" : "Lead Developer",
                                "emailAddress" : "admin@leinir.dk",
                                "webAddress" : "https://leinir.dk/",
                                "ocsUsername" : "leinir"
                            }
                        ],
                "credits" : [],
                "translators" : [],
                "copyrightStatement" : "© 2020 The KDE Community",
                "desktopFileName" : "org.kde.knewstuff.tools.dialog"
            }
        }
    }
    Component.onCompleted: {
        pageStack.push(startPageComponent);
    }
}
