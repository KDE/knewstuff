/*
    SPDX-FileCopyrightText: 2021 Oleg Solovyov <mcpain@altlinux.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3ACTION_H
#define KNEWSTUFF3ACTION_H

#include <KNSCore/Entry>
#include <QAction>

#include "knewstuffwidgets_export.h"

namespace KNSWidgets
{
class ActionPrivate;
/**
 * @class Action action.h <KNSWidgets/Action>
 *
 * Shows the KNS3::QtQuickDialogWrapper when the action is triggered.
 * If GHNS is disabled using KAuthorized, it is hidden.
 *
 * @since 5.90
 */
class KNEWSTUFFWIDGETS_EXPORT Action : public QAction
{
    Q_OBJECT

public:
    /**
     * Constructor used when the details of the KHotNewStuff
     * download is known when the action is created.
     *
     * @param text describing what is being downloaded.
     *        It should be a text beginning with "Download New ..." for consistency
     * @param configFile the name of the .knsrc file
     * @param parent the parent object
     */
    explicit Action(const QString &text, const QString &configFile, QObject *parent);

    ~Action();

Q_SIGNALS:
    /**
     * Emitted when the Hot New Stuff dialog has been closed.
     */
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);

private:
    std::unique_ptr<ActionPrivate> d;
};

}

#endif // KNEWSTUFFACTION_H
