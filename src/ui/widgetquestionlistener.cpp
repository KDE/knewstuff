/*
    This file is part of KNewStuffCore.

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

#include "widgetquestionlistener.h"

#include "core/question.h"

#include <kmessagebox.h>
#include <KPasswordDialog>
#include <QInputDialog>

using namespace KNS3;

class WidgetQuestionListenerHelper {
public:
    WidgetQuestionListenerHelper() : q(nullptr) {}
    ~WidgetQuestionListenerHelper() { delete q; }
    WidgetQuestionListenerHelper(const WidgetQuestionListenerHelper &) = delete;
    WidgetQuestionListenerHelper& operator=(const WidgetQuestionListenerHelper &) = delete;
    WidgetQuestionListener *q;
};
Q_GLOBAL_STATIC(WidgetQuestionListenerHelper, s_kns3_widgetQuestionListener)

WidgetQuestionListener* WidgetQuestionListener::instance()
{
    if(!s_kns3_widgetQuestionListener()->q) {
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

void KNS3::WidgetQuestionListener::askQuestion(KNSCore::Question* question)
{
    switch(question->questionType())
    {
    case KNSCore::Question::SelectFromListQuestion:
        {
            bool ok = false;
            question->setResponse(QInputDialog::getItem(nullptr, question->title(), question->question(), question->list(), 0, false, &ok));
            if(ok) {
                question->setResponse(KNSCore::Question::OKResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::ContinueCancelQuestion:
        {
            KMessageBox::ButtonCode response = KMessageBox::warningContinueCancel(nullptr, question->question(), question->title());
            if(response == KMessageBox::Continue) {
                question->setResponse(KNSCore::Question::ContinueResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::PasswordQuestion:
        {
            KPasswordDialog dlg;
            dlg.setPrompt(question->question());
            if(dlg.exec()) {
                question->setResponse(dlg.password());
                question->setResponse(KNSCore::Question::ContinueResponse);
            }
            else {
                question->setResponse(KNSCore::Question::CancelResponse);
            }
        }
        break;
    case KNSCore::Question::YesNoQuestion:
    default:
        {
            KMessageBox::ButtonCode response = KMessageBox::questionYesNo(nullptr, question->question(), question->title());
            if(response == KMessageBox::Yes) {
                question->setResponse(KNSCore::Question::YesResponse);
            }
            else {
                question->setResponse(KNSCore::Question::NoResponse);
            }
        }
        break;
    }
}
