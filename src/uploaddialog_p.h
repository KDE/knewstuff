/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_P_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_P_H

#include "ui_uploaddialog.h"
#include "upload/atticahelper_p.h"

#include <attica/category.h>
#include <attica/content.h>
#include <attica/license.h>
#include <attica/listjob.h>
#include <attica/postjob.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include <QUrl>

class QDialogButtonBox;
class QPushButton;
class KPixmapSequenceWidget;

namespace KNS3
{
class UploadDialogPrivate
{
public:
    UploadDialogPrivate(UploadDialog *q)
        : q(q)
        , currentPage(UserPasswordPage)
        , finished(false)
        , finishedPreview1(false)
        , finishedPreview2(false)
        , finishedPreview3(false)
        , finishedContents(false)
    {
    }

    UploadDialog *q;

    enum WizardPage {
        UserPasswordPage,
        FileNewUpdatePage,
        Details1Page,
        Details2Page,
        UploadFinalPage,
    };
    WizardPage currentPage;

    Attica::Provider currentProvider()
    {
        return atticaHelper->provider();
    }

    Ui::UploadDialog ui;
    QDialogButtonBox *buttonBox;
    QPushButton *finishButton;
    QPushButton *nextButton;
    QPushButton *backButton;
    KPixmapSequenceWidget *busyWidget;

    KNSCore::AtticaHelper *atticaHelper;

    QUrl uploadFile;
    QUrl previewFile1;
    QUrl previewFile2;
    QUrl previewFile3;
    QStringList categoryNames;
    Attica::Category::List categories;
    QString contentId;
    bool finished;
    bool finishedPreview1;
    bool finishedPreview2;
    bool finishedPreview3;
    bool finishedContents;

    bool init(const QString &configfile);
    void setBusy(const QString &message);
    void setIdle(const QString &message);

    // change to page, set the focus also calls updatePage()
    void _k_showPage(int page);

    // check after user input - for example enable the next button
    void _k_updatePage();

    // next wizard page (next button clicked)
    void _k_nextPage();
    // go back one page
    void _k_backPage();

    // after all has been done and said, do the uploading
    void _k_startUpload();

    void _k_providersLoaded(const QStringList &providerNames);
    void _k_providerChanged(const QString &providerName);

    // validation of login is done, go to next page if successful, otherwise ask again
    void _k_checkCredentialsFinished(bool);
    void _k_categoriesLoaded(const Attica::Category::List &loadedCategories);
    void _k_licensesLoaded(const Attica::License::List &licenses);
    void _k_currencyLoaded(const QString &currency);
    void _k_contentByCurrentUserLoaded(const Attica::Content::List &contentList);
    void _k_updatedContentFetched(const Attica::Content &content);
    void _k_previewLoaded(int index, const QImage &image);

    void _k_changePreview1();
    void _k_changePreview2();
    void _k_changePreview3();
    void _k_preview1UploadFinished(Attica::BaseJob *);
    void _k_preview2UploadFinished(Attica::BaseJob *);
    void _k_preview3UploadFinished(Attica::BaseJob *);

    void _k_contentAdded(Attica::BaseJob *);
    void _k_fileUploadFinished(Attica::BaseJob *);

    void uploadFileFinished();
    void doUpload(const QString &index, const QUrl &filePath);

    void _k_priceToggled(bool);
    void _k_updateContentsToggled(bool update);
    void _k_detailsLinkLoaded(const QUrl &url);
    void _k_openRegisterAccountWebpage(QString);

    QStringList _supportedMimeTypes() const;
};
}

#endif
