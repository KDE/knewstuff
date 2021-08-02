/*
    knewstuff3/ui/uploaddialog.cpp.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "uploaddialog.h"

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 80)

#include "knewstuff_debug.h"
#include "uploaddialog_p.h"

#include <QDialogButtonBox>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QVBoxLayout>

using namespace KNS3;

bool UploadDialogPrivate::init(const QString &configfile)
{
    bool success = true;

    QQuickView *view = new QQuickView;
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    q->setLayout(layout);
    q->layout()->addWidget(QWidget::createWindowContainer(view, q));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, q);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, q, &UploadDialog::accept);
    q->layout()->addWidget(buttonBox);

    view->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), configfile);
    view->setSource(QUrl(QStringLiteral("qrc:///uploaddialog.qml")));
    QQuickItem *item = view->rootObject();
    // If there is an error on the QML side of things we get a nullptr
    if (item) {
        q->resize(view->rootObject()->implicitWidth(), view->rootObject()->implicitHeight());
        // Forward relevant signals
        QObject::connect(item, SIGNAL(closed()), q, SLOT(accept()));
    } else {
        qCDebug(KNEWSTUFF) << "Failed to load the UploadDialog components. The QML Engine reported the following errors:" << view->errors();
        success = false;
    }
    return success;
}

UploadDialog::UploadDialog(QWidget *parent)
    : QDialog(parent)
    , d(new UploadDialogPrivate(this))
{
    const QString name = QCoreApplication::applicationName();
    init(name + QStringLiteral(".knsrc"));
}

UploadDialog::UploadDialog(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new UploadDialogPrivate(this))
{
    init(configFile);
}

UploadDialog::~UploadDialog()
{
    delete d;
}

bool UploadDialog::init(const QString &configfile)
{
    return d->init(configfile);
}

void UploadDialog::setUploadFile(const QUrl &)
{
}

void UploadDialog::setUploadName(const QString &)
{
}

void UploadDialog::selectCategory(const QString &)
{
}

void UploadDialog::setChangelog(const QString &)
{
}

void UploadDialog::setDescription(const QString &)
{
}

void UploadDialog::setPriceEnabled(bool)
{
}

void UploadDialog::setPrice(double)
{
}

void UploadDialog::setPriceReason(const QString &)
{
}

void UploadDialog::setVersion(const QString &)
{
}

void UploadDialog::setPreviewImageFile(uint, const QUrl &)
{
}

void UploadDialog::accept()
{
    QDialog::accept();
}

#endif

#include "moc_uploaddialog.cpp"
