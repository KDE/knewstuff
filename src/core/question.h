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

namespace KNSCore
{
/**
 * @short A way to ask a user a question from inside a GUI-less library (like KNewStuffCore)
 *
 * Rather than using a message box (which is a UI thing), when you want to ask your user
 * a question, create an instance of this class and use that instead. The consuming library
 * (in most cases KNewStuff or KNewStuffQuick) will listen to any question being asked,
 * and act appropriately (that is, KNewStuff will show a dialog with an appropriate dialog
 * box, and KNewStuffQuick will either request a question be asked if the developer is using
 * the plugin directly, or ask the question using an appropriate method for Qt Quick based
 * applications)
 *
 * The following is an example of a question asking the user to select an item from a list.
 *
 * @code
QStringList choices() << "foo" << "bar";
Question question(Question::SelectFromListQuestion);
question.setTitle("Pick your option");
question.setQuestion("Please select which option you would like");
question.setList(choices);
if(question.ask() == Question::OKResponse) {
    QString theChoice = question.response();
}
@endcode
 */
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
    Q_ENUM(Response)

    enum QuestionType {
        YesNoQuestion = 0,
        ContinueCancelQuestion = 1,
        InputTextQuestion = 2,
        SelectFromListQuestion = 3,
        PasswordQuestion = 4
    };
    Q_ENUM(QuestionType)

    explicit Question(QuestionType = YesNoQuestion, QObject *parent = nullptr);
    virtual ~Question();

    Response ask();

    void setQuestionType(QuestionType newType = YesNoQuestion);
    QuestionType questionType() const;

    void setQuestion(const QString &newQuestion);
    QString question() const;
    void setTitle(const QString &newTitle);
    QString title() const;
    void setList(const QStringList &newList);
    QStringList list() const;

    /**
     * When the user makes a choice on a question, that is a response. This is the return value in ask().
     * @param response This will set the response, and mark the question as answered
     */
    void setResponse(Response response);
    /**
     * If the user has any way of inputting data to go along with the response above, consider this a part
     * of the response. As such, you can set, and later get, that response as well. This does NOT mark the
     * question as answered ( @see setResponse(Response) ).
     * @param response This sets the string response for the question
     */
    void setResponse(const QString &response);
    QString response() const;
private:
    class Private;
    Private* d;
};
}

#endif//KNS3_QUESTION_H
