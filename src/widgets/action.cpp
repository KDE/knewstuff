/*
    SPDX-FileCopyrightText: 2021 Oleg Solovyov <mcpain@altlinux.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "action.h"

#include "qtquickdialogwrapper.h"
#include <KAuthorized>
#include <KLocalizedString>

#include <QPointer>

namespace KNSWidgets
{
class ActionPrivate
{
public:
    QString configFile;
    QPointer<QtQuickDialogWrapper> dialog;
};

Action::Action(const QString &text, const QString &configFile, QObject *parent)
    : QAction(parent)
    , d(new ActionPrivate)
{
    if (text.isEmpty()) {
        setText(i18n("Download New Stuff..."));
    } else {
        setText(text);
    }
    d->configFile = configFile;
    init();
}

Action::~Action()
{
}

void Action::init()
{
    const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAction::triggered, this, &Action::showDialog);
}

void Action::setConfigFile(const QString &configFile)
{
    d->configFile = configFile;
}

void Action::showDialog()
{
    if (!KAuthorized::authorize(KAuthorized::GHNS)) {
        return;
    }
    Q_EMIT aboutToShowDialog();

    if (!d->dialog) {
        d->dialog = new KNSWidgets::QtQuickDialogWrapper(d->configFile, this);
        connect(d->dialog.data(), &KNSWidgets::QtQuickDialogWrapper::closed, this, [this]() {
            const QList<KNSCore::Entry> changedInternalEntries = d->dialog->changedEntries();
            Q_EMIT dialogFinished(changedInternalEntries);
        });
    }
    d->dialog->open();
}

}
