/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "imagepreviewwidget_p.h"

#include <QPaintEvent>
#include <QPainter>

#include <qstandardpaths.h>

#include <core/entryinternal.h>

using namespace KNS3;

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    // installEventFilter(this);
}

void ImagePreviewWidget::setImage(const QImage &preview)
{
    m_image = preview;
    m_scaledImage = QImage();
    updateGeometry();
    repaint();
}

void ImagePreviewWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    Q_EMIT clicked();
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_scaledImage = QImage();
    repaint();
}

void ImagePreviewWidget::paintEvent(QPaintEvent * /*event*/)
{
    if (m_image.isNull()) {
        return;
    }

    QPainter painter(this);
    int margin = painter.fontMetrics().height() / 2;
    // painter.drawImage(contentsRect(), m_image);

    int width = contentsRect().width();
    int height = contentsRect().height();

    if (m_scaledImage.isNull()) {
        QSize scaled = QSize(qMin(width - 2 * margin, m_image.width() * 2), qMin(height - 2 * margin, m_image.height() * 2));
        m_scaledImage = m_image.scaled(scaled, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QPoint point;

    point.setX(contentsRect().left() + ((width - m_scaledImage.width()) / 2));
    point.setY(contentsRect().top() + ((height - m_scaledImage.height()) / 2));

    QPoint framePoint(point.x() - 5, point.y() - 5);
    painter.drawImage(point, m_scaledImage);
}

QSize ImagePreviewWidget::sizeHint() const
{
    if (m_image.isNull()) {
        return QSize();
    }
    QSize sh = m_image.size();
    sh.scale(maximumSize(), Qt::KeepAspectRatio);
    return sh;
}
