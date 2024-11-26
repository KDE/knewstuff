/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "transaction.h"
#include "enginebase.h"
#include "enginebase_p.h"
#include "entry_p.h"
#include "providerbase_p.h"
#include "providercore.h"
#include "providercore_p.h"
#include "question.h"

#include <KLocalizedString>
#include <KShell>
#include <QDir>
#include <QProcess>
#include <QTimer>
#include <QVersionNumber>

#include <knewstuffcore_debug.h>

using namespace KNSCore;

namespace
{
std::optional<int> linkIdFromVersions(const QList<DownloadLinkInformationV2Private> &downloadLinksInformationList)
{
    switch (downloadLinksInformationList.size()) {
    case 0:
        return {};
    case 1:
        return downloadLinksInformationList.at(0).id;
    }

    QMap<QVersionNumber, int> infoByVersion;
    for (const auto &info : downloadLinksInformationList) {
        const auto number = QVersionNumber::fromString(info.version);
        if (number.isNull()) {
            qCDebug(KNEWSTUFFCORE) << "Found no valid version number on linkid" << info.id << info.version;
            continue;
        }
        if (infoByVersion.contains(number)) {
            qCWarning(KNEWSTUFFCORE) << "Encountered version number" << info.version << "more than once. Ignoring duplicates." << info.distributionType;
            continue;
        }
        infoByVersion[number] = info.id;
    }

    if (infoByVersion.isEmpty()) { // found no valid version
        return {};
    }

    return infoByVersion.last(); // map is sorted by keys, highest version is last entry.
}
} // namespace

class KNSCore::TransactionPrivate
{
public:
    [[nodiscard]] static Transaction *createInstallTransaction(const KNSCore::Entry &_entry, EngineBase *engine, int linkId)
    {
        auto ret = new Transaction(_entry, engine);
        QObject::connect(engine->d->installation, &Installation::signalInstallationError, ret, [ret, _entry](const QString &msg, const KNSCore::Entry &entry) {
            if (_entry.uniqueId() == entry.uniqueId()) {
                Q_EMIT ret->signalErrorCode(KNSCore::ErrorCode::InstallationError, msg, {});
            }
        });
        QTimer::singleShot(0, ret, [ret, linkId] {
            ret->d->installLinkId(linkId);
        });
        return ret;
    }

    TransactionPrivate(const KNSCore::Entry &entry, EngineBase *engine, Transaction *q)
        : m_engine(engine)
        , q(q)
        , subject(entry)
    {
    }

    void finish()
    {
        m_finished = true;
        Q_EMIT q->finished();
        q->deleteLater();
    }

    int findLinkIdToInstall(KNSCore::Entry &entry)
    {
        const auto downloadLinksInformationList = entry.d.constData()->mDownloadLinkInformationList;
        const auto optionalLinkId = linkIdFromVersions(downloadLinksInformationList);
        if (optionalLinkId.has_value()) {
            qCDebug(KNEWSTUFFCORE) << "Found linkid by version" << optionalLinkId.value();
            payloadToIdentify[entry] = QString{};
            return optionalLinkId.value();
        }

        if (downloadLinksInformationList.size() == 1 || !entry.payload().isEmpty()) {
            // If there is only one downloadable item (which also includes a predefined payload name), then we can fairly safely assume that's
            // what we're wanting to update, meaning we can bypass some of the more expensive operations in downloadLinkLoaded
            qCDebug(KNEWSTUFFCORE) << "Just the one download link, so let's use that";
            payloadToIdentify[entry] = QString{};
            return 1;
        }

        qCDebug(KNEWSTUFFCORE) << "Try and identify a download link to use from a total of" << entry.downloadLinkCount();
        // While this seems silly, the payload gets reset when fetching the new download link information
        payloadToIdentify[entry] = entry.payload();
        // Drop a fresh list in place so we've got something to work with when we get the links
        payloads[entry] = QStringList{};
        return 1;
    }

    // linkid may be -1 to denote the latest link id
    void installLinkId(int linkId)
    {
        if (subject.downloadLinkCount() == 0 && subject.payload().isEmpty()) {
            // Turns out this happens sometimes, so we should deal with that and spit out an error
            qCDebug(KNEWSTUFFCORE) << "There were no downloadlinks defined in the entry we were just asked to update: " << subject.uniqueId() << "on provider"
                                   << subject.providerId();
            Q_EMIT q->signalErrorCode(
                KNSCore::ErrorCode::InstallationError,
                i18n("Could not perform an installation of the entry %1 as it does not have any downloadable items defined. Please contact the "
                     "author so they can fix this.",
                     subject.name()),
                subject.uniqueId());
            finish();
            return;
        }

        KNSCore::Entry entry = subject;
        if (entry.status() == KNSCore::Entry::Updateable) {
            entry.setStatus(KNSCore::Entry::Updating);
        } else {
            entry.setStatus(KNSCore::Entry::Installing);
        }
        Q_EMIT q->signalEntryEvent(entry, Entry::StatusChangedEvent);

        qCDebug(KNEWSTUFFCORE) << "Install " << entry.name() << " from: " << entry.providerId();
        auto provider = m_engine->d->providerCores.value(entry.providerId());
        if (!provider) {
            return;
        }

        QObject::connect(provider->d->base, &ProviderBase::payloadLinkLoaded, q, &Transaction::downloadLinkLoaded);
        // If linkId is -1, assume we don't know what to update
        if (linkId == -1) {
            linkId = findLinkIdToInstall(entry);
        } else {
            qCDebug(KNEWSTUFFCORE) << "Link ID already known" << linkId;
            // If there is no payload to identify, we will assume the payload is already known and just use that
            payloadToIdentify[entry] = QString{};
        }

        provider->d->base->loadPayloadLink(entry, linkId);

        m_finished = false;
        m_engine->updateStatus();
    }

    EngineBase *const m_engine;
    Transaction *const q;
    bool m_finished = false;
    // Used for updating purposes - we ought to be saving this information, but we also have to deal with old stuff, and so... this will have to do for now
    // TODO KF6: Installed state needs to move onto a per-downloadlink basis rather than per-entry
    QMap<Entry, QStringList> payloads;
    QMap<Entry, QString> payloadToIdentify;
    const Entry subject;
};

/**
 * we look for the directory where all the resources got installed.
 * assuming it was extracted into a directory
 */
static QDir sharedDir(QStringList dirs, QString rootPath)
{
    // Ensure that rootPath definitely is a clean path with a slash at the end
    rootPath = QDir::cleanPath(rootPath) + QStringLiteral("/");
    qCInfo(KNEWSTUFFCORE) << Q_FUNC_INFO << dirs << rootPath;
    while (!dirs.isEmpty()) {
        QString thisDir(dirs.takeLast());
        if (thisDir.endsWith(QStringLiteral("*"))) {
            qCInfo(KNEWSTUFFCORE) << "Directory entry" << thisDir
                                  << "ends in a *, indicating this was installed from an archive - see Installation::archiveEntries";
            thisDir.chop(1);
        }

        const QString currentPath = QDir::cleanPath(thisDir);
        qCInfo(KNEWSTUFFCORE) << "Current path is" << currentPath;
        if (!currentPath.startsWith(rootPath)) {
            qCInfo(KNEWSTUFFCORE) << "Current path" << currentPath << "does not start with" << rootPath << "and should be ignored";
            continue;
        }

        const QFileInfo current(currentPath);
        qCInfo(KNEWSTUFFCORE) << "Current file info is" << current;
        if (!current.isDir()) {
            qCInfo(KNEWSTUFFCORE) << "Current path" << currentPath << "is not a directory, and should be ignored";
            continue;
        }

        const QDir dir(currentPath);
        if (dir.path() == (rootPath + dir.dirName())) {
            qCDebug(KNEWSTUFFCORE) << "Found directory" << dir;
            return dir;
        }
    }
    qCWarning(KNEWSTUFFCORE) << "Failed to locate any shared installed directory in" << dirs << "and this is almost certainly very bad.";
    return {};
}

static QString getAdoptionCommand(const QString &command, const KNSCore::Entry &entry, Installation *inst)
{
    auto adoption = command;
    if (adoption.isEmpty()) {
        return {};
    }

    const QLatin1String dirReplace("%d");
    if (adoption.contains(dirReplace)) {
        QString installPath = sharedDir(entry.installedFiles(), inst->targetInstallationPath()).path();
        adoption.replace(dirReplace, KShell::quoteArg(installPath));
    }

    const QLatin1String fileReplace("%f");
    if (adoption.contains(fileReplace)) {
        if (entry.installedFiles().isEmpty()) {
            qCWarning(KNEWSTUFFCORE) << "no installed files to adopt";
            return {};
        } else if (entry.installedFiles().count() != 1) {
            qCWarning(KNEWSTUFFCORE) << "can only adopt one file, will be using the first" << entry.installedFiles().at(0);
        }

        adoption.replace(fileReplace, KShell::quoteArg(entry.installedFiles().at(0)));
    }
    return adoption;
}

Transaction::Transaction(const KNSCore::Entry &entry, EngineBase *engine)
    : QObject(engine)
    , d(new TransactionPrivate(entry, engine, this))
{
    connect(d->m_engine->d->installation, &Installation::signalEntryChanged, this, [this](const KNSCore::Entry &changedEntry) {
        Q_EMIT signalEntryEvent(changedEntry, Entry::StatusChangedEvent);
        d->m_engine->cache()->registerChangedEntry(changedEntry);
    });
    connect(d->m_engine->d->installation, &Installation::signalInstallationFailed, this, [this](const QString &message, const KNSCore::Entry &entry) {
        if (entry == d->subject) {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::InstallationError, message, {});
            d->finish();
        }
    });
}

Transaction::~Transaction() = default;

#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(6, 9)
Transaction *Transaction::install(EngineBase *engine, const KNSCore::Entry &_entry, int _linkId)
{
    return TransactionPrivate::createInstallTransaction(_entry, engine, _linkId);
}
#endif

Transaction *Transaction::installLatest(EngineBase *engine, const KNSCore::Entry &_entry)
{
    return TransactionPrivate::createInstallTransaction(_entry, engine, -1);
}

Transaction *Transaction::installLinkId(EngineBase *engine, const KNSCore::Entry &_entry, quint8 _linkId)
{
    return TransactionPrivate::createInstallTransaction(_entry, engine, _linkId);
}

void Transaction::downloadLinkLoaded(const KNSCore::Entry &entry)
{
    if (entry.status() == KNSCore::Entry::Updating) {
        if (d->payloadToIdentify[entry].isEmpty()) {
            // If there's nothing to identify, and we've arrived here, then we know what the payload is
            qCDebug(KNEWSTUFFCORE) << "If there's nothing to identify, and we've arrived here, then we know what the payload is";
            d->m_engine->d->installation->install(entry);
            connect(d->m_engine->d->installation, &Installation::signalInstallationFinished, this, [this, entry](const KNSCore::Entry &finishedEntry) {
                if (entry.uniqueId() == finishedEntry.uniqueId()) {
                    d->finish();
                }
            });
            d->payloadToIdentify.remove(entry);
        } else if (d->payloads[entry].count() < entry.downloadLinkCount()) {
            // We've got more to get before we can attempt to identify anything, so fetch the next one...
            qCDebug(KNEWSTUFFCORE) << "We've got more to get before we can attempt to identify anything, so fetch the next one...";
            QStringList payloads = d->payloads[entry];
            payloads << entry.payload();
            d->payloads[entry] = payloads;
            const auto &p = d->m_engine->d->providerCores.value(entry.providerId());
            if (p) {
                // ok, so this should definitely always work, but... safety first, kids!
                p->d->base->loadPayloadLink(entry, payloads.count());
            }
        } else {
            // We now have all the links, so let's try and identify the correct one...
            qCDebug(KNEWSTUFFCORE) << "We now have all the links, so let's try and identify the correct one...";
            QString identifiedLink;
            const QString payloadToIdentify = d->payloadToIdentify[entry];
            const QList<Entry::DownloadLinkInformation> downloadLinks = entry.downloadLinkInformationList();
            const QStringList &payloads = d->payloads[entry];

            if (payloads.contains(payloadToIdentify)) {
                // Simplest option, the link hasn't changed at all
                qCDebug(KNEWSTUFFCORE) << "Simplest option, the link hasn't changed at all";
                identifiedLink = payloadToIdentify;
            } else {
                // Next simplest option, filename is the same but in a different folder
                qCDebug(KNEWSTUFFCORE) << "Next simplest option, filename is the same but in a different folder";
                const QString fileName = payloadToIdentify.split(QChar::fromLatin1('/')).last();
                for (const QString &payload : payloads) {
                    if (payload.endsWith(fileName)) {
                        identifiedLink = payload;
                        break;
                    }
                }

                // Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...
                qCDebug(KNEWSTUFFCORE) << "Possibly the payload itself is named differently (by a CDN, for example), but the link identifier is the same...";
                QStringList payloadNames;
                for (const Entry::DownloadLinkInformation &downloadLink : downloadLinks) {
                    qCDebug(KNEWSTUFFCORE) << "Download link" << downloadLink.name << downloadLink.id << downloadLink.size << downloadLink.descriptionLink;
                    payloadNames << downloadLink.name;
                    if (downloadLink.name == fileName) {
                        identifiedLink = payloads[payloadNames.count() - 1];
                        qCDebug(KNEWSTUFFCORE) << "Found a suitable download link for" << fileName << "which should match" << identifiedLink;
                    }
                }

                if (identifiedLink.isEmpty()) {
                    // Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)
                    qCDebug(KNEWSTUFFCORE)
                        << "Least simple option, no match - ask the user to pick (and if we still haven't got one... that's us done, no installation)";
                    auto question = std::make_unique<Question>(Question::SelectFromListQuestion);
                    question->setTitle(i18n("Pick Update Item"));
                    question->setQuestion(
                        i18n("Please pick the item from the list below which should be used to apply this update. We were unable to identify which item to "
                             "select, based on the original item, which was named %1",
                             fileName));
                    question->setList(payloadNames);
                    if (question->ask() == Question::OKResponse) {
                        identifiedLink = payloads.value(payloadNames.indexOf(question->response()));
                    }
                }
            }
            if (!identifiedLink.isEmpty()) {
                KNSCore::Entry theEntry(entry);
                theEntry.setPayload(identifiedLink);
                d->m_engine->d->installation->install(theEntry);
                connect(d->m_engine->d->installation, &Installation::signalInstallationFinished, this, [this, entry](const KNSCore::Entry &finishedEntry) {
                    if (entry.uniqueId() == finishedEntry.uniqueId()) {
                        d->finish();
                    }
                });
            } else {
                qCWarning(KNEWSTUFFCORE) << "We failed to identify a good link for updating" << entry.name() << "and are unable to perform the update";
                KNSCore::Entry theEntry(entry);
                theEntry.setStatus(KNSCore::Entry::Updateable);
                Q_EMIT signalEntryEvent(theEntry, Entry::StatusChangedEvent);
                Q_EMIT signalErrorCode(ErrorCode::InstallationError,
                                       i18n("We failed to identify a good link for updating %1, and are unable to perform the update", entry.name()),
                                       {entry.uniqueId()});
            }
            // As the serverside data may change before next time this is called, even in the same session,
            // let's not make assumptions, and just get rid of this
            d->payloads.remove(entry);
            d->payloadToIdentify.remove(entry);
            d->finish();
        }
    } else {
        d->m_engine->d->installation->install(entry);
        connect(d->m_engine->d->installation, &Installation::signalInstallationFinished, this, [this, entry](const KNSCore::Entry &finishedEntry) {
            if (entry.uniqueId() == finishedEntry.uniqueId()) {
                d->finish();
            }
        });
    }
}

Transaction *Transaction::uninstall(EngineBase *engine, const KNSCore::Entry &_entry)
{
    auto ret = new Transaction(_entry, engine);
    const KNSCore::Entry::List list = ret->d->m_engine->cache()->registryForProvider(_entry.providerId());
    // we have to use the cached entry here, not the entry from the provider
    // since that does not contain the list of installed files
    KNSCore::Entry actualEntryForUninstall;
    for (const KNSCore::Entry &eInt : list) {
        if (eInt.uniqueId() == _entry.uniqueId()) {
            actualEntryForUninstall = eInt;
            break;
        }
    }
    if (!actualEntryForUninstall.isValid()) {
        qCDebug(KNEWSTUFFCORE) << "could not find a cached entry with following id:" << _entry.uniqueId() << " ->  using the non-cached version";
        actualEntryForUninstall = _entry;
    }

    QTimer::singleShot(0, ret, [actualEntryForUninstall, _entry, ret] {
        KNSCore::Entry entry = _entry;
        entry.setStatus(KNSCore::Entry::Installing);

        Entry actualEntryForUninstall2 = actualEntryForUninstall;
        actualEntryForUninstall2.setStatus(KNSCore::Entry::Installing);
        Q_EMIT ret->signalEntryEvent(entry, Entry::StatusChangedEvent);

        // We connect to/forward the relevant signals
        qCDebug(KNEWSTUFFCORE) << "about to uninstall entry " << entry.uniqueId();
        ret->d->m_engine->d->installation->uninstall(actualEntryForUninstall2);

        // FIXME: signalEntryEvent to uninstalled already happened in installation.cpp:584
        // Update the correct entry
        entry.setStatus(actualEntryForUninstall2.status());
        Q_EMIT ret->signalEntryEvent(entry, Entry::StatusChangedEvent);

        ret->d->finish();
    });

    return ret;
}

Transaction *Transaction::adopt(EngineBase *engine, const Entry &entry)
{
    if (!engine->hasAdoptionCommand()) {
        qCWarning(KNEWSTUFFCORE) << "no adoption command specified";
        return nullptr;
    }

    auto ret = new Transaction(entry, engine);
    const QString command = getAdoptionCommand(engine->d->adoptionCommand, entry, engine->d->installation);

    QTimer::singleShot(0, ret, [command, entry, ret] {
        QStringList split = KShell::splitArgs(command);
        QProcess *process = new QProcess(ret);
        process->setProgram(split.takeFirst());
        process->setArguments(split);

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        // The debug output is too talkative to be useful
        env.insert(QStringLiteral("QT_LOGGING_RULES"), QStringLiteral("*.debug=false"));
        process->setProcessEnvironment(env);

        process->start();

        connect(process, &QProcess::finished, ret, [ret, process, entry, command](int exitCode) {
            if (exitCode == 0) {
                Q_EMIT ret->signalEntryEvent(entry, Entry::EntryEvent::AdoptedEvent);

                // Handle error output as warnings if the process hasn't crashed
                const QString stdErr = QString::fromLocal8Bit(process->readAllStandardError());
                if (!stdErr.isEmpty()) {
                    Q_EMIT ret->signalMessage(stdErr);
                }
            } else {
                const QString errorMsg = i18n("Failed to adopt '%1'\n%2", entry.name(), QString::fromLocal8Bit(process->readAllStandardError()));
                Q_EMIT ret->signalErrorCode(KNSCore::ErrorCode::AdoptionError, errorMsg, QVariantList{command});
            }
            ret->d->finish();
        });
    });
    return ret;
}

bool Transaction::isFinished() const
{
    return d->m_finished;
}

#include "moc_transaction.cpp"
