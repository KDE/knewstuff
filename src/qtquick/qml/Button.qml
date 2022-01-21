/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A button which when clicked will open a dialog with a NewStuff.Page at the base
 *
 * This component is equivalent to the old Button
 * @see KNewStuff::Button
 * @since 5.63
 */

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls

import org.kde.newstuff 1.81 as NewStuff

QtControls.Button {
    id: component

    /*
     * The configuration file is not aliased, because then we end up initialising the
     * KNSCore::Engine immediately the Button is shown, which we want to avoid (as that
     * is effectively a phone-home scenario, and causes internet traffic in situations
     * where it would not seem likely that there should be any).
     * If we want, in the future, to add some status display to Button (such as "there
     * are updates to be had" or somesuch, then we can do this, but until that choice is
     * made, let's not)
     */
    /**
     * The configuration file to use for this button
     */
    property string configFile

    /**
     * Set the text that should appear on the button. Will be set as
     * i18nd("knewstuff5", "Download New %1...").
     *
     * @note For the sake of consistency, you should NOT override the text property, just set this one
     */
    property string downloadNewWhat: i18ndc("knewstuff5", "Used to construct the button's label (which will become Download New 'this value'...)", "Stuff")
    text: i18nd("knewstuff5", "Download New %1...", downloadNewWhat)

    /**
     * The default view mode of the dialog spawned by this button. This should be
     * set using the NewStuff.Page.ViewMode enum
     * @see NewStuff.Page.ViewMode
     */
    property int viewMode: NewStuff.Page.ViewMode.Preview

    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    signal aboutToShowDialog();

    /**
     * The engine which handles the content in this Button
     */
    property QtObject engine: null

    /**
     * This forwards the entryEvent from the QtQuick engine
     * @see Engine::entryEvent
     * @since 5.82
     */
    signal entryEvent(QtObject entry, int event);
    property Connections engineConnections: Connections {
        target: component.engine
        function onEntryEvent(entry, event) {
            entryEvent(entry, event);
        }
    }

    /**
     * Contains the entries which have been changed.
     * @note This is cleared when the dialog is shown, so the changed entries are those
     * changed since the dialog was opened most recently (rather than the lifetime
     * of the instance of the Button component)
     * @deprecated Since 5.82, use entryEvent instead
     */
    property var changedEntries
    property Binding changedEntriesBinding: Binding {
        target: component
        property: "changedEntries"
        value: component.engine ? component.engine.changedEntries : []
    }

    /**
     * Show the details page for a specific entry.
     * If you call this function before the engine initialisation has been completed,
     * the action itself will be postponed until that has happened.
     * @param providerId The provider ID for the entry you wish to show details for
     * @param entryId The unique ID for the entry you wish to show details for
     * @since 5.79
     */
    function showEntryDetails(providerId, entryId) {
        newStuffPage.showEntryDetails(providerId, entryId);
    }

    /**
     * If this is true (default is false), the button will be shown when the Kiosk settings are such
     * that Get Hot New Stuff is disallowed (and any other time enabled is set to false).
     * Usually you would want to leave this alone, but occasionally you may have a reason to
     * leave a button in place that the user is unable to enable.
     */
    property bool visibleWhenDisabled: false

    /**
     * Show the dialog (same as clicking the button), if allowed by the Kiosk settings
     */
    function showDialog() {
        if (NewStuff.Settings.allowedByKiosk) {
            component.aboutToShowDialog();
            ghnsDialog.open();
        } else {
            // make some noise, because silently doing nothing is a bit annoying
        }
    }

    onClicked: { showDialog(); }

    icon.name: "get-hot-new-stuff"
    visible: enabled || visibleWhenDisabled
    enabled: NewStuff.Settings.allowedByKiosk
    onEnabledChanged: {
        // If the user resets this when kiosk has disallowed ghns, force enabled back to false
        if (enabled === true && NewStuff.Settings.allowedByKiosk === false) {
            enabled = false;
        }
    }

    property Item ghnsDialog : Loader {
        // Use this function to open the dialog. It seems roundabout, but this ensures
        // that the dialog is not constructed until we want it to be shown the first time,
        // since it will initialise itself on the first load (which causes it to phone
        // home) and we don't want that until the user explicitly asks for it.
        function open() {
            if (item) {
                item.open();
            } else {
                active = true;
            }
        }
        onLoaded: {
            item.open();
            component.engine = item.engine
        }

        active: false
        asynchronous: true

        sourceComponent: NewStuff.Dialog {
            configFile: component.configFile
            viewMode: component.viewMode
        }
    }
}
