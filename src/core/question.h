/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNS3_QUESTION_H
#define KNS3_QUESTION_H

#include <QObject>

#include "knewstuffcore_export.h"

#include <memory>

namespace KNSCore
{
class Entry;
class QuestionPrivate;
/*!
   \class KNSCore::Question
   \inmodule KNewStuffCore

   \brief A way to ask a user a question from inside a GUI-less library (like KNewStuffCore).

   Rather than using a message box (which is a UI thing), when you want to ask your user
   a question, create an instance of this class and use that instead. The consuming library
   (in most cases KNewStuff or KNewStuffQuick) will listen to any question being asked,
   and act appropriately (that is, KNewStuff will show a dialog with an appropriate dialog
   box, and KNewStuffQuick will either request a question be asked if the developer is using
   the plugin directly, or ask the question using an appropriate method for Qt Quick based
   applications)

   The following is an example of a question asking the user to select an item from a list.

   \code
QStringList choices() << "foo" << "bar";
Question question(Question::SelectFromListQuestion);
question.setTitle("Pick your option");
question.setQuestion("Please select which option you would like");
question.setList(choices);
question.setEntry(entry);
if (question.ask() == Question::OKResponse) {
    QString theChoice = question.response();
}
\endcode
 */
class KNEWSTUFFCORE_EXPORT Question : public QObject
{
    Q_OBJECT
public:
    /*!
     * \enum KNSCore::Question::Response
     *
     * \brief Defines how the user responded to the question.
     *
     * \sa QuestionType
     *
     * \value InvalidResponse
     * The user did not provide a valid response.
     *
     * \value YesResponse
     * The user selected yes.
     *
     * \value NoResponse
     * The user selected no.
     *
     * \value ContinueResponse
     * The user selected continue.
     *
     * \value CancelResponse
     * The user selected cancel.
     *
     * \value OKResponse
     * The user selected OK.
     */
    enum Response {
        InvalidResponse = 0,
        YesResponse = 1,
        NoResponse = 2,
        ContinueResponse = 3,
        CancelResponse = 4,
        OKResponse = YesResponse,
    };
    Q_ENUM(Response)

    /*!
     * \enum KNSCore::Question::QuestionType
     *
     * \brief Defines the type of question to be presented to the user.
     *
     * \sa Response
     *
     * \value YesNoQuestion
     * The question can be answered with either yes or no.
     *
     * \value ContinueCancelQuestion
     * The question can be answered with either continue or cancel.
     *
     * \value InputTextQuestion
     * Answering the question requires text input from the user.
     *
     * \value SelectFromListQuestion
     * Answering the question requires selecting from a list of choices.
     *
     * \value PasswordQuestion
     * Answering the question requires the user to input a password.
     */
    enum QuestionType {
        YesNoQuestion = 0,
        ContinueCancelQuestion = 1,
        InputTextQuestion = 2,
        SelectFromListQuestion = 3,
        PasswordQuestion = 4,
    };
    Q_ENUM(QuestionType)

    explicit Question(QuestionType = YesNoQuestion, QObject *parent = nullptr);
    ~Question() override;

    /*!
     * Returns the user's reponse to the question.
     *
     * \sa setResponse
     */
    Response ask();

    /*!
     * Sets the type of question presented to the user.
     *
     * \a newType is the question type
     */
    void setQuestionType(QuestionType newType = YesNoQuestion);

    /*!
     * Returns the configured question type.
     */
    QuestionType questionType() const;

    /*!
     * Sets the question to be presented to the user.
     *
     * \a newQuestion is the question message
     *
     */
    void setQuestion(const QString &newQuestion);

    /*!
     * Returns the question message to be presented to the user.
     */
    QString question() const;

    /*!
     * Sets the title of the UX element presented to the user.
     *
     * \a newTitle is the title text
     */
    void setTitle(const QString &newTitle);

    /*!
     * Returns the title of the UX element.
     */
    QString title() const;

    /*!
     * Sets the list of optional choices to present to the user to \a newList
     * for a SelectFromListQuestion.
     */
    void setList(const QStringList &newList);

    /*!
     * Returns the list of choices to present to the user, if any.
     */
    QStringList list() const;

    /*!
     * Sets the data \a entry container this question refers to.
     */
    void setEntry(const Entry &entry);

    /*!
     * Returns the configured entry.
     */
    Entry entry() const;

    /*!
     * When the user makes a choice on a question, that is a response. This is the return value in ask().
     *
     * \a response This will set the response, and mark the question as answered
     *
     */
    void setResponse(Response response);

    /*!
     * If the user has any way of inputting data to go along with the response above, consider this a part
     * of the response. As such, you can set, and later get, that response as well. This does NOT mark the
     * question as answered.
     *
     * \a response This sets the string response for the question
     *
     * \sa setResponse
     */
    void setResponse(const QString &response);

    /*!
     * Returns the response data provided by the user, if any.
     */
    QString response() const;

private:
    const std::unique_ptr<QuestionPrivate> d;
};
}

#endif // KNS3_QUESTION_H
