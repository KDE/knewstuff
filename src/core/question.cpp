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

#include "question.h"

#include "questionmanager.h"

#include <QCoreApplication>

using namespace KNSCore;

class Question::Private {
public:
    Private()
        : questionActive(false)
        , questionType(YesNoQuestion)
        , response(InvalidResponse)
    {}
    QString question;
    QString title;
    QStringList list;

    bool questionActive;
    Question::QuestionType questionType;
    Question::Response response;
    QString textResponse;
};

Question::Question(QuestionType questionType, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->questionType = questionType;
}

Question::~Question()
{
    delete d;
}

Question::Response Question::ask()
{
    d->questionActive = true;

    emit QuestionManager::instance()->askQuestion(this);
    while(d->questionActive)
    {
        qApp->processEvents();
    }

    return d->response;
}

Question::QuestionType Question::questionType() const
{
    return d->questionType;
}

void Question::setQuestionType(Question::QuestionType newType)
{
    d->questionType = newType;
}

void Question::setQuestion(QString newQuestion)
{
    d->question = newQuestion;
}

QString Question::question() const
{
    return d->question;
}

void Question::setTitle(QString newTitle)
{
    d->title = newTitle;
}

QString Question::title() const
{
    return d->title;
}

void Question::setList(QStringList newList)
{
    d->list = newList;
}

QStringList Question::list() const
{
    return d->list;
}

void Question::setResponse(Response response)
{
    d->response = response;
    d->questionActive = false;
}

void Question::setResponse(QString response)
{
    d->textResponse = response;
}

QString Question::response()
{
    return d->textResponse;
}
