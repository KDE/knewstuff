// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#include "transientmagicianassistant.h"

#include <QQuickItem>
#include <QQuickWindow>

#include "knewstuffquickprivate_debug.h"

void TransientMagicanAssistant::classBegin()
{
}

void TransientMagicanAssistant::componentComplete()
{
    auto optionalWindow = findWindowParent();
    if (!optionalWindow) {
        qCWarning(KNEWSTUFFQUICKPRIVATE) << "Unexpectedly have not found a window as parent of TransientMagicanAssistant";
        return;
    }
    auto window = optionalWindow.value();

    if (window->transientParent()) {
        return;
    }
    qCWarning(KNEWSTUFFQUICKPRIVATE)
        << "You have not set a transientParent on KNewStuff.Dialog or .Action. This may cause severe problems with window and lifetime management. "
           "We'll try to fix the situation automatically but you should really provide an explicit transientParent";

    qCDebug(KNEWSTUFFQUICKPRIVATE) << "Applying transient parent magic assistance to " << window << "🪄";
    for (auto sentinel = qobject_cast<QObject *>(window)->parent(); sentinel; sentinel = sentinel->parent()) {
        if (auto item = qobject_cast<QQuickItem *>(sentinel); item && item->window()) {
            qCDebug(KNEWSTUFFQUICKPRIVATE) << window << "is now transient for" << item->window();
            connect(item, &QQuickItem::windowChanged, window, [window](QQuickWindow *newParent) {
                window->setTransientParent(newParent);
            });
            window->setTransientParent(item->window());
            return;
        }
    }
    qCWarning(KNEWSTUFFQUICKPRIVATE) << "Failed to do magic. Found no suitable window to become transient for.";
}

std::optional<QQuickWindow *> TransientMagicanAssistant::findWindowParent()
{
    // Finds the KNewStuff.Dialog though practically it is always parent()->parent() it is a bit tidier to search
    // for it instead.
    for (auto sentinel = parent(); sentinel; sentinel = sentinel->parent()) {
        if (auto window = qobject_cast<QQuickWindow *>(sentinel)) {
            return window;
        }
    }
    return {};
}
