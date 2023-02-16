/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPACKAGEJOB_H
#define KPACKAGEJOB_H

#include <KCoreAddons/KJob>

namespace KNSCore
{
class KPackageJobPrivate;
/**
 * @brief A job for performing basic actions on KPackage packages asynchronously
 *
 * The internals of KPackage's Package functions are synchronous, which makes it easy to work with in some cases,
 * but has the unfortunate side effect of blocking the UI. This job will perform those operations in a separate
 * thread, which allows you to perform the work in a fire-and-forget fashion (as suggested by KJob's documentation).
 *
 * @since 5.71
 */
class KPackageJob : public KJob
{
    Q_OBJECT
public:
    /**
     * Create a job for updating the given package, or installing it if it is not already, the given package into the
     * package root, and treat it as the given service type.
     *
     * @param sourcePackage The full path name to the package you wish to update (e.g. /tmp/downloaded-archive.tar.xz)
     * @param packageRoot The full path name to the location the package should be installed into (e.g. /home/username/.share/plasma/desktoptheme/)
     * @param serviceType The name of the type of KPackage you intend to update (e.g. Plasma/Theme)
     * @return A job which you can use to track the completion of the process (there will be useful details in error() and errorText() on failures)
     */
    static KPackageJob *update(const QString &sourcePackage, const QString &packageRoot, const QString &serviceType);
    /**
     * Create a job for removing the given installed package
     *
     * @param packageName The name to the package you wish to remove (this is the plugin name, not the full path name, e.g. The.Package.Name)
     * @param packageRoot The full path name to the location the package is currently installed (e.g.
     * /home/username/.share/plasma/desktoptheme/The.Package.Name)
     * @param serviceType The name of the type of KPackage you intend to remove (e.g. Plasma/Theme)
     * @return A job which you can use to track the completion of the process (there will be useful details in error() and errorText() on failures)
     */
    static KPackageJob *uninstall(const QString &packageName, const QString &packageRoot, const QString &serviceType);

    ~KPackageJob() override;

    /**
     * Start the process asynchronously
     * @see KJob::start()
     */
    Q_SLOT void start() override;

private:
    explicit KPackageJob(QObject *parent = nullptr);
    const std::unique_ptr<KPackageJobPrivate> d;
};

}

#endif // KPACKAGEJOB_H
