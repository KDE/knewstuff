/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF2_CACHE_H
#define KNEWSTUFF2_CACHE_H

#include <knewstuff2/core/entry.h>

#include <QObject>

namespace KNS
{
class CoreEngine;
class Feed;
class Provider;
}

class KNewStuff2Cache : public QObject
{
    Q_OBJECT
public:
    KNewStuff2Cache();
    void run();
public Q_SLOTS:
    void slotEntryLoaded(KNS::Entry *entry, const KNS::Feed *feed, const KNS::Provider *provider);
    void slotEntriesFailed();
    void slotEntriesFinished();
private:
    void quitTest();
    KNS::CoreEngine *m_engine;
};

#endif
