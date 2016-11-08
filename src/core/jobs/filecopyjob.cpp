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
#include "filecopyworker.h"

#include "knewstuffcore_debug.h"

#include <QTimer>

using namespace KNSCore;

class FileCopyJob::Private
{
public:
    Private()
        : permissions(-1)
        , flags(DefaultFlags)
        , worker(0)
    {}
    QUrl source;
    QUrl destination;
    int permissions;
    JobFlags flags;

    FileCopyWorker* worker;
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
    if(d->worker) {
        // already started...
        return;
    }
    d->worker = new FileCopyWorker(d->source, d->destination, this);
    connect(d->worker, &FileCopyWorker::progress, this, &FileCopyJob::handleProgressUpdate);
    connect(d->worker, &FileCopyWorker::completed, this, &FileCopyJob::handleCompleted);
    d->worker->start();
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
    FileCopyJob* job = 0;
    if(source.isLocalFile() && destination.isLocalFile()) {
        qCDebug(KNEWSTUFFCORE) << "File copy job is local only";
        job = new FileCopyJob(source, destination, permissions, flags, parent);
    }
    else {
        qCDebug(KNEWSTUFFCORE) << "File copy job is from (or to) a remote URL";
        job = new DownloadJob(source, destination, permissions, flags, parent);
    }
    QTimer::singleShot(1, job, SLOT(start()));
    return job;
}

void FileCopyJob::handleProgressUpdate(qlonglong current, qlonglong total)
{
    setTotalAmount(KJob::Bytes, total);
    setProcessedAmount(KJob::Bytes, current);
    emitPercent(current, total);
}

void FileCopyJob::handleCompleted()
{
    d->worker->deleteLater();
    d->worker = 0;
    emitResult();
}
