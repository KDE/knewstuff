/*
    SPDX-FileCopyrightText: 2021 Oleg Solovyov <mcpain@altlinux.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "action.h"

#include "dialog.h"
#include <KAuthorized>
#include <KLocalizedString>

namespace KNSWidgets
{
class ActionPrivate
{
public:
    QString configFile;
    std::unique_ptr<Dialog> dialog;
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

    const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAction::triggered, this, [this]() {
        if (!KAuthorized::authorize(KAuthorized::GHNS)) {
            return;
        }

        if (!d->dialog) {
            d->dialog.reset(new KNSWidgets::Dialog(d->configFile));
            connect(d->dialog.get(), &KNSWidgets::Dialog::finished, this, [this]() {
                Q_EMIT dialogFinished(d->dialog->changedEntries());
            });
        }
        d->dialog->open();
    });
}

Action::~Action() = default;
}
