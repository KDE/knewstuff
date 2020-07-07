/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (c) 2008 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (c) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_PROGRESSINDICATOR_P_H
#define KNEWSTUFF3_PROGRESSINDICATOR_P_H

#include "errorcode.h"

#include <QFrame>
#include <KPixmapSequence>

class QVBoxLayout;
class QLabel;
class QString;
class KPixmapSequenceWidget;

namespace KNS3
{

/**
 * Embedded progress indicator for the download dialog.
 *
 * The indicator can display various asynchronous operations at once.
 * Each operation can also individually be cancelled.
 *
 * @internal
 */
class ProgressIndicator : public QFrame
{
    Q_OBJECT
public:
    explicit ProgressIndicator(QWidget *parent);

public Q_SLOTS:
    void busy(const QString &message);
    void error(const KNSCore::ErrorCode &errorCode, const QString &message, const QVariant &metadata);
    void idle(const QString &message);

private:
    QLabel *m_statusLabel;
    KPixmapSequenceWidget *busyWidget;

    KPixmapSequence m_busyPixmap;
    KPixmapSequence m_errorPixmap;
};
}

#endif
