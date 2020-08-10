/*
    knewstuff3/ui/downloaddialog.cpp.
    SPDX-FileCopyrightText: 2005 Enrico Ros <eros.kde@email.it>
    SPDX-FileCopyrightText: 2005-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
    SPDX-FileCopyrightText: 2007-2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_UI_DOWNLOADWIDGET_P_H
#define KNEWSTUFF3_UI_DOWNLOADWIDGET_P_H

#include <QTimer>
#include <QListView>

#include "core/itemsmodel.h"

#include "ui/itemsviewbasedelegate_p.h"
#include "ui/entrydetailsdialog_p.h"

#include "ui_downloadwidget.h"

namespace KNS3
{
class DownloadWidget;

class DownloadWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    DownloadWidget *q;
    EntryDetails *details;

    // The engine that does all the work
    KNSCore::Engine *engine;
    Ui::DownloadWidget ui;
    // Model to show the entries
    KNSCore::ItemsModel *model;
    // Timeout for message display
    QTimer *messageTimer;

    ItemsViewBaseDelegate *delegate;

    QString searchTerm;
    QSet<KNSCore::EntryInternal> changedEntries;

    QSet<QString> categories;
    QSet<QString> providers;

    QString titleText;
    QString m_configFile;
    bool dialogMode;

    explicit DownloadWidgetPrivate(DownloadWidget *q);
    ~DownloadWidgetPrivate();

    void init(const QString &configFile);
    void slotShowMessage(const QString& msg);
    void displayMessage(const QString &msg, KTitleWidget::MessageType type, int timeOutMs = 0);

    void slotProvidersLoaded();
    void slotEntriesLoaded(const KNSCore::EntryInternal::List &entries);
    void slotEntryChanged(const KNSCore::EntryInternal &entry);

    void slotShowDetails(const KNSCore::EntryInternal &entry);
    void slotShowOverview();

    void slotPayloadFailed(const KNSCore::EntryInternal &entry);
    void slotPayloadLoaded(QUrl url);

    void slotResetMessage();
    void slotNetworkTimeout();
    void sortingChanged();
    void slotSearchTextChanged();
    void slotUpdateSearch();
    void slotCategoryChanged(int);

    void slotInfo(QString provider, QString server, QString version);
    void slotError(const QString &message);
    void scrollbarValueChanged(int value);

    void slotUpload();
    void slotListViewListMode();
    void slotListViewIconMode();
    void setListViewMode(QListView::ViewMode mode);
};

}

#endif
