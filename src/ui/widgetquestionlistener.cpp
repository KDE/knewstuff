/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "widgetquestionlistener.h"

#include "core/question.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KPasswordDialog>
#include <QInputDialog>

using namespace KNS3;

class WidgetQuestionListenerHelper
{
public:
    WidgetQuestionListenerHelper()
        : q(nullptr)
    {
    }
    ~WidgetQuestionListenerHelper()
    {
        delete q;
    }
    WidgetQuestionListenerHelper(const WidgetQuestionListenerHelper &) = delete;
    WidgetQuestionListenerHelper &operator=(const WidgetQuestionListenerHelper &) = delete;
    WidgetQuestionListener *q;
};
Q_GLOBAL_STATIC(WidgetQuestionListenerHelper, s_kns3_widgetQuestionListener)

WidgetQuestionListener *WidgetQuestionListener::instance()
{
    if (!s_kns3_widgetQuestionListener()->q) {
        new WidgetQuestionListener;
    }
    return s_kns3_widgetQuestionListener()->q;
}

WidgetQuestionListener::WidgetQuestionListener()
    : KNSCore::QuestionListener(nullptr)
{
    s_kns3_widgetQuestionListener()->q = this;
}

WidgetQuestionListener::~WidgetQuestionListener()
{
}

void KNS3::WidgetQuestionListener::askQuestion(KNSCore::Question *question)
{
    switch (question->questionType()) {
    case KNSCore::Question::SelectFromListQuestion: {
        bool ok = false;
        question->setResponse(QInputDialog::getItem(nullptr, question->title(), question->question(), question->list(), 0, false, &ok));
        if (ok) {
            question->setResponse(KNSCore::Question::OKResponse);
        } else {
            question->setResponse(KNSCore::Question::CancelResponse);
        }
    } break;
    case KNSCore::Question::ContinueCancelQuestion: {
        KMessageBox::ButtonCode response = KMessageBox::warningContinueCancel(nullptr, question->question(), question->title());
        if (response == KMessageBox::Continue) {
            question->setResponse(KNSCore::Question::ContinueResponse);
        } else {
            question->setResponse(KNSCore::Question::CancelResponse);
        }
    } break;
    case KNSCore::Question::PasswordQuestion: {
        KPasswordDialog dlg;
        dlg.setPrompt(question->question());
        if (dlg.exec()) {
            question->setResponse(dlg.password());
            question->setResponse(KNSCore::Question::ContinueResponse);
        } else {
            question->setResponse(KNSCore::Question::CancelResponse);
        }
    } break;
    case KNSCore::Question::YesNoQuestion:
    default: {
        KMessageBox::ButtonCode response = KMessageBox::questionTwoActions(nullptr,
                                                                           question->question(),
                                                                           question->title(),
                                                                           KGuiItem(i18nc("@action:button", "Yes"), QStringLiteral("dialog-ok")),
                                                                           KGuiItem(i18nc("@action:button", "No"), QStringLiteral("dialog-cancel")));
        if (response == KMessageBox::PrimaryAction) {
            question->setResponse(KNSCore::Question::YesResponse);
        } else {
            question->setResponse(KNSCore::Question::NoResponse);
        }
    } break;
    }
}
