// SPDX-License-Identifier: LGPL-2.1-or-later
// SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
// SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
// SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
// SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

#include "knewstuffcore_export.h"

class Engine;

namespace KNSCore
{

class ProviderBase;

/**
 * @short KNewStuff Base Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * static website providers.
 *
 * @author Jeremy Whiting <jpwhiting@kde.org>
 */
class KNEWSTUFFCORE_EXPORT ProviderCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl website READ website NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl host READ host NOTIFY basicsLoaded)
    Q_PROPERTY(QString contactEmail READ contactEmail NOTIFY basicsLoaded)
    Q_PROPERTY(bool supportsSsl READ supportsSsl NOTIFY basicsLoaded)
public:
    ~ProviderCore() override;
    Q_DISABLE_COPY_MOVE(ProviderCore)

    [[nodiscard]] QString version() const;
    [[nodiscard]] QUrl website() const;
    [[nodiscard]] QUrl host() const;
    /**
     * The general contact email for this provider
     * @return The general contact email for this provider
     */
    [[nodiscard]] QString contactEmail() const;
    /**
     * Whether or not the provider supports SSL connections
     * @return True if the server supports SSL connections, false if not
     */
    [[nodiscard]] bool supportsSsl() const;

Q_SIGNALS:
    void basicsLoaded();

private:
    friend class EngineBase;
    friend class EngineBasePrivate;
    friend class ResultsStream;
    friend class ProviderBubbleWrap;
    friend class Transaction;
    friend class TransactionPrivate;
    friend class ::Engine; // quick engine
    ProviderCore(ProviderBase *base, QObject *parent = nullptr);
    const std::unique_ptr<class ProviderCorePrivate> d;
};

} // namespace KNSCore
