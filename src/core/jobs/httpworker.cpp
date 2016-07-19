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
    QUrl url;
    QNetworkAccessManager* qnam;
    QNetworkReply* reply;
    QUrl redirectUrl;
};

HTTPWorker::HTTPWorker(JobType jobType, const QUrl& url, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    qDebug() << Q_FUNC_INFO;
    d->jobType = jobType;
    d->url = url;

    d->qnam = new QNetworkAccessManager(parent);
    connect(d->qnam, &QNetworkAccessManager::finished, this, &HTTPWorker::handleFinished);
}

HTTPWorker::~HTTPWorker()
{
    delete d;
}

void HTTPWorker::setUrl(const QUrl& url)
{
    d->url = url;
}

void HTTPWorker::startRequest()
{
    if(d->reply) {
        // only run one request at a time...
        return;
    }

    QNetworkRequest request(d->url);
    d->reply = d->qnam->get(request);
    connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
}

void HTTPWorker::handleReadyRead()
{
//     qDebug() << Q_FUNC_INFO;
    if (d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
        do {
            emit data(d->reply->read(32768));
        } while(!d->reply->atEnd());
    }
}

void HTTPWorker::handleFinished(QNetworkReply* reply)
{
    qDebug() << Q_FUNC_INFO;
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << reply->errorString() << '\n';
    }

    // Handle redirections
    const QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != d->redirectUrl) {
        d->redirectUrl = reply->url().resolved(possibleRedirectUrl);
        if (d->redirectUrl.scheme().startsWith("http")) {
            qDebug() << "Redirected to " << d->redirectUrl.toDisplayString() << "...\n";
            reply->deleteLater();
            d->reply = d->qnam->get(QNetworkRequest(d->redirectUrl));
            connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
            return;
        } else {
            qDebug() << "Redirection to" << d->redirectUrl.toDisplayString() << "forbidden.\n";
        }
    }
    d->redirectUrl.clear();

    emit completed();
}
