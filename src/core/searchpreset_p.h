// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once
#include "searchpreset.h"

namespace KNSCore
{

class SearchPresetPrivate
{
public:
    SearchRequest request;
    QString displayName;
    QString iconName;
    SearchPreset::Type type;
    QString providerId; // not all providers can handle all search requests.
};

} // namespace KNSCore
