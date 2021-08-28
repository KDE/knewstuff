/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commentsmodel.h"

#include "entryinternal.h"
#include "knewstuffcore_debug.h"

#include <KLocalizedString>

#include <QTimer>

namespace KNSCore
{
class CommentsModel::Private
{
public:
    Private(CommentsModel *qq)
        : q(qq)
    {
    }
    CommentsModel *const q;
    Engine *engine = nullptr;

    EntryInternal entry;

    QList<std::shared_ptr<KNSCore::Comment>> comments;

    enum FetchOptions {
        NoOption,
        ClearModel,
    };
    bool fetchThrottle = false;
    void fetch(FetchOptions option = NoOption)
    {
        if (fetchThrottle) {
            return;
        }
        fetchThrottle = true;
        QTimer::singleShot(1, q, [this]() {
            fetchThrottle = false;
        });
        // Sanity checks, because we need a few things to be correct before we can actually fetch comments...
        if (!engine) {
            qCWarning(KNEWSTUFFCORE) << "CommentsModel must be parented on a KNSCore::Engine instance to be able to fetch comments";
        }
        if (!entry.isValid()) {
            qCWarning(KNEWSTUFFCORE) << "Without an entry to fetch comments for, CommentsModel cannot fetch comments for it";
        }

        if (engine && entry.isValid()) {
            QSharedPointer<Provider> provider = engine->provider(entry.providerId());
            if (option == ClearModel) {
                q->beginResetModel();
                comments.clear();
                provider->disconnect(q);
                q->connect(provider.data(), &Provider::commentsLoaded, q, [=](const QList<std::shared_ptr<KNSCore::Comment>> &newComments) {
                    QList<std::shared_ptr<KNSCore::Comment>> actualNewComments;
                    for (const std::shared_ptr<KNSCore::Comment> &comment : newComments) {
                        bool commentIsKnown = false;
                        for (const std::shared_ptr<KNSCore::Comment> &existingComment : std::as_const(comments)) {
                            if (existingComment->id == comment->id) {
                                commentIsKnown = true;
                                break;
                            }
                        }
                        if (commentIsKnown) {
                            continue;
                        }
                        actualNewComments << comment;
                    }
                    if (actualNewComments.count() > 0) {
                        q->beginInsertRows(QModelIndex(), comments.count(), comments.count() + actualNewComments.count() - 1);
                        qCDebug(KNEWSTUFFCORE) << "Appending" << actualNewComments.count() << "new comments";
                        comments.append(actualNewComments);
                        q->endInsertRows();
                    }
                });
                q->endResetModel();
            }
            int commentsPerPage = 100;
            int pageToLoad = comments.count() / commentsPerPage;
            qCDebug(KNEWSTUFFCORE) << "Loading comments, page" << pageToLoad << "with current comment count" << comments.count() << "out of a total of"
                                   << entry.numberOfComments();
            Q_EMIT provider->loadComments(entry, commentsPerPage, pageToLoad);
        }
    }
};
}

KNSCore::CommentsModel::CommentsModel(Engine *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->engine = parent;
}

KNSCore::CommentsModel::~CommentsModel()
{
    delete d;
}

QHash<int, QByteArray> KNSCore::CommentsModel::roleNames() const
{
    // clang-format off
    static const QHash<int, QByteArray> roles{
        {IdRole, "id"},
        {SubjectRole, "subject"},
        {TextRole, "text"},
        {ChildCountRole, "childCound"},
        {UsernameRole, "username"},
        {DateRole, "date"},
        {ScoreRole, "score"},
        {ParentIndexRole, "parentIndex"},
        {DepthRole, "depth"}
    };
    // clang-format on
    return roles;
}

QVariant KNSCore::CommentsModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (checkIndex(index)) {
        std::shared_ptr<KNSCore::Comment> comment = d->comments[index.row()];
        switch (role) {
        case IdRole:
            value.setValue(comment->id);
            break;
        case SubjectRole:
            value.setValue(comment->subject);
            break;
        case TextRole:
            value.setValue(comment->text);
            break;
        case ChildCountRole:
            value.setValue(comment->childCount);
            break;
        case UsernameRole:
            value.setValue(comment->username);
            break;
        case DateRole:
            value.setValue(comment->date);
            break;
        case ScoreRole:
            value.setValue(comment->score);
            break;
        case ParentIndexRole: {
            int idx{-1};
            if (comment->parent) {
                idx = d->comments.indexOf(comment->parent);
            }
            value.setValue(idx);
        } break;
        case DepthRole: {
            int depth{0};
            if (comment->parent) {
                std::shared_ptr<KNSCore::Comment> child = comment->parent;
                while (child) {
                    ++depth;
                    child = child->parent;
                }
            }
            value.setValue(depth);
            break;
        }
        default:
            value.setValue(i18nc("The value returned for an unknown role when requesting data from the model.", "Unknown CommentsModel role"));
            break;
        }
    }
    return value;
}

int KNSCore::CommentsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->comments.count();
}

bool KNSCore::CommentsModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }
    if (d->entry.numberOfComments() > d->comments.count()) {
        return true;
    }
    return false;
}

void KNSCore::CommentsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }
    d->fetch();
}

const KNSCore::EntryInternal &KNSCore::CommentsModel::entry() const
{
    return d->entry;
}

void KNSCore::CommentsModel::setEntry(const KNSCore::EntryInternal &newEntry)
{
    d->entry = newEntry;
    d->fetch(Private::ClearModel);
    Q_EMIT entryChanged();
}
