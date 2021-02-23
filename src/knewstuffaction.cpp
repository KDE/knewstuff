/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knewstuffaction.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>

using namespace KNS3;

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 78)
QAction *KNS3::standardAction(const QString &what, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QLatin1String(name), action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}
#endif

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 78)
QAction *KNS3::standardActionUpload(const QString &what, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name)
{
    QAction *action = new QAction(what, parent);
    parent->addAction(QLatin1String(name), action);
    action->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    QObject::connect(action, SIGNAL(triggered(bool)), receiver, slot);

    return action;
}
#endif
