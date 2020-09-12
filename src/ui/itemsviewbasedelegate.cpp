/*
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "itemsviewbasedelegate_p.h"

#include "core/itemsmodel.h"

#include "entrydetailsdialog_p.h"

#include <QAction>

#include <knewstuff_debug.h>
#include <qstandardpaths.h>

namespace KNS3
{
ItemsViewBaseDelegate::ItemsViewBaseDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent)
    : KWidgetItemDelegate(itemView, parent)
    , m_engine(engine)
    , m_itemView(itemView)
    , m_iconInvalid(QIcon::fromTheme(QStringLiteral("dialog-error")))
    , m_iconInstall(QIcon::fromTheme(QStringLiteral("dialog-ok")))
    , m_iconUpdate(QIcon::fromTheme(QStringLiteral("system-software-update")))
    , m_iconDelete(QIcon::fromTheme(QStringLiteral("edit-delete")))
{
    const QString framefile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/knewstuff/pics/thumb_frame.png"));
    m_frameImage = QPixmap(framefile);
}

ItemsViewBaseDelegate::~ItemsViewBaseDelegate()
{
}

bool ItemsViewBaseDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        slotDetailsClicked();
        return true;
    }

    return KWidgetItemDelegate::eventFilter(watched, event);
}

void ItemsViewBaseDelegate::slotLinkClicked(const QString &url)
{
    Q_UNUSED(url)
    QModelIndex index = focusedIndex();
    Q_ASSERT(index.isValid());

    KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
    m_engine->contactAuthor(entry);
}

void ItemsViewBaseDelegate::slotInstallClicked()
{
    QModelIndex index = focusedIndex();
    if (index.isValid()) {
        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        if (!entry.isValid()) {
            qCDebug(KNEWSTUFF) << "Invalid entry: " << entry.name();
            return;
        }

        if (entry.status() == Entry::Installed) {
            m_engine->uninstall(entry);
        } else {
            m_engine->install(entry);
        }
    }
}

void ItemsViewBaseDelegate::slotInstallActionTriggered(QAction *action)
{
    if (action->data().isNull())
        return;

    QPoint rowDownload = action->data().toPoint();
    int row = rowDownload.x();
    QModelIndex index = m_itemView->model()->index(row, 0);
    if (index.isValid()) {
        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        m_engine->install(entry, rowDownload.y());
    }
}

void ItemsViewBaseDelegate::slotDetailsClicked()
{
    QModelIndex index = focusedIndex();
    slotDetailsClicked(index);
}

void ItemsViewBaseDelegate::slotDetailsClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        if (!entry.isValid()) {
            return;
        }
        qCDebug(KNEWSTUFF) << "Details: " << entry.name();
        Q_EMIT signalShowDetails(entry);
    }
}
}
