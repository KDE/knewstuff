/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
