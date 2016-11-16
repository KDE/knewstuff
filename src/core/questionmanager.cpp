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

#include "questionmanager.h"

using namespace KNSCore;

class QuestionManagerHelper {
public:
    QuestionManagerHelper() : q(0) {}
    ~QuestionManagerHelper() { delete q; }
    QuestionManager *q;
};
Q_GLOBAL_STATIC(QuestionManagerHelper, s_kns3_questionManager)

class QuestionManager::Private
{
public:
    Private() {}
};


QuestionManager* QuestionManager::instance()
{
    if(!s_kns3_questionManager()->q) {
        new QuestionManager;
    }
    return s_kns3_questionManager()->q;
}

QuestionManager::QuestionManager()
    : QObject(0)
    , d(new Private)
{
    s_kns3_questionManager()->q = this;
}

QuestionManager::~QuestionManager()
{
    delete d;
}
