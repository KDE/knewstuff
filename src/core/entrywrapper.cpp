/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "entrywrapper.h"

namespace KNSCore
{
class EntryWrapperPrivate
{
public:
    EntryWrapperPrivate(const Entry &entry)
        : entry(entry)
    {
    }
    const Entry entry;
};
}

KNSCore::EntryWrapper::EntryWrapper(const KNSCore::Entry &entry, QObject *parent)
    : QObject(parent)
    , d(new EntryWrapperPrivate(entry))
{
}

KNSCore::EntryWrapper::~EntryWrapper() = default;

KNSCore::Entry KNSCore::EntryWrapper::entry() const
{
    return d->entry;
}
