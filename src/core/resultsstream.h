/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef RESULTSSTREAM_H
#define RESULTSSTREAM_H

#include <QObject>

#include "enginebase.h"
#include "provider.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class SearchRequest;
class ResultsStreamPrivate;
/*!
 * \class KNSCore::ResultsStream
 * \inmodule KNewStuffCore
 *
 * \brief The ResultsStream is returned by EngineBase::search. It is used to communicate
 * the different entries in response to a request using the signal \c entriesFound.
 *
 * Initially the stream will communicate the entries part of the page as specified
 * in the request. Further pages can be requested using \c fetchMore.
 *
 * Once we have reached the end of the requested stream, the object shall emit
 * \c finished and delete itself.
 *
 * \since 6.0
 */
class KNEWSTUFFCORE_EXPORT ResultsStream : public QObject
{
    Q_OBJECT
public:
    ~ResultsStream() override;

    /// Issues the search, make sure all signals are connected before calling
    void fetch();

    /// Increments the requested page and issues another search
    void fetchMore();

Q_SIGNALS:
    void entriesFound(const KNSCore::Entry::List &entries);
    void finished();

private:
    friend class EngineBase;
#if KNEWSTUFFCORE_ENABLE_DEPRECATED_SINCE(6, 9)
    /// @deprecated since 6.9 Use SearchRequest constructor
    KNEWSTUFFCORE_DEPRECATED_VERSION(6, 9, "Use SearchRequest constructor")
    ResultsStream(const Provider::SearchRequest &request, EngineBase *base);
#endif
    /**
     * @param request The search request to be issued
     * @param base The engine issuing the request
     * @since 6.9
     */
    ResultsStream(const SearchRequest &request, EngineBase *base);
    void finish();

    std::unique_ptr<ResultsStreamPrivate> d;
};

}

#endif // RESULTSSTREAM_H
