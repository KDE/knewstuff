/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QQmlListProperty>

#include "enginebase.h"
#include "entry.h"
#include "errorcode.h"
#include "provider.h"
#include "transaction.h"

class EnginePrivate;

/**
 * @short Encapsulates a KNSCore::Engine for use in Qt Quick
 *
 * This class takes care of initialisation of a KNSCore::Engine when assigned a config file.
 * The actual KNSCore:Engine can be read through the Engine::engine property.
 *
 * @see ItemsModel
 */
class Engine : public KNSCore::EngineBase
{
    Q_OBJECT
    Q_PROPERTY(QString configFile READ configFile WRITE setConfigFile NOTIFY configFileChanged)
    /**
     * Whether or not the engine is performing its initial loading operations
     * @since 5.65
     */
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasAdoptionCommand READ hasAdoptionCommand NOTIFY engineInitialized)
    Q_PROPERTY(QString name READ name NOTIFY engineInitialized)
    Q_PROPERTY(QObject *categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList categoriesFilter READ categoriesFilter WRITE setCategoriesFilter RESET resetCategoriesFilter NOTIFY categoriesFilterChanged)
    Q_PROPERTY(KNSCore::Provider::Filter filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(KNSCore::Provider::SortMode sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm RESET resetSearchTerm NOTIFY searchTermChanged)
    Q_PROPERTY(QObject *searchPresetModel READ searchPresetModel NOTIFY searchPresetModelChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY engineInitialized)

    /**
     * Current state of the engine, the state con contain multiple operations
     * an empty BusyState represents the idle status
     * @since 5.74
     */
    Q_PROPERTY(BusyState busyState READ busyState WRITE setBusyState NOTIFY busyStateChanged)
public:
    explicit Engine(QObject *parent = nullptr);
    ~Engine() override;
    bool init(const QString &configfile) override;

    enum class BusyOperation {
        Initializing,
        LoadingData,
        LoadingPreview,
        InstallingEntry,
    };
    Q_DECLARE_FLAGS(BusyState, BusyOperation)

    enum EntryEvent {
        UnknownEvent = KNSCore::Entry::UnknownEvent,
        StatusChangedEvent = KNSCore::Entry::StatusChangedEvent,
        AdoptedEvent = KNSCore::Entry::AdoptedEvent,
        DetailsLoadedEvent = KNSCore::Entry::DetailsLoadedEvent,
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

    Engine::BusyState busyState() const;
    void setBusyState(Engine::BusyState state);
    void setBusy(BusyState state, const QString &busyMessage);

    /**
     * Signal gets emitted when the busy state changes
     * @since 5.74
     */
    Q_SIGNAL void busyStateChanged();

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

    Q_SIGNAL void engineInitialized();

    QObject *categories() const;
    Q_SIGNAL void categoriesChanged();

    QStringList categoriesFilter() const;
    void setCategoriesFilter(const QStringList &newCategoriesFilter);
    Q_INVOKABLE void resetCategoriesFilter()
    {
        setCategoriesFilter(categoriesFilter());
    }
    Q_SIGNAL void categoriesFilterChanged();

    KNSCore::Provider::Filter filter() const;
    void setFilter(KNSCore::Provider::Filter filter);
    Q_SIGNAL void filterChanged();

    KNSCore::Provider::SortMode sortOrder() const;
    void setSortOrder(KNSCore::Provider::SortMode newSortOrder);
    Q_SIGNAL void sortOrderChanged();

    QString searchTerm() const;
    void setSearchTerm(const QString &newSearchTerm);
    Q_INVOKABLE void resetSearchTerm()
    {
        setSearchTerm(QString());
    }
    Q_SIGNAL void searchTermChanged();

    QObject *searchPresetModel() const;
    Q_SIGNAL void searchPresetModelChanged();

    bool isValid();
    void reloadEntries();

    void loadPreview(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type);

    void addProvider(QSharedPointer<KNSCore::Provider> provider) override;

    /**
     * Adopt an entry using the adoption command. This will also take care of displaying error messages
     * @param entry Entry that should be adopted
     * @see signalErrorCode
     * @see signalEntryEvent
     * @since 5.77
     */
    Q_INVOKABLE void adoptEntry(const KNSCore::Entry &entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     *
     * @param entry Entry to be installed
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    void install(const KNSCore::Entry &entry, int linkId = 1);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    void uninstall(const KNSCore::Entry &entry);

    void requestMoreData();

    Q_INVOKABLE void revalidateCacheEntries();
    Q_INVOKABLE void restoreSearch();
    Q_INVOKABLE void storeSearch();
Q_SIGNALS:
    void message(const QString &message);
    void idleMessage(const QString &message);
    void busyMessage(const QString &message);

    void signalResetView();

    /**
     * This is fired for events related directly to a single Entry instance
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
     * @see Entry::EntryEvent for details on which specific event is being notified
     */
    void entryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event);

    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * This forwards the signal from KNSCore::Engine::signalErrorCode, but with QML friendly
     * enumerations.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::Entry::ErrorCode for the
     * metadata details)
     * @see KNSCore::Engine::signalErrorCode
     * @since 5.84
     */
    void errorCode(Engine::ErrorCode errorCode, const QString &message, const QVariant &metadata);

    void entryPreviewLoaded(const KNSCore::Entry &, KNSCore::Entry::PreviewType);

    void signalEntriesLoaded(const KNSCore::Entry::List &entries); ///@internal
    void signalUpdateableEntriesLoaded(const KNSCore::Entry::List &entries); ///@internal
private:
    Q_SIGNAL void signalEntryPreviewLoaded(const KNSCore::Entry &, KNSCore::Entry::PreviewType);
    Q_SIGNAL void signalEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event);
    void registerTransaction(KNSCore::Transaction *transactions);
    void doRequest();
    const std::unique_ptr<EnginePrivate> d;
};

#endif // ENGINE_H
