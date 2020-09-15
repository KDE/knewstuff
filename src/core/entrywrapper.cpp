/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "entrywrapper.h"

namespace KNSCore {
    class EntryWrapper::Private {
    public:
        Private(const EntryInternal& entry
        )
            : entry(entry)
        {}
        const EntryInternal entry;
    };
}

KNSCore::EntryWrapper::EntryWrapper(const KNSCore::EntryInternal& entry, QObject* parent)
    : QObject(parent)
    , d(new Private(entry))
{}

KNSCore::EntryWrapper::~EntryWrapper()
{
    delete d;
}

KNSCore::EntryInternal KNSCore::EntryWrapper::entry() const
{
    return d->entry;
}
