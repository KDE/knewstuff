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

import org.kde.newstuff 1.0 as NewStuff

/**
 * To use NewStuffList, simply instantiate it and pass the
 * local file location of a knsrc file to the configFile property.
 * The components will, in this case, take care of the rest for you.
 * If you want more, you can look at what NewStuffItem does with the
 * various bits, and be inspired by that.
 *
 * An (overly simple) example which might be used for managing
 * wallpapers and just outputting any messages onto the console can
 * be seen below. Note that you should obviously not be using
 * hardcoded paths, it is done here to get the idea across.
 *
 * \code
    NewStuff.NewStuffList {
        configFile: "/some/filesystem/location/wallpaper.knsrc";
        onMessage: console.log("KNS Message: " + message);
        onIdleMessage: console.log("KNS Idle: " + message);
        onBusyMessage: console.log("KNS Busy: " + message);
        onErrorMessage: console.log("KNS Error: " + message);
    }
    \endcode
 */
ListView {
    id: root;
    /**
     * @brief The configuration file which describes the application (knsrc)
     *
     * The format and location of this file is found in the documentation for
     * KNS3::DownloadDialog
     */
    property alias configFile: newStuffEngine.configFile;
    signal message(string message);
    signal idleMessage(string message);
    signal busyMessage(string message);
    signal errorMessage(string message);
    signal downloadedItemClicked(variant installedFiles);
    delegate: NewStuffItem {
        listModel: newStuffModel;
        onClicked: {
            if(model.status == NewStuff.ItemsModel.InstalledStatus) {
                root.downloadedItemClicked(model.installedFiles);
            }
        }
    }
    model: NewStuff.ItemsModel {
        id: newStuffModel;
        engine: newStuffEngine.engine;
    }
    NewStuff.Engine {
        id: newStuffEngine;
        onMessage: root.message(message);
        onIdleMessage: root.idleMessage(message);
        onBusyMessage: root.busyMessage(message);
        onErrorMessage: root.errorMessage(message);
    }
}
