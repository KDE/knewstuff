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

#include "engine.h"

#include "engine_p.h"

class Engine::Private
{
public:
    Private()
        : engine(0)
    {}
    KNSCore::Engine* engine;
    QString configFile;
};

Engine::Engine(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
}

Engine::~Engine()
{
    delete d;
}

QString Engine::configFile() const
{
    return d->configFile;
}

void Engine::setConfigFile(const QString& newFile)
{
    d->configFile = newFile;
    emit configFileChanged();

    if(!d->engine) {
        d->engine = new KNSCore::Engine(this);
        connect(d->engine, &KNSCore::Engine::signalMessage, this, &Engine::message);
        connect(d->engine, &KNSCore::Engine::signalIdle, this, &Engine::idleMessage);
        connect(d->engine, &KNSCore::Engine::signalBusy, this, &Engine::busyMessage);
        connect(d->engine, &KNSCore::Engine::signalError, this, &Engine::errorMessage);
        emit engineChanged();
    }
    d->engine->init(d->configFile);
}

QObject * Engine::engine() const
{
    return d->engine;
}
