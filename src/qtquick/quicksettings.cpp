/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "quicksettings.h"

#include <KAuthorized>

#include <QCoreApplication>

using namespace KNewStuffQuick;

class QuickSettingsHelper
{
public:
    QuickSettingsHelper()
        : q(nullptr)
    {
    }
    ~QuickSettingsHelper()
    {
    }
    QuickSettingsHelper(const QuickSettingsHelper &) = delete;
    QuickSettingsHelper &operator=(const QuickSettingsHelper &) = delete;
    Settings *q;
};
Q_GLOBAL_STATIC(QuickSettingsHelper, s_kns3_quickSettingsListener)

class KNewStuffQuick::SettingsPrivate
{
public:
    SettingsPrivate()
    {
    }
};

Settings *KNewStuffQuick::Settings::instance()
{
    if (!s_kns3_quickSettingsListener()->q) {
        new Settings;
    }
    return s_kns3_quickSettingsListener()->q;
}

Settings::Settings()
    : QObject(qApp)
    , d(new KNewStuffQuick::SettingsPrivate)
{
    s_kns3_quickSettingsListener->q = this;
}

Settings::~Settings() = default;

bool KNewStuffQuick::Settings::allowedByKiosk() const
{
    return KAuthorized::authorize(KAuthorized::GHNS);
}
