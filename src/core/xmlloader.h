/*
    knewstuff3/xmlloader.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_XMLLOADER_P_H
#define KNEWSTUFF3_XMLLOADER_P_H

#include <qdom.h>
#include <QObject>
#include <QString>
#include <QUrl>

#include "knewstuffcore_export.h"

class KJob;

namespace KNSCore
{

QDomElement addElement(QDomDocument &doc, QDomElement &parent,
                       const QString &tag, const QString &value);

/**
 * KNewStuff xml loader.
 * This class loads an xml document from a kurl and returns the
 * resulting domdocument once completed.
 * It should probably not be used directly by the application.
 *
 * @internal
 */
class KNEWSTUFFCORE_EXPORT XmlLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit XmlLoader(QObject *parent);

    /**
     * Starts asynchronously loading the xml document from the
     * specified URL.
     *
     * @param url location of the XML file
     */
    void load(const QUrl &url);

Q_SIGNALS:
    /**
     * Indicates that the list of providers has been successfully loaded.
     */
    void signalLoaded(const QDomDocument &);
    void signalFailed();

    void jobStarted(KJob *);

protected Q_SLOTS:
    void slotJobData(KJob *, const QByteArray &);
    void slotJobResult(KJob *);

private:
    QByteArray m_jobdata;
};

}

#endif
