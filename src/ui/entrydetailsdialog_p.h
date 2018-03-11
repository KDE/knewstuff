/*
    Copyright (C) 2009 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_ENTRYDETAILSDIALOG_P_H
#define KNEWSTUFF3_ENTRYDETAILSDIALOG_P_H

#include <QObject>
#include "core/entryinternal.h"

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
    void uninstall();

    void ratingChanged(uint rating);
    void becomeFan();
    // more details loaded
    void entryChanged(const KNSCore::EntryInternal &entry);
    // installed/updateable etc
    void entryStatusChanged(const KNSCore::EntryInternal &entry);
    void updateButtons();

    void preview1Selected();
    void preview2Selected();
    void preview3Selected();

private:
    void init();
    void previewSelected(int current);

    KNSCore::Engine *m_engine;
    Ui::DownloadWidget *ui;
    KNSCore::EntryInternal m_entry;
    QImage m_currentPreview;
    QListWidgetItem *m_previewItem1;
    QListWidgetItem *m_previewItem2;
    QListWidgetItem *m_previewItem3;
};

}

#endif
