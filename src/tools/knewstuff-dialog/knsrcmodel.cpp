/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knsrcmodel.h"

#include "engine.h"

#include <KConfig>
#include <KConfigGroup>
#include <QDir>

class KNSRCModel::Private {
public:
    struct Entry {
        QString name;
        QString filePath;
    };
    Private(KNSRCModel* qq)
        : q(qq)
    {}
    KNSRCModel* q;
    QUrl folder;
    QList<Entry*> entries;

    void refreshEntries() {
        q->beginResetModel();
        qDeleteAll(entries);
        entries.clear();
        QDir configDir(folder.toLocalFile());
        for(const QFileInfo& file : configDir.entryInfoList(QDir::Files)) {
            KConfig conf(file.absoluteFilePath());
            KConfigGroup group;
            if (conf.hasGroup("KNewStuff3")) {
                group = conf.group("KNewStuff3");
            } else if (conf.hasGroup("KNewStuff2")) {
                group = conf.group("KNewStuff2");
            } else {
                qWarning() << file.absoluteFilePath() << " doesn't contain a KNewStuff3 (or KNewStuff2) section.";
                continue;
            }

            Entry* entry = new Entry;
            entry->name = group.readEntry("Name", file.baseName());
            entry->filePath = file.absoluteFilePath();
            entries << entry;
        }
        q->endResetModel();
    }
};

KNSRCModel::KNSRCModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

KNSRCModel::~KNSRCModel()
{
    delete d;
}

QHash<int, QByteArray> KNSRCModel::roleNames() const
{
    static const QHash<int, QByteArray> roleNames{
        {NameRole, "name"},
        {FilePathRole, "filePath"}
    };
    return roleNames;
}

int KNSRCModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        return 0;
    }
    return d->entries.count();
}

QVariant KNSRCModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    if(checkIndex(index)) {
        Private::Entry* entry = d->entries[index.row()];
        switch(role) {
            case NameRole:
                result.setValue(entry->name);
                break;
            case FilePathRole:
                result.setValue(entry->filePath);
                break;
            default:
                break;
        }
    }
    return result;
}

QUrl KNSRCModel::folder() const
{
    return d->folder;
}

void KNSRCModel::setFolder(const QUrl& folder)
{
    if(d->folder != folder) {
        d->folder = folder;
        d->refreshEntries();
        Q_EMIT folderChanged();
    }
}
