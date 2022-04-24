/*
    SPDX-FileCopyrightText: 2021 Oleg Solovyov <mcpain@altlinux.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "action.h"

#include "entry_p.h"
#include "qtquickdialogwrapper.h"
#include "ui/widgetquestionlistener.h"
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>

using namespace KNS3;

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
        d->dialog = new QtQuickDialogWrapper(d->configFile, this);
        connect(d->dialog.data(), &KNS3::QtQuickDialogWrapper::closed, this, [this]() {
            const QList<KNSCore::EntryInternal> changedInternalEntries = d->dialog->changedEntries();
#if KNEWSTUFFWIDGETS_BUILD_DEPRECATED_SINCE(5, 91)
            QList<KNS3::Entry> changedEntries;
            for (const KNSCore::EntryInternal &e : changedInternalEntries) {
                changedEntries << EntryPrivate::fromInternal(&e);
            }
            Q_EMIT dialogFinished(changedEntries);
#endif
            Q_EMIT dialogFinished(changedInternalEntries);
        });
    }
    d->dialog->open();
}

}
