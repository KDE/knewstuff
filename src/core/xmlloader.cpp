/*
    knewstuff3/xmlloader.cpp.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "xmlloader.h"

#include "jobs/httpjob.h"
#include "knewstuffcore_debug.h"

#include <KConfig>

#include <QByteArray>

namespace KNSCore
{
XmlLoader::XmlLoader(QObject *parent)
    : QObject(parent)
{
}

void XmlLoader::load(const QUrl &url)
{
    m_jobdata.clear();

    qCDebug(KNEWSTUFFCORE) << "XmlLoader::load(): url: " << url;

    HTTPJob *job = HTTPJob::get(url, Reload, JobFlag::HideProgressInfo);
    connect(job, &KJob::result, this, &XmlLoader::slotJobResult);
    connect(job, &HTTPJob::data, this, &XmlLoader::slotJobData);

    Q_EMIT jobStarted(job);
}

void XmlLoader::slotJobData(KJob *, const QByteArray &data)
{
    qCDebug(KNEWSTUFFCORE) << "XmlLoader::slotJobData()";

    m_jobdata.append(data);
}

void XmlLoader::slotJobResult(KJob *job)
{
    deleteLater();
    if (job->error()) {
        Q_EMIT signalFailed();
        return;
    }
    qCDebug(KNEWSTUFFCORE) << "--Xml Loader-START--";
    qCDebug(KNEWSTUFFCORE) << QString::fromUtf8(m_jobdata);
    qCDebug(KNEWSTUFFCORE) << "--Xml Loader-END--";
    QDomDocument doc;
    if (!doc.setContent(m_jobdata)) {
        Q_EMIT signalFailed();
        return;
    }
    Q_EMIT signalLoaded(doc);
}

QDomElement addElement(QDomDocument &doc, QDomElement &parent, const QString &tag, const QString &value)
{
    QDomElement n = doc.createElement(tag);
    n.appendChild(doc.createTextNode(value));
    parent.appendChild(n);

    return n;
}
} // end KNS namespace
