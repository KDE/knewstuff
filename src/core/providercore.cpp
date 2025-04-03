// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "providercore.h"

#include "providerbase_p.h"

using namespace KNSCore;

class KNSCore::ProviderCorePrivate
{
public:
    ProviderBase *base;
};

KNSCore::ProviderCore::ProviderCore(ProviderBase *base, QObject *parent)
    : QObject(parent)
    , d(new ProviderCorePrivate{.base = [this, base] {
        connect(base, &ProviderBase::basicsLoaded, this, &ProviderCore::basicsLoaded);
        base->setParent(this);
        return base;
    }()})
{
}

KNSCore::ProviderCore::~ProviderCore() = default;

QString KNSCore::ProviderCore::version() const
{
    return d->base->version();
}

QUrl KNSCore::ProviderCore::website() const
{
    return d->base->website();
}

QUrl KNSCore::ProviderCore::host() const
{
    return d->base->host();
}

QString KNSCore::ProviderCore::contactEmail() const
{
    return d->base->contactEmail();
}

bool KNSCore::ProviderCore::supportsSsl() const
{
    return d->base->supportsSsl();
}

#include "moc_providercore.cpp"
