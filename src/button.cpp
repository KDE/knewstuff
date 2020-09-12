/*
    knewstuff3/ui/knewstuffbutton.cpp.
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "button.h"

#include <KLocalizedString>
#include <KAuthorized>
#include <KMessageBox>
#include "downloaddialog.h"
#include "ui/widgetquestionlistener.h"

#include <QPointer>

namespace KNS3
{
class ButtonPrivate
{
public:
    QString configFile;
};

Button::Button(const QString &text,
               const QString &configFile,
               QWidget *parent)
    : QPushButton(parent),
      d(new ButtonPrivate)
{
    setButtonText(text);
    d->configFile = configFile;
    init();
}

Button::Button(QWidget *parent)
    : QPushButton(parent),
      d(new ButtonPrivate)
{
    setButtonText(i18n("Download New Stuff..."));
    init();
}

Button::~Button()
{
    delete d;
}

void Button::init()
{
    const bool authorized = KAuthorized::authorize(QStringLiteral("ghns"));
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAbstractButton::clicked, this, &Button::showDialog);
    WidgetQuestionListener::instance();
}

void Button::setButtonText(const QString &what)
{
    setText(what);
}

void Button::setConfigFile(const QString &configFile)
{
    d->configFile = configFile;
}

void Button::showDialog()
{
    if (!KAuthorized::authorize(QStringLiteral("ghns"))) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    Q_EMIT aboutToShowDialog();

    QPointer<DownloadDialog> dialog = new DownloadDialog(d->configFile, this);
    dialog->exec();

    if (dialog)
        Q_EMIT dialogFinished(dialog->changedEntries());

    delete dialog;
}

}

