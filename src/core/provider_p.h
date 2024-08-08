// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
// SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

#include "provider.h"

#include <KLocalizedString>

#include <QTimer>

namespace KNSCore
{

class ProviderPrivate
{
public:
    ProviderPrivate(Provider *qq)
        : q(qq)
    {
    }
    Provider *const q;
    QStringList tagFilter;
    QStringList downloadTagFilter;

    QTimer *basicsThrottle{nullptr};
    QString version;
    QUrl website;
    QUrl host;
    QString contactEmail;
    QString name;
    QUrl icon;
    bool supportsSsl{false};
    bool basicsGot{false};

    void updateOnFirstBasicsGet()
    {
        if (!basicsGot) {
            basicsGot = true;
            QTimer::singleShot(0, q, &Provider::loadBasics);
        }
    };
    void throttleBasics()
    {
        if (!basicsThrottle) {
            basicsThrottle = new QTimer(q);
            basicsThrottle->setInterval(0);
            basicsThrottle->setSingleShot(true);
            QObject::connect(basicsThrottle, &QTimer::timeout, q, &Provider::basicsLoaded);
        }
        basicsThrottle->start();
    }
};

} // namespace KNSCore
