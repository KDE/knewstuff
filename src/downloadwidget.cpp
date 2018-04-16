/*
    knewstuff3/ui/downloaddialog.cpp.
    Copyright (C) 2005 by Enrico Ros <eros.kde@email.it>
    Copyright (C) 2005 - 2007 Josef Spillner <spillner@kde.org>
    Copyright (C) 2007 Dirk Mueller <mueller@kde.org>
    Copyright (C) 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    Copyright (C) 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    Copyright (C) 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "downloadwidget.h"
#include "downloadwidget_p.h"

#include <QTimer>
#include <QScrollBar>
#include <QKeyEvent>
#include <QCoreApplication>

#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <knewstuff_debug.h>

#include "core/itemsmodel.h"

#include "ui/itemsviewdelegate_p.h"
#include "ui/itemsgridviewdelegate_p.h"
#include "ui/widgetquestionlistener.h"

#include "entry_p.h"

using namespace KNS3;

DownloadWidget::DownloadWidget(QWidget *parent)
    : QWidget(parent)
    , d(new DownloadWidgetPrivate(this))
{
    const QString name = QCoreApplication::applicationName();
    init(name + QStringLiteral(".knsrc"));
}

DownloadWidget::DownloadWidget(const QString &configFile, QWidget *parent)
    : QWidget(parent)
    , d(new DownloadWidgetPrivate(this))
{
    init(configFile);
}

void DownloadWidget::init(const QString &configFile)
{
    d->init(configFile);
    WidgetQuestionListener::instance();
}

DownloadWidget::~DownloadWidget()
{
    delete d;
}

void DownloadWidget::setTitle(const QString &title)
{
    d->ui.m_titleWidget->setText(title);
}

QString DownloadWidget::title() const
{
    return d->ui.m_titleWidget->text();
}

KNSCore::Engine *DownloadWidget::engine()
{
    return d->engine;
}

Entry::List DownloadWidget::changedEntries()
{
    Entry::List entries;
    foreach (const KNSCore::EntryInternal &e, d->changedEntries) {
        entries.append(EntryPrivate::fromInternal(&e));
    }
    return entries;
}

Entry::List DownloadWidget::installedEntries()
{
    Entry::List entries;
    foreach (const KNSCore::EntryInternal &e, d->changedEntries) {
        if (e.status() == Entry::Installed) {
            entries.append(EntryPrivate::fromInternal(&e));
        }
    }
    return entries;
}

DownloadWidgetPrivate::DownloadWidgetPrivate(DownloadWidget *q)
    : q(q)
    , engine(new KNSCore::Engine)
    , model(new KNSCore::ItemsModel(engine))
    , messageTimer(nullptr)
    , dialogMode(false)
{
}

DownloadWidgetPrivate::~DownloadWidgetPrivate()
{
    delete messageTimer;
    delete delegate;
    delete model;
    delete engine;
}

void DownloadWidgetPrivate::slotResetMessage() // SLOT
{
    ui.m_titleWidget->setComment(QString());
}

void DownloadWidgetPrivate::slotNetworkTimeout() // SLOT
{
    displayMessage(i18n("Timeout. Check Internet connection."), KTitleWidget::ErrorMessage);
}

void DownloadWidgetPrivate::sortingChanged()
{
    KNSCore::Provider::SortMode sortMode = KNSCore::Provider::Newest;
    KNSCore::Provider::Filter filter = KNSCore::Provider::None;
    if (ui.ratingRadio->isChecked()) {
        sortMode = KNSCore::Provider::Rating;
    } else if (ui.mostDownloadsRadio->isChecked()) {
        sortMode = KNSCore::Provider::Downloads;
    } else if (ui.installedRadio->isChecked()) {
        filter = KNSCore::Provider::Installed;
    }

    model->clearEntries();
    if (filter == KNSCore::Provider::Installed) {
        ui.m_searchEdit->clear();
    }
    ui.m_searchEdit->setEnabled(filter != KNSCore::Provider::Installed);

    engine->setSortMode(sortMode);
    engine->setFilter(filter);
}

void DownloadWidgetPrivate::slotUpdateSearch()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
}

void DownloadWidgetPrivate::slotSearchTextChanged()
{
    if (searchTerm == ui.m_searchEdit->text().trimmed()) {
        return;
    }
    searchTerm = ui.m_searchEdit->text().trimmed();
    engine->setSearchTerm(searchTerm);
}

void DownloadWidgetPrivate::slotCategoryChanged(int idx)
{
    if (idx == 0) {
        // All Categories item selected, reset filter
        engine->setCategoriesFilter(QStringList());

    } else {
        QString category = ui.m_categoryCombo->currentData().toString();

        if (!category.isEmpty()) {
            QStringList filter(category);
            engine->setCategoriesFilter(filter);
        }
    }
}

void DownloadWidgetPrivate::slotInfo(QString provider, QString server, QString version)
{
    QString link = QStringLiteral("<a href=\"%1\">%1</a>").arg(server);
    QString infostring = i18n("Server: %1", link);
    infostring += i18n("<br />Provider: %1", provider);
    infostring += i18n("<br />Version: %1", version);

    KMessageBox::information(nullptr,
                             infostring,
                             i18n("Provider information"));
}

void DownloadWidgetPrivate::slotEntryChanged(const KNSCore::EntryInternal &entry)
{
    changedEntries.insert(entry);
    model->slotEntryChanged(entry);
}

void DownloadWidgetPrivate::slotPayloadFailed(const KNSCore::EntryInternal &entry)
{
    KMessageBox::error(nullptr, i18n("Could not install %1", entry.name()),
                       i18n("Get Hot New Stuff!"));
}

void DownloadWidgetPrivate::slotPayloadLoaded(QUrl url)
{
    Q_UNUSED(url)
}

void DownloadWidgetPrivate::slotError(const QString &message)
{
    KMessageBox::error(nullptr, message, i18n("Get Hot New Stuff"));
}

void DownloadWidgetPrivate::scrollbarValueChanged(int value)
{
    if (static_cast<double>(value) / ui.m_listView->verticalScrollBar()->maximum() > 0.9) {
        engine->requestMoreData();
    }
}

void DownloadWidgetPrivate::init(const QString &configFile)
{
    m_configFile = configFile;
    ui.setupUi(q);
    ui.m_titleWidget->setVisible(false);
    ui.closeButton->setVisible(dialogMode);
    ui.backButton->setVisible(false);
    KStandardGuiItem::assign(ui.backButton, KStandardGuiItem::Back);
    q->connect(ui.backButton, &QPushButton::clicked, this, &DownloadWidgetPrivate::slotShowOverview);

    q->connect(engine, &KNSCore::Engine::signalMessage, this, &DownloadWidgetPrivate::slotShowMessage);

    q->connect(engine, &KNSCore::Engine::signalBusy, ui.progressIndicator, &ProgressIndicator::busy);
    q->connect(engine, &KNSCore::Engine::signalError, ui.progressIndicator, &ProgressIndicator::error);
    q->connect(engine, &KNSCore::Engine::signalIdle, ui.progressIndicator, &ProgressIndicator::idle);

    q->connect(engine, &KNSCore::Engine::signalProvidersLoaded, this, &DownloadWidgetPrivate::slotProvidersLoaded);
    // Entries have been fetched and should be shown:
    q->connect(engine, &KNSCore::Engine::signalEntriesLoaded, this, &DownloadWidgetPrivate::slotEntriesLoaded);

    // An entry has changes - eg because it was installed
    q->connect(engine, &KNSCore::Engine::signalEntryChanged, this, &DownloadWidgetPrivate::slotEntryChanged);

    q->connect(engine, &KNSCore::Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);
    q->connect(engine, &KNSCore::Engine::signalEntryPreviewLoaded,
               model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

    engine->init(configFile);

    delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
    ui.m_listView->setItemDelegate(delegate);
    ui.m_listView->setModel(model);

    ui.iconViewButton->setIcon(QIcon::fromTheme(QStringLiteral("view-list-icons")));
    ui.iconViewButton->setToolTip(i18n("Icons view mode"));
    ui.listViewButton->setIcon(QIcon::fromTheme(QStringLiteral("view-list-details")));
    ui.listViewButton->setToolTip(i18n("Details view mode"));

    q->connect(ui.listViewButton, &QPushButton::clicked, this, &DownloadWidgetPrivate::slotListViewListMode);
    q->connect(ui.iconViewButton, &QPushButton::clicked, this, &DownloadWidgetPrivate::slotListViewIconMode);

    q->connect(ui.newestRadio,        &QRadioButton::clicked, this, &DownloadWidgetPrivate::sortingChanged);
    q->connect(ui.ratingRadio,        &QRadioButton::clicked, this, &DownloadWidgetPrivate::sortingChanged);
    q->connect(ui.mostDownloadsRadio, &QRadioButton::clicked, this, &DownloadWidgetPrivate::sortingChanged);
    q->connect(ui.installedRadio,     &QRadioButton::clicked, this, &DownloadWidgetPrivate::sortingChanged);

    q->connect(ui.m_searchEdit, &KLineEdit::textChanged,     this, &DownloadWidgetPrivate::slotSearchTextChanged);
    q->connect(ui.m_searchEdit, &KLineEdit::editingFinished, this, &DownloadWidgetPrivate::slotUpdateSearch);

    ui.m_providerLabel->setVisible(false);
    ui.m_providerCombo->setVisible(false);
    ui.m_providerCombo->addItem(i18n("All Providers"));

    QStringList categories = engine->categories();
    if (categories.size() < 2) {
        ui.m_categoryLabel->setVisible(false);
        ui.m_categoryCombo->setVisible(false);
    } else {
        ui.m_categoryCombo->addItem(i18n("All Categories"));
        //NOTE: categories will be populated when we will get metadata from the server
    }

    connect(engine, &KNSCore::Engine::signalCategoriesMetadataLoded,
             this, [this](const QList<KNSCore::Provider::CategoryMetadata> &categories) {
                for (const auto &data : categories) {
                    if (!data.displayName.isEmpty()) {
                        ui.m_categoryCombo->addItem(data.displayName, data.name);
                    } else {
                        ui.m_categoryCombo->addItem(data.name, data.name);
                    }
                }
            });
    ui.detailsStack->widget(0)->layout()->setMargin(0);
    ui.detailsStack->widget(1)->layout()->setMargin(0);

    q->connect(ui.m_categoryCombo, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated),
               this, &DownloadWidgetPrivate::slotCategoryChanged);

    // let the search line edit trap the enter key, otherwise it closes the dialog
    ui.m_searchEdit->setTrapReturnKey(true);

    q->connect(ui.m_listView->verticalScrollBar(), &QScrollBar::valueChanged, this, &DownloadWidgetPrivate::scrollbarValueChanged);
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));

    details = new EntryDetails(engine, &ui);
    q->connect(delegate, &KNS3::ItemsViewBaseDelegate::signalShowDetails, this, &DownloadWidgetPrivate::slotShowDetails);

    slotShowOverview();
}

void DownloadWidgetPrivate::slotListViewListMode()
{
    ui.listViewButton->setChecked(true);
    ui.iconViewButton->setChecked(false);
    setListViewMode(QListView::ListMode);
}

void DownloadWidgetPrivate::slotListViewIconMode()
{
    ui.listViewButton->setChecked(false);
    ui.iconViewButton->setChecked(true);
    setListViewMode(QListView::IconMode);
}

void DownloadWidgetPrivate::setListViewMode(QListView::ViewMode mode)
{
    if (ui.m_listView->viewMode() == mode) {
        return;
    }

    ItemsViewBaseDelegate *oldDelegate = delegate;
    if (mode == QListView::ListMode) {
        delegate = new ItemsViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::ListMode);
        ui.m_listView->setResizeMode(QListView::Fixed);
    } else {
        delegate = new ItemsGridViewDelegate(ui.m_listView, engine, q);
        ui.m_listView->setViewMode(QListView::IconMode);
        ui.m_listView->setResizeMode(QListView::Adjust);
    }
    ui.m_listView->setItemDelegate(delegate);
    delete oldDelegate;
    q->connect(ui.m_listView, SIGNAL(doubleClicked(QModelIndex)), delegate, SLOT(slotDetailsClicked(QModelIndex)));
    q->connect(delegate, &KNS3::ItemsViewBaseDelegate::signalShowDetails, this, &DownloadWidgetPrivate::slotShowDetails);
}

void DownloadWidgetPrivate::slotProvidersLoaded()
{
    qCDebug(KNEWSTUFF) << "providers loaded";
    engine->reloadEntries();
}

void DownloadWidgetPrivate::slotEntriesLoaded(const KNSCore::EntryInternal::List &entries)
{
    for (const KNSCore::EntryInternal &entry : entries) {
        if (!categories.contains(entry.category())) {
            qCDebug(KNEWSTUFF) << "Found category: " << entry.category();
            categories.insert(entry.category());
        }
    }
    model->slotEntriesLoaded(entries);
}

void DownloadWidgetPrivate::slotShowMessage(const QString& msg)
{
    displayMessage(msg, KTitleWidget::InfoMessage);
}

void DownloadWidgetPrivate::displayMessage(const QString &msg, KTitleWidget::MessageType type, int timeOutMs)
{
    if (!messageTimer) {
        messageTimer = new QTimer;
        messageTimer->setSingleShot(true);
        q->connect(messageTimer, &QTimer::timeout, this, &DownloadWidgetPrivate::slotResetMessage);
    }
    // stop the pending timer if present
    messageTimer->stop();

    // set text to messageLabel
    ui.m_titleWidget->setComment(msg, type);

    // single shot the resetColors timer (and create it if null)
    if (timeOutMs > 0) {
        qCDebug(KNEWSTUFF) << "starting the message timer for " << timeOutMs;
        messageTimer->start(timeOutMs);
    }
}

void DownloadWidgetPrivate::slotShowDetails(const KNSCore::EntryInternal &entry)
{
    if (!entry.isValid()) {
        qCDebug(KNEWSTUFF) << "invalid entry";
        return;
    }
    titleText = ui.m_titleWidget->text();

    ui.backButton->setVisible(true);
    ui.detailsStack->setCurrentIndex(1);
    ui.descriptionScrollArea->verticalScrollBar()->setValue(0);
    ui.preview1->setImage(QImage());
    ui.preview2->setImage(QImage());
    ui.preview3->setImage(QImage());
    ui.previewBig->setImage(QImage());
    details->setEntry(entry);
}

void DownloadWidgetPrivate::slotShowOverview()
{
    ui.backButton->setVisible(false);

    ui.updateButton->setVisible(false);
    ui.installButton->setVisible(false);
    ui.becomeFanButton->setVisible(false);
    ui.uninstallButton->setVisible(false);

    ui.detailsStack->setCurrentIndex(0);
    ui.m_titleWidget->setText(titleText);
}

#include "moc_downloadwidget.cpp"
