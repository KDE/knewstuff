/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef HTTPWORKER_H
#define HTTPWORKER_H

#include <QThread>
#include <QUrl>

class QNetworkReply;
namespace KNSCore
{
class HTTPWorker : public QObject
{
    Q_OBJECT
public:
    enum JobType {
        GetJob,
        DownloadJob, // Much the same as a get... except with a filesystem destination, rather than outputting data
    };
    explicit HTTPWorker(const QUrl &url, JobType jobType = GetJob, QObject *parent = nullptr);
    explicit HTTPWorker(const QUrl &source, const QUrl &destination, JobType jobType = DownloadJob, QObject *parent = nullptr);
    virtual ~HTTPWorker();

    void startRequest();

    void setUrl(const QUrl &url);

    Q_SIGNAL void error(QString error);
    Q_SIGNAL void progress(qlonglong current, qlonglong total);
    Q_SIGNAL void completed();
    Q_SIGNAL void data(const QByteArray &data);

    Q_SLOT void handleReadyRead();
    Q_SLOT void handleFinished();
    Q_SLOT void handleData(const QByteArray &data);

private:
    class Private;
    Private *d;
};

}

#endif // HTTPWORKER_H
