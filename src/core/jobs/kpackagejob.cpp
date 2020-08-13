/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpackagejob.h"

#include <knewstuffcore_debug.h>

#include <KLocalizedString>

#include <KPackage/PackageStructure>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include <QCoreApplication>
#include <QRunnable>
#include <QStandardPaths>
#include <QThreadPool>
#include <QTimer>

using namespace KNSCore;

enum Operation {
    UnknownOperation,
    InstallOperation,
    UpdateOperation,
    UninstallOperation
};
class KPackageTask;
class KPackageJob::Private {
public:
    Private() {}

    QString package;
    QString packageRoot;
    QString serviceType;
    Operation operation{UnknownOperation};

    KPackageTask* runnable{nullptr};
};

class KPackageTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    QString package;
    QString packageRoot;
    QString serviceType;
    Operation operation{UnknownOperation};
    void run() override
    {
        qCDebug(KNEWSTUFFCORE) << "Attempting to perform an installation operation of type" << operation << "on the package" << package << "of type" << serviceType << "in the package root" << packageRoot;
        KPackage::PackageStructure *structure = KPackage::PackageLoader::self()->loadPackageStructure(serviceType);
        if (structure) {
            qCDebug(KNEWSTUFFCORE) << "Service type understood";
            KPackage::Package installer = KPackage::Package(structure);
            if (installer.hasValidStructure()) {
                qCDebug(KNEWSTUFFCORE) << "Installer successfully created and has a valid structure";
                KJob *job{nullptr};
                switch(operation)
                {
                case InstallOperation:
                    job = installer.install(package, packageRoot);
                    break;
                case UpdateOperation:
                    job = installer.update(package, packageRoot);
                    break;
                case UninstallOperation:
                    job = installer.uninstall(package, packageRoot);
                    break;
                case UnknownOperation:
                default:
                    // This should really not be happening, can't create one of these without going through one
                    // of the functions below, so how'd you get it in this state?
                    break;
                };
                if (job) {
                    qCDebug(KNEWSTUFFCORE) << "Created job, now let's wait for it to do its thing...";
                    QEventLoop loop;
                    connect(job, &KJob::result, this, [this,job,&loop](){
                        emit error(job->error(), job->errorText());
                        emit result();
                        loop.exit(0);
                    });
                    loop.exec();
                } else {
                    qCWarning(KNEWSTUFFCORE) << "Failed to create a job to perform our task";
                    emit error(3, i18n("Failed to create a job for the package management task. This is usually because the package is invalid. We attempted to operate on the package %1", package));
                }
            } else {
                qCWarning(KNEWSTUFFCORE) << "Failed to create package installer";
                emit error(2, i18n("Could not create a package installer for the service type %1: The installer does not have a valid structure", serviceType));
            }
        } else {
            qCWarning(KNEWSTUFFCORE) << "Service type was not understood";
            emit error(1, i18n("The service type %1 was not understood by the KPackage installer", serviceType));
        }
    }
    Q_SIGNAL void result();
    Q_SIGNAL void error(int errorCode, const QString& errorText);
};

KPackageJob::KPackageJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

KPackageJob::~KPackageJob()
{
    delete d;
}

void KPackageJob::start()
{
    if (d->runnable) {
        // refuse to start the task more than once
        return;
    }
    d->runnable = new KPackageTask();
    d->runnable->package = d->package;
    d->runnable->packageRoot = d->packageRoot;
    d->runnable->serviceType = d->serviceType;
    d->runnable->operation = d->operation;
    connect(d->runnable, &KPackageTask::error, this, [this](int errorCode, const QString& errorText){
        setError(errorCode);
        setErrorText(errorText);
    }, Qt::QueuedConnection);
    connect(d->runnable, &KPackageTask::result, this, [this](){ emitResult(); }, Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(d->runnable);
}

KNSCore::KPackageJob * KNSCore::KPackageJob::install(const QString &sourcePackage, const QString &packageRoot, const QString &serviceType)
{
    KPackageJob* job = new KPackageJob();
    job->d->package = sourcePackage;
    job->d->packageRoot = packageRoot;
    job->d->serviceType = serviceType;
    job->d->operation = InstallOperation;
    QTimer::singleShot(0, job, &KPackageJob::start);
    return job;
}

KPackageJob * KPackageJob::update(const QString &sourcePackage, const QString &packageRoot, const QString &serviceType)
{
    KPackageJob* job = new KPackageJob();
    job->d->package = sourcePackage;
    job->d->packageRoot = packageRoot;
    job->d->serviceType = serviceType;
    job->d->operation = UpdateOperation;
    QTimer::singleShot(0, job, &KPackageJob::start);
    return job;
}

KPackageJob * KPackageJob::uninstall(const QString &packageName, const QString &packageRoot, const QString &serviceType)
{
    KPackageJob* job = new KPackageJob();
    job->d->package = packageName;
    job->d->packageRoot = packageRoot;
    job->d->serviceType = serviceType;
    job->d->operation = UninstallOperation;
    QTimer::singleShot(0, job, &KPackageJob::start);
    return job;
}

#include "kpackagejob.moc"
