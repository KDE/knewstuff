/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

Question::Question(QuestionType questionType, QObject *parent)
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

    Q_EMIT QuestionManager::instance()->askQuestion(this);
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

void Question::setQuestion(const QString &newQuestion)
{
    d->question = newQuestion;
}

QString Question::question() const
{
    return d->question;
}

void Question::setTitle(const QString &newTitle)
{
    d->title = newTitle;
}

QString Question::title() const
{
    return d->title;
}

void Question::setList(const QStringList &newList)
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

void Question::setResponse(const QString &response)
{
    d->textResponse = response;
}

QString Question::response() const
{
    return d->textResponse;
}
