// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "transientmagicianassistant.h"

class Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void initializeEngine([[maybe_unused]] QQmlEngine *engine, [[maybe_unused]] const char *uri) override
    {
    }

    void registerTypes(const char *uri) override
    {
        qmlRegisterType<TransientMagicanAssistant>(uri, 1, 0, "TransientMagicanAssistant");
    }
};

#include "plugin.moc"
