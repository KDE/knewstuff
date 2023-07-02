/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "questionmanager.h"

#include "question.h"

using namespace KNSCore;

class QuestionManagerHelper
{
public:
    QuestionManagerHelper()
        : q(nullptr)
    {
    }
    ~QuestionManagerHelper()
    {
        delete q;
    }
    QuestionManagerHelper(const QuestionManagerHelper &) = delete;
    QuestionManagerHelper &operator=(const QuestionManagerHelper &) = delete;
    QuestionManager *q;
};
Q_GLOBAL_STATIC(QuestionManagerHelper, s_kns3_questionManager)

class KNSCore::QuestionManagerPrivate
{
public:
    QuestionManagerPrivate() = default;
};

QuestionManager *QuestionManager::instance()
{
    if (!s_kns3_questionManager()->q) {
        new QuestionManager;
    }
    return s_kns3_questionManager()->q;
}

QuestionManager::QuestionManager()
    : QObject(nullptr)
    , d(new QuestionManagerPrivate)
{
    s_kns3_questionManager()->q = this;
}

QuestionManager::~QuestionManager() = default;

#include "moc_questionmanager.cpp"
