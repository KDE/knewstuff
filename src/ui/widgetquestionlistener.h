/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    static WidgetQuestionListener *instance();
    ~WidgetQuestionListener() override;

    Q_SLOT void askQuestion(KNSCore::Question *question) override;

private:
    WidgetQuestionListener();
};
}

#endif // KNS3_WIDGETQUESTIONLISTENER_H
