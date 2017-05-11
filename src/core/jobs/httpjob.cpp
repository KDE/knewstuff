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

#include "knewstuffcore_debug.h"
#include "httpworker.h"

#include <QTimer>

using namespace KNSCore;

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
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    HTTPWorker* worker = new HTTPWorker(d->source, HTTPWorker::GetJob, this);
    connect(worker, &HTTPWorker::data, this, &HTTPJob::handleWorkerData);
    connect(worker, &HTTPWorker::completed, this, &HTTPJob::handleWorkerCompleted);
    connect(worker, &HTTPWorker::error, this, &HTTPJob::handleWorkerError);
    worker->startRequest();
}

void HTTPJob::handleWorkerData(const QByteArray& data)
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO << data;
    emit HTTPJob::data(this, data);
}

void HTTPJob::handleWorkerCompleted()
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    emitResult();
}

void KNSCore::HTTPJob::handleWorkerError(const QString& error)
{
    setError(KJob::UserDefinedError);
    setErrorText(error);
}

HTTPJob* HTTPJob::get(const QUrl& source, LoadType loadType, JobFlags flags, QObject* parent)
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    HTTPJob* job = new HTTPJob(source, loadType, flags, parent);
    QTimer::singleShot(0, job, &HTTPJob::start);
    return job;
}
