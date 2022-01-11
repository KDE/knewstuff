/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief An action which when triggered will open a NewStuff.Dialog or a NewStuff.Page, depending on settings
 *
 * This component is equivalent to the old Button component, but functions in more modern applications
 *
 * The following is a simple example of how to use this Action to show wallpapers from the KDE Store, on a
 * system where Plasma has been installed (and consequently the wallpaper knsrc file is available). This also
 * shows how to make the action push a page to a pageStack rather than opening a dialog:
 *
\code{.qml}
import org.kde.newstuff 1.91 as NewStuff

NewStuff.Action {
    configFile: "wallpaper.knsrc"
    text: i18n("&Get New Wallpapers...")
    pageStack: applicationWindow().pageStack
    onEntryEvent: function(entry, event) {
        if (event === NewStuff.Entry.StatusChangedEvent) {
            // A entry was installed, updated or removed
        } else if (event === NewStuff.Entry.AdoptedEvent) {
            // The "AdoptionCommand" from the knsrc file was run for the given entry.
            // This should not require refreshing the data for the model
        }
    }
}
\endcode
 *
 * @see NewStuff.Button
 * @since 5.81
 */

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import org.kde.kirigami 2.5 as Kirigami

import org.kde.newstuff 1.81 as NewStuff

Kirigami.Action {
    id: component

    /*
     * The configuration file is not aliased, because then we end up initialising the
     * KNSCore::Engine immediately the Action is instantiated, which we want to avoid (as that
     * is effectively a phone-home scenario, and causes internet traffic in situations
     * where it would not seem likely that there should be any).
     * If we want, in the future, to add some status display to the Action (such as "there
     * are updates to be had" or somesuch, then we can do this, but until that choice is
     * made, let's not)
     */
    /**
     * The configuration file to use for the Page created by this action
     */
    property string configFile

    /**
     * The default view mode of the page spawned by this action. This should be
     * set using the NewStuff.Page.ViewMode enum
     * @see NewStuff.Page.ViewMode
     */
    property int viewMode: NewStuff.Page.ViewMode.Preview

    /**
     * If this is set, the action will push a NewStuff.Page onto this page stack
     * (and request it is made visible if triggered again). If you do not set this
     * property, the action will spawn a NewStuff.Dialog instead.
     * @note If you are building a KCM, set this to your ```kcm``` object.
     */
    property QtObject pageStack: null

    /**
     * The engine which handles the content in this Action
     * This will be null until the action has been triggered the first time
     */
    readonly property QtObject engine: component._private.pageItem ? component._private.pageItem.engine : null

    /**
     * Contains the entries which have been changed.
     * @note This is cleared when the page is shown, so the changed entries are those
     * changed since the page was opened most recently (rather than the lifetime
     * of the instance of the Action component)
     * @deprecated Since 5.82, use entryEvent instead
     */
    property var changedEntries

    /**
     * This forwards the entry changed event from the QtQuick engine
     * @see Engine::entryEvent
     */
    signal entryEvent(QtObject entry, int event);

    /**
     * Show the details page for a specific entry.
     * If you call this function before the engine initialisation has been completed,
     * the action itself will be postponed until that has happened.
     * @param providerId The provider ID for the entry you wish to show details for
     * @param entryId The unique ID for the entry you wish to show details for
     * @since 5.79
     */
    function showEntryDetails(providerId, entryId) {
        component._private.providerId = providerId;
        component._private.entryId = entryId;
        component._private.showHotNewStuff();
    }

    /**
     * If this is true (default is false), the action will be shown when the Kiosk settings are such
     * that Get Hot New Stuff is disallowed (and any other time enabled is set to false).
     * Usually you would want to leave this alone, but occasionally you may have a reason to
     * leave a action in place that the user is unable to enable.
     */
    property bool visibleWhenDisabled: false

    /**
    * Show the page/dialog (same as activating the action), if allowed by the Kiosk settings
    */
    function showHotNewStuff() {
        component._private.showHotNewStuff();
    }

    onTriggered: { component._private.showHotNewStuff(); }

    icon.name: "get-hot-new-stuff"
    visible: enabled || visibleWhenDisabled
    enabled: NewStuff.Settings.allowedByKiosk
    onEnabledChanged: {
        // If the user resets this when kiosk has disallowed ghns, force enabled back to false
        if (enabled === true && NewStuff.Settings.allowedByKiosk === false) {
            enabled = false;
        }
    }

    readonly property QtObject _private: QtObject {
        property QtObject engine: pageItem ? pageItem.engine : null
        // Probably wants to be deleted and cleared if the "mode" changes at runtime...
        property QtObject pageItem;

        property string providerId;
        property string entryId;
        property Connections showSpecificEntryConnection: Connections {
            target: component.engine
            function onInitialized() {
                pageItem.showEntryDetails(providerId, component._private.entryId);
            }
        }

        property Connections engineConnections: Connections {
            target: component.engine
            function onEntryEvent(entry, event) {
                component.entryEvent(entry, event);
            }
        }
        property Binding changedEntriesBinding: Binding {
            target: component
            property: "changedEntries"
            value: component.engine ? component.engine.changedEntries : []
        }
        function showHotNewStuff() {
            if (NewStuff.Settings.allowedByKiosk) {
                if (component.pageStack !== null) {
                    if (component._private.pageItem // If we already have a page created...
                        && (component.pageStack.columnView !== undefined // first make sure that this pagestack is a Kirigami-style one (otherwise just assume we're ok)
                            && component.pageStack.columnView.contains(component._private.pageItem))) // and then check if the page is still in the stack before attempting to...
                    {
                        // ...set the already existing page as the current page
                        component.pageStack.currentItem = component._private.pageItem;
                    } else {
                        component._private.pageItem = newStuffPage.createObject(component);
                        component.pageStack.push(component._private.pageItem);
                    }
                } else {
                    newStuffDialog.open();
                }
            } else {
                // make some noise, because silently doing nothing is a bit annoying
            }
        }
        property Component newStuffPage: Component {
            NewStuff.Page {
                configFile: component.configFile
                viewMode: component.viewMode
            }
        }
        property Item newStuffDialog: Loader {
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
                component._private.pageItem = item;
                item.open();
            }

            active: false
            asynchronous: true

            sourceComponent: NewStuff.Dialog {
                configFile: component.configFile
                viewMode: component.viewMode
            }
        }
    }
}
