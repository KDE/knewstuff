/*
    Copyright (C) 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

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

#include "httpjob.h"

using namespace KNS3;

class HTTPJob::Private
{
public:
    Private()
        : loadType(Reload)
        , flags(DefaultFlags)
    {}
    QUrl source;
    LoadType loadType;
    JobFlags flags;
};

HTTPJob::HTTPJob(const QUrl& source, LoadType loadType, JobFlags flags, QObject* parent)
    : KJob(parent)
    , d(new Private)
{
    d->source = source;
    d->loadType = loadType;
    d->flags = flags;
}

HTTPJob::HTTPJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

HTTPJob::~HTTPJob()
{
    delete d;
}

void HTTPJob::start()
{
}

HTTPJob* HTTPJob::get(const QUrl& source, LoadType loadType, JobFlags flags, QObject* parent)
{
    return new HTTPJob(source, loadType, flags, parent);
}
