/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF2_STANDARD_H
#define KNEWSTUFF2_STANDARD_H

#include <knewstuff2/core/entry.h>

#include <QObject>

namespace KNS
{
class Engine;
}

class KNewStuff2Standard : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Standard();
    void run(bool upload, bool modal, QString file);
private:
    KNS::Engine *m_engine;
};

#endif
