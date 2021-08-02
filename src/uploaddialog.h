/*
    knewstuff3/ui/uploaddialog.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_H

#include "knewstuff_export.h"

#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 80)

#include <QDialog>
#include <QUrl>

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
 * This dialog shows the user how to add new content to the remote service represented
 * by the configuration file passed into it.
 *
 * @note This dialog originally allowed for performing direct uploads to an OCS service,
 * however there is no such service available at this time, and it is unlikely we will
 * have one any time soon (as we have an issue where such functionality is essentially
 * just a vector for directly visible spam). As such, we have decided to let this dialog
 * instead reflect reality, and just give information on how to manually perform those
 * uploads through a remote service's web based upload system.
 *
 * \par Maintainer:
 * Dan Leinir Turthra Jensen (admin@leinir.dk)
 *
 * @since 4.4
 * @deprecated  Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation. Use KNS3::QtQuickDialogWrapper which
 * includes automatic integration for NewStuff.UploadPage
 */
class KNEWSTUFF_EXPORT UploadDialog : public QDialog
{
    Q_OBJECT
public:
    /**
      Create a new upload dialog.

      @param parent the parent window
      @deprecated  Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation. Use KNS3::QtQuickDialogWrapper which
      includes automatic integration for NewStuff.UploadPage. If the OCS backend supports upload, you can use KNSCore::AtticaHelper to do so, or implement it
      manually.
    */
    KNEWSTUFF_DEPRECATED_VERSION(5, 85, "See API documentation")
    explicit UploadDialog(QWidget *parent = nullptr);

    /**
      Create a new upload dialog.

      @param parent the parent window
      @deprecated  Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation. Use KNS3::QtQuickDialogWrapper which
      includes automatic integration for NewStuff.UploadPage. If the OCS backend supports upload, you can use KNSCore::AtticaHelper to "
        "do so, or implement it manually.
    */
    KNEWSTUFF_DEPRECATED_VERSION(5, 85, "See API documentation")
    explicit UploadDialog(const QString &configFile, QWidget *parent = nullptr);

    /**
      Destructor.
    */
    ~UploadDialog() override;
    /**
      Set the file to be uploaded.
      This has to be set for the dialog to work, before displaying the dialog.

      @param payloadFile the payload data file
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setUploadFile(const QUrl &payloadFile);

    /**
      Set the suggested title for the upload.
      The application can suggest a title which can then be edited by the user before uploading.
      The name field will be left empty if no title was set.

      @param name the suggested name for the upload
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setUploadName(const QString &name);

    /**
      Set the suggested version displayed in the upload dialog.
      The user can still change this.
      @param version
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setVersion(const QString &version);

    /**
      Set the suggested description displayed in the upload dialog.
      The user can still change this.
      @param description
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setDescription(const QString &description);

    /**
      Set the suggested changelog displayed in the upload dialog.
      The user can still change this.
      @param version version
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setChangelog(const QString &changelog);

    /**
      Set one of the three preview images displayed in the upload dialog.
      The user can still change this.
      @param number The number of the preview image to set, either 1, 2, or 3.
      @param file A URL to the file to be used as preview image
      @since 4.6
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setPreviewImageFile(uint number, const QUrl &file);

    /**
     Enable the UI to let the user to set a price for the uploaded item.
     @param enabled enable the price option - it is enabled by default
     @since 4.5
     @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setPriceEnabled(bool enabled);

    /**
      Set the suggested price displayed in the upload dialog.
      The user can still change this.
      @param version version
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setPrice(double price);

    /**
      Set the suggested rationale why this item costs something to download.
      The user can still change this.
      @param version version
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void setPriceReason(const QString &reason);

    /**
      Set the suggested category for the upload.
      The .knsrc file may contain multiple upload categories, this sets which one is pre-selected.
      It does not add any new category to the list of available categories.

      @param category the suggested category for the upload
      @deprecated Since 5.85, Upload functionality is no longer directly supported and needs complete reimplementation
     */
    void selectCategory(const QString &category);

public Q_SLOTS:
    void accept() override;

private:
    bool init(const QString &configfile);

    UploadDialogPrivate *const d;

    Q_DISABLE_COPY(UploadDialog)
};

}

#endif

#endif
