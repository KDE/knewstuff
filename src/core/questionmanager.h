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

#ifndef KNS3_QUESTIONMANAGER_H
#define KNS3_QUESTIONMANAGER_H

#include <QObject>

#include "knewstuffcore_export.h"

#include "question.h"

namespace KNS3
{
class KNEWSTUFFCORE_EXPORT QuestionManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QuestionManager)
public:
    static QuestionManager* instance();
    virtual ~QuestionManager();

Q_SIGNALS:
    void askQuestion(KNS3::Question* question);

private:
    QuestionManager();
    class Private;
    Private* d;
};
}

#endif//KNS3_QUESTIONMANAGER_H
