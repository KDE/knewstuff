/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/**
 * @brief A component used to forward questions from KNewStuff's engine to the UI
 * 
 * This component is equivalent to the WidgetQuestionListener
 * @see KNewStuff::WidgetQuestionListener
 * @see KNewStuffCore::Question
 * @since 5.63
 */

import QtQuick 2.11
import QtQuick.Controls 2.11 as QtControls
import QtQuick.Layouts 1.11 as QtLayouts

import org.kde.kirigami 2.7 as Kirigami

import org.kde.newstuff.core 1.62 as NewStuffCore
import org.kde.newstuff 1.62 as NewStuff

QtControls.Dialog {
    id: dialog
    modal: true
    focus: true
    property int questionType
    anchors.centerIn: QtControls.Overlay.overlay
    margins: Kirigami.Units.largeSpacing
    padding: Kirigami.Units.largeSpacing
    standardButtons: {
        switch (questionType) {
            case NewStuffCore.Question.SelectFromListQuestion:
            case NewStuffCore.Question.InputTextQuestion:
            case NewStuffCore.Question.PasswordQuestion:
            case NewStuffCore.Question.ContinueCancelQuestion:
                // QtControls Dialog standardButtons does not have a Continue button...
                return QtControls.Dialog.Ok | QtControls.Dialog.Cancel;
                break;
            case NewStuffCore.Question.YesNoQuestion:
                return QtControls.Dialog.Yes | QtControls.Dialog.No;
                break;
            default:
                break;
        }
    }

    Connections {
        target: NewStuff.QuickQuestionListener
        function onAskListQuestion(title, question, list) {
            dialog.questionType = NewStuffCore.Question.SelectFromListQuestion;
            dialog.title = title;
            questionLabel.text = question;
            for (var i = 0; i < list.length; i++) {
                listView.model.append({ text: list[i] });
            }
            listView.currentIndex = 0;
            listView.visible = true;
            dialog.open();
        }
        function onAskContinueCancelQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.ContinueCancelQuestion;
            dialog.title = title;
            questionLabel.text = question;
            dialog.open();
        }
        function onAskTextInputQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.InputTextQuestion;
            dialog.title = title;
            questionLabel.text = question;
            textInput.visible = true;
            dialog.open();
        }
        function onAskPasswordQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.PasswordQuestion;
            dialog.title = title;
            questionLabel.text = question;
            textInput.echoMode = QtControls.TextInput.PasswordEchoOnEdit;
            textInput.visible = true;
            dialog.open();
        }
        function onAskYesNoQuestion(title, question) {
            dialog.questionType = NewStuffCore.Question.YesNoQuestion;
            dialog.title = title;
            questionLabel.text = question;
            dialog.open();
        }
    }
    function passResponse(responseIsContinue) {
        var input = "";
        switch(dialog.questionType) {
        case NewStuffCore.Question.SelectFromListQuestion:
            input = listView.currentItem.text;
            listView.model.clear();
            listView.visible = false;
            break;
        case NewStuffCore.Question.InputTextQuestion:
            input = textInput.text;
            textInput.text = "";
            textInput.visible = false;
            break;
        case NewStuffCore.Question.PasswordQuestion:
            input = textInput.text;
            textInput.text = "";
            textInput.visible = false;
            textInput.echoMode = QtControls.TextInput.Normal;
            break;
        case NewStuffCore.Question.ContinueCancelQuestion:
        case NewStuffCore.Question.YesNoQuestion:
        default:
            // Nothing special to do for these types of question, we just pass along the positive or negative response
            break;
        }
        NewStuff.QuickQuestionListener.passResponse(responseIsContinue, input);
    }

    QtLayouts.ColumnLayout {
        anchors.fill: parent
        property int maxWidth: applicationWindow().width - (dialog.leftPadding + dialog.leftMargin + dialog.rightMargin + dialog.rightPadding)
        QtControls.Label {
            id: questionLabel
            QtLayouts.Layout.maximumWidth: parent.maxWidth
            wrapMode: Text.Wrap
        }
        ListView {
            id: listView
            visible: false
            QtLayouts.Layout.maximumWidth: parent.maxWidth
            QtLayouts.Layout.fillWidth: true
            QtLayouts.Layout.minimumHeight: Kirigami.Units.gridUnit * 6
            model: ListModel { }
            delegate: Kirigami.BasicListItem {
                reserveSpaceForIcon: false
                text: model.text
            }
        }
        QtControls.TextField {
            id: textInput
            visible: false
            QtLayouts.Layout.maximumWidth: parent.maxWidth
            QtLayouts.Layout.fillWidth: true
        }
    }
    onAccepted: {
        passResponse(true);
    }
    onRejected: {
        passResponse(false);
    }
}
