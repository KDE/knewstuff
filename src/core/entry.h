/*
    knewstuff3/entry.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENTRY
#define KNEWSTUFF3_ENTRY

#include <QDate>
#include <QImage>
#include <QString>
#include <QUrl>

#include "author.h"
#include "knewstuffcore_export.h"

class testEntry;
class QDomElement;
class QXmlStreamReader;

namespace KNSCore
{
static const int PreviewWidth = 96;
static const int PreviewHeight = 72;
class EntryPrivate;

/*!
 function to remove bb code formatting that opendesktop sends
 */
KNEWSTUFFCORE_EXPORT QString replaceBBCode(const QString &unformattedText);

/*!
 * \class KNSCore::Entry
 * \inmodule KNewStuffCore
 *
 * \brief KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 *
 * \br Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 */
class KNEWSTUFFCORE_EXPORT Entry
{
    Q_GADGET
public:
    typedef QList<Entry> List;

    /*!
     * \qmlproperty string Entry::providerId
     */
    /*!
     * \property KNSCore::Entry::providerId
     */
    Q_PROPERTY(QString providerId READ providerId)

    /*!
     * \qmlproperty string Entry::uniqueId
     */
    /*!
     * \property KNSCore::Entry::uniqueId
     */
    Q_PROPERTY(QString uniqueId READ uniqueId)

    /*!
     * \qmlproperty string Entry::status
     */
    /*!
     * \property KNSCore::Entry::status
     */
    Q_PROPERTY(KNSCore::Entry::Status status READ status)

    /*!
     * \qmlproperty EntryType Entry::entryType
     */
    /*!
     * \property KNSCore::Entry::entryType
     */
    Q_PROPERTY(KNSCore::Entry::EntryType entryType READ entryType)

    /*!
     * \qmlproperty string Entry::name
     */
    /*!
     * \property KNSCore::Entry::name
     */
    Q_PROPERTY(QString name READ name)

    /*!
     * \qmlproperty Author Entry::author
     */
    /*!
     * \property KNSCore::Entry::author
     */
    Q_PROPERTY(KNSCore::Author author READ author)

    /*!
     * \qmlproperty string Entry::shortSummary
     */
    /*!
     * \property KNSCore::Entry::shortSummary
     */
    Q_PROPERTY(QString shortSummary READ shortSummary)

    /*!
     * \qmlproperty string Entry::summary
     */
    /*!
     * \property KNSCore::Entry::summary
     */
    Q_PROPERTY(QString summary READ summary)
    // TODO  Q_PROPERTY(QString previews READ previews)

    /*!
     * \qmlproperty url Entry::homepage
     */
    /*!
     * \property KNSCore::Entry::homepage
     */
    Q_PROPERTY(QUrl homepage READ homepage)

    /*!
     * \qmlproperty string Entry::donationLink
     */
    /*!
     * \property KNSCore::Entry::donationLink
     */
    Q_PROPERTY(QString donationLink READ donationLink)

    /*!
     * \qmlproperty int Entry::numberOfComments
     */
    /*!
     * \property KNSCore::Entry::numberOfComments
     */
    Q_PROPERTY(int numberOfComments READ numberOfComments)

    /*!
     * \qmlproperty int Entry::rating
     */
    /*!
     * \property KNSCore::Entry::rating
     */
    Q_PROPERTY(int rating READ rating)

    /*!
     * \qmlproperty int Entry::downloadCount
     */
    /*!
     * \property KNSCore::Entry::downloadCount
     */
    Q_PROPERTY(int downloadCount READ downloadCount)

    /*!
     * \qmlproperty list<DownloadLinkInformation> Entry::downloadLinks
     */
    /*!
     * \property KNSCore::Entry::downloadLinks
     */
    Q_PROPERTY(QList<KNSCore::Entry::DownloadLinkInformation> downloadLinks READ downloadLinkInformationList)

    /*!
     * \enum KNSCore::Entry::Status
     *
     * \brief Status of the entry.
     *
     * An entry will be downloadable from the provider's
     * site prior to the download. Once downloaded and installed, it will
     * be either installed or updateable, implying an out-of-date
     * installation. Finally, the entry can be deleted and hence show up as
     * downloadable again.
     *
     * Entries not taking part in this cycle, for example those in upload,
     * have an invalid status.
     *
     * \sa setStatus
     * \sa status
     *
     * \value Invalid
     * \value Downloadable
     * \value Installed
     * \value Updateable
     * \value Deleted
     * \value Installing
     * \value Updating
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
    Q_ENUM(Status)

    /*!
     * \enum KNSCore::Entry::Source
     *
     * \brief Source of the entry.
     *
     * An entry's data is coming from either cache, or an online provider.
     * This helps the engine know which data to use when merging cached entries with online
     * entry data.
     *
     * \sa setSource
     * \sa source
     *
     * \value Cache
     * \value Online
     * \value Registry
     */
    enum Source {
        Cache,
        Online,
        Registry,
    };

    /*!
     * \enum KNSCore::Entry::PreviewType
     *
     * \brief Defines the entry's preview image type.
     *
     * \sa previewImage
     * \sa previewUrl
     * \sa setPreviewImage
     * \sa setPreviewUrl
     *
     * \value PreviewSmall1
     * \value PreviewSmall2
     * \value PreviewSmall3
     * \value PreviewBig1
     * \value PreviewBig2
     * \value PreviewBig3
     */
    enum PreviewType {
        PreviewSmall1,
        PreviewSmall2,
        PreviewSmall3,
        PreviewBig1,
        PreviewBig2,
        PreviewBig3,
    };

    /*!
     * \class KNSCore::Entry::DownloadLinkInformation
     * \inmodule KNewStuffCore
     *
     * \brief Describes a download link option for an Entry.
     *
     * \sa appendDownloadLinkInformation
     * \sa downloadLinkInformationList
     */
    struct DownloadLinkInformation {
        /*!
         * Displayed name.
         */
        QString name;

        /*!
         * Price formatted as a string.
         */
        QString priceAmount;

        /*!
         * OCS Distribution Type, this is for which OS the file is useful.
         */
        QString distributionType;

        /*!
         * Link to intermediary description.
         */
        QString descriptionLink;

        /*!
         * Unique integer representing the download number in the list.
         */
        int id;

        /*!
         * TODO
         */
        bool isDownloadtypeLink;

        /*!
         * Size in kilobytes.
         */
        quint64 size = 0;

        /*!
         * Variety of tags that can represent mimetype or url location.
         */
        QStringList tags;
    };

    /*!
     * \enum KNSCore::Entry::EntryEvent
     *
     * \value UnknownEvent
     * A generic event, not generally used.
     *
     * \value StatusChangedEvent
     * Used when an event's status is set (use Entry::status() to get the new status).
     *
     * \value AdoptedEvent
     * Used when an entry has been successfully adopted (use this to determine
     * whether a call to Engine::adoptEntry() succeeded).
     *
     * \value DetailsLoadedEvent
     * Used when more details have been added to an existing entry (such as the
     * full description), and the UI should be updated.
     */
    enum EntryEvent {
        UnknownEvent = 0,
        StatusChangedEvent = 1,
        AdoptedEvent = 2,
        DetailsLoadedEvent = 3,
    };
    Q_ENUM(EntryEvent)

    /*!
     * \enum KNSCore::Entry::EntryType
     *
     * \brief Represents whether the current entry is an actual catalog entry,
     * or an entry that represents a set of entries.
     *
     * \value CatalogEntry
     * These are the main entries that KNewStuff can get the details about and
     * download links for.
     *
     * \value GroupEntry
     * These are entries whose payload is another feed. Currently only used by the OPDS provider.
     *
     * \since 5.83
     */
    enum EntryType {
        CatalogEntry = 0,
        GroupEntry
    };
    Q_ENUM(EntryType)

    /*!
     * Constructor.
     */
    Entry();

    Entry(const Entry &other);
    Entry &operator=(const Entry &other);

    bool operator==(const Entry &other) const;
    bool operator<(const Entry &other) const;

    /*!
     * Destructor.
     */
    ~Entry();

    bool isValid() const;

    /*!
     * Sets the \a name for this data object.
     */
    void setName(const QString &name);

    /*!
     * Returns the name of the data object (potentially translated).
     */
    QString name() const;

    /*!
     * Set the object's unique ID. This must be unique to the provider.
     *
     * \a id The unique ID of this entry as unique to this provider
     *
     * \sa KNSCore::Provider
     */
    void setUniqueId(const QString &id);

    /*!
     * Get the object's unique ID. This will be unique to the provider.
     * This is not intended as user-facing information - though it can
     * be useful for certain purposes, this is supposed to only be used
     * for keeping track of the entry.
     *
     * Returns The unique ID of this entry
     */
    QString uniqueId() const;

    /*!
     * Sets the data \a category, e.g. "KWin Scripts" or "Plasma Theme".
     */
    void setCategory(const QString &category);

    /*!
     * Returns the category of the data object. This is the category's
     * name or ID (as opposed to displayName).
     *
     * \sa KNSCore::Provider::CategoryMetadata
     * \sa KNSCore::EngineBase::categories()
     */
    QString category() const;

    /*!
     * Set a link to a website containing information about this entry
     *
     * \a page The URL representing the entry's website
     */
    void setHomepage(const QUrl &page);

    /*!
     * A link to a website containing information about this entry
     *
     * Returns The URL representing the entry's website
     */
    QUrl homepage() const;

    /*!
     * Sets the \a author of the object.
     */
    void setAuthor(const Author &author);

    /*!
     * Returns the author of the object.
     */
    Author author() const;

    /*!
     * Sets the \a license (abbreviation) applicable to the object.
     */
    void setLicense(const QString &license);

    /*!
     * Returns the license name of the object.
     */
    QString license() const;

    /*!
     * Sets a \a summary (which can potentially be very long)
     */
    void setSummary(const QString &summary);

    /*!
     * Returns a short description of what the object is all about (should be very short).
     */
    QString shortSummary() const;

    /*!
     * Sets a short \a summary of what the object is all about (should be very short)
     */
    void setShortSummary(const QString &summary);

    /*!
     * Returns a (potentially very long) description of the object.
     */
    QString summary() const;

    /*!
     * Sets the user written \a changelog.
     */
    void setChangelog(const QString &changelog);

    /*!
     * Returns the user written changelog.
     */
    QString changelog() const;

    /*!
     * Sets the \a version number.
     */
    void setVersion(const QString &version);

    /*!
     * Returns the version number string of the object.
     */
    QString version() const;

    /*!
     * Sets the release date to \a releasedate.
     */
    void setReleaseDate(const QDate &releasedate);

    /*!
     * Returns the object's release date.
     */
    QDate releaseDate() const;

    /*!
     * Sets the \a version number that is available as update.
     */
    void setUpdateVersion(const QString &version);

    /*!
     * Returns the version string of the object that is available as update.
     */
    QString updateVersion() const;

    /*!
     * Sets the release date that is available as update to \a releasedate.
     */
    void setUpdateReleaseDate(const QDate &releasedate);

    /*!
     * Returns the date of the newer version that is available as update.
     */
    QDate updateReleaseDate() const;

    /*!
     * Sets the object's filename to \a url.
     */
    void setPayload(const QString &url);

    /*!
     * Returns the object's filename.
     */
    QString payload() const;

    /*!
     * Sets the object's preview image file, if available.
     *
     * \a url is the URL of the preview image file
     *
     * \a type is the preview image type
     */
    void setPreviewUrl(const QString &url, PreviewType type = PreviewSmall1);

    /*!
     * Returns the file name of an image containing a preview of the object.
     *
     * \a type is the desired preview image type
     */
    QString previewUrl(PreviewType type = PreviewSmall1) const;

    /*!
     * Returns the entry's preview image of the specified type.
     *
     * \a type is the desired preview image type
     *
     * \note This will not be loaded automatically, instead use Engine to load the actual images.
     */
    QImage previewImage(PreviewType type = PreviewSmall1) const;

    /*!
     * Sets the entry's preview image for the specified preview type.
     *
     * \a image is the image URL
     *
     * \a type is the preview image type
     *
     */
    void setPreviewImage(const QImage &image, PreviewType type = PreviewSmall1);

    /*!
     * Sets the files that have been installed by the install command.
     *
     * \a files is the list of local file names
     *
     */
    void setInstalledFiles(const QStringList &files);

    /*!
     * Returns the locally installed files.
     */
    QStringList installedFiles() const;

    /*!
     * Returns the locally uninstalled files.
     * \since 4.1
     */
    QStringList uninstalledFiles() const;

    /*!
     * Sets the \a rating between 0 (worst) and 100 (best).
     *
     * \internal
     */
    void setRating(int rating);

    /*!
     * Returns the rating for the object, which has been determined by its
     * users and thus might change over time.
     */
    int rating() const;

    /*!
     * Sets the number of comments in the asset
     *
     * \internal
     */
    void setNumberOfComments(int comments);

    /*!
     * Returns the number of comments against the asset.
     */
    int numberOfComments() const;

    /*!
     * Sets the number of downloads.
     *
     * \internal
     */
    void setDownloadCount(int downloads);

    /*!
     * Returns the download count for the object, which has been determined
     * by its hosting sites and thus might change over time.
     */
    int downloadCount() const;

    /*!
     * Returns how many people have marked themselves as fans of this entry.
     *
     * \sa KNSCore::EngineBase::becomeFan
     */
    int numberFans() const;

    /*!
     * Sets how many people are fans.
     *
     * \note This is purely informational. To become a fan, call the
     * KNSCore::Engine::becomeFan function.
     *
     * \a fans The number of fans this entry has
     *
     * \sa KNSCore::EngineBase::becomeFan
     */
    void setNumberFans(int fans);

    /*!
     * Returns the number of entries in the knowledgebase for this entry.
     */
    int numberKnowledgebaseEntries() const;

    /*!
     * Sets the number of knowledgebase entries for this entry.
     *
     * \a num is the number of entries
     */
    void setNumberKnowledgebaseEntries(int num);

    /*!
     * Returns the link for the knowledgebase for this entry.
     */
    QString knowledgebaseLink() const;

    /*!
     * Set the link for the knowledgebase.
     *
     * \note This is not checked for validity, the caller must do this.
     *
     * \a link is the string version of the URL for the knowledgebase
     */
    void setKnowledgebaseLink(const QString &link);

    /*!
     * Returns the number of available download options for this entry.
     *
     * \sa downloadLinkInformationList
     */
    int downloadLinkCount() const;

    /*!
     * Returns a list of downloadable data for this entry.
     *
     * \sa appendDownloadLinkInformation
     * \sa DownloadLinkInformation
     */
    QList<DownloadLinkInformation> downloadLinkInformationList() const;

    /*!
     * Adds a new download option to this entry.
     *
     * \a info is the new download option
     *
     * \sa clearDownloadLinkInformation
     * \sa downloadLinkInformationList
     */
    void appendDownloadLinkInformation(const DownloadLinkInformation &info);

    /*!
     * Removes all download options from this entry
     *
     * \sa appendDownloadLinkInformation
     * \sa downloadLinkInformationList
     */
    void clearDownloadLinkInformation();

    /*!
     * Returns a string representing the URL for a website where the user can donate
     * to the author of this entry.
     */
    QString donationLink() const;

    /*!
     * Sets a string representation of the URL for the donation website for this entry.
     * \note This is not checked for validity, the caller must do this.
     *
     * \a link is the string version of the URL for the entry's donation website
     */
    void setDonationLink(const QString &link);

    /*!
     * Returns the set of tags assigned specifically to this content item. This does not include
     * tags for the download links. To get those, you must concatenate the lists yourself.
     *
     * \sa downloadLinkInformationList()
     * \sa KNSCore::Entry::DownloadLinkInformation
     * \sa EngineBase::setTagFilter
     * \since 5.51
     */
    QStringList tags() const;

    /*!
     * Sets the tags for the content item.
     *
     * \a tags is a string list containing the tags for this entry
     *
     * \since 5.51
     */
    void setTags(const QStringList &tags);

    /*!
     * Returns the id of the provider this entry belongs to.
     */
    QString providerId() const;

    /*!
     * Sets the \a id of the provider this entry belongs to.
     */
    void setProviderId(const QString &id);

    /*!
     * Sets the \a source of this entry.
     */
    void setSource(Source source);

    /*!
     * Returns the source of this entry.
     */
    Source source() const;

    /*!
     * Sets the entry's \a type.
     * \since 5.83
     */
    void setEntryType(EntryType type);

    /*!
     * Returns the entry's type.
     */
    EntryType entryType() const;

    /*!
     * Sets the XML for the entry.
     *
     * This method parses the xml and sets the private members accordingly
     * used to deserialize data loaded from provider.
     *
     * \a reader defines where to read the XML string from
     *
     * Returns whether or not setting the values was successful
     *
     * \since 5.36
     */
    bool setEntryXML(QXmlStreamReader &reader);

    /*!
     * Sets the entry's status. If no status is set, the default will be
     * Invalid.
     *
     * Note that while this enum is currently found in KNS3::Entry,
     * it will be moved to this class once the binary compatibility
     * lock is lifted for Frameworks 6. For now, you should read any
     * reference to the KNS3::Entry::Status enumerator as KNSCore::Entry::Status
     *
     * \a status New status of the entry
     */
    void setStatus(KNSCore::Entry::Status status);

    /*!
     * Returns the entry's status.
     */
    KNSCore::Entry::Status status() const;

    /// \internal
    void setEntryDeleted();

private:
    friend class StaticXmlProvider;
    friend class Cache;
    friend class Cache2;
    friend class Installation;
    friend class AtticaProvider;
    friend class AtticaRequester;
    friend class Transaction;
    friend class TransactionPrivate;
    friend testEntry;
    KNEWSTUFFCORE_NO_EXPORT void setEntryRequestedId(const QString &id);
    QDomElement entryXML() const;
    bool setEntryXML(const QDomElement &xmldata);
    QExplicitlySharedDataPointer<EntryPrivate> d;
};

inline size_t qHash(const KNSCore::Entry &entry, size_t seed = 0)
{
    return qHash(entry.uniqueId(), seed);
}

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug debug, const KNSCore::Entry &entry);
}

Q_DECLARE_TYPEINFO(KNSCore::Entry, Q_RELOCATABLE_TYPE);

#endif
