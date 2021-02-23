/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
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
        DepthRole,
    };
    Q_ENUM(Roles)

    QHash<int, QByteArray> roleNames() const override;
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

#endif // KNSCORE_COMMENTSMODEL_H
