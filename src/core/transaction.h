/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_TRANSACTION_H
#define KNEWSTUFF3_TRANSACTION_H

#include <QObject>
#include <memory>

#include "entry.h"
#include "errorcode.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class EngineBase;
class TransactionPrivate;

/*!
 * \class KNSCore::Transaction
 * \inmodule KNewStuffCore
 *
 * \brief KNewStuff Transaction.
 *
 * Exposes different actions that can be done on an entry and means to track them to completion.
 *
 * To create a Transaction we should call one of the static methods that
 * represent the different actions we can take. These will return the Transaction
 * and we can use it to track mesages, the entries' states and eventually its
 * completion using the \c finished signal.
 *
 * The Transaction will delete itself once it has finished.
 *
 * \since 6.0
 */
class KNEWSTUFFCORE_EXPORT Transaction : public QObject
{
    Q_OBJECT
public:
    ~Transaction() override;

#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(6, 9)
    /*!
     * Performs an install on the given \a entry from the \a engine.
     *
     * \a linkId specifies which of the assets we want to see installed.
     *
     * Returns a Transaction object that we can use to track the progress to completion
     * \deprecated[6.9]
     * Use installLatest or installLinkId instead
     */
    KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "use installLatest or installLinkId instead")
    static Transaction *install(EngineBase *engine, const Entry &entry, int linkId = 1);
#endif

    /*!
     * Performs an install on the given \a entry from the \a engine.
     *
     * \a linkId specifies which of the assets we want to see installed.
     *
     * Returns a Transaction object that we can use to track the progress to completion
     * \since 6.9
     */
    [[nodiscard]] static Transaction *installLinkId(EngineBase *engine, const Entry &entry, quint8 linkId);

    /*!
     * Performs an install of the latest version on the given \a entry from the \a engine.
     *
     * The latest version is determined using heuristics. If you want tight control over which offering gets installed
     * you need to use installLinkId and manually figure out the id.
     *
     * Returns a Transaction object that we can use to track the progress to completion
     * \since 6.9
     */
    [[nodiscard]] static Transaction *installLatest(EngineBase *engine, const Entry &entry);

    /*!
     * Uninstalls the given \a entry from the \a engine.
     *
     * It reverses the step done when install() was called.
     * Returns a Transaction object that we can use to track the progress to completion
     */
    static Transaction *uninstall(EngineBase *engine, const Entry &entry);

    /*!
     * Adopt the \a entry from \a engine using the adoption command.
     *
     * For more information, see the documentation about AdoptionCommand from
     * the knsrc file.
     */
    static Transaction *adopt(EngineBase *engine, const Entry &entry);

    /*!
     * Returns true as soon as the Transaction is completed as it gets ready to
     * clean itself up
     */
    bool isFinished() const;

Q_SIGNALS:
    void finished();

    /*!
     * Provides the \a message to update our users about how the Transaction progressed
     */
    void signalMessage(const QString &message);

    /*!
     * Informs about how the \a entry has changed
     *
     * \a event nature of the change
     *
     */
    void signalEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event);

    /*!
     * Fires in the case of any critical or serious errors, such as network or API problems.
     *
     * \a errorCode Represents the specific type of error which has occurred
     *
     * \a message A human-readable message which can be shown to the end user
     *
     * \a metadata Any additional data which might be helpful to further work
     * out the details of the error (see KNSCore::Entry::ErrorCode for the
     * metadata details)
     *
     * \sa KNSCore::ErrorCode
     */
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

private:
    friend class TransactionPrivate;

    Transaction(const KNSCore::Entry &entry, EngineBase *engine);
    void downloadLinkLoaded(const KNSCore::Entry &entry);

    std::unique_ptr<TransactionPrivate> d;
};

}

#endif
