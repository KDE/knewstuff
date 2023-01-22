/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QQmlListProperty>

#include "entrywrapper.h"
#include "errorcode.h"
#include "knewstuffquick_export.h"

class EnginePrivate;

/**
 * @short Encapsulates a KNSCore::Engine for use in Qt Quick
 *
 * This class takes care of initialisation of a KNSCore::Engine when assigned a config file.
 * The actual KNSCore:Engine can be read through the Engine::engine property.
 *
 * @see ItemsModel
 */
class Engine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configFile READ configFile WRITE setConfigFile NOTIFY configFileChanged)
    Q_PROPERTY(QObject *engine READ engine NOTIFY engineChanged)
    /**
     * Whether or not the engine is performing its initial loading operations
     * @since 5.65
     */
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasAdoptionCommand READ hasAdoptionCommand NOTIFY engineInitialized)
    Q_PROPERTY(QString name READ name NOTIFY engineInitialized)
    Q_PROPERTY(QObject *categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList categoriesFilter READ categoriesFilter WRITE setCategoriesFilter RESET resetCategoriesFilter NOTIFY categoriesFilterChanged)
    Q_PROPERTY(int filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(int sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm RESET resetSearchTerm NOTIFY searchTermChanged)
    Q_PROPERTY(QObject *searchPresetModel READ searchPresetModel NOTIFY searchPresetModelChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY engineInitialized)
public:
    explicit Engine(QObject *parent = nullptr);
    ~Engine() override;

    enum EntryEvent {
        UnknownEvent = KNSCore::EntryInternal::UnknownEvent,
        StatusChangedEvent = KNSCore::EntryInternal::StatusChangedEvent,
        AdoptedEvent = KNSCore::EntryInternal::AdoptedEvent,
        DetailsLoadedEvent = KNSCore::EntryInternal::DetailsLoadedEvent,
    };
    Q_ENUM(EntryEvent)

    /**
     * Registering the error codes from KNSCore to allow them to be used easily in QtQuick
     * @see KNSCore::ErrorCode
     * @since 5.84
     */
    enum ErrorCode {
        UnknownError = KNSCore::ErrorCode::UnknownError,
        NetworkError = KNSCore::ErrorCode::NetworkError,
        OcsError = KNSCore::ErrorCode::OcsError,
        ConfigFileError = KNSCore::ErrorCode::ConfigFileError,
        ProviderError = KNSCore::ErrorCode::ProviderError,
        InstallationError = KNSCore::ErrorCode::InstallationError,
        ImageError = KNSCore::ErrorCode::ImageError,
        AdoptionError = KNSCore::ErrorCode::AdoptionError,
        TryAgainLaterError = KNSCore::ErrorCode::TryAgainLaterError,
    };
    Q_ENUM(ErrorCode)

    QString configFile() const;
    void setConfigFile(const QString &newFile);
    Q_SIGNAL void configFileChanged();

    QObject *engine() const;
    Q_SIGNAL void engineChanged();

    /**
     * Whether or not the engine is performing its initial loading operations
     * @since 5.65
     */
    bool isLoading() const;
    /**
     * Fired when the isLoading value changes
     * @since 5.65
     */
    Q_SIGNAL void isLoadingChanged();

    bool hasAdoptionCommand() const;
    QString name() const;
    Q_SIGNAL void engineInitialized();

    QObject *categories() const;
    Q_SIGNAL void categoriesChanged();

    QStringList categoriesFilter() const;
    void setCategoriesFilter(const QStringList &newCategoriesFilter);
    Q_INVOKABLE void resetCategoriesFilter();
    Q_SIGNAL void categoriesFilterChanged();

    int filter() const;
    void setFilter(int newFilter);
    Q_SIGNAL void filterChanged();

    int sortOrder() const;
    void setSortOrder(int newSortOrder);
    Q_SIGNAL void sortOrderChanged();

    QString searchTerm() const;
    void setSearchTerm(const QString &newSearchTerm);
    Q_INVOKABLE void resetSearchTerm();
    Q_SIGNAL void searchTermChanged();

    QObject *searchPresetModel() const;
    Q_SIGNAL void searchPresetModelChanged();

    bool isValid();
Q_SIGNALS:
    void message(const QString &message);
    void idleMessage(const QString &message);
    void busyMessage(const QString &message);
    void errorMessage(const QString &message);

    /**
     * This is fired for events related directly to a single EntryInternal instance
     * The intermediate states Updating and Installing are not forwarded. In case you
     * need those you have to listen to the signals of the KNSCore::Engine instance of the engine property.
     *
     * As an example, if you need to know when the status of an entry changes, you might write:
     \code
        function onEntryEvent(entry, event) {
            if (event == NewStuff.Engine.StatusChangedEvent) {
                myModel.ghnsEntryChanged(entry);
            }
        }
     \endcode
     *
     * nb: The above example is also how one would port a handler for the old changedEntries signal
     *
     * @see EntryInternal::EntryEvent for details on which specific event is being notified
     * @since 5.81
     */
    void entryEvent(KNSCore::EntryWrapper *entry, EntryEvent event);

    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * This forwards the signal from KNSCore::Engine::signalErrorCode, but with QML friendly
     * enumerations.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::EntryInternal::ErrorCode for the
     * metadata details)
     * @see KNSCore::Engine::signalErrorCode
     * @since 5.84
     */
    void errorCode(const Engine::ErrorCode &errorCode, const QString &message, const QVariant &metadata);

private:
    const std::unique_ptr<EnginePrivate> d;
};

#endif // ENGINE_H
