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

#include "installation.h"

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QUrlQuery>
#include <QDesktopServices>

#include "qmimedatabase.h"
#include "karchive.h"
#include "kzip.h"
#include "ktar.h"
#include "krandom.h"
#include "kshell.h"

#include <qstandardpaths.h>
#include "klocalizedstring.h"
#include <knewstuffcore_debug.h>

#include "jobs/filecopyjob.h"
#include "question.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#endif

using namespace KNSCore;

Installation::Installation(QObject *parent)
    : QObject(parent)
    , checksumPolicy(Installation::CheckIfPossible)
    , signaturePolicy(Installation::CheckIfPossible)
    , scope(Installation::ScopeUser)
    , customName(false)
    , acceptHtml(false)
{
}

bool Installation::readConfig(const KConfigGroup &group)
{
    // FIXME: add support for several categories later on
    // FIXME: read out only when actually installing as a performance improvement?
    QString uncompresssetting = group.readEntry("Uncompress", QStringLiteral("never"));
    // support old value of true as equivalent of always
    if (uncompresssetting == QLatin1String("subdir")) {
        uncompresssetting = QStringLiteral("subdir");
    }
    else if (uncompresssetting == QLatin1String("true")) {
        uncompresssetting = QStringLiteral("always");
    }
    if (uncompresssetting != QLatin1String("always") && uncompresssetting != QLatin1String("archive") && uncompresssetting != QLatin1String("never") && uncompresssetting != QLatin1String("subdir")) {
        qCritical() << "invalid Uncompress setting chosen, must be one of: subdir, always, archive, or never" << endl;
        return false;
    }
    uncompression = uncompresssetting;
    postInstallationCommand = group.readEntry("InstallationCommand", QString());
    uninstallCommand = group.readEntry("UninstallCommand", QString());
    standardResourceDirectory = group.readEntry("StandardResource", QString());
    targetDirectory = group.readEntry("TargetDir", QString());
    xdgTargetDirectory = group.readEntry("XdgTargetDir", QString());

    // Provide some compatibility
    if (standardResourceDirectory == QLatin1String("wallpaper")) {
        xdgTargetDirectory = QStringLiteral("wallpapers");
    }

    // also, ensure wallpapers are decompressed into subdirectories
    // this ensures that wallpapers with multiple resolutions continue to function
    // as expected
    if (xdgTargetDirectory == QStringLiteral("wallpapers")) {
        uncompression = QStringLiteral("subdir");
    }

    installPath = group.readEntry("InstallPath", QString());
    absoluteInstallPath = group.readEntry("AbsoluteInstallPath", QString());
    customName = group.readEntry("CustomName", false);
    acceptHtml = group.readEntry("AcceptHtmlDownloads", false);

    if (standardResourceDirectory.isEmpty() &&
            targetDirectory.isEmpty() &&
            xdgTargetDirectory.isEmpty() &&
            installPath.isEmpty() &&
            absoluteInstallPath.isEmpty()) {
        qCritical() << "No installation target set";
        return false;
    }

    QString checksumpolicy = group.readEntry("ChecksumPolicy", QString());
    if (!checksumpolicy.isEmpty()) {
        if (checksumpolicy == QLatin1String("never")) {
            checksumPolicy = Installation::CheckNever;
        } else if (checksumpolicy == QLatin1String("ifpossible")) {
            checksumPolicy = Installation::CheckIfPossible;
        } else if (checksumpolicy == QLatin1String("always")) {
            checksumPolicy = Installation::CheckAlways;
        } else {
            qCritical() << "The checksum policy '" + checksumpolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString signaturepolicy = group.readEntry("SignaturePolicy", QString());
    if (!signaturepolicy.isEmpty()) {
        if (signaturepolicy == QLatin1String("never")) {
            signaturePolicy = Installation::CheckNever;
        } else if (signaturepolicy == QLatin1String("ifpossible")) {
            signaturePolicy = Installation::CheckIfPossible;
        } else if (signaturepolicy == QLatin1String("always")) {
            signaturePolicy = Installation::CheckAlways;
        } else {
            qCritical() << "The signature policy '" + signaturepolicy + "' is unknown." << endl;
            return false;
        }
    }

    QString scopeString = group.readEntry("Scope", QString());
    if (!scopeString.isEmpty()) {
        if (scopeString == QLatin1String("user")) {
            scope = ScopeUser;
        } else if (scopeString == QLatin1String("system")) {
            scope = ScopeSystem;
        } else {
            qCritical() << "The scope '" + scopeString + "' is unknown." << endl;
            return false;
        }

        if (scope == ScopeSystem) {
            if (!installPath.isEmpty()) {
                qCritical() << "System installation cannot be mixed with InstallPath." << endl;
                return false;
            }
        }
    }
    return true;
}

bool Installation::isRemote() const
{
    if (!installPath.isEmpty()) {
        return false;
    }
    if (!targetDirectory.isEmpty()) {
        return false;
    }
    if (!xdgTargetDirectory.isEmpty()) {
        return false;
    }
    if (!absoluteInstallPath.isEmpty()) {
        return false;
    }
    if (!standardResourceDirectory.isEmpty()) {
        return false;
    }
    return true;
}

void Installation::install(const EntryInternal& entry)
{
    downloadPayload(entry);
}

void Installation::downloadPayload(const KNSCore::EntryInternal &entry)
{
    if (!entry.isValid()) {
        emit signalInstallationFailed(i18n("Invalid item."));
        return;
    }
    QUrl source = QUrl(entry.payload());

    if (!source.isValid()) {
        qCritical() << "The entry doesn't have a payload." << endl;
        emit signalInstallationFailed(i18n("Download of item failed: no download URL for \"%1\".", entry.name()));
        return;
    }

    // FIXME no clue what this is supposed to do
    if (isRemote()) {
        // Remote resource
        qCDebug(KNEWSTUFFCORE) << "Relaying remote payload '" << source << "'";
        install(entry, source.toDisplayString(QUrl::PreferLocalFile));
        emit signalPayloadLoaded(source);
        // FIXME: we still need registration for eventual deletion
        return;
    }

    QString fileName(source.fileName());
    QTemporaryFile tempFile(QDir::tempPath() + QStringLiteral("/XXXXXX-") + fileName);
    if (!tempFile.open()) {
        return;    // ERROR
    }
    QUrl destination = QUrl::fromLocalFile(tempFile.fileName());
    qCDebug(KNEWSTUFFCORE) << "Downloading payload" << source << "to" << destination;

    // FIXME: check for validity
    FileCopyJob *job = FileCopyJob::file_copy(source, destination, -1, JobFlag::Overwrite | JobFlag::HideProgressInfo);
    connect(job,
            &KJob::result,
            this, &Installation::slotPayloadResult);

    entry_jobs[job] = entry;
}

void Installation::slotPayloadResult(KJob *job)
{
    // for some reason this slot is getting called 3 times on one job error
    if (entry_jobs.contains(job)) {
        EntryInternal entry = entry_jobs[job];
        entry_jobs.remove(job);

        if (job->error()) {
            emit signalInstallationFailed(i18n("Download of \"%1\" failed, error: %2", entry.name(), job->errorString()));
        } else {
            FileCopyJob *fcjob = static_cast<FileCopyJob *>(job);

            // check if the app likes html files - disabled by default as too many bad links have been submitted to opendesktop.org
            if (!acceptHtml) {
                QMimeDatabase db;
                QMimeType mimeType = db.mimeTypeForFile(fcjob->destUrl().toLocalFile());
                if (mimeType.inherits(QStringLiteral("text/html")) || mimeType.inherits(QStringLiteral("application/x-php"))) {
                    Question question;
                    question.setQuestion(i18n("The downloaded file is a html file. This indicates a link to a website instead of the actual download. Would you like to open the site with a browser instead?"));
                    question.setTitle(i18n("Possibly bad download link"));
                    if(question.ask() == Question::YesResponse) {
                        QDesktopServices::openUrl(fcjob->srcUrl());
                        emit signalInstallationFailed(i18n("Downloaded file was a HTML file. Opened in browser."));
                        entry.setStatus(KNS3::Entry::Invalid);
                        emit signalEntryChanged(entry);
                        return;
                    }
                }
            }

            emit signalPayloadLoaded(fcjob->destUrl());
            install(entry, fcjob->destUrl().toLocalFile());
        }
    }
}

void KNSCore::Installation::install(KNSCore::EntryInternal entry, const QString& downloadedFile)
{
    qCDebug(KNEWSTUFFCORE) << "Install: " << entry.name() << " from " << downloadedFile;

    if (entry.payload().isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "No payload associated with: " << entry.name();
        return;
    }

    // this means check sum comparison and signature verification
    // signature verification might take a long time - make async?!
    /*
    if (checksumPolicy() != Installation::CheckNever) {
        if (entry.checksum().isEmpty()) {
            if (checksumPolicy() == Installation::CheckIfPossible) {
                qCDebug(KNEWSTUFFCORE) << "Skip checksum verification";
            } else {
                qCritical() << "Checksum verification not possible" << endl;
                return false;
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Verify checksum...";
        }
    }
    if (signaturePolicy() != Installation::CheckNever) {
        if (entry.signature().isEmpty()) {
            if (signaturePolicy() == Installation::CheckIfPossible) {
                qCDebug(KNEWSTUFFCORE) << "Skip signature verification";
            } else {
                qCritical() << "Signature verification not possible" << endl;
                return false;
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Verify signature...";
        }
    }
    */

    QString targetPath = targetInstallationPath();
    QStringList installedFiles = installDownloadedFileAndUncompress(entry, downloadedFile, targetPath);

    if (installedFiles.isEmpty()) {
        if (entry.status() == KNS3::Entry::Installing) {
            entry.setStatus(KNS3::Entry::Downloadable);
        } else if (entry.status() == KNS3::Entry::Updating) {
            entry.setStatus(KNS3::Entry::Updateable);
        }
        emit signalEntryChanged(entry);
        emit signalInstallationFailed(i18n("Could not install \"%1\": file not found.", entry.name()));
        return;
    }

    entry.setInstalledFiles(installedFiles);

    auto installationFinished = [this, entry]() {
        EntryInternal newentry = entry;
        // update version and release date to the new ones
        if (newentry.status() == KNS3::Entry::Updating) {
            if (!newentry.updateVersion().isEmpty()) {
                newentry.setVersion(newentry.updateVersion());
            }
            if (newentry.updateReleaseDate().isValid()) {
                newentry.setReleaseDate(newentry.updateReleaseDate());
            }
        }

        newentry.setStatus(KNS3::Entry::Installed);
        emit signalEntryChanged(newentry);
        emit signalInstallationFinished();
    };
    if (!postInstallationCommand.isEmpty()) {
        QProcess* p = runPostInstallationCommand(installedFiles.size() == 1 ? installedFiles.first() : targetPath);
        connect(p, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, installationFinished);
    } else {
        installationFinished();
    }
}

QString Installation::targetInstallationPath() const
{
    QString installdir;

    if (!isRemote()) {
        // installdir is the target directory

        // installpath also contains the file name if it's a single file, otherwise equal to installdir
        int pathcounter = 0;
#if 0 // not available in KF5
        if (!standardResourceDirectory.isEmpty()) {
            if (scope == ScopeUser) {
                installdir = KStandardDirs::locateLocal(standardResourceDirectory.toUtf8(), "/");
            } else { // system scope
                installdir = KStandardDirs::installPath(standardResourceDirectory.toUtf8());
            }
            pathcounter++;
        }
#endif
       /* this is a partial reimplementation of the above, it won't ensure a perfect 1:1
        porting, but will make many kde4 ksnsrc files work out of the box*/
       //wallpaper is already managed in the case of !xdgTargetDirectory.isEmpty()
        if (!standardResourceDirectory.isEmpty() && standardResourceDirectory != QLatin1String("wallpaper")) {
            QStandardPaths::StandardLocation location = QStandardPaths::TempLocation;
            //crude translation KStandardDirs names -> QStandardPaths enum
            if (standardResourceDirectory == QLatin1String("tmp")) {
                location = QStandardPaths::TempLocation;
            } else if (standardResourceDirectory == QLatin1String("config")) {
                location = QStandardPaths::ConfigLocation;
            }

            if (scope == ScopeUser) {
                installdir = QStandardPaths::writableLocation(location);
            } else { // system scope
                installdir = QStandardPaths::standardLocations(location).constLast();
            }
            pathcounter++;
        }
        if (!targetDirectory.isEmpty() && targetDirectory != QLatin1String("/")) {
            if (scope == ScopeUser) {
                installdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + targetDirectory + QLatin1Char('/');
            } else { // system scope
                installdir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, targetDirectory, QStandardPaths::LocateDirectory) + QLatin1Char('/');
            }
            pathcounter++;
        }
        if (!xdgTargetDirectory.isEmpty() && xdgTargetDirectory != QLatin1String("/")) {
            installdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + xdgTargetDirectory + QLatin1Char('/');
            pathcounter++;
        }
        if (!installPath.isEmpty()) {
#if defined(Q_OS_WIN)
            WCHAR wPath[MAX_PATH + 1];
            if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
                installdir = QString::fromUtf16((const ushort *) wPath) + QLatin1Char('/') + installPath + QLatin1Char('/');
            } else {
                installdir =  QDir::homePath() + QLatin1Char('/') + installPath + QLatin1Char('/');
            }
#else
            installdir = QDir::homePath() + QLatin1Char('/') + installPath + QLatin1Char('/');
#endif
            pathcounter++;
        }
        if (!absoluteInstallPath.isEmpty()) {
            installdir = absoluteInstallPath + QLatin1Char('/');
            pathcounter++;
        }

        if (pathcounter != 1) {
            qCritical() << "Wrong number of installation directories given." << endl;
            return QString();
        }

        qCDebug(KNEWSTUFFCORE) << "installdir: " << installdir;

        // create the dir if it doesn't exist (QStandardPaths doesn't create it, unlike KStandardDirs!)
        QDir().mkpath(installdir);
    }

    return installdir;
}

QStringList Installation::installDownloadedFileAndUncompress(const KNSCore::EntryInternal  &entry, const QString &payloadfile, const QString installdir)
{
    // Collect all files that were installed
    QStringList installedFiles;

    if (!isRemote()) {
        bool isarchive = true;

        // respect the uncompress flag in the knsrc
        if (uncompression == QLatin1String("always") || uncompression == QLatin1String("archive") || uncompression == QLatin1String("subdir")) {
            // this is weird but a decompression is not a single name, so take the path instead
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFile(payloadfile);
            qCDebug(KNEWSTUFFCORE) << "Postinstallation: uncompress the file";

            // FIXME: check for overwriting, malicious archive entries (../foo) etc.
            // FIXME: KArchive should provide "safe mode" for this!
            QScopedPointer<KArchive> archive;

            if (mimeType.inherits(QStringLiteral("application/zip"))) {
                archive.reset(new KZip(payloadfile));
            } else if (mimeType.inherits(QStringLiteral("application/tar"))
                       || mimeType.inherits(QStringLiteral("application/x-gzip"))
                       || mimeType.inherits(QStringLiteral("application/x-bzip"))
                       || mimeType.inherits(QStringLiteral("application/x-lzma"))
                       || mimeType.inherits(QStringLiteral("application/x-xz"))
                       || mimeType.inherits(QStringLiteral("application/x-bzip-compressed-tar"))
                       || mimeType.inherits(QStringLiteral("application/x-compressed-tar"))) {
                archive.reset(new KTar(payloadfile));
            } else {
                qCritical() << "Could not determine type of archive file '" << payloadfile << "'";
                if (uncompression == QLatin1String("always")) {
                    return QStringList();
                }
                isarchive = false;
            }

            if (isarchive) {
                bool success = archive->open(QIODevice::ReadOnly);
                if (!success) {
                    qCritical() << "Cannot open archive file '" << payloadfile << "'";
                    if (uncompression == QLatin1String("always")) {
                        return QStringList();
                    }
                    // otherwise, just copy the file
                    isarchive = false;
                }

                if (isarchive) {
                    const KArchiveDirectory *dir = archive->directory();
                    //if there is more than an item in the file, and we are requested to do so
                    //put contents in a subdirectory with the same name as the file
                    QString installpath;
                    if (uncompression == QLatin1String("subdir") && dir->entries().count() > 1) {
                        installpath = installdir + QLatin1Char('/') + QFileInfo(archive->fileName()).baseName();
                    } else {
                        installpath = installdir;
                    }

                    if (dir->copyTo(installpath)) {
                        installedFiles << archiveEntries(installpath, dir);
                        installedFiles << installpath + QLatin1Char('/');
                    } else
                        qCWarning(KNEWSTUFFCORE) << "could not install" << entry.name() << "to" << installpath;

                    archive->close();
                    QFile::remove(payloadfile);
                }
            }
        }

        qCDebug(KNEWSTUFFCORE) << "isarchive: " << isarchive;

        //some wallpapers are compressed, some aren't
        if ((!isarchive && standardResourceDirectory == QLatin1String("wallpaper")) ||
            (uncompression == QLatin1String("never") || (uncompression == QLatin1String("archive") && !isarchive))) {
            // no decompress but move to target

            /// @todo when using KIO::get the http header can be accessed and it contains a real file name.
            // FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
            QUrl source = QUrl(entry.payload());
            qCDebug(KNEWSTUFFCORE) << "installing non-archive from " << source.url();
            QString installfile;
            QString ext = source.fileName().section('.', -1);
            if (customName) {
                installfile = entry.name();
                installfile += '-' + entry.version();
                if (!ext.isEmpty()) {
                    installfile += '.' + ext;
                }
            } else {
                // TODO HACK This is a hack, the correct way of fixing it would be doing the KIO::get
                // and using the http headers if they exist to get the file name, but as discussed in
                // Randa this is not going to happen anytime soon (if ever) so go with the hack
                if (source.url().startsWith(QLatin1String("http://newstuff.kde.org/cgi-bin/hotstuff-access?file="))) {
                    installfile = QUrlQuery(source).queryItemValue(QStringLiteral("file"));
                    int lastSlash = installfile.lastIndexOf('/');
                    if (lastSlash >= 0) {
                        installfile = installfile.mid(lastSlash);
                    }
                }
                if (installfile.isEmpty()) {
                    installfile = source.fileName();
                }
            }
            QString installpath = installdir + QLatin1Char('/') + installfile;

            qCDebug(KNEWSTUFFCORE) << "Install to file " << installpath;
            // FIXME: copy goes here (including overwrite checking)
            // FIXME: what must be done now is to update the cache *again*
            //        in order to set the new payload filename (on root tag only)
            //        - this might or might not need to take uncompression into account
            // FIXME: for updates, we might need to force an overwrite (that is, deleting before)
            QFile file(payloadfile);
            bool success = true;
            const bool update = ((entry.status() == KNS3::Entry::Updateable) || (entry.status() == KNS3::Entry::Updating));

            if (QFile::exists(installpath) && QDir::tempPath() != installdir) {
                if (!update) {
                    Question question(Question::ContinueCancelQuestion);
                    question.setQuestion(i18n("Overwrite existing file?") + "\n'" + installpath + '\'');
                    question.setTitle(i18n("Download File"));
                    if(question.ask() != Question::ContinueResponse) {
                        return QStringList();
                    }
                }
                success = QFile::remove(installpath);
            }
            if (success) {
                //remove in case it's already present and in a temporary directory, so we get to actually use the path again
                if (installpath.startsWith(QDir::tempPath())) {
                    file.remove(installpath);
                }
                success = file.rename(installpath);
                qCDebug(KNEWSTUFFCORE) << "move: " << file.fileName() << " to " << installpath;
            }
            if (!success) {
                qCritical() << "Cannot move file '" << payloadfile << "' to destination '"  << installpath << "'";
                return QStringList();
            }
            installedFiles << installpath;
        }
    }
    return installedFiles;
}

QProcess* Installation::runPostInstallationCommand(const QString &installPath)
{
    QString command(postInstallationCommand);
    QString fileArg(KShell::quoteArg(installPath));
    command.replace(QLatin1String("%f"), fileArg);

    qCDebug(KNEWSTUFFCORE) << "Run command: " << command;

    QProcess* ret = new QProcess(this);
    connect(ret, static_cast<void(QProcess::*)(int)>(&QProcess::finished), this, [this, command](int exitcode){
        if (exitcode) {
            qCritical() << "Command '" << command << "' failed with code" << exitcode;
        }
        sender()->deleteLater();
    });

    QStringList args = KShell::splitArgs(command);
    ret->setProgram(args.takeFirst());
    ret->setArguments(args);
    ret->start();
    return ret;
}

void Installation::uninstall(EntryInternal entry)
{
    entry.setStatus(KNS3::Entry::Deleted);

    if (!uninstallCommand.isEmpty()) {
        foreach (const QString &file, entry.installedFiles()) {
            QFileInfo info(file);
            if (info.isFile()) {
                QString fileArg(KShell::quoteArg(file));
                QString command(uninstallCommand);
                command.replace(QLatin1String("%f"), fileArg);

                int exitcode = QProcess::execute(command);

                if (exitcode) {
                    qCritical() << "Command failed" << command;
                } else {
                    qCDebug(KNEWSTUFFCORE) << "Command executed successfully: " << command;
                }
            }
        }
    }

    foreach (const QString &file, entry.installedFiles()) {
        if (file.endsWith('/')) {
            QDir dir;
            bool worked = dir.rmdir(file);
            if (!worked) {
                // Maybe directory contains user created files, ignore it
                continue;
            }
        } else if (file.endsWith(QLatin1String("/*"))) {
            QDir dir(file.left(file.size()-2));
            bool worked = dir.removeRecursively();
            if (!worked) {
                qCWarning(KNEWSTUFFCORE) << "Couldn't remove" << dir.path();
                continue;
            }
        } else {
            QFileInfo info(file);
            if (info.exists() || info.isSymLink()) {
                bool worked = QFile::remove(file);
                if (!worked) {
                    qWarning() << "unable to delete file " << file;
                    return;
                }
            } else {
                qWarning() << "unable to delete file " << file << ". file does not exist.";
            }
        }
    }
    entry.setUnInstalledFiles(entry.installedFiles());
    entry.setInstalledFiles(QStringList());

    emit signalEntryChanged(entry);
}

void Installation::slotInstallationVerification(int result)
{
    Q_UNUSED(result)
    // Deprecated, was wired up to defunct Security class.
}

QStringList Installation::archiveEntries(const QString &path, const KArchiveDirectory *dir)
{
    QStringList files;
    foreach (const QString &entry, dir->entries()) {
        const auto currentEntry = dir->entry(entry);

        const QString childPath = path + QLatin1Char('/') + entry;
        if (currentEntry->isFile()) {
            files << childPath;
        } else if (currentEntry->isDirectory()) {
            files << childPath + QStringLiteral("/*");
        }
    }
    return files;
}

