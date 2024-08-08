// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include "compat_p.h"
#include "provider.h"
#include "provider_p.h"
#include "providerbase_p.h"
#include "providercore.h"
#include "providercore_p.h"
#include "searchrequest_p.h"

namespace KNSCore
{

class ProviderBubbleWrap : public Provider
{
    Q_OBJECT
public:
    ProviderBubbleWrap(const QSharedPointer<KNSCore::ProviderCore> &core)
        : m_core(core)
    {
        // These are unidirectional.
        connect(m_core->d->base, &ProviderBase::basicsLoaded, this, [this] {
            // The Provider async loads these and eventually emits loaded. Technically they
            // could get set by the outside, we actively do not support this though.
            setVersion(m_core->d->base->version());
            setWebsite(m_core->d->base->website());
            setHost(m_core->d->base->host());
            setContactEmail(m_core->d->base->contactEmail());
            setSupportsSsl(m_core->d->base->supportsSsl());
            Q_EMIT basicsLoaded();
        });
        connect(m_core->d->base, &ProviderBase::providerInitialized, this, [this](const auto & /*providerBase*/) {
            Q_EMIT providerInitialized(this);
        });
        // const KNSCore::SearchRequest & _t1, const KNSCore::Entry::List &
        connect(m_core->d->base, &ProviderBase::loadingFinished, this, [this](const auto &request, const auto &entries) {
            Q_EMIT loadingFinished(KNSCompat::searchRequestToLegacy(request), entries);
        });
        connect(m_core->d->base, &ProviderBase::loadingFailed, this, [this](const auto &request) {
            Q_EMIT loadingFailed(KNSCompat::searchRequestToLegacy(request));
        });
        connect(m_core->d->base, &ProviderBase::entryDetailsLoaded, this, &Provider::entryDetailsLoaded);
        connect(m_core->d->base, &ProviderBase::payloadLinkLoaded, this, &Provider::payloadLinkLoaded);
        connect(m_core->d->base, &ProviderBase::commentsLoaded, this, &Provider::commentsLoaded);
        connect(m_core->d->base, &ProviderBase::personLoaded, this, &Provider::personLoaded);
        connect(m_core->d->base, &ProviderBase::searchPresetsLoaded, this, [this](const auto &presets) {
            QList<KNSCore::Provider::SearchPreset> legacies;
            legacies.reserve(presets.size());
            for (const auto &preset : presets) {
                legacies.append(KNSCompat::searchPresetToLegacy(preset));
            }
            Q_EMIT searchPresetsLoaded(legacies);
        });
        connect(m_core->d->base, &ProviderBase::signalInformation, this, &Provider::signalInformation);
        connect(m_core->d->base, &ProviderBase::signalError, this, &Provider::signalError);
        connect(m_core->d->base, &ProviderBase::signalErrorCode, this, &Provider::signalErrorCode);
        connect(m_core->d->base, &ProviderBase::categoriesMetadataLoaded, this, [this](const QList<KNSCore::CategoryMetadata> &categories) {
            QList<KNSCore::Provider::CategoryMetadata> legacies;
            legacies.reserve(categories.size());
            for (const auto &category : categories) {
                legacies.append(KNSCompat::categoryMetadataToLegacy(category));
            }
            Q_EMIT categoriesMetadataLoded(legacies);
        });

        // These are bidirectional. We do not use public setters for these to avoid change signal loops.
        // Bit awkward to hop through all the d pointers but this class is not going to outlife KF6 anyway.
        connect(m_core->d->base, &ProviderBase::tagFilterChanged, this, [this] {
            d->tagFilter = m_core->d->base->tagFilter();
        });
        connect(this, &Provider::tagFilterChanged, this, [this] {
            m_core->d->base->d->tagFilter = d->tagFilter;
        });
        connect(m_core->d->base, &ProviderBase::downloadTagFilterChanged, this, [this] {
            d->downloadTagFilter = m_core->d->base->downloadTagFilter();
        });
        connect(this, &Provider::downloadTagFilterChanged, this, [this] {
            m_core->d->base->d->downloadTagFilter = d->downloadTagFilter;
        });
    }

    [[nodiscard]] QString id() const override
    {
        return m_core->d->base->id();
    }

    bool setProviderXML(const QDomElement &xmldata) override
    {
        return m_core->d->base->setProviderXML(xmldata);
    }

    [[nodiscard]] bool isInitialized() const override
    {
        return m_core->d->base->isInitialized();
    }

    void setCachedEntries(const KNSCore::Entry::List &cachedEntries) override
    {
        m_core->d->base->setCachedEntries(cachedEntries);
    }

    [[nodiscard]] QString name() const override
    {
        return m_core->d->base->name();
    }

    [[nodiscard]] QUrl icon() const override
    {
        return m_core->d->base->icon();
    }

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override
    {
        m_core->d->base->loadEntries(searchRequestFromLegacy(request));
    }

    void loadEntryDetails(const KNSCore::Entry &entry) override
    {
        m_core->d->base->loadEntryDetails(entry);
    }

    void loadPayloadLink(const Entry &entry, int linkId) override
    {
        m_core->d->base->loadPayloadLink(entry, linkId);
    }

    void loadComments(const KNSCore::Entry &entry, int commentsPerPage, int page) override
    {
        m_core->d->base->loadComments(entry, commentsPerPage, page);
    }

    void loadPerson(const QString &username) override
    {
        m_core->d->base->loadPerson(username);
    }

    void loadBasics() override
    {
        // Noop. Basics (host, website, ssl etc.) now load on-demand.
    }

    bool userCanVote() override
    {
        return m_core->d->base->userCanVote();
    }

    void vote(const Entry &entry, uint rating) override
    {
        m_core->d->base->vote(entry, rating);
    }

    bool userCanBecomeFan() override
    {
        return m_core->d->base->userCanBecomeFan();
    }

    void becomeFan(const Entry &entry) override
    {
        m_core->d->base->becomeFan(entry);
    }

private:
    QSharedPointer<KNSCore::ProviderCore> m_core;
};

} // namespace KNSCore
