/*
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "button.h"

#include "qtquickdialogwrapper.h"
#include "ui/widgetquestionlistener.h"
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>

namespace KNSWidgets
{
class ButtonPrivate
{
public:
    explicit ButtonPrivate(Button *qq)
        : q(qq)
    {
    }

    Button *q;
    QString configFile;
    QPointer<KNS3::QtQuickDialogWrapper> dialog;
    void init()
    {
        const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
        if (!authorized) {
            q->setEnabled(false);
            q->setVisible(false);
        }

        q->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
        q->connect(q, &QAbstractButton::clicked, q, &Button::showDialog);
        KNS3::WidgetQuestionListener::instance();
    }
};

Button::Button(const QString &text, const QString &configFile, QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate(this))
{
    setText(text);
    d->configFile = configFile;
    d->init();
}

Button::Button(QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate(this))
{
    setText(i18n("Download New Stuff..."));
    d->init();
}

Button::~Button() = default;

void Button::setConfigFile(const QString &configFile)
{
    d->configFile = configFile;
}
QString Button::configFile()
{
    return d->configFile;
}

void Button::showDialog()
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    Q_ASSERT_X(!d->configFile.isEmpty(), Q_FUNC_INFO, "The configFile for the KNSWidgets::Button must be explicitly set");
    Q_EMIT aboutToShowDialog();

    if (!d->dialog) {
        d->dialog = new KNS3::QtQuickDialogWrapper(d->configFile, this);
    }
    Q_EMIT dialogFinished(d->dialog->exec());
}

}
