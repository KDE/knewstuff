/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_IMAGEPREVIEWWIDGET_P_H
#define KNEWSTUFF3_IMAGEPREVIEWWIDGET_P_H

#include <QWidget>
#include <QImage>

namespace KNS3
{

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);

    void setImage(const QImage &preview);

    QSize sizeHint() const override;

Q_SIGNALS:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QImage m_image;
    QImage m_scaledImage;
};

}

#endif // IMAGEPREVIEWWIDGET_H
