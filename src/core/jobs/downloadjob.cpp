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

#include "downloadjob.h"

#include "httpworker.h"

#include "knewstuffcore_debug.h"

using namespace KNSCore;

class DownloadJob::Private
{
public:
    Private() {}
    QUrl source;
    QUrl destination;
};

DownloadJob::DownloadJob(const QUrl& source, const QUrl& destination, int permissions, JobFlags flags, QObject* parent)
    : FileCopyJob(source, destination, permissions, flags, parent)
    , d(new Private)
{
    d->source = source;
    d->destination = destination;
}

DownloadJob::DownloadJob(QObject* parent)
    : FileCopyJob(parent)
    , d(new Private)
{
}

DownloadJob::~DownloadJob()
{
    delete d;
}

void DownloadJob::start()
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    HTTPWorker* worker = new HTTPWorker(d->source, d->destination, HTTPWorker::DownloadJob, this);
    connect(worker, &HTTPWorker::completed, this, &DownloadJob::handleWorkerCompleted);
    connect(worker, &HTTPWorker::error, this, &DownloadJob::handleWorkerError);
    worker->startRequest();
}

void DownloadJob::handleWorkerCompleted()
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    emitResult();
}

void KNSCore::DownloadJob::handleWorkerError(const QString& error)
{
    setError(KJob::UserDefinedError);
    setErrorText(error);
}
