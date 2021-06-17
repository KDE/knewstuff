/*
    knewstuff3/ui/downloaddialog.h.
    SPDX-FileCopyrightText: 2005 Enrico Ros <eros.kde@email.it>
    SPDX-FileCopyrightText: 2005-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF3_UI_DOWNLOADDIALOG_H

#include <QDialog>

#include "entry.h"
#include "knewstuff_export.h"

#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 80)

namespace KNSCore
{
class Engine;
}

namespace KNS3
{
class DownloadDialogPrivate;

/**
 * KNewStuff download dialog.
 *
 * The download dialog will present items to the user
 * for installation, updates and removal.
 * Preview images as well as other meta information can be seen.
 *
 * @since 4.4
 * @deprecated Since 5.80, use the QML components or the KNS3::QtQuickDialogWrapper instead
 */
class KNEWSTUFF_EXPORT DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Create a DownloadDialog that lets the user install, update and uninstall
     * contents. It will try to find a appname.knsrc file with the configuration.
     * Appname is the name of your application as provided in the about data->
     *
     * @param parent the parent of the dialog
     * @deprecated Since 5.80, use the QML components or the KNS3::QtQuickDialogWrapper instead
     */
    KNEWSTUFF_DEPRECATED_VERSION(5, 80, "Use the QML components or the KNS3::QtQuickDialogWrapper instead")
    explicit DownloadDialog(QWidget *parent = nullptr);

    /**
     * Create a DownloadDialog that lets the user install, update and uninstall
     * contents. Manually specify the name of a .knsrc file where the
     * KHotNewStuff configuration can be found.
     *
     * @param configFile the name of the configuration file
     * @param parent the parent of the dialog
     * @deprecated Since 5.80, use the QML components or the KNS3::QtQuickDialogWrapper instead
     */
    KNEWSTUFF_DEPRECATED_VERSION(5, 80, "Use the QML components or the KNS3::QtQuickDialogWrapper instead")
    explicit DownloadDialog(const QString &configFile, QWidget *parent = nullptr);

    /**
     * destructor
     */
    ~DownloadDialog() override;

    /**
     * The list of entries with changed status (installed/uninstalled)
     * @return the list of entries
     */
    KNS3::Entry::List changedEntries();

    /**
     * The list of entries that have been newly installed
     * @return the list of entries
     */
    KNS3::Entry::List installedEntries();

    /**
     * Set the title for display purposes in the widget's title.
     * @param title the title of the application (or category or whatever)
     */
    void setTitle(const QString &title);

    /**
     * Get the current title
     * @return the current title
     */
    QString title() const;

    /**
     * @return the engine used by this dialog
     * @since 5.30
     */
    KNSCore::Engine *engine();

public Q_SLOTS:
    // Override these slots so we can add KAuthorized checks to them.
    int exec() override;
    void open() override;

protected:
    void showEvent(QShowEvent *event) override;

private:
    void init(const QString &configFile);

    DownloadDialogPrivate *const d;
    Q_DISABLE_COPY(DownloadDialog)
};

}

#endif
#endif
