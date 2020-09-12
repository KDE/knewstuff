/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "quickquestionlistener.h"

#include "core/question.h"

#include <QCoreApplication>

using namespace KNewStuffQuick;

class QuickQuestionListenerHelper {
public:
    QuickQuestionListenerHelper() : q(nullptr) {}
    ~QuickQuestionListenerHelper() { }
    QuickQuestionListenerHelper(const QuickQuestionListenerHelper &) = delete;
    QuickQuestionListenerHelper& operator=(const QuickQuestionListenerHelper &) = delete;
    QuickQuestionListener *q;
};
Q_GLOBAL_STATIC(QuickQuestionListenerHelper, s_kns3_quickQuestionListener)

class QuickQuestionListener::Private {
public:
    Private() {}
    KNSCore::Question *question = nullptr;
};

QuickQuestionListener *QuickQuestionListener::instance()
{
    if(!s_kns3_quickQuestionListener()->q) {
        new QuickQuestionListener;
    }
    return s_kns3_quickQuestionListener()->q;
}

QuickQuestionListener::QuickQuestionListener()
    : KNSCore::QuestionListener(nullptr)
    , d(new Private)
{
    setParent(qApp);
    s_kns3_quickQuestionListener()->q = this;
}

QuickQuestionListener::~QuickQuestionListener()
{
    if (d->question) {
        d->question->setResponse(KNSCore::Question::CancelResponse);
    }
    delete d;
}

void QuickQuestionListener::askQuestion(KNSCore::Question *question)
{
    d->question = question;
    switch(question->questionType())
    {
    case KNSCore::Question::SelectFromListQuestion:
        Q_EMIT askListQuestion(question->title(), question->question(), question->list());
        break;
    case KNSCore::Question::ContinueCancelQuestion:
        Q_EMIT askContinueCancelQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::InputTextQuestion:
        Q_EMIT askTextInputQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::PasswordQuestion:
        Q_EMIT askPasswordQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::YesNoQuestion:
    default:
        Q_EMIT askYesNoQuestion(d->question->title(), d->question->question());
        break;
    }
}

void KNewStuffQuick::QuickQuestionListener::passResponse(bool responseIsContinue, QString input)
{
    if (d->question) {
        if (responseIsContinue) {
            d->question->setResponse(input);
            switch(d->question->questionType())
            {
            case KNSCore::Question::ContinueCancelQuestion:
                d->question->setResponse(KNSCore::Question::ContinueResponse);
                break;
            case KNSCore::Question::YesNoQuestion:
                d->question->setResponse(KNSCore::Question::YesResponse);
                break;
            case KNSCore::Question::SelectFromListQuestion:
            case KNSCore::Question::InputTextQuestion:
            case KNSCore::Question::PasswordQuestion:
            default:
                d->question->setResponse(KNSCore::Question::OKResponse);
                break;
            }
        } else {
            switch(d->question->questionType())
            {
            case KNSCore::Question::YesNoQuestion:
                d->question->setResponse(KNSCore::Question::NoResponse);
                break;
            case KNSCore::Question::SelectFromListQuestion:
            case KNSCore::Question::InputTextQuestion:
            case KNSCore::Question::PasswordQuestion:
            case KNSCore::Question::ContinueCancelQuestion:
            default:
                d->question->setResponse(KNSCore::Question::CancelResponse);
                break;
            }
        }
        d->question = nullptr;
    }
}
