// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "../src/attica/atticaprovider_p.h"

using namespace KNSCore;

class AtticaProviderTest : public QObject
{
    Q_OBJECT
    std::unique_ptr<AtticaProvider> m_provider;

private Q_SLOTS:
    void initTestCase()
    {
    }
    void testReentrancy()
    {
        Attica::ProviderManager manager;
        {
            QSignalSpy spy(&manager, &Attica::ProviderManager::defaultProvidersLoaded);
            manager.loadDefaultProviders();
            QVERIFY(spy.wait());
            QVERIFY(spy.size() > 0);
        }
        AtticaProvider provider(manager.providers().at(0), QStringList{}, QString{});

        QSignalSpy spy(&provider, &AtticaProvider::loadingFinished);

        provider.loadEntries(Provider::SearchRequest(Provider::SortMode::Downloads, Provider::Filter::None, "kora"));
        provider.loadEntries(Provider::SearchRequest(Provider::SortMode::Downloads, Provider::Filter::None, "kde"));

        QVERIFY(spy.wait());
        if (spy.size() != 2) {
            spy.wait();
        }
        QCOMPARE(spy.size(), 2);
    }
};

QTEST_MAIN(AtticaProviderTest)

#include "atticaprovidertest.moc"
