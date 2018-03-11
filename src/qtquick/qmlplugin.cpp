/*
 * Copyright (C) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qmlplugin.h"

#include "quickengine.h"
#include "quickitemsmodel.h"
#include "downloadlinkinfo.h"

#include <QQmlEngine>
#include <qqml.h>

void QmlPlugins::initializeEngine(QQmlEngine *engine, const char *)
{
    Q_UNUSED(engine);
}

void QmlPlugins::registerTypes(const char *uri)
{
    qmlRegisterType<Engine>(uri, 1, 0, "Engine");
    qmlRegisterType<ItemsModel>(uri, 1, 0, "ItemsModel");
    qmlRegisterUncreatableType<DownloadLinkInfo>(uri, 1, 0, "DownloadLinkInfo", QStringLiteral("This should only be created by the ItemsModel, and is associated with one entry in that model"));
}
