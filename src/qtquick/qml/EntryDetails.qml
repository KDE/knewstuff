/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A Kirigami.Page component used for displaying the details for a single entry
 *
 * This component is equivalent to the details view in the old DownloadDialog
 * @see KNewStuff::DownloadDialog
 * @since 5.63
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami 2 as Kirigami
import org.kde.kcm as KCM
import org.kde.newstuff as NewStuff

import "private" as Private

KCM.SimpleKCM {
    id: component

    property QtObject newStuffModel
    property int index
    property string name
    property var author
    property alias shortSummary: shortSummaryItem.text
    property alias summary: summaryItem.text
    property alias previews: screenshotsItem.screenshotsModel
    property string homepage
    property string donationLink
    property int status
    property int commentsCount
    property int rating
    property int downloadCount
    property var downloadLinks
    property string providerId
    property int entryType

    NewStuff.DownloadItemsSheet {
        id: downloadItemsSheet

        onItemPicked: {
            const entryName = newStuffModel.data(newStuffModel.index(entryId, 0), NewStuff.ItemsModel.NameRole);
            applicationWindow().showPassiveNotification(i18ndc("knewstuff6", "A passive notification shown when installation of an item is initiated", "Installing %1 from %2", downloadName, entryName), 1500);
            newStuffModel.installItem(entryId, downloadItemId);
        }
    }

    Private.ErrorDisplayer {
        engine: component.newStuffModel.engine
        active: component.isCurrentPage
    }

    Connections {
        target: newStuffModel

        function onEntryChanged(index) {
            const status = newStuffModel.data(newStuffModel.index(index, 0), NewStuff.ItemsModel.StatusRole);
            switch (status) {
            case NewStuff.ItemsModel.DownloadableStatus:
            case NewStuff.ItemsModel.InstalledStatus:
            case NewStuff.ItemsModel.UpdateableStatus:
            case NewStuff.ItemsModel.DeletedStatus:
                statusCard.message = "";
                break;
            case NewStuff.ItemsModel.InstallingStatus:
                statusCard.message = i18ndc("knewstuff6", "Status message to be shown when the entry is in the process of being installed OR uninstalled", "Currently working on the item %1 by %2. Please wait…", component.name, entryAuthor.name);
                break;
            case NewStuff.ItemsModel.UpdatingStatus:
                statusCard.message = i18ndc("knewstuff6", "Status message to be shown when the entry is in the process of being updated", "Currently updating the item %1 by %2. Please wait…", component.name, entryAuthor.name);
                break;
            default:
                statusCard.message = i18ndc("knewstuff6", "Status message which should only be shown when the entry has been given some unknown or invalid status.", "This item is currently in an invalid or unknown state. <a href=\"https://bugs.kde.org/enter_bug.cgi?product=frameworks-knewstuff\">Please report this to the KDE Community in a bug report</a>.");
                break;
            }
            component.status = status;
        }
    }

    NewStuff.Author {
        id: entryAuthor
        engine: component.newStuffModel.engine
        providerId: component.providerId
        username: author.name
    }

    title: i18ndc("knewstuff6", "Combined title for the entry details page made of the name of the entry, and the author's name", "%1 by %2", component.name, entryAuthor.name)

    actions: [
        Kirigami.Action {
            text: component.downloadLinks.length == 1 ? i18ndc("knewstuff6", "Request installation of this item, available when there is exactly one downloadable item", "Install") : i18ndc("knewstuff6", "Show installation options, where there is more than one downloadable item", "Install…")
            icon.name: "install"
            onTriggered: {
                if (component.downloadLinks.length == 1) {
                    newStuffModel.installItem(component.index, NewStuff.ItemsModel.FirstLinkId);
                } else {
                    downloadItemsSheet.downloadLinks = component.downloadLinks;
                    downloadItemsSheet.entryId = component.index;
                    downloadItemsSheet.open();
                }
            }
            enabled: component.status == NewStuff.ItemsModel.DownloadableStatus || component.status == NewStuff.ItemsModel.DeletedStatus
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request updating of this item", "Update")
            icon.name: "update-none"
            onTriggered: newStuffModel.updateItem(component.index)
            enabled: component.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled
        },
        Kirigami.Action {
            text: i18ndc("knewstuff6", "Request uninstallation of this item", "Uninstall")
            icon.name: "edit-delete"
            onTriggered: newStuffModel.uninstallItem(component.index)
            enabled: component.status == NewStuff.ItemsModel.InstalledStatus || component.status == NewStuff.ItemsModel.UpdateableStatus
            visible: enabled
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.AbstractCard {
            id: statusCard

            property string message

            visible: opacity > 0
            opacity: message.length > 0 ? 1 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    Layout.fillWidth: true
                    text: statusCard.message
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link);
                }

                QQC2.BusyIndicator {
                    running: statusCard.opacity > 0
                }
            }
        }

        Item {
            Layout.fillWidth: true
            height: Kirigami.Units.gridUnit * 3
        }

        Private.EntryScreenshots {
            id: screenshotsItem
            Layout.fillWidth: true
        }

        Kirigami.Heading {
            id: shortSummaryItem
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Kirigami.LinkButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "Comments and Reviews:")
                enabled: component.commentsCount > 0
                text: i18ndc("knewstuff6", "A link which, when clicked, opens a new sub page with comments (comments with or without ratings) for this entry", "%1 Reviews and Comments", component.commentsCount)
                onClicked: pageStack.push(commentsPage)
            }

            Private.Rating {
                id: ratingsItem
                Kirigami.FormData.label: i18nd("knewstuff6", "Rating:")
                rating: component.rating
            }

            Kirigami.UrlButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "Homepage:")
                text: i18ndc("knewstuff6", "A link which, when clicked, opens the website associated with the entry (this could be either one specific to the project, the author's homepage, or any other website they have chosen for the purpose)", "Open the homepage for %1", component.name)
                url: component.homepage
                visible: component.homepage
            }

            Kirigami.UrlButton {
                Kirigami.FormData.label: i18nd("knewstuff6", "How To Donate:")
                text: i18ndc("knewstuff6", "A link which, when clicked, opens a website with information on donation in support of the entry", "Find out how to donate to this project")
                url: component.donationLink
                visible: component.donationLink
            }
        }

        Kirigami.SelectableLabel {
            id: summaryItem

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing

            textFormat: Text.RichText
        }

        Component {
            id: commentsPage

            Private.EntryCommentsPage {
                itemsModel: component.newStuffModel
                entryIndex: component.index
                entryName: component.name
                entryAuthorId: component.author.name
                entryProviderId: component.providerId
            }
        }
    }
}
