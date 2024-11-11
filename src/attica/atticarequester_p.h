// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <attica/content.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "entry.h"
#include "searchrequest.h"

namespace KNSCore
{
class AtticaProvider;

class AtticaRequester : public QObject
{
    Q_OBJECT
public:
    explicit AtticaRequester(const KNSCore::SearchRequest &request, AtticaProvider *provider, QObject *parent = nullptr);
    void start();
    [[nodiscard]] KNSCore::SearchRequest request() const;

Q_SIGNALS:
    void entriesLoaded(const KNSCore::Entry::List &list);
    void loadingDone();
    void loadingFailed();
    void entryDetailsLoaded(const KNSCore::Entry &entry);

private Q_SLOTS:
    void detailsLoaded(Attica::BaseJob *job);
    void categoryContentsLoaded(Attica::BaseJob *job);

private:
    void startInternal();
    void checkForUpdates();
    [[nodiscard]] Entry::List installedEntries() const;
    [[nodiscard]] Entry entryFromAtticaContent(const Attica::Content &content);

    KNSCore::SearchRequest m_request;
    AtticaProvider *m_provider;
    QSet<Attica::BaseJob *> m_updateJobs;
};

} // namespace KNSCore
