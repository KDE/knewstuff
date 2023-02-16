/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpackagejob.h"

#include <knewstuffcore_debug.h>

#include <KLocalizedString>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KPackage/PackageStructure>

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
    UninstallOperation,
};
class KPackageTask;
class KNSCore::KPackageJobPrivate
{
public:
    KPackageJobPrivate()
    {
    }

    QString package;
    QString packageRoot;
    QString serviceType;
    Operation operation{UnknownOperation};

    KPackageTask *runnable{nullptr};
};

class KPackageTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    QString package;
    QString packageRoot;
    QString serviceType;
    Operation operation{UnknownOperation};

    explicit KPackageTask(QObject *parent = nullptr)
        : QObject(parent)
        , QRunnable()
    {
        // We'll handle our own deletion - otherwise we may end up deleted
        // before things have been read out that we need to have read
        // As this has to be set before QThreadPool runs things, we need to do so here
        setAutoDelete(false);
    };
    ~KPackageTask() override
    {
    }
    void run() override
    {
        qCDebug(KNEWSTUFFCORE) << "Attempting to perform an installation operation of type" << operation << "on the package" << package << "of type"
                               << serviceType << "in the package root" << packageRoot;
        int errorlevel{0};
        QString errordescription;
        // PackageStructure instances are managed internally by KPackage, never delete them
        KPackage::PackageStructure *structure = KPackage::PackageLoader::self()->loadPackageStructure(serviceType);
        if (structure) {
            qCDebug(KNEWSTUFFCORE) << "Service type understood";
            installer.reset(new KPackage::Package(structure));
            if (installer->hasValidStructure()) {
                qCDebug(KNEWSTUFFCORE) << "Installer successfully created and has a valid structure";
                switch (operation) {
                case InstallOperation:
                    job.reset(installer->install(package, packageRoot));
                    break;
                case UpdateOperation:
                    job.reset(installer->update(package, packageRoot));
                    break;
                case UninstallOperation:
                    job.reset(installer->uninstall(package, packageRoot));
                    break;
                case UnknownOperation:
                default:
                    // This should really not be happening, can't create one of these without going through one
                    // of the functions below, so how'd you get it in this state?
                    break;
                };
                if (job) {
                    qCDebug(KNEWSTUFFCORE) << "Created job, now let's wait for it to do its thing...";
                    job->setAutoDelete(false);
                    QEventLoop loop;
                    connect(
                        job.get(),
                        &KJob::result,
                        this,
                        [&loop, &errordescription](KJob *job) { // clazy:exclude=lambda-in-connect
                            errordescription = job->errorText();
                            loop.exit(job->error());
                        },
                        Qt::BlockingQueuedConnection);
                    errorlevel = loop.exec();
                } else {
                    errorlevel = 3;
                    errordescription = i18n(
                        "Failed to create a job for the package management task. This is usually because the package is invalid. We attempted to operate on "
                        "the package %1",
                        package);
                }
            } else {
                errorlevel = 2;
                errordescription =
                    i18n("Could not create a package installer for the service type %1: The installer does not have a valid structure", serviceType);
            }
        } else {
            errorlevel = 1;
            errordescription = i18n("The service type %1 was not understood by the KPackage installer", serviceType);
        }
        if (errorlevel > 0) {
            Q_EMIT error(errorlevel, errordescription);
        }
        Q_EMIT result();
    }
    Q_SIGNAL void result();
    Q_SIGNAL void error(int errorCode, const QString &errorText);

private:
    QScopedPointer<KPackage::Package> installer;
    QScopedPointer<KJob> job;
};

KPackageJob::KPackageJob(QObject *parent)
    : KJob(parent)
    , d(new KPackageJobPrivate)
{
}

KPackageJob::~KPackageJob() = default;

void KPackageJob::start()
{
    if (d->runnable) {
        // refuse to start the task more than once
        return;
    }
    d->runnable = new KPackageTask(this);
    d->runnable->package = d->package;
    d->runnable->packageRoot = d->packageRoot;
    d->runnable->serviceType = d->serviceType;
    d->runnable->operation = d->operation;
    connect(
        d->runnable,
        &KPackageTask::error,
        this,
        [this](int errorCode, const QString &errorText) {
            setError(errorCode);
            setErrorText(errorText);
        },
        Qt::QueuedConnection);
    connect(
        d->runnable,
        &KPackageTask::result,
        this,
        [this]() {
            emitResult();
        },
        Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(d->runnable);
}

KPackageJob *KPackageJob::update(const QString &sourcePackage, const QString &packageRoot, const QString &serviceType)
{
    KPackageJob *job = new KPackageJob();
    job->d->package = sourcePackage;
    job->d->packageRoot = packageRoot;
    job->d->serviceType = serviceType;
    job->d->operation = UpdateOperation;
    QTimer::singleShot(0, job, &KPackageJob::start);
    return job;
}

KPackageJob *KPackageJob::uninstall(const QString &packageName, const QString &packageRoot, const QString &serviceType)
{
    KPackageJob *job = new KPackageJob();
    job->d->package = packageName;
    job->d->packageRoot = packageRoot;
    job->d->serviceType = serviceType;
    job->d->operation = UninstallOperation;
    QTimer::singleShot(0, job, &KPackageJob::start);
    return job;
}

#include "kpackagejob.moc"
