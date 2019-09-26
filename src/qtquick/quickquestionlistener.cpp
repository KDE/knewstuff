/*
    This file is part of KNewStuffQuick.

    Copyright (c) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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
        emit askListQuestion(question->title(), question->question(), question->list());
        break;
    case KNSCore::Question::ContinueCancelQuestion:
        emit askContinueCancelQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::InputTextQuestion:
        emit askTextInputQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::PasswordQuestion:
        emit askPasswordQuestion(d->question->title(), d->question->question());
        break;
    case KNSCore::Question::YesNoQuestion:
    default:
        emit askYesNoQuestion(d->question->title(), d->question->question());
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
