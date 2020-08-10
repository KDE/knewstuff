/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief The contents of the NewStuff.Dialog component
 *
 * This component is equivalent to the old DownloadWidget, but you should consider
 * using NewStuff.Page instead for a more modern style of integration into your
 * application's flow.
 * @see KNewStuff::DownloadWidget
 * @since 5.63
 */

import QtQuick 2.11
import QtQuick.Layouts 1.11 as QtLayouts

import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff 1.62 as NewStuff

Kirigami.ApplicationItem {
    id: component

    property alias downloadNewWhat: newStuffPage.title
    /**
     * The configuration file to use for this button
     */
    property alias configFile: newStuffPage.configFile

    /**
     * The engine which handles the content in this dialog
     */
    property alias engine: newStuffPage.engine

    /**
     * The default view mode of the dialog spawned by this button. This should be
     * set using the NewStuff.Page.ViewMode enum
     * @see NewStuff.Page.ViewMode
     */
    property alias viewMode: newStuffPage.viewMode

    QtLayouts.Layout.preferredWidth: Kirigami.Units.gridUnit * 50
    QtLayouts.Layout.preferredHeight: Kirigami.Units.gridUnit * 40
    pageStack.defaultColumnWidth: pageStack.width
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.Auto
    pageStack.initialPage: NewStuff.Page {
        id: newStuffPage
        function showMessage(message) {
            // As the Page shows something nice and friendly while loading,
            // there's no reason to do the passive notification thing for those.
            if (!engine.isLoading) {
                component.showPassiveNotification(message);
            }
        }
        onMessage: component.showPassiveNotification(message);
        onIdleMessage: component.showPassiveNotification(message);
    }
}
