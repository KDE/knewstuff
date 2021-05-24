/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SEARCHPRESETMODEL_H
#define SEARCHPRESETMODEL_H

#include <QAbstractListModel>
#include "provider.h"
#include "quickengine.h"

/**
 * @brief The SearchPresetModel class
 *
 * this class handles search presets.
 */
class SearchPresetModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SearchPresetModel(Engine *parent = nullptr);
    ~SearchPresetModel();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void loadSearch(const QModelIndex &index);
private:
    class Private;
    // TODO KF6: Switch all the pimpls to const std::unique_ptr<Private> d;
    Private *d;
};

#endif // SEARCHPRESETMODEL_H
