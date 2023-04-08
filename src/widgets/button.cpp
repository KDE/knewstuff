/*
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "button.h"

#include "qtquickdialogwrapper.h"
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
        const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
        if (!authorized) {
            q->setEnabled(false);
            q->setVisible(false);
        }

        q->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
        q->connect(q, &QAbstractButton::clicked, q, [this]() {
            showDialog();
        });
    }

    void showDialog()
    {
        if (!KAuthorized::authorize(KAuthorized::GHNS)) {
            KMessageBox::information(q, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
            return;
        }
        Q_ASSERT_X(!configFile.isEmpty(), Q_FUNC_INFO, "The configFile for the KNSWidgets::Button must be explicitly set");
        Q_EMIT q->aboutToShowDialog();

        if (!dialog) {
            dialog.reset(new KNSWidgets::QtQuickDialogWrapper(configFile, q));
            QObject::connect(dialog.get(), &KNSWidgets::QtQuickDialogWrapper::finished, q, [this]() {
                Q_EMIT q->dialogFinished(dialog->changedEntries());
            });
        }
        dialog->show();
    }

    Button *q;
    QString configFile;
    std::unique_ptr<KNSWidgets::QtQuickDialogWrapper> dialog;
};

Button::Button(const QString &text, const QString &configFile, QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate(this))
{
    setText(text);
    d->configFile = configFile;
}

Button::Button(QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate(this))
{
    setText(i18n("Download New Stuff..."));
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
}
