/*
    knewstuff3/ui/knewstuffbutton.cpp.
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "button.h"

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 91)

#include "entry_p.h"
#include "qtquickdialogwrapper.h"
#include "ui/widgetquestionlistener.h"
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>

namespace KNS3
{
class ButtonPrivate
{
public:
    QString configFile;
    QPointer<QtQuickDialogWrapper> dialog;
};

Button::Button(const QString &text, const QString &configFile, QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate)
{
    setText(text);
    d->configFile = configFile;
    init();
}

Button::Button(QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate)
{
    setText(i18n("Download New Stuff..."));
    init();
}

Button::~Button() = default;

void Button::init()
{
    const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAbstractButton::clicked, this, &Button::showDialog);
    WidgetQuestionListener::instance();
}

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 76)
void Button::setButtonText(const QString &what)
{
    setText(what);
}
#endif

void Button::setConfigFile(const QString &configFile)
{
    d->configFile = configFile;
}

void Button::showDialog()
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        KMessageBox::information(this, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
        return;
    }
    Q_EMIT aboutToShowDialog();

    if (!d->dialog) {
        d->dialog = new QtQuickDialogWrapper(d->configFile, this);
    }
    const auto changedInternalEntries = d->dialog->exec();
    QList<KNS3::Entry> changedEntries;
    for (const KNSCore::EntryInternal &e : changedInternalEntries) {
        changedEntries << EntryPrivate::fromInternal(&e);
    }
    Q_EMIT dialogFinished(changedEntries);
}

}

#include "moc_button.cpp"

#endif
