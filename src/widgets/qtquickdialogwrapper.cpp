/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "qtquickdialogwrapper.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlIncubationController>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include <KLocalizedContext>

#include "core/engine.h"
#include "knewstuffwidgets_debug.h"

using namespace KNSWidgets;

class KNSWidgets::QtQuickDialogWrapperPrivate
{
public:
    QObject *item = nullptr;
    KNSCore::Engine *coreEngine = nullptr;
    QList<KNSCore::Entry> changedEntries;
};

class PeriodicIncubationController : public QObject, public QQmlIncubationController
{
public:
    explicit PeriodicIncubationController(QObject *parent)
        : QObject(parent)
    {
        startTimer(16);
    }

protected:
    void timerEvent(QTimerEvent *) override
    {
        incubateFor(5);
    }
};

QtQuickDialogWrapper::QtQuickDialogWrapper(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new QtQuickDialogWrapperPrivate())
{
    auto engine = new QQmlEngine(this);
    auto context = new KLocalizedContext(engine);
    engine->setIncubationController(new PeriodicIncubationController(nullptr));

    context->setTranslationDomain(QStringLiteral("knewstuff6"));
    engine->rootContext()->setContextObject(context);
    engine->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), configFile);

    auto page = new QQuickWidget(engine, this);
    page->setSource(QUrl(QStringLiteral("qrc:/knswidgets/page.qml")));
    page->setResizeMode(QQuickWidget::SizeRootObjectToView);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(page);

    if (QQuickItem *root = page->rootObject()) {
        QObject *qtquickEngine = root->property("engine").value<QObject *>();
        Q_ASSERT(qtquickEngine);
        d->coreEngine = qtquickEngine->property("engine").value<KNSCore::Engine *>();
        Q_ASSERT(d->coreEngine);

        connect(d->coreEngine, &KNSCore::Engine::signalEntryEvent, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            if (event == KNSCore::Entry::StatusChangedEvent) {
                if (entry.status() == KNSCore::Entry::Installing || entry.status() == KNSCore::Entry::Updating) {
                    return; // We do not care about intermediate states
                }
                // To make sure we have no duplicates and always the latest entry
                d->changedEntries.removeOne(entry);
                d->changedEntries.append(entry);
            }
        });
    } else {
        qWarning(KNEWSTUFFWIDGETS) << "Error creating QtQuickDialogWrapper component:" << page->errors();
    }
}

QtQuickDialogWrapper::~QtQuickDialogWrapper() = default;

KNSCore::Engine *QtQuickDialogWrapper::engine()
{
    return d->coreEngine;
}

QList<KNSCore::Entry> QtQuickDialogWrapper::changedEntries() const
{
    return d->changedEntries;
}

void QtQuickDialogWrapper::open()
{
    QDialog::open();
    d->changedEntries.clear();
}
