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

#ifndef FILECOPYJOB_H
#define FILECOPYJOB_H

#include "jobbase.h"

#include <QUrl>

namespace KNSCore
{

class FileCopyJob : public KJob
{
    Q_OBJECT
public:

    explicit FileCopyJob(const QUrl& source, const QUrl& destination, int permissions=-1, JobFlags flags = DefaultFlags, QObject* parent = nullptr);
    explicit FileCopyJob(QObject* parent = nullptr);
    ~FileCopyJob() override;

    Q_SCRIPTABLE void start() override;

    QUrl destUrl() const;
    QUrl srcUrl() const;

    // This will create either a FileCopyJob, or an instance of
    // a subclass, depending on the nature of the URLs passed to
    // it
    static FileCopyJob* file_copy(const QUrl& source, const QUrl& destination, int permissions=-1, JobFlags flags = DefaultFlags, QObject* parent = nullptr);

protected Q_SLOTS:
    void handleProgressUpdate(qlonglong current, qlonglong total);
    void handleCompleted();
private:
    class Private;
    Private* d;
};

}

#endif//FILECOPYJOB_H

