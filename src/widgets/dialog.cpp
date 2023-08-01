/*
    SPDX-FileCopyrightText: 2020-2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "dialog.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlIncubationController>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include <KLocalizedContext>

#include "core/enginebase.h"
#include "knewstuffwidgets_debug.h"

using namespace KNSWidgets;

class KNSWidgets::DialogPrivate
{
public:
    KNSCore::EngineBase *engine = nullptr;
    QQuickItem *item = nullptr;
    QList<KNSCore::Entry> changedEntries;
};

class PeriodicIncubationController : public QObject, public QQmlIncubationController
{
public:
    explicit PeriodicIncubationController()
        : QObject()
    {
        startTimer(16);
    }

protected:
    void timerEvent(QTimerEvent *) override
    {
        incubateFor(5);
    }
};

Dialog::Dialog(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new DialogPrivate())
{
    auto engine = new QQmlEngine(this);
    auto context = new KLocalizedContext(engine);
    engine->setIncubationController(new PeriodicIncubationController());

    resize(600, 400);
    context->setTranslationDomain(QStringLiteral("knewstuff6"));
    engine->rootContext()->setContextObject(context);
    engine->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), configFile);

    auto page = new QQuickWidget(engine, this);
    page->setSource(QUrl(QStringLiteral("qrc:/knswidgets/page.qml")));
    page->setResizeMode(QQuickWidget::SizeRootObjectToView);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(page);
    layout->setContentsMargins(0, 0, 0, 0);

    if (QQuickItem *root = page->rootObject()) {
        d->item = root;
        d->engine = qvariant_cast<KNSCore::EngineBase *>(root->property("engine"));
        Q_ASSERT(d->engine);

        /*connect(d->coreEngine, &KNSCore::Engine::signalEntryEvent, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            if (event == KNSCore::Entry::StatusChangedEvent) {
                if (entry.status() == KNSCore::Entry::Installing || entry.status() == KNSCore::Entry::Updating) {
                    return; // We do not care about intermediate states
                }
                // To make sure we have no duplicates and always the latest entry
                d->changedEntries.removeOne(entry);
                d->changedEntries.append(entry);
            }
        });*/
    } else {
        qWarning(KNEWSTUFFWIDGETS) << "Error creating QtQuickDialogWrapper component:" << page->errors();
    }
}

Dialog::~Dialog()
{
    delete d->item;
}

KNSCore::EngineBase *Dialog::engine()
{
    return d->engine;
}

QList<KNSCore::Entry> Dialog::changedEntries() const
{
    return d->changedEntries;
}

void Dialog::open()
{
    QDialog::open();
    d->changedEntries.clear();
}

#include "moc_dialog.cpp"
