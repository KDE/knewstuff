/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knewstuffaction.h"

#include <QAction>
#include <KLocalizedString>
#include <KActionCollection>

using namespace KNS3;

QAction *KNS3::standardAction(const QString &what,
                              const QObject *receiver,
                              const char *slot, KActionCollection *parent,
                              const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QLatin1String(name), action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}

QAction *KNS3::standardActionUpload(const QString &what,
                                    const QObject *receiver,
                                    const char *slot, KActionCollection *parent,
                                    const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QLatin1String(name), action);
    // FIXME: Get a specific upload icon!
    action->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}
