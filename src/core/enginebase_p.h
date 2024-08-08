/*
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENGINEBASE_P_H
#define KNEWSTUFF3_ENGINEBASE_P_H

#include "cache.h"
#include "cache2_p.h"
#include "categorymetadata.h"
#include "enginebase.h"
#include "installation_p.h"
#include "searchpreset.h"
#include <Attica/ProviderManager>

namespace KNSCore
{
class ProviderCore;

class EngineBasePrivate
{
public:
    EngineBase *q;
    QString name;
    QStringList categories;
    QString adoptionCommand;
    QString useLabel;
    bool uploadEnabled = false;
    QUrl providerFileUrl;
    QStringList tagFilter;
    QStringList downloadTagFilter;
    Installation *installation = new Installation();
    Attica::ProviderManager *atticaProviderManager = nullptr;
    QList<SearchPreset> searchPresets;
    QSharedPointer<Cache2> cache;
    bool shouldRemoveDeletedEntries = false;
    QList<CategoryMetadata> categoriesMetadata;
    KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Only here for backwards compatible API") QHash<QString, QSharedPointer<KNSCore::Provider>> legacyProviders;
    QHash<QString, QSharedPointer<KNSCore::ProviderCore>> providerCores;
    KNSCore::EngineBase::ContentWarningType contentWarningType = KNSCore::EngineBase::ContentWarningType::Static;

    EngineBasePrivate(EngineBase *qptr);
    void addProvider(const QSharedPointer<KNSCore::ProviderCore> &provider);

private:
    // Don't use this. Use cache instead.
    friend class EngineBase; // we may use it though for backwards compat, albeit carefully ;)
    QSharedPointer<Cache> legacyCache;
};

} // namespace KNSCore

#endif
