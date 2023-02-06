/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2006, 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_IMAGELOADER_P_H
#define KNEWSTUFF3_IMAGELOADER_P_H

#include <QByteArray>
#include <QObject>

#include "entryinternal.h"
#include "jobs/httpjob.h"

class KJob;

namespace KNSCore
{
/**
 * Convenience class for images with remote sources.
 *
 * This class represents a fire-and-forget approach of loading images
 * in applications. The image will load itself.
 * Using this class also requires using QAsyncFrame or similar UI
 * elements which allow for asynchronous image loading.
 *
 * This class is used internally by the DownloadDialog class.
 *
 * @internal
 */
class ImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit ImageLoader(const EntryInternal &entry, EntryInternal::PreviewType type, QObject *parent);
    void start();
    /**
     * Get the job doing the image loading in the background (to have progress information available)
     * @return the job
     */
    KJob *job();

Q_SIGNALS:
    void signalPreviewLoaded(const KNSCore::EntryInternal &, KNSCore::EntryInternal::PreviewType);
    void signalError(const KNSCore::EntryInternal &, KNSCore::EntryInternal::PreviewType, const QString &);

private Q_SLOTS:
    void slotDownload(KJob *job);
    void slotData(KJob *job, const QByteArray &buf);

private:
    EntryInternal m_entry;
    const EntryInternal::PreviewType m_previewType;
    QByteArray m_buffer;
    HTTPJob *m_job = nullptr;
};
}
#endif
