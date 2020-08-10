/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "progressindicator_p.h"

#include <QPushButton>
#include <QLabel>
#include <QVariant>
#include <QHBoxLayout>

#include <KJob>

#include <KIconLoader>
#include <KPixmapSequenceWidget>

using namespace KNS3;

ProgressIndicator::ProgressIndicator(QWidget *parent)
    : QFrame(parent)
    , m_busyPixmap(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22))
    , m_errorPixmap(KIconLoader::global()->loadPixmapSequence(QStringLiteral("dialog-error"), 22))
{
    setFrameStyle(QFrame::NoFrame);
    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0, 0, 0, 0);

    //Busy widget
    busyWidget = new KPixmapSequenceWidget(this);
    busyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    busyWidget->setVisible(false);
    hbox->addWidget(busyWidget);

    m_statusLabel = new QLabel();
    hbox->addWidget(m_statusLabel);
}

void ProgressIndicator::busy(const QString &message)
{
    m_statusLabel->setText(message);
    busyWidget->setVisible(true);
    busyWidget->setSequence(m_busyPixmap);
}

void KNS3::ProgressIndicator::error(const KNSCore::ErrorCode& errorCode, const QString& message, const QVariant& metadata)
{
    if(errorCode == KNSCore::OcsError && metadata.value<int>() == 405) {
        return;
    }
    m_statusLabel->setText(message);
    busyWidget->setVisible(true);
    busyWidget->setSequence(m_errorPixmap);
}

void ProgressIndicator::idle(const QString &message)
{
    m_statusLabel->setText(message);
    busyWidget->setVisible(false);
}

