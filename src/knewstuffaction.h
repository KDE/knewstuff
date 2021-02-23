/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KNEWSTUFF3_KNEWSTUFFACTION_H
#define KNEWSTUFF3_KNEWSTUFFACTION_H

#include "knewstuff_export.h"
#include <QString>

class QObject;
class QAction;
class KActionCollection;

/**
 * The namespace for the KNewStuff classes
 */
namespace KNS3
{
#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 78)
/**
 * @brief Standard action for the Hot New Stuff Download
 *
 * This action can be used to add KNewStuff support to menus and toolbars.
 *
 * @param what text describing what is being downloaded. For consistency,
 *             set it to "Get New Foobar...".
 *             Examples: "Get New Wallpapers...", "Get New Emoticons..."
 * @param receiver the QObject to connect the triggered(bool) signal to.
 * @param slot the slot to connect the triggered(bool) signal to.
 * @param parent the action's parent collection.
 * @param name The name by which the action will be retrieved again from the collection.
 * @since 4.4
 * @deprecated Since 5.78, create the QAction instance manually.
 */
KNEWSTUFF_DEPRECATED_VERSION(5, 78, "Create the QAction instance manually")
KNEWSTUFF_EXPORT QAction *standardAction(const QString &what, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name = nullptr);
#endif

#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 78)
/**
 * @brief Standard action for Uploading files with Hot New Stuff
 *
 * This action can be used to add KNewStuff support to menus and toolbars.
 *
 * @param what text describing what is being downloaded. For consistency,
 *             set it to "Upload Current Foobar...".
 *             Examples: "Upload Current Wallpaper...", "Upload Current Document..."
 * @param receiver the QObject to connect the triggered(bool) signal to.
 * @param slot the slot to connect the triggered(bool) signal to.
 * @param parent the action's parent collection.
 * @param name The name by which the action will be retrieved again from the collection.
 * @since 4.5
 * @deprecated Since 5.78, create the QAction instance manually.
 */
KNEWSTUFF_DEPRECATED_VERSION(5, 78, "Create the QAction instance manually")
KNEWSTUFF_EXPORT QAction *
standardActionUpload(const QString &what, const QObject *receiver, const char *slot, KActionCollection *parent, const char *name = nullptr);
#endif
}

#endif // KNEWSTUFFACTION_H
