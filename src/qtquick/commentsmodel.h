/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNSQUICK_COMMENTSMODEL_H
#define KNSQUICK_COMMENTSMODEL_H

#include <QSortFilterProxyModel>
#include <QQmlParserStatus>
#include <entryinternal.h>

namespace KNewStuffQuick
{
/**
 * @short Encapsulates a KNSCore::CommentsModel for use in Qt Quick
 *
 * This class takes care of initialisation of a KNSCore::CommentsModel when assigned an engine,
 * providerId and entryId. If the data is not yet cached, it will be requested from the provider,
 * and updated for display
 * @since 5.63
 */
class CommentsModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    /**
     * The KNewStufQuick::ItemsModel to interact with servers through
     */
    Q_PROPERTY(QObject *itemsModel READ itemsModel WRITE setItemsModel NOTIFY itemsModelChanged)
    /**
     * The index in the model of the entry to fetch comments for
     */
    Q_PROPERTY(int entryIndex READ entryIndex WRITE setEntryIndex NOTIFY entryIndexChanged)
    /**
     * Which types of comments should be included
     * @default AllComments
     * @since 5.65
     */
    Q_PROPERTY(KNewStuffQuick::CommentsModel::IncludedComments includedComments READ includedComments WRITE setIncludedComments NOTIFY includedCommentsChanged)
public:
    /**
     * The options which can be set for which comments to include
     * @since 5.65
     */
    enum IncludedComments {
        IncludeAllComments = 0, //< All comments should be included
        IncludeOnlyReviews = 1, //< Only comments which have a rating (and thus is considered a review) should be included
        IncludeReviewsAndReplies = 2, //< Reviews (as OnlyReviews), except child comments are also included
    };
    Q_ENUM(IncludedComments)

    explicit CommentsModel(QObject *parent = nullptr);
    ~CommentsModel() override;
    void classBegin() override;
    void componentComplete() override;

    QObject *itemsModel() const;
    void setItemsModel(QObject *newItemsModel);
    Q_SIGNAL void itemsModelChanged();

    int entryIndex() const;
    void setEntryIndex(int entryIndex);
    Q_SIGNAL void entryIndexChanged();

    /**
     * Which comments should be included
     * @since 5.65
     */
    CommentsModel::IncludedComments includedComments() const;
    /**
     * Set which comments should be included
     * @since 5.65
     */
    void setIncludedComments(CommentsModel::IncludedComments includedComments);
    /**
     * Fired when the value of includedComments changes
     * @since 5.65
     */
    Q_SIGNAL void includedCommentsChanged();

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    class Private;
    Private *d;
};
}
Q_DECLARE_METATYPE(KNewStuffQuick::CommentsModel::IncludedComments)
#endif//KNSQUICK_COMMENTSMODEL_H
