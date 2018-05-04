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

#ifndef FILECOPYWORKER_H
#define FILECOPYWORKER_H

#include <QThread>
#include <QUrl>

namespace KNSCore {

class FileCopyWorker : public QThread {
    Q_OBJECT
public: 
    explicit FileCopyWorker(const QUrl& source, const QUrl& destination, QObject* parent = nullptr); 
    ~FileCopyWorker() override;
    void run() override;

    Q_SIGNAL void progress(qlonglong current, qlonglong total);
    Q_SIGNAL void completed();
private:
    class Private;
    Private* d;
};

}

#endif//FILECOPYWORKER_H
