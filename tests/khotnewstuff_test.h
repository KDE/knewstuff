/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
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
    void addMessage(const QString &message, const QString &iconName = QStringLiteral());

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
