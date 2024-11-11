// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include <QSignalSpy>
#include <QTest>
#include <QtGlobal>

#include "../src/attica/atticaprovider_p.h"

using namespace KNSCore;
using namespace std::chrono_literals;

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
        constexpr auto networkTimeout = 30s; // when this test does network IO it may be slow, in particular on the CI
        Attica::ProviderManager manager;
        {
            QSignalSpy spy(&manager, &Attica::ProviderManager::defaultProvidersLoaded);
            manager.loadDefaultProviders();
            QVERIFY(spy.wait(networkTimeout));
            QVERIFY(spy.size() > 0);
        }
        AtticaProvider provider(manager.providers().at(0), QStringList{}, QString{});

        QSignalSpy spy(&provider, &AtticaProvider::loadingDone);

        provider.loadEntries(SearchRequest(SortMode::Downloads, Filter::None, "kora"));
        provider.loadEntries(SearchRequest(SortMode::Downloads, Filter::None, "kde"));

        QVERIFY(spy.wait(networkTimeout));
        if (spy.size() != 2) {
            spy.wait(networkTimeout);
        }
        QCOMPARE(spy.size(), 2);
    }
};

QTEST_MAIN(AtticaProviderTest)

#include "atticaprovidertest.moc"
