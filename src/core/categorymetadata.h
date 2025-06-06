// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QString>

#include "knewstuffcore_export.h"

namespace KNSCore
{

class CategoryMetadataPrivate;

/*!
 * \class KNSCore::CategoryMetadata
 * \inheaderfile KNSCore/CategoryMetadata
 * \inmodule KNewStuffCore
 * \brief Describes a category.
 * \since 6.9
 */
class KNEWSTUFFCORE_EXPORT CategoryMetadata
{
public:
    [[nodiscard]] QString id() const;
    [[nodiscard]] QString name() const;
    [[nodiscard]] QString displayName() const;

private:
    friend class AtticaProvider;
    friend class ProviderBubbleWrap;
    CategoryMetadata(CategoryMetadataPrivate *dptr);
    std::shared_ptr<CategoryMetadataPrivate> d;
};

} // namespace KNSCore
