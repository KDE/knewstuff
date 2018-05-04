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

#ifndef HTTPJOB_H
#define HTTPJOB_H

#include "jobbase.h"

#include <QUrl>

namespace KNSCore
{

class HTTPJob : public KJob
{
    Q_OBJECT
public:
    explicit HTTPJob(const QUrl& source, LoadType loadType = Reload, JobFlags flags = DefaultFlags, QObject* parent = nullptr);
    explicit HTTPJob(QObject* parent = nullptr);
    ~HTTPJob() override;

    Q_SLOT void start() override;

    static HTTPJob* get(const QUrl& source, LoadType loadType = Reload, JobFlags flags = DefaultFlags, QObject* parent = nullptr);

Q_SIGNALS:
    /**
     * Data from the slave has arrived.
     * @param job the job that emitted this signal
     * @param data data received from the slave.
     *
     * End of data (EOD) has been reached if data.size() == 0, however, you
     * should not be certain of data.size() == 0 ever happening (e.g. in case
     * of an error), so you should rely on result() instead.
     */
    void data(KJob *job, const QByteArray& data);

protected Q_SLOTS:
    void handleWorkerData(const QByteArray& data);
    void handleWorkerCompleted();
    void handleWorkerError(const QString& error);
private:
    class Private;
    Private* d;
};

}

#endif//HTTPJOB_H
