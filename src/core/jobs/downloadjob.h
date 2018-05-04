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

#ifndef DOWNLOADJOB_H
#define DOWNLOADJOB_H

#include "filecopyjob.h"

namespace KNSCore
{

class DownloadJob : public FileCopyJob
{
    Q_OBJECT
public:
    explicit DownloadJob(const QUrl& source, const QUrl& destination, int permissions=-1, JobFlags flags = DefaultFlags, QObject* parent = nullptr);
    explicit DownloadJob(QObject* parent = nullptr);
    ~DownloadJob() override;

    Q_SCRIPTABLE void start() override;

protected Q_SLOTS:
    void handleWorkerCompleted();
    void handleWorkerError(const QString& error);
private:
    class Private;
    Private* d;
};

}

#endif//DOWNLOADJOB_H
