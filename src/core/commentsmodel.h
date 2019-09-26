/*
 * Copyright (C) 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef KNSCORE_COMMENTSMODEL_H
#define KNSCORE_COMMENTSMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

#include "engine.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class EntryInternal;

struct Comment {
    QString id;
    QString subject;
    QString text;
    int childCount = 0;
    QString username;
    QDateTime date;
    int score = 0;
    std::shared_ptr<KNSCore::Comment> parent;
};

/**
 * @brief A model which takes care of the comments for a single EntryInternal
 *
 * This model should preferably be constructed by asking the Engine to give a model
 * instance to you for a specific entry using the commentsForEntry function. If you
 * insist, you can construct an instance yourself as well, but this is not recommended.
 *
 * @see Engine::commentsForEntry(KNSCore::EntryInternal)
 * @since 5.63
 */
class KNEWSTUFFCORE_EXPORT CommentsModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The Entry for which this model should handle comments
     */
    Q_PROPERTY(KNSCore::EntryInternal entry READ entry WRITE setEntry NOTIFY entryChanged)
public:
    /**
     * Construct a new CommentsModel instance.
     * @note The class is intended to be constructed using the Engine::commentsForEntry function
     * @see Engine::commentsForEntry(KNSCore::EntryInternal)
     */
    explicit CommentsModel(Engine *parent = nullptr);
    ~CommentsModel() override;

    enum Roles {
        SubjectRole = Qt::DisplayRole,
        IdRole = Qt::UserRole + 1,
        TextRole,
        ChildCountRole,
        UsernameRole,
        DateRole,
        ScoreRole,
        ParentIndexRole,
        DepthRole
    };
    Q_ENUM(Roles)

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    const KNSCore::EntryInternal &entry() const;
    void setEntry(const KNSCore::EntryInternal &newEntry);
    Q_SIGNAL void entryChanged();

private:
    class Private;
    Private *d;
};
}

#endif//KNSCORE_COMMENTSMODEL_H
