/*
    SPDX-FileCopyrightText: 2015 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml.Models
import org.kde.newstuff as NewStuff

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
        configFile: "/some/filesystem/location/wallpaper.knsrc"
        onMessage: console.log("KNS Message: " + message)
        onIdleMessage: console.log("KNS Idle: " + message)
        onBusyMessage: console.log("KNS Busy: " + message)
        onErrorMessage: console.log("KNS Error: " + message)
    }
    \endcode
 */
ListView {
    id: root

    /**
     * @brief The configuration file which describes the application (knsrc)
     *
     * The format and location of this file is found in the documentation for
     * KNS3::DownloadDialog
     */
    property alias configFile: newStuffEngine.configFile

    signal message(string message)
    signal idleMessage(string message)
    signal busyMessage(string message)
    signal errorMessage(string message)
    signal downloadedItemClicked(var installedFiles)

    header: RowLayout {
        anchors {
            left: parent.left
            right: parent.right
        }
        spacing: Kirigami.Units.smallSpacing

        QQC2.ComboBox {
            id: categoriesCombo

            Layout.fillWidth: true

            model: newStuffEngine.categories
            textRole: "displayName"

            onCurrentIndexChanged: {
                newStuffEngine.categoriesFilter = model.data(model.index(currentIndex, 0), NewStuff.CategoriesModel.NameRole);
            }
        }

        QQC2.ComboBox {
            id: filterCombo

            Layout.fillWidth: true

            model: ListModel { }

            Component.onCompleted: {
                filterCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the filter to show everything", "Show Everything") });
                filterCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the filter so only installed items are shown", "Installed Only") });
                filterCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the filter so only installed items with updates available are shown", "Updateable Only") });
                filterCombo.currentIndex = newStuffEngine.filter;
            }

            onCurrentIndexChanged: {
                newStuffEngine.filter = currentIndex;
            }
        }

        QQC2.ComboBox {
            id: sortCombo

            Layout.fillWidth: true

            model: ListModel { }

            Component.onCompleted: {
                sortCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the sort order to based on when items were most recently updated", "Show most recent first") });
                sortCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the sort order to be alphabetical based on the name", "Sort alphabetically") });
                sortCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the sort order to based on user ratings", "Show highest rated first") });
                sortCombo.model.append({ text: i18ndc("knewstuff6", "List option which will set the sort order to based on number of downloads", "Show most downloaded first") });
                sortCombo.currentIndex = newStuffEngine.sortOrder;
            }

            onCurrentIndexChanged: {
                newStuffEngine.sortOrder = currentIndex;
            }
        }
    }

    delegate: NewStuffItem {
        listModel: newStuffModel

        onClicked: {
            if (model.status == NewStuff.ItemsModel.InstalledStatus) {
                root.downloadedItemClicked(model.installedFiles);
            }
        }
    }

    model: NewStuff.ItemsModel {
        id: newStuffModel

        engine: newStuffEngine
    }

    NewStuff.Engine {
        id: newStuffEngine

        onMessage: root.message(message)
        onIdleMessage: root.idleMessage(message)
        onBusyMessage: root.busyMessage(message)
        onErrorCode: (code, msg) => root.errorMessage(message)
    }

    NewStuff.QuestionAsker {}
}
