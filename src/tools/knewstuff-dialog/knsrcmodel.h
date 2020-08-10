/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNSRCMODEL_H
#define KNSRCMODEL_H

#include <QAbstractListModel>
#include <QUrl>

class KNSRCModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged)
public:
    explicit KNSRCModel(QObject *parent = nullptr);
    virtual ~KNSRCModel();

    enum Roles {
        NameRole = Qt::UserRole + 1,
        FilePathRole
    };

    QHash< int, QByteArray > roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QUrl folder() const;
    void setFolder(const QUrl& folder);
    Q_SIGNAL void folderChanged();

private:
    class Private;
    Private *d;
};

#endif//KNSRCMODEL_H
