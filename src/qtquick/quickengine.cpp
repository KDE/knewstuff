/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickengine.h"

#include <KAuthorized>
#include <KLocalizedString>

#include "categoriesmodel.h"
#include "entrywrapper.h"
#include "quickquestionlistener.h"

#include "engine.h"

class Engine::Private
{
public:
    Private()
        : engine(nullptr)
        , categoriesModel(nullptr)
    {}
    KNSCore::Engine *engine;
    bool isLoading{false};
    CategoriesModel *categoriesModel;
    QString configFile;

    KNSCore::EntryInternal::List changedEntries;
    static KNSCore::EntryWrapper *getChangedEntry(QQmlListProperty<KNSCore::EntryWrapper>* property, int i)
    {
        KNSCore::EntryWrapper *entry{nullptr};
        if (property) {
            Private* d = static_cast<Engine::Private*>(property->data);
            if (d) {
                if (i >= 0 && i < d->changedEntries.count()) {
                    // Lifetime management for these objects should be done by the consumer,
                    // but are also parented for auto-delete on application shutdown
                    entry = new KNSCore::EntryWrapper(d->changedEntries[i], property->object);
                }
            }
        }
        return entry;
    }
    static int getChangedEntriesCount(QQmlListProperty<KNSCore::EntryWrapper>* property)
    {
        int count{0};
        if (property) {
            Private* d = static_cast<Engine::Private*>(property->data);
            if (d) {
                count = d->changedEntries.count();
            }
        }
        return count;
    }
};

Engine::Engine(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Engine::~Engine()
{
    delete d;
}

bool Engine::allowedByKiosk() const
{
    return KAuthorized::authorize(QStringLiteral("ghns"));
}

QString Engine::configFile() const
{
    return d->configFile;
}

void Engine::setConfigFile(const QString &newFile)
{
    if (d->configFile != newFile) {
        d->isLoading = true;
        emit isLoadingChanged();
        d->configFile = newFile;
        emit configFileChanged();

        if (allowedByKiosk()) {
            if (!d->engine) {
                d->engine = new KNSCore::Engine(this);
                connect(d->engine, &KNSCore::Engine::signalProvidersLoaded, this, [=](){
                    d->isLoading = false;
                    emit isLoadingChanged();
                });
                connect(d->engine, &KNSCore::Engine::signalMessage, this, &Engine::message);
                connect(d->engine, &KNSCore::Engine::signalIdle, this, &Engine::idleMessage);
                connect(d->engine, &KNSCore::Engine::signalBusy, this, &Engine::busyMessage);
                connect(d->engine, &KNSCore::Engine::signalError, this, &Engine::errorMessage);
                connect(d->engine, &KNSCore::Engine::signalErrorCode, this, [=](const KNSCore::ErrorCode &errorCode, const QString &message, const QVariant &/*metadata*/) {
                    if (errorCode == KNSCore::ProviderError) {
                        // This means loading the providers file failed entirely and we cannot complete the
                        // initialisation. It also means the engine is done loading, but that nothing will
                        // work, and we need to inform the user of this.
                        d->isLoading = false;
                        emit isLoadingChanged();
                    }
                    emit errorMessage(message);
                });
                connect(d->engine, &KNSCore::Engine::signalEntryChanged, this, [this](const KNSCore::EntryInternal &entry){
                    if (d->changedEntries.contains(entry) ) {
                        d->changedEntries.removeAll(entry);
                    }
                    d->changedEntries << entry;
                    emit changedEntriesChanged();
                });
                emit engineChanged();
                KNewStuffQuick::QuickQuestionListener::instance();
                d->categoriesModel = new CategoriesModel(this);
                emit categoriesChanged();
                // And finally, let's just make sure we don't miss out the various things here getting changed
                // In other words, when we're asked to reset the view, actually do that
                connect(d->engine, &KNSCore::Engine::signalResetView, this, &Engine::categoriesFilterChanged);
                connect(d->engine, &KNSCore::Engine::signalResetView, this, &Engine::filterChanged);
                connect(d->engine, &KNSCore::Engine::signalResetView, this, &Engine::sortOrderChanged);
                connect(d->engine, &KNSCore::Engine::signalResetView, this, &Engine::searchTermChanged);
                emit categoriesFilterChanged();
                emit filterChanged();
                emit sortOrderChanged();
                emit searchTermChanged();
            }
            d->engine->init(d->configFile);
            emit engineInitialized();
        } else {
            // This is not an error message in the proper sense, and the message is not intended to look like an error (as there is really
            // nothing the user can do to fix it, and we just tell them so they're not wondering what's wrong)
            emit message(i18nc("An informational message which is shown to inform the user they are not authorized to use GetHotNewStuff functionality", "You are not authorized to Get Hot New Stuff. If you think this is in error, please contact the person in charge of your permissions."));
        }
    }
}

QObject *Engine::engine() const
{
    return d->engine;
}

bool Engine::isLoading() const
{
    return d->isLoading;
}

bool Engine::hasAdoptionCommand() const
{
    if (d->engine) {
        return d->engine->hasAdoptionCommand();
    }
    return false;
}

QString Engine::name() const
{
    if (d->engine) {
        return d->engine->name();
    }
    return QString{};
}

QObject *Engine::categories() const
{
    return d->categoriesModel;
}

QStringList Engine::categoriesFilter() const
{
    if (d->engine) {
        return d->engine->categoriesFilter();
    }
    return QStringList{};
}

void Engine::setCategoriesFilter(const QStringList &newCategoriesFilter)
{
    if (d->engine) {
        // This ensures that if we somehow end up with any empty entries (such as the default
        // option in the categories dropdowns), our list will remain empty.
        QStringList filter{newCategoriesFilter};
        filter.removeAll({});
        if (d->engine->categoriesFilter() != filter) {
            d->engine->setCategoriesFilter(filter);
            emit categoriesFilterChanged();
        }
    }
}

void Engine::resetCategoriesFilter()
{
    if (d->engine) {
        d->engine->setCategoriesFilter(d->engine->categories());
    }
}

int Engine::filter() const
{
    if (d->engine) {
        d->engine->filter();
    }
    return 0;
}

void Engine::setFilter(int newFilter)
{
    if (d->engine && d->engine->filter() != newFilter) {
        d->engine->setFilter(static_cast<KNSCore::Provider::Filter>(newFilter));
        emit filterChanged();
    }
}

int Engine::sortOrder() const
{
    if (d->engine) {
        return d->engine->sortMode();
    }
    return 0;
}

void Engine::setSortOrder(int newSortOrder)
{
    if (d->engine && d->engine->sortMode() != newSortOrder) {
        d->engine->setSortMode(static_cast<KNSCore::Provider::SortMode>(newSortOrder));
        emit sortOrderChanged();
    }
}

QString Engine::searchTerm() const
{
    if (d->engine) {
        return d->engine->searchTerm();
    }
    return QString{};
}

void Engine::setSearchTerm(const QString &newSearchTerm)
{
    if (d->engine && d->engine->searchTerm() != newSearchTerm) {
        d->engine->setSearchTerm(newSearchTerm);
        emit searchTermChanged();
    }
}

void Engine::resetSearchTerm()
{
    setSearchTerm(QString{});
}

QQmlListProperty<KNSCore::EntryWrapper> Engine::changedEntries()
{
    return QQmlListProperty<KNSCore::EntryWrapper>(this, d, &Private::getChangedEntriesCount, &Private::getChangedEntry);
}

int Engine::changedEntriesCount() const
{
    return d->changedEntries.count();
}

void Engine::resetChangedEntries()
{
    d->changedEntries.clear();
    emit changedEntriesChanged();
}
