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

#ifndef KNS3_QUESTION_H
#define KNS3_QUESTION_H

#include <QObject>

#include "knewstuffcore_export.h"

namespace KNS3
{
class KNEWSTUFFCORE_EXPORT Question : public QObject
{
    Q_OBJECT
public:
    enum Response {
        InvalidResponse = 0,
        YesResponse = 1,
        NoResponse = 2,
        ContinueResponse = 3,
        CancelResponse = 4,
        OKResponse = YesResponse
    };

    enum QuestionType {
        YesNoQuestion = 0,
        ContinueCancelQuestion = 1,
        InputTextQuestion = 2,
        SelectFromListQuestion = 3,
        PasswordQuestion = 4
    };

    explicit Question(QuestionType = YesNoQuestion, QObject* parent = 0);
    virtual ~Question();

    Response ask();

    void setQuestionType(QuestionType newType = YesNoQuestion);
    QuestionType questionType() const;

    void setQuestion(QString newQuestion);
    QString question() const;
    void setTitle(QString newTitle);
    QString title() const;
    void setList(QStringList newList);
    QStringList list() const;

    // When the user makes a choice on a question, that is a response. This is the return value in ask().
    void setResponse(Response response);
    // If the user has any way of inputting data to go along with the response above, consider this a part
    // of the response. As such, you can set, and later get, that response as well.
    void setResponse(QString response);
    QString response();
private:
    class Private;
    Private* d;
};
}

#endif//KNS3_QUESTION_H
