/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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
#ifndef KNEWSTUFF3_INSTALLATION_P_H
#define KNEWSTUFF3_INSTALLATION_P_H

#include <QObject>
#include <QString>

#include <kconfiggroup.h>

#include "entryinternal.h"

#include "knewstuffcore_export.h"

class QProcess;
class KArchiveDirectory;
class KJob;

namespace KNSCore
{

/**
 * @short KNewStuff entry installation.
 *
 * The installation class stores all information related to an entry's
 * installation.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFFCORE_EXPORT Installation : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit Installation(QObject *parent = nullptr);

    enum Policy {
        CheckNever,
        CheckIfPossible,
        CheckAlways
    };

    enum Scope {
        ScopeUser,
        ScopeSystem
    };

    bool readConfig(const KConfigGroup &group);
    bool isRemote() const;

public Q_SLOTS:
    /**
     * Downloads a payload file. The payload file matching most closely
     * the current user language preferences will be downloaded.
     * The file will not be installed set, for this \ref install must
     * be called.
     *
     * @param entry Entry to download payload file for
     *
     * @see signalPayloadLoaded
     * @see signalPayloadFailed
     */
    void downloadPayload(const KNSCore::EntryInternal &entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     * Note that this method is asynchronous and thus the return value will
     * only report the successful start of the installation.
     * Note also that while entry is const at this point, it will change later
     * during the actual installation (the installedFiles list will change, as
     * will its status)
     *
     * @param entry Entry to be installed
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(const KNSCore::EntryInternal &entry);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * The entry instance will be updated with any new information:
     * <ul>
     * <li>Status will be set to Deleted
     * <li>uninstalledFiles will list files which were removed during uninstallation
     * <li>installedFiles will become empty
     * </ul>
     *
     * @param entry The entry to deinstall
     *
     * @note FIXME: I don't believe this works yet :)
     */
    void uninstall(KNSCore::EntryInternal entry);

    // TODO KF6: remove, was used with deprecated Security class.
    Q_DECL_DEPRECATED void slotInstallationVerification(int result);

    void slotPayloadResult(KJob *job);

    /**
     * @returns the installation path
     *
     * @since 5.31
     */
    QString targetInstallationPath() const;

Q_SIGNALS:
    void signalEntryChanged(const KNSCore::EntryInternal &entry);
    void signalInstallationFinished();
    void signalInstallationFailed(const QString &message);

    void signalPayloadLoaded(QUrl payload); // FIXME: return Entry

    // TODO KF6: remove, was used with deprecated Security class.
    Q_DECL_DEPRECATED void signalInformation(const QString &) const;
    Q_DECL_DEPRECATED void signalError(const QString &) const;

private:
    void install(KNSCore::EntryInternal entry, const QString &downloadedFile);

    QStringList installDownloadedFileAndUncompress(const KNSCore::EntryInternal  &entry, const QString &payloadfile, const QString installdir);
    QProcess* runPostInstallationCommand(const QString &installPath);

    static QStringList archiveEntries(const QString &path, const KArchiveDirectory *dir);

    // applications can set this if they want the installed files/directories to be piped into a shell command
    QString postInstallationCommand;
    // a custom command to run for the uninstall
    QString uninstallCommand;
    // compression policy
    QString uncompression;

    // only one of the five below can be set, that will be the target install path/file name
    // FIXME: check this when reading the config and make one path out of it if possible?
    QString standardResourceDirectory;
    QString targetDirectory;
    QString xdgTargetDirectory;
    QString installPath;
    QString absoluteInstallPath;

    // policies whether verification needs to be done
    Policy checksumPolicy;
    Policy signaturePolicy;
    // scope: install into user or system dirs
    Scope scope;

    // FIXME this throws together a file name from entry name and version - why would anyone want that?
    bool customName;
    bool acceptHtml;

    QMap<KJob *, EntryInternal> entry_jobs;

    Q_DISABLE_COPY(Installation)
};

}

#endif
