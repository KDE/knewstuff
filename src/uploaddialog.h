/*
    knewstuff3/ui/uploaddialog.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_H

#include <QDialog>
#include <QUrl>

#include "knewstuff_export.h"

namespace Attica
{
class BaseJob;
}

// KDE5: this class should inherit from the wizard class - KAssistantDialog

namespace KNS3
{
class UploadDialogPrivate;
/**
 * @short KNewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 *
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 *
 * @since 4.4
 */
class KNEWSTUFF_EXPORT UploadDialog : public QDialog
{
    Q_OBJECT
public:
    /**
      Create a new upload dialog.

      @param parent the parent window
    */
    explicit UploadDialog(QWidget *parent = nullptr);

    /**
      Create a new upload dialog.

      @param parent the parent window
    */
    explicit UploadDialog(const QString &configFile, QWidget *parent = nullptr);

    /**
      Destructor.
    */
    ~UploadDialog() override;

    /**
      Set the file to be uploaded.
      This has to be set for the dialog to work, before displaying the dialog.

      @param payloadFile the payload data file
    */
    void setUploadFile(const QUrl &payloadFile);

    /**
      Set the suggested title for the upload.
      The application can suggest a title which can then be edited by the user before uploading.
      The name field will be left empty if no title was set.

      @param name the suggested name for the upload
    */
    void setUploadName(const QString &name);

    /**
      Set the suggested version displayed in the upload dialog.
      The user can still change this.
      @param version
      */
    void setVersion(const QString &version);

    /**
      Set the suggested description displayed in the upload dialog.
      The user can still change this.
      @param description
      */
    void setDescription(const QString &description);

    /**
      Set the suggested changelog displayed in the upload dialog.
      The user can still change this.
      @param version version
      */
    void setChangelog(const QString &changelog);

    /**
      Set one of the three preview images displayed in the upload dialog.
      The user can still change this.
      @param number The number of the preview image to set, either 1, 2, or 3.
      @param file A URL to the file to be used as preview image
      @since 4.6
      */
    void setPreviewImageFile(uint number, const QUrl &file);

    /**
     Enable the UI to let the user to set a price for the uploaded item.
     @param enabled enable the price option - it is enabled by default
     @since 4.5
     */
    void setPriceEnabled(bool enabled);

    /**
      Set the suggested price displayed in the upload dialog.
      The user can still change this.
      @param version version
      */
    void setPrice(double price);

    /**
      Set the suggested rationale why this item costs something to download.
      The user can still change this.
      @param version version
      */
    void setPriceReason(const QString &reason);

    /**
      Set the suggested category for the upload.
      The .knsrc file may contain multiple upload categories, this sets which one is pre-selected.
      It does not add any new category to the list of available categories.

      @param category the suggested category for the upload
    */
    void selectCategory(const QString &category);

public Q_SLOTS:
    void accept() override;

private:
    bool init(const QString &configfile);

    UploadDialogPrivate *const d;

    Q_PRIVATE_SLOT(d, void _k_nextPage())
    Q_PRIVATE_SLOT(d, void _k_backPage())
    Q_PRIVATE_SLOT(d, void _k_updatePage())

    Q_PRIVATE_SLOT(d, void _k_providerChanged(QString))
    Q_PRIVATE_SLOT(d, void _k_checkCredentialsFinished(bool))
    Q_PRIVATE_SLOT(d, void _k_contentByCurrentUserLoaded(Attica::Content::List))
    Q_PRIVATE_SLOT(d, void _k_providersLoaded(QStringList))
    Q_PRIVATE_SLOT(d, void _k_categoriesLoaded(Attica::Category::List))
    Q_PRIVATE_SLOT(d, void _k_licensesLoaded(Attica::License::List))
    Q_PRIVATE_SLOT(d, void _k_currencyLoaded(QString))
    Q_PRIVATE_SLOT(d, void _k_previewLoaded(int, QImage))

    Q_PRIVATE_SLOT(d, void _k_changePreview1())
    Q_PRIVATE_SLOT(d, void _k_changePreview2())
    Q_PRIVATE_SLOT(d, void _k_changePreview3())
    Q_PRIVATE_SLOT(d, void _k_priceToggled(bool))
    Q_PRIVATE_SLOT(d, void _k_updateContentsToggled(bool update))

    Q_PRIVATE_SLOT(d, void _k_startUpload())
    Q_PRIVATE_SLOT(d, void _k_contentAdded(Attica::BaseJob *))
    Q_PRIVATE_SLOT(d, void _k_fileUploadFinished(Attica::BaseJob *))
    Q_PRIVATE_SLOT(d, void _k_preview1UploadFinished(Attica::BaseJob *))
    Q_PRIVATE_SLOT(d, void _k_preview2UploadFinished(Attica::BaseJob *))
    Q_PRIVATE_SLOT(d, void _k_preview3UploadFinished(Attica::BaseJob *))

    Q_PRIVATE_SLOT(d, void _k_updatedContentFetched(Attica::Content))
    Q_PRIVATE_SLOT(d, void _k_detailsLinkLoaded(QUrl))

    Q_PRIVATE_SLOT(d, void _k_openRegisterAccountWebpage(QString))

    Q_DISABLE_COPY(UploadDialog)
};

}

#endif
