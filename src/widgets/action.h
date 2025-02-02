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

/*!
 * \namespace KNSWidgets
 * \inmodule KNewStuffWidgets
 *
 * \brief Contains UX widgets for KNewStuffCore.
 */
namespace KNSWidgets
{
class ActionPrivate;
/*!
 * \class KNSWidgets::Action
 * \inmodule KNewStuffWidgets
 *
 * \brief QAction subclass that encapsulates the logic for showing the KNewStuff dialog.
 *
 * If KNewStuff is disabled using KAuthorized, the action is hidden.
 *
 * \sa KAuthorized::GenericRestriction::GHNS
 *
 * \since 5.90
 */
class KNEWSTUFFWIDGETS_EXPORT Action : public QAction
{
    Q_OBJECT

public:
    /*!
     * Constructs a KNSWidgets::Action instance.
     *
     * \a text describing what is being downloaded.
     * It should be a text beginning with "Download New ..." for consistency
     *
     * \a configFile the name of the .knsrc file
     *
     * \a parent the parent object
     *
     */
    explicit Action(const QString &text, const QString &configFile, QObject *parent);

    ~Action();

Q_SIGNALS:
    /*!
     * Emitted when the dialog has been closed.
     *
     * \a changedEntries contains the entries that were changed
     *
     */
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);

private:
    std::unique_ptr<ActionPrivate> d;
};

}

#endif // KNEWSTUFFACTION_H
