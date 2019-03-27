/*
    knewstuff3/ui/downloaddialog.h.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KNEWSTUFF3_UI_DOWNLOADDIALOG_H
#define KNEWSTUFF3_UI_DOWNLOADDIALOG_H

#include <QDialog>

#include "knewstuff_export.h"
#include "entry.h"

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
 * \section knsrc knsrc Files
 * The Dialog is configured by a .knsrc file containing the KHotNewStuff configuration.
 * Your application should install a file into the XDG configuration location called: <em>/etc/xdg/appname.knsrc</em>
 *
 * The file could look like this for wallpapers:
 * <pre>
   [KNewStuff3]
   ProvidersUrl=https://download.kde.org/ocs/providers.xml
   Categories=KDE Wallpaper 1920x1200,KDE Wallpaper 1600x1200
   XdgTargetDir=wallpapers
   Uncompress=archive
 * </pre>
 *
 * Uncompress can be one of: always, never or archive:
 * <ol>
 * <li>always: assume all downloaded files are archives and need to be extracted</li>
 * <li>never: never try to extract the file</li>
 * <li>archive: if the file is an archive, uncompress it, otherwise just pass it on</li>
 * <li>subdir: logic as archive, but decompress into a subdirectory named after the payload filename</li>
 * </ol>
 *
 * You have different options to set the target install directory:
 *   <ol><li>StandardResource: not available in KF5, use XdgTargetDir instead.</li>
 *       <li>TargetDir: since KF5, this is equivalent to XdgTargetDir.
 *       <li>XdgTargetDir: a directory in the $XDG_DATA_HOME directory such as <em>.local/share/wallpapers</em>.
 *           This is what QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + name will return.</li>
 *   </ol>
 *
 * @since 4.4
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
     */
    explicit DownloadDialog(QWidget *parent = nullptr);

    /**
     * Create a DownloadDialog that lets the user install, update and uninstall
     * contents. Manually specify the name of a .knsrc file where the
     * KHotNewStuff configuration can be found.
     *
     * @param configFile the name of the configuration file
     * @param parent the parent of the dialog
     */
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
