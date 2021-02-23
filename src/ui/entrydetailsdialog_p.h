/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENTRYDETAILSDIALOG_P_H
#define KNEWSTUFF3_ENTRYDETAILSDIALOG_P_H

#include "core/entryinternal.h"
#include <QObject>

#include "ui_downloadwidget.h"

class QListWidgetItem;

namespace KNSCore
{
class Engine;
}

namespace KNS3
{
class EntryDetails : public QObject
{
    Q_OBJECT

public:
    EntryDetails(KNSCore::Engine *engine, Ui::DownloadWidget *widget);
    ~EntryDetails();

public Q_SLOTS:
    void setEntry(const KNSCore::EntryInternal &entry);

private Q_SLOTS:
    void slotEntryPreviewLoaded(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::PreviewType type);
    void install();
    void installAction(QAction *action);
    void uninstall();

    void ratingChanged(uint rating);
    void becomeFan();
    // more details loaded
    void entryChanged(const KNSCore::EntryInternal &entry);
    // installed/updateable etc
    void updateButtons();

    void preview1Selected();
    void preview2Selected();
    void preview3Selected();

private:
    void init();
    void previewSelected(int current);

    KNSCore::Engine *const m_engine;
    Ui::DownloadWidget *const ui;
    KNSCore::EntryInternal m_entry;
    QImage m_currentPreview;
    QListWidgetItem *m_previewItem1 = nullptr;
    QListWidgetItem *m_previewItem2 = nullptr;
    QListWidgetItem *m_previewItem3 = nullptr;
};

}

#endif
