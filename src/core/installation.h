/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_INSTALLATION_P_H
#define KNEWSTUFF3_INSTALLATION_P_H

#include <QObject>
#include <QString>

#include <KConfigGroup>

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
#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 79)
    enum Policy {
        CheckNever,
        CheckIfPossible,
        CheckAlways,
    };

    enum Scope {
        ScopeUser,
        ScopeSystem,
    };
#endif

    enum UncompressionOptions {
        NeverUncompress, ///@< Never attempt to decompress a file, whatever format it is. Matches "never" knsrc setting
        AlwaysUncompress, ///@< Assume all downloaded files are archives, and attempt to decompress them. Will cause failure if decompression fails. Matches
                          ///"always" knsrc setting
        UncompressIfArchive, ///@< If the file is an archive, decompress it, otherwise just pass it on. Matches "archive" knsrc setting
        UncompressIntoSubdirIfArchive, ///@< If the file is an archive, decompress it in a subdirectory if it contains multiple files, otherwise just pass it
                                       /// on. Matches "subdir-archive" knsrc setting
        UncompressIntoSubdir, ///@< As Archive, except that if there is more than an item in the file, put contents in a subdirectory with the same name as the
                              /// file. Matches "subdir" knsrc setting
        UseKPackageUncompression, ///@< Use the internal KPackage support for installing and uninstalling the package. Matches "kpackage" knsrc setting
    };
    Q_ENUM(UncompressionOptions)

    bool readConfig(const KConfigGroup &group);

#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(5, 71)
    KNEWSTUFFCORE_DEPRECATED_VERSION(5, 71, "No longer use, feature obsolete")
    bool isRemote() const;
#endif

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
     * The entry emitted by signalEntryChanged will be updated with any new information, in particular the following:
     * <ul>
     * <li>Status will be set to Deleted, unless the uninstall
     * script exists with an error and the user chooses to cancel the uninstallation
     * <li>uninstalledFiles will list files which were removed during uninstallation
     * <li>installedFiles will become empty
     * </ul>
     *
     * @param entry The entry to deinstall
     *
     */
    void uninstall(KNSCore::EntryInternal entry);

    /**
     * Returns the uncompression setting, in a computer-readable format
     *
     * @return The value of this setting
     * @since 5.71
     */
    UncompressionOptions uncompressionSetting() const;

    // TODO KF6: remove, was used with deprecated Security class.
#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(5, 31)
    KNEWSTUFFCORE_DEPRECATED_VERSION(5, 31, "No longer use")
    void slotInstallationVerification(int result);
#endif

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
    /**
     * An informational signal fired when a serious error occurs during the installation.
     * @param message The description of the error (a message intended to be human readable)
     * @since 5.69
     */
    void signalInstallationError(const QString &message);

    void signalPayloadLoaded(QUrl payload); // FIXME: return Entry

    // TODO KF6: remove, was used with deprecated Security class.
#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(5, 31)
    KNEWSTUFFCORE_DEPRECATED_VERSION(5, 31, "No longer use")
    void signalInformation(const QString &) const;
    KNEWSTUFFCORE_DEPRECATED_VERSION(5, 31, "No longer use")
    void signalError(const QString &) const;
#endif

    void installedFilesGenerated(KNSCore::EntryInternal entry, const QStringList &installedFile, const QString &installdir);

private Q_SLOTS:
    void slotPostInstall(KNSCore::EntryInternal entry, const QStringList &installedFiles, const QString &targetPath);
    void slotOverwriteFile(KNSCore::EntryInternal entry, const QString &installdir, int response, const QString &installpath, const QString &payloadfile);

private:
    void install(KNSCore::EntryInternal entry, const QString &downloadedFile);

    QStringList installDownloadedFileAndUncompress(const KNSCore::EntryInternal &entry, const QString &payloadfile, const QString installdir);
    void asyncInstallDownloadedFileAndUncompress(const KNSCore::EntryInternal &entry, const QString &payloadfile, const QString installdir);
    QProcess *runPostInstallationCommand(const QString &installPath);

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
#if KNEWSTUFFCORE_BUILD_DEPRECATED_SINCE(5, 79)
    // policies whether verification needs to be done
    Policy checksumPolicy = CheckIfPossible;
    Policy signaturePolicy = CheckIfPossible;
    // scope: install into user or system dirs
    Scope scope = ScopeUser;
    // FIXME this throws together a file name from entry name and version - why would anyone want that?
    bool customName = false;
    bool acceptHtml = false;
#endif

    QMap<KJob *, EntryInternal> entry_jobs;

    Q_DISABLE_COPY(Installation)
};

}

#endif
