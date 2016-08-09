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
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace KNS3;

class HTTPWorker::Private
{
public:
    Private()
        : jobType(GetJob)
        , qnam(0)
        , reply(0)
    {}
    JobType jobType;
    QUrl source;
    QUrl destination;
    QNetworkAccessManager* qnam;
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

    d->qnam = new QNetworkAccessManager(parent);
    connect(d->qnam, &QNetworkAccessManager::finished, this, &HTTPWorker::handleFinished);
}

HTTPWorker::HTTPWorker(const QUrl& source, const QUrl& destination, KNS3::HTTPWorker::JobType jobType, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    d->jobType = jobType;
    d->source = source;
    d->destination = destination;

    d->qnam = new QNetworkAccessManager(parent);
    connect(d->qnam, &QNetworkAccessManager::finished, this, &HTTPWorker::handleFinished);
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
    d->reply = d->qnam->get(request);
    connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
    if(d->jobType == DownloadJob) {
        d->dataFile.setFileName(d->destination.toLocalFile());
        if(!d->dataFile.open(QIODevice::WriteOnly)) {
            qCWarning(KNEWSTUFFCORE) << "Failed to open file for writing!";
        }
        connect(this, &HTTPWorker::data, this, &HTTPWorker::handleData);
    }
}

void HTTPWorker::handleReadyRead()
{
//     qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    if (d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
        do {
            emit data(d->reply->read(32768));
        } while(!d->reply->atEnd());
    }
}

void HTTPWorker::handleFinished(QNetworkReply* reply)
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(KNEWSTUFFCORE) << reply->errorString() << '\n';
    }

    // Handle redirections
    const QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != d->redirectUrl) {
        d->redirectUrl = reply->url().resolved(possibleRedirectUrl);
        if (d->redirectUrl.scheme().startsWith("http")) {
            qCInfo(KNEWSTUFFCORE) << "Redirected to " << d->redirectUrl.toDisplayString() << "...\n";
            reply->deleteLater();
            d->reply = d->qnam->get(QNetworkRequest(d->redirectUrl));
            connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
            return;
        } else {
            qCWarning(KNEWSTUFFCORE) << "Redirection to" << d->redirectUrl.toDisplayString() << "forbidden.\n";
        }
    }
    d->redirectUrl.clear();

    if(d->dataFile.isOpen()) {
        d->dataFile.close();
    }
    emit completed();
}

void KNS3::HTTPWorker::handleData(const QByteArray& data)
{
    qCDebug(KNEWSTUFFCORE) << "Writing" << data.length() << "bytes of data to" << d->dataFile.fileName();
    d->dataFile.write(data);
}
