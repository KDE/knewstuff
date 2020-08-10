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
 * <li>kpackage: require that the downloaded file is a kpackage, and use the kpackage framework for handling installation and removal (since 5.70)</li>
 * </ol>
 *
 * You have different options to set the target install directory:
 *   <ol><li>StandardResource: not available in KF5, use XdgTargetDir instead.</li>
 *       <li>TargetDir: since KF5, this is equivalent to XdgTargetDir.
 *       <li>XdgTargetDir: a directory in the $XDG_DATA_HOME directory such as <em>.local/share/wallpapers</em>.
 *           This is what QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + name will return.</li>
 *   </ol>
 *
 * \subsection Remove Dead Entries
 *
 * If you set <em>RemoveDeadEntries=true</em>, entries whose installed files have all been deleted without going through
 * KNewStuff will be removed from the cache. The removal will happen if, and only if, all listed files were removed, which
 * means that if, for example, an entry was installed from archive, which was decompressed to yield multiple installed files,
 * if even one of those files remains, the entry will remain marked as installed.
 *
 * \subsection KPackage Support
 *
 * To make use of the KPackage option described above, in addition to the Uncompress setting above, you should also specify
 * the type of archive expected by KPackage. While it is possible to deduce this from the package metadata in many situations,
 * it is not a requirement of the format that this information exists, and we need to have a fallback in the case it is not
 * available there. As such, you will want to add a KPackageType entry to your knsrc file. The following example shows how this
 * is done for Plasma themes:
 *
 * <pre>
   ProvidersUrl=https://autoconfig.kde.org/ocs/providers.xml
   Categories=Plasma Theme
   StandardResource=tmp
   TagFilter=ghns_excluded!=1,plasma##version==5
   DownloadTagFilter=plasma##version==5
   Uncompress=kpackage
   KPackageType=Plasma/Theme
 * </pre>
 *
 * \note Using KPackage support will automatically set the removal of dead entries option to true. You can override this if you
 * want to, by explicitly adding <em>RemoveDeadEntries=false</em> to your knsrc file
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
