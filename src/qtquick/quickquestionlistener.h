/*
    This file is part of KNewStuffQuick.

    Copyright (c) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

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

#ifndef KNSQ_QUICKQUESTIONLISTENER_H
#define KNSQ_QUICKQUESTIONLISTENER_H

#include "core/questionlistener.h"

namespace KNewStuffQuick
{
class QuickQuestionListener : public KNSCore::QuestionListener
{
    Q_OBJECT
    Q_DISABLE_COPY(QuickQuestionListener)
public:
    static QuickQuestionListener *instance();
    virtual ~QuickQuestionListener();

    Q_SLOT void askQuestion(KNSCore::Question *question) override;

    Q_SIGNAL void askListQuestion(QString title, QString question, QStringList list);
    Q_SIGNAL void askContinueCancelQuestion(QString title, QString question);
    Q_SIGNAL void askTextInputQuestion(QString title, QString question);
    Q_SIGNAL void askPasswordQuestion(QString title, QString question);
    Q_SIGNAL void askYesNoQuestion(QString title, QString question);

    Q_SLOT void passResponse(bool responseIsContinue, QString input);
private:
    QuickQuestionListener();
    class Private;
    Private *d;
};
}

#endif//KNSQ_QUICKQUESTIONLISTENER_H
