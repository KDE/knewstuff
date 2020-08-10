/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNSQUICK_AUTHOR_H
#define KNSQUICK_AUTHOR_H

#include <QObject>
#include <QQmlParserStatus>
#include <entryinternal.h>

namespace KNewStuffQuick
{
/**
 * @short Encapsulates a KNSCore::Author for use in Qt Quick
 *
 * This class takes care of initialisation of a KNSCore::Author when assigned an engine, provider ID and username.
 * If the data is not yet cached, it will be requested from the provider, and updated for display
 * @since 5.63
 */
class Author : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The NewStuffQuickEngine to interact with servers through
     */
    Q_PROPERTY(QObject *engine READ engine WRITE setEngine NOTIFY engineChanged)
    /**
     * The ID of the provider which the user is registered on
     */
    Q_PROPERTY(QString providerId READ providerId WRITE setProviderId NOTIFY providerIdChanged)
    /**
     * The user ID for the user this object represents
     */
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)

    Q_PROPERTY(QString name READ name NOTIFY dataChanged)
    Q_PROPERTY(QString description READ description NOTIFY dataChanged)
    Q_PROPERTY(QString homepage READ homepage NOTIFY dataChanged)
    Q_PROPERTY(QString profilepage READ profilepage NOTIFY dataChanged)
    Q_PROPERTY(QUrl avatarUrl READ avatarUrl NOTIFY dataChanged)
public:
    explicit Author(QObject *parent = nullptr);
    virtual ~Author();
    void classBegin() override;
    void componentComplete() override;

    QObject *engine() const;
    void setEngine(QObject *newEngine);
    Q_SIGNAL void engineChanged();

    QString providerId() const;
    void setProviderId(const QString &providerId);
    Q_SIGNAL void providerIdChanged();

    QString username() const;
    void setUsername(const QString &username);
    Q_SIGNAL void usernameChanged();

    QString name() const;
    QString description() const;
    QString homepage() const;
    QString profilepage() const;
    QUrl avatarUrl() const;
    Q_SIGNAL void dataChanged();

private:
    class Private;
    Private *d;
};
}

#endif//KNSQUICK_AUTHOR_H
