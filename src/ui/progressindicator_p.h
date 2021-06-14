/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_PROGRESSINDICATOR_P_H
#define KNEWSTUFF3_PROGRESSINDICATOR_P_H

#include "errorcode.h"

#include <KPixmapSequence>
#include <QFrame>

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
    QLabel *m_statusLabel = nullptr;
    KPixmapSequenceWidget *busyWidget = nullptr;

    const KPixmapSequence m_busyPixmap;
    const KPixmapSequence m_errorPixmap;
};
}

#endif
