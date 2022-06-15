/*
    SPDX-FileCopyrightText: 2021 Oleg Solovyov <mcpain@altlinux.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3ACTION_H
#define KNEWSTUFF3ACTION_H

#include <QAction>

#include "KNS3/Entry"
#include "KNSCore/EntryInternal"
#include "knewstuffwidgets_export.h"

namespace KNSCore
{
using Entry = KNSCore::EntryInternal;
}

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
    Action(const QString &text, const QString &configFile, QObject *parent);

    ~Action();

    /**
     * set the name of the .knsrc file to use
     */
    void setConfigFile(const QString &configFile);

Q_SIGNALS:
    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the action
     */
    void aboutToShowDialog();

#if KNEWSTUFFWIDGETS_ENABLE_DEPRECATED_SINCE(5, 90)
    /// emitted when the Hot New Stuff dialog has been closed
    void dialogFinished(const KNS3::Entry::List &changedEntries);
#endif
    // Only show this overload when we hide deprecated methods or build the lib
#if !KNEWSTUFFWIDGETS_ENABLE_DEPRECATED_SINCE(5, 90) || KF5NewStuffWidgets_EXPORTS
    /// emitted when the Hot New Stuff dialog has been closed
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);
#endif

private Q_SLOTS:
    void showDialog();

private:
    void init();

    std::unique_ptr<ActionPrivate> d;
};

}

#endif // KNEWSTUFFACTION_H
