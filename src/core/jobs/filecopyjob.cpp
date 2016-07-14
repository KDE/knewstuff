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

#include "filecopyjob.h"

#include "downloadjob.h"

using namespace KNS3;

class FileCopyJob::Private
{
public:
    Private()
        : permissions(-1)
        , flags(DefaultFlags)
    {}
    QUrl source;
    QUrl destination;
    int permissions;
    JobFlags flags;
};

FileCopyJob::FileCopyJob(const QUrl& source, const QUrl& destination, int permissions, JobFlags flags, QObject* parent)
    : KJob(parent)
    , d(new Private)
{
    d->source = source;
    d->destination = destination;
    d->permissions = permissions;
    d->flags = flags;
}

FileCopyJob::FileCopyJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

FileCopyJob::~FileCopyJob()
{
    delete d;
}

void FileCopyJob::start()
{
}

QUrl FileCopyJob::destUrl() const
{
    return d->destination;
}

QUrl FileCopyJob::srcUrl() const
{
    return d->source;
}

FileCopyJob* FileCopyJob::file_copy(const QUrl& source, const QUrl& destination, int permissions, JobFlags flags, QObject* parent)
{
    if(source.isLocalFile() && destination.isLocalFile()) {
        return new FileCopyJob(source, destination, permissions, flags, parent);
    }
    return new DownloadJob(source, destination, permissions, flags, parent);
}
