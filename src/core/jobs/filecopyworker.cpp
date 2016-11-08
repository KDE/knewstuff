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

#include "filecopyworker.h"

#include <QFile>

using namespace KNSCore;

class FileCopyWorker::Private {
public:
    Private() {}
    QFile source;
    QFile destination;
};

FileCopyWorker::FileCopyWorker(const QUrl& source, const QUrl& destination, QObject* parent)
    : QThread(parent)
    , d(new Private)
{
    d->source.setFileName(source.toLocalFile());
    d->destination.setFileName(destination.toLocalFile());
}

FileCopyWorker::~FileCopyWorker()
{
    delete d;
}

void FileCopyWorker::run()
{
    qint64 totalSize = d->source.size();

    for (qint64 i = 0; i < totalSize; i += 1024) {
        d->destination.write(d->source.read(1024));
        d->source.seek(i);
        d->destination.seek(i);

        emit progress(i, totalSize / 1024);
    }
    emit completed();
}
