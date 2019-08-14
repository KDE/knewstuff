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

#include "httpworker.h"

#include "knewstuffcore_debug.h"

#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QStorageInfo>

class HTTPWorkerNAM {
public:
    HTTPWorkerNAM()
    {
        QMutexLocker locker(&mutex);
        const QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/knewstuff");
        cache.setCacheDirectory(cacheLocation);
        QStorageInfo storageInfo(cacheLocation);
        cache.setMaximumCacheSize(qMin(50 * 1024 * 1024, (int)(storageInfo.bytesTotal() / 1000)));
        nam.setCache(&cache);
    }
    QNetworkAccessManager nam;
    QMutex mutex;

    QNetworkReply* get(const QNetworkRequest& request)
    {
        QMutexLocker locker(&mutex);
        return nam.get(request);
    }

private:
    QNetworkDiskCache cache;
};

Q_GLOBAL_STATIC(HTTPWorkerNAM, s_httpWorkerNAM)

using namespace KNSCore;

class HTTPWorker::Private
{
public:
    Private()
        : jobType(GetJob)
        , reply(nullptr)
    {}
    JobType jobType;
    QUrl source;
    QUrl destination;
    QNetworkReply* reply;
    QUrl redirectUrl;

    QFile dataFile;
};

HTTPWorker::HTTPWorker(const QUrl& url, JobType jobType, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    d->jobType = jobType;
    d->source = url;
}

HTTPWorker::HTTPWorker(const QUrl& source, const QUrl& destination, KNSCore::HTTPWorker::JobType jobType, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    d->jobType = jobType;
    d->source = source;
    d->destination = destination;
}

HTTPWorker::~HTTPWorker()
{
    delete d;
}

void HTTPWorker::setUrl(const QUrl& url)
{
    d->source = url;
}

void HTTPWorker::startRequest()
{
    if(d->reply) {
        // only run one request at a time...
        return;
    }

    QNetworkRequest request(d->source);
    d->reply = s_httpWorkerNAM->get(request);
    connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
    connect(d->reply, &QNetworkReply::finished, this, &HTTPWorker::handleFinished);
    if(d->jobType == DownloadJob) {
        d->dataFile.setFileName(d->destination.toLocalFile());
        connect(this, &HTTPWorker::data, this, &HTTPWorker::handleData);
    }
}

void HTTPWorker::handleReadyRead()
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    QMutexLocker locker(&s_httpWorkerNAM->mutex);
    if (d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
        do {
            emit data(d->reply->read(32768));
        } while(!d->reply->atEnd());
    }
}

void HTTPWorker::handleFinished()
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO << d->reply->url();
    if (d->reply->error() != QNetworkReply::NoError) {
        qCWarning(KNEWSTUFFCORE) << d->reply->errorString();
        emit error(d->reply->errorString());
    }

    // Check if the data was obtained from cache or not
    QString fromCache = d->reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() ? QStringLiteral("(cached)") : QStringLiteral("(NOT cached)");

    // Handle redirections
    const QUrl possibleRedirectUrl = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != d->redirectUrl) {
        d->redirectUrl = d->reply->url().resolved(possibleRedirectUrl);
        if (d->redirectUrl.scheme().startsWith(QLatin1String("http"))) {
            qCDebug(KNEWSTUFFCORE) << d->reply->url().toDisplayString() << "was redirected to" << d->redirectUrl.toDisplayString() << fromCache << d->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            d->reply->deleteLater();
            QNetworkRequest request(d->redirectUrl);
            d->reply = s_httpWorkerNAM->get(request);
            connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
            connect(d->reply, &QNetworkReply::finished, this, &HTTPWorker::handleFinished);
            return;
        } else {
            qCWarning(KNEWSTUFFCORE) << "Redirection to" << d->redirectUrl.toDisplayString() << "forbidden.";
        }
    }
    else {
        qCDebug(KNEWSTUFFCORE) << "Data for" << d->reply->url().toDisplayString() << "was fetched" << fromCache;
    }

    if(d->dataFile.isOpen()) {
        d->dataFile.close();
    }

    d->redirectUrl.clear();
    emit completed();
}

void HTTPWorker::handleData(const QByteArray& data)
{
    // It turns out that opening a file and then leaving it hanging without writing to it immediately will, at times
    // leave you with a file that suddenly (seemingly magically) no longer exists. Thanks for that.
    if(!d->dataFile.isOpen()) {
        if(d->dataFile.open(QIODevice::WriteOnly)) {
            qCDebug(KNEWSTUFFCORE) << "Opened file" << d->dataFile.fileName() << "for writing.";
        }
        else {
            qCWarning(KNEWSTUFFCORE) << "Failed to open file for writing!";
            emit error(QStringLiteral("Failed to open file %1 for writing!").arg(d->destination.toLocalFile()));
        }
    }
    qCDebug(KNEWSTUFFCORE) << "Writing" << data.length() << "bytes of data to" << d->dataFile.fileName();
    quint64 written = d->dataFile.write(data);
    if(d->dataFile.error()) {
        qCDebug(KNEWSTUFFCORE) << "File has error" << d->dataFile.errorString();
    }
    qCDebug(KNEWSTUFFCORE) << "Wrote" << written << "bytes. File is now size" << d->dataFile.size();
}
