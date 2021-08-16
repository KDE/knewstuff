/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "qtquickdialogwrapper.h"

#include <QEventLoop>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QTimer>

#include <KLocalizedContext>

#include "core/engine.h"

using namespace KNS3;

class KNS3::QtQuickDialogWrapperPrivate
{
public:
    QQmlEngine *engine = nullptr;
    QObject *item = nullptr;
    KNSCore::Engine *coreEngine = nullptr;
    QList<KNSCore::EntryInternal> changedEntries;
};

QtQuickDialogWrapper::QtQuickDialogWrapper(const QString &configFile, QObject *parent)
    : QObject(parent)
    , d(new QtQuickDialogWrapperPrivate())
{
    d->engine = new QQmlEngine(this);
    auto *context = new KLocalizedContext(d->engine);
    context->setTranslationDomain(QStringLiteral("knewstuff5"));
    d->engine->rootContext()->setContextObject(context);
    QQmlComponent component(d->engine);
    d->engine->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), configFile);
    component.setData(QByteArrayLiteral("import QtQuick 2.7\n"
                                        "import org.kde.newstuff 1.62 as NewStuff\n"
                                        "\n"
                                        "NewStuff.Dialog {\n"
                                        "    id: component\n"
                                        "    signal closed()\n"
                                        "    configFile: knsrcfile\n"
                                        "    onVisibleChanged: if (!visible) {closed()}\n"
                                        "}"),
                      QUrl());
    d->item = component.create();
    // If there is an error on the QML side of things we get a nullptr
    if (d->item) {
        QObject *qtquickEngine = d->item->property("engine").value<QObject *>();
        Q_ASSERT(qtquickEngine);
        d->coreEngine = qtquickEngine->property("engine").value<KNSCore::Engine *>();
        Q_ASSERT(d->coreEngine);

        connect(d->coreEngine, &KNSCore::Engine::signalEntryEvent, this, [this](const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::EntryEvent event) {
            if (event == KNSCore::EntryInternal::StatusChangedEvent) {
                if (entry.status() == KNS3::Entry::Installing || entry.status() == KNS3::Entry::Updating) {
                    return; // We do not care about intermediate states
                }
                // To make sure we have no duplicates and always the latest entry
                d->changedEntries.removeOne(entry);
                d->changedEntries.append(entry);
            }
        });

        // Forward relevant signals
        connect(d->item, SIGNAL(closed()), this, SIGNAL(closed()));
    }
}

QtQuickDialogWrapper::~QtQuickDialogWrapper()
{
    // Empty destructor needed for std::unique_ptr to incomplete class.
}

void QtQuickDialogWrapper::open()
{
    if (d->item) {
        d->changedEntries.clear();
        QMetaObject::invokeMethod(d->item, "open");
    }
}

KNSCore::Engine *QtQuickDialogWrapper::engine()
{
    return d->coreEngine;
}

QList<KNSCore::EntryInternal> QtQuickDialogWrapper::exec()
{
    open();
    QEventLoop loop;
    connect(this, &QtQuickDialogWrapper::closed, &loop, &QEventLoop::quit);
    loop.exec();
    return d->changedEntries;
}
