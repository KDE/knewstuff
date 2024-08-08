// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "searchpreset.h"

using namespace KNSCore;

class KNSCore::SearchPresetPrivate
{
public:
    SearchRequest request;
    QString displayName;
    QString iconName;
    SearchPreset::Type type;
    QString providerId; // not all providers can handle all search requests.
};

KNSCore::SearchPreset::SearchPreset(SearchPresetPrivate *dptr)
    : d(dptr)
{
}

SearchRequest KNSCore::SearchPreset::request() const
{
    return d->request;
}

QString KNSCore::SearchPreset::displayName() const
{
    return d->displayName;
}

QString KNSCore::SearchPreset::iconName() const
{
    return d->iconName;
}

KNSCore::SearchPreset::Type KNSCore::SearchPreset::type() const
{
    return d->type;
}

QString KNSCore::SearchPreset::providerId() const
{
    return d->providerId;
}
