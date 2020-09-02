/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KHOTNEWSTUFF_TEST_H
#define KHOTNEWSTUFF_TEST_H

#include <KNSCore/Provider>
#include <KNSCore/EntryInternal>

#include <QObject>
#include <QStandardItemModel>

namespace KNSCore
{
class Engine;
}

class KNewStuff2Test : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool testAll READ testAll WRITE setTestAll NOTIFY testAllChanged)
public:
    KNewStuff2Test(const QString &configFile);

    void setTestAll(bool testall);
    bool testAll() const;
    Q_SIGNAL void testAllChanged();

    Q_INVOKABLE void entryTest();
    Q_INVOKABLE void providerTest();
    Q_INVOKABLE void engineTest();

    Q_INVOKABLE QObject *messages();
    void addMessage(const QString &message, const QString &iconName = QString());

public Q_SLOTS:
    void slotProvidersLoaded();
    void slotEngineError(const QString &error);
    void slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void slotInstallationFinished();

private:
    KNSCore::Engine *m_engine;
    bool m_testall;
    QString m_configFile;
    QStandardItemModel* m_messages;
};

#endif
