/*
    knewstuff3/ui/downloaddialog.cpp.
    SPDX-FileCopyrightText: 2005 Enrico Ros <eros.kde@email.it>
    SPDX-FileCopyrightText: 2005-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "downloaddialog.h"

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 80)

#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>
#include <KTitleWidget>
#include <QCoreApplication>
#include <kwindowconfig.h>

#include "downloadwidget.h"
#include "downloadwidget_p.h"
#include "ui/widgetquestionlistener.h"

namespace KNS3
{
class DownloadDialogPrivate
{
public:
    ~DownloadDialogPrivate()
    {
        delete downloadWidget;
    }

    DownloadWidget *downloadWidget = nullptr;
};
}

using namespace KNS3;

const char ConfigGroup[] = "DownloadDialog Settings";

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
    , d(new DownloadDialogPrivate)
{
    const QString name = QCoreApplication::applicationName();
    init(name + QStringLiteral(".knsrc"));
}

DownloadDialog::DownloadDialog(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new DownloadDialogPrivate)
{
    init(configFile);
}

void DownloadDialog::init(const QString &configFile)
{
    // load the last size from config
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::restoreWindowSize(windowHandle(), group);
    setMinimumSize(700, 400);

    setWindowTitle(i18n("Get Hot New Stuff"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    d->downloadWidget = new DownloadWidget(configFile, this);
    // DownloadWidget already provides margins, which would duplicate the dialog layout margins.
    // As the widget theme could use different margin sizes for a dialog/window outer layout,
    // so from the duplicated margins we pick the internal margins to remove here.
    d->downloadWidget->layout()->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(d->downloadWidget);

    if (group.hasKey("Name")) {
        d->downloadWidget->setTitle(group.readEntry("Name"));
    } else {
        QString displayName = QGuiApplication::applicationDisplayName();
        if (displayName.isEmpty()) {
            displayName = QCoreApplication::applicationName();
        }
        d->downloadWidget->setTitle(i18nc("Program name followed by 'Add On Installer'", "%1 Add-On Installer", displayName));
    }
    // d->downloadWidget->d->ui.m_titleWidget->setPixmap(QIcon::fromTheme(KGlobal::activeComponent().aboutData()->programIconName()));
    d->downloadWidget->d->ui.m_titleWidget->setVisible(true);
    d->downloadWidget->d->ui.closeButton->setVisible(true);
    KStandardGuiItem::assign(d->downloadWidget->d->ui.closeButton, KStandardGuiItem::Close);
    d->downloadWidget->d->dialogMode = true;
    connect(d->downloadWidget->d->ui.closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
    WidgetQuestionListener::instance();
}

DownloadDialog::~DownloadDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), ConfigGroup);
    KWindowConfig::saveWindowSize(windowHandle(), group, KConfigBase::Persistent);
    delete d;
}

int DownloadDialog::exec()
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return QDialog::Rejected;
    }
    return QDialog::exec();
}

void DownloadDialog::open()
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    QDialog::open();
}

void DownloadDialog::showEvent(QShowEvent *event)
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    QWidget::showEvent(event);
}

void DownloadDialog::setTitle(const QString &title)
{
    d->downloadWidget->setTitle(title);
}

QString DownloadDialog::title() const
{
    return d->downloadWidget->title();
}

KNSCore::Engine *DownloadDialog::engine()
{
    return d->downloadWidget->engine();
}

Entry::List DownloadDialog::changedEntries()
{
    return d->downloadWidget->changedEntries();
}

Entry::List DownloadDialog::installedEntries()
{
    return d->downloadWidget->installedEntries();
}

#endif
