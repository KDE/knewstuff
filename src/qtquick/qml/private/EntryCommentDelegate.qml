/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A card based delegate for showing a comment from a KNewStuffQuick::QuickCommentsModel
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami

import org.kde.newstuff 1.85 as NewStuff

RowLayout {
    id: component

    /**
     * The KNSQuick Engine object which handles all our content
     */
    property NewStuff.Engine engine

    /**
     * The username of the author of whatever the comment is attached to
     */
    property string entryAuthorId

    /**
     * The provider ID as supplied by the entry the comment is attached to
     */
    property string entryProviderId

    /**
     * The username of the comment's author
     */
    property string author

    /**
     * The OCS score, an integer from 1 to 100. It will be interpreted
     * as a 5 star rating, with half star support (0-10)
     */
    property int score

    /**
     * The title or subject line for the comment
     */
    property string title

    /**
     * The actual text of the comment
     */
    property alias reviewText: reviewLabel.text

    /**
     * The depth of the comment (in essence, how many parents the comment has)
     */
    property int depth

    readonly property NewStuff.Author commentAuthor: NewStuff.Author {
        id: commentAuthor
        engine: component.engine
        providerId: component.entryProviderId
        username: component.author
    }

    // prefer homepage, if any
    readonly property string authorUrl: (commentAuthor.homepage !== "")
        ? commentAuthor.homepage : commentAuthor.profilepage

    readonly property string authorLabel: if (commentAuthor.name) {
        if (author === entryAuthorId)  {
            return i18ndc("knewstuff5", "The author label in case the comment was written by the author of the content entry the comment is attached to", "%1 <i>(author)</i>", commentAuthor.name);
        } else {
            return commentAuthor.name;
        }
    } else {
        // This should be done automatically by Author class. But due to defered initialization, it is not.
        return component.author;
    }

    spacing: 0

    Repeater {
        model: component.depth
        delegate: Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: Kirigami.Units.largeSpacing
            Layout.leftMargin: 1
            color: Qt.tint(Kirigami.Theme.textColor, Qt.rgba(Kirigami.Theme.backgroundColor.r, Kirigami.Theme.backgroundColor.g, Kirigami.Theme.backgroundColor.b, 0.8))
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Separator {
            Layout.topMargin: (component.depth === 0) ? Kirigami.Units.largeSpacing : 0
            Layout.fillWidth: true
        }

        RowLayout {
            visible: (component.title !== "" || component.score !== 0)
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                text: (component.title === "")
                    ? i18ndc("knewstuff5", "Placeholder title for when a comment has no subject, but does have a rating", "<i>(no title)</i>")
                    : component.title
                level: 4
                Layout.fillWidth: true
            }

            Rating {
                rating: component.score
                reverseLayout: true
            }
        }

        Kirigami.SelectableLabel {
            id: reviewLabel
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            textFormat: TextEdit.RichText
            wrapMode: Text.Wrap
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
            }
            Kirigami.UrlButton {
                visible: component.authorUrl !== ""
                url: component.authorUrl
                text: component.authorLabel
            }
            QQC2.Label {
                visible: component.authorUrl === ""
                text: component.authorLabel
            }
            Kirigami.Avatar {
                id: authorIcon

                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                Layout.rightMargin: Kirigami.Units.largeSpacing

                source: commentAuthor.avatarUrl
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.largeSpacing
        }
    }
}
