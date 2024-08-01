// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <optional>

#include <QObject>
#include <QQmlEngine>
#include <QQmlParserStatus>

class QQuickWindow;

class TransientMagicianAssistant : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT
public:
    using QObject::QObject;

    void classBegin() override;
    void componentComplete() override;
    std::optional<QQuickWindow *> findWindowParent();
};
