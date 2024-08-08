// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "categorymetadata.h"
#include "categorymetadata_p.h"

using namespace KNSCore;

KNSCore::CategoryMetadata::CategoryMetadata(CategoryMetadataPrivate *dptr)
    : d(dptr)
{
}

QString KNSCore::CategoryMetadata::id() const
{
    return d->id;
}

QString KNSCore::CategoryMetadata::name() const
{
    return d->name;
}

QString KNSCore::CategoryMetadata::displayName() const
{
    return d->displayName;
}
