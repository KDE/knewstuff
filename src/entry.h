/*
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_KNEWSTUFFENTRY_H
#define KNEWSTUFF3_KNEWSTUFFENTRY_H

#include <QLoggingCategory>
#include <QSharedDataPointer>
#include <QStringList>

#include "knewstuff_export.h"
namespace KNSCore
{
class EntryInternal;
}
namespace KNS3
{
class EntryPrivate;
/**
 * @short KNewStuff information about changed entries
 *
 * This class provides information about the entries that
 * have been installed while the new stuff dialog was shown.
 * It is a minimal version that only gives applications what they need
 * to know.
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT Entry
{
public:
    typedef QList<Entry> List;

    /**
     * Status of the entry. An entry will be downloadable from the provider's
     * site prior to the download. Once downloaded and installed, it will
     * be either installed or updateable, implying an out-of-date
     * installation. Finally, the entry can be deleted and hence show up as
     * downloadable again.
     * Entries not taking part in this cycle, for example those in upload,
     * have an invalid status.
     */
    enum Status {
        Invalid,
        Downloadable,
        Installed,
        Updateable,
        Deleted,
        Installing,
        Updating,
    };

    ~Entry();
    Entry(const Entry &other);
    Entry &operator=(const Entry &other);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name
     */
    QString name() const;

    /**
     * Retrieve the category of the data object.
     *
     * @return object category
     */
    QString category() const;

    /**
     * Retrieve the locally installed files.
     * @return file names
     */
    QStringList installedFiles() const;

    /**
     * Retrieve the locally uninstalled files.
     * @return file names
     */
    QStringList uninstalledFiles() const;

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    Status status() const;

    /**
     * Retrieve the license name of the object.
     *
     * @return object license
     */
    QString license() const;

    /**
     * Retrieve a short description about the object.
     *
     * @return object description
     */
    QString summary() const;

    /**
     * Retrieve the version string of the object.
     *
     * @return object version
     *
     * @sa installedVersion()
     */
    QString version() const;

    /**
     * Id of this Entry. It is guaranteed to be unique for one provider.
     * Id and ProviderId together identify this entry.
     * @return the id
     * @since 4.5
     */
    QString id() const;

    /**
     * The Provider which is the source of the Entry.
     * @return the Id of the Provider
     * @since 4.5
     */
    QString providerId() const;

    /**
     * @returns if available an url identifying the asset
     * @since 5.23
     */
    QUrl url() const;

    /**
     * @returns a list of urls to small previews to be displayed as thumbnails
     * @since 5.23
     */
    QList<QUrl> previewThumbnails() const;

    /**
     * @returns a list of full previews of the asset
     * @since 5.23
     */
    QList<QUrl> previewImages() const;

    /**
     * @returns the advertised disk size of the asset
     * @since 5.23
     */
    quint64 size() const;

    /**
     * @returns the number of comments in the asset
     * @since 5.23
     */
    uint numberOfComments() const;

    /**
     * @returns the rating of the asset, between 0 and 100
     * @since 5.23
     */
    uint rating() const;

    /**
     * @returns the asset's change log
     * @since 5.23
     */
    QString changelog() const;

    /**
     * @returns a short one-line summary of the asset
     * @since 5.23
     */
    QString shortSummary() const;

    /**
     * @returns the available version
     *
     * If the entry is not updateable, it will be the same as version.
     *
     * @sa version()
     *
     * @since 5.23
     */
    QString updateVersion() const;

private:
    Entry();

    QExplicitlySharedDataPointer<EntryPrivate> d;

    friend class KNSCore::EntryInternal;
    friend class EntryPrivate;
};

}

#endif
