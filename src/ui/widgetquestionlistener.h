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

#ifndef KNS3_WIDGETQUESTIONLISTENER_H
#define KNS3_WIDGETQUESTIONLISTENER_H

#include "core/questionlistener.h"

namespace KNS3
{
class WidgetQuestionListener : public KNSCore::QuestionListener
{
    Q_OBJECT
    Q_DISABLE_COPY(WidgetQuestionListener)
public:
    static WidgetQuestionListener* instance();
    virtual ~WidgetQuestionListener();

    Q_SLOT void askQuestion(KNSCore::Question* question) override;
private:
    WidgetQuestionListener();
};
}

#endif//KNS3_WIDGETQUESTIONLISTENER_H
