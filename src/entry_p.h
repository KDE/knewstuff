/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_KNEWSTUFFENTRY_P_H
#define KNEWSTUFF3_KNEWSTUFFENTRY_P_H

#include "core/entryinternal.h"

namespace KNS3
{
class EntryPrivate : public QSharedData
{
public:
    KNSCore::EntryInternal e;
    static Entry fromInternal(const KNSCore::EntryInternal *internal)
    {
        Entry e;
        e.d->e = *internal;
        return e;
    }
};
}

#endif
