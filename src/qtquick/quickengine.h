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
#if KNEWSTUFFQUICK_BUILD_DEPRECATED_SINCE(5, 81)
    KNEWSTUFFQUICK_DEPRECATED_VERSION(5, 81, "Use NewStuff.Settings.allowedByKiosk instead")
    Q_PROPERTY(bool allowedByKiosk READ allowedByKiosk CONSTANT)
#endif
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
#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 82)
    Q_PROPERTY(QQmlListProperty<KNSCore::EntryWrapper> changedEntries READ changedEntries NOTIFY changedEntriesChanged)
    Q_PROPERTY(int changedEntriesCount READ changedEntriesCount NOTIFY changedEntriesChanged)
#endif
    Q_PROPERTY(bool isValid READ isValid NOTIFY engineInitialized)
public:
    explicit Engine(QObject *parent = nullptr);
    virtual ~Engine();

#if KNEWSTUFFQUICK_BUILD_DEPRECATED_SINCE(5, 81)
    KNEWSTUFFQUICK_DEPRECATED_VERSION(5, 81, "Use KNewStuffQuick::Settings::allowedByKiosk() instead")
    bool allowedByKiosk() const;
#endif

    enum EntryEvent {
        UnknownEvent = KNSCore::EntryInternal::UnknownEvent,
        StatusChangedEvent = KNSCore::EntryInternal::StatusChangedEvent,
        AdoptedEvent = KNSCore::EntryInternal::AdoptedEvent,
        DetailsLoadedEvent = KNSCore::EntryInternal::DetailsLoadedEvent,
    };
    Q_ENUM(EntryEvent);

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

#if KNEWSTUFF_BUILD_DEPRECATED_SINCE(5, 82)
    QQmlListProperty<KNSCore::EntryWrapper> changedEntries();
    Q_INVOKABLE void resetChangedEntries();
    Q_SIGNAL void changedEntriesChanged();
    int changedEntriesCount() const;
#endif

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
     * @see EntryInternal::EntryEvent for details on which specific event is being notified
     * @since 5.81
     */
    void entryEvent(KNSCore::EntryWrapper *entry, EntryEvent event);

private:
    class Private;
    Private *d;
};

#endif // ENGINE_H
