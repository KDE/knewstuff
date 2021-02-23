/*
    knewstuff3/ui/uploaddialog.cpp.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "uploaddialog.h"
#include "ui/widgetquestionlistener.h"
#include "uploaddialog_p.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QImageReader>
#include <QLayout>
#include <QPointer>
#include <QString>

#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>
#include <KPixmapSequence>
#include <KPixmapSequenceWidget>

#include <KConfig>
#include <KConfigGroup>
#include <QLoggingCategory>
#include <knewstuff_debug.h>
#include <qstandardpaths.h>

using namespace KNS3;

bool UploadDialogPrivate::init(const QString &configfile)
{
    QVBoxLayout *layout = new QVBoxLayout;
    q->setLayout(layout);

    QWidget *_mainWidget = new QWidget(q);
    ui.setupUi(_mainWidget);

    layout->addWidget(_mainWidget);

    backButton = new QPushButton;
    KGuiItem::assign(backButton, KStandardGuiItem::back(KStandardGuiItem::UseRTL));

    nextButton = new QPushButton;
    nextButton->setText(i18nc("Opposite to Back", "Next"));
    nextButton->setIcon(KStandardGuiItem::forward(KStandardGuiItem::UseRTL).icon());
    nextButton->setDefault(true);

    finishButton = new QPushButton;
    finishButton->setText(i18n("Finish"));
    finishButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));

    buttonBox = new QDialogButtonBox(q);
    buttonBox->addButton(backButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(finishButton, QDialogButtonBox::AcceptRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    atticaHelper = new KNSCore::AtticaHelper(q);

    bool success = true;
    QFileInfo fi(configfile);
    if (!fi.exists()) {
        if (!fi.isAbsolute())
            fi.setFile(QStandardPaths::locate(QStandardPaths::GenericConfigLocation, configfile));
        if (!fi.exists()) {
            qCCritical(KNEWSTUFF) << "No knsrc file named '" << fi.absoluteFilePath() << "' was found.";
            success = false;
        }
    }

    KConfig conf(fi.absoluteFilePath());
    if (conf.accessMode() == KConfig::NoAccess) {
        qCCritical(KNEWSTUFF) << "Knsrc file named '" << fi.absoluteFilePath() << "' could not be accessed.";
        success = false;
    }

    KConfigGroup group;
    if (conf.hasGroup("KNewStuff3")) {
        qCDebug(KNEWSTUFF) << "Loading KNewStuff3 config: " << fi.absoluteFilePath();
        group = conf.group("KNewStuff3");
    } else {
        qCCritical(KNEWSTUFF) << "A knsrc file was found but it doesn't contain a KNewStuff3 section." << fi.absoluteFilePath();
        success = false;
    }

    if (success) {
        const QString providersFileUrl = group.readEntry("ProvidersUrl");

        categoryNames = group.readEntry("UploadCategories", QStringList());
        // fall back to download categories
        if (categoryNames.isEmpty()) {
            categoryNames = group.readEntry("Categories", QStringList());
        }

        atticaHelper->addProviderFile(QUrl(providersFileUrl));
    }

    ui.mCategoryCombo->addItems(categoryNames);

    if (categoryNames.size() == 1) {
        ui.mCategoryLabel->setVisible(false);
        ui.mCategoryCombo->setVisible(false);
    }

    qCDebug(KNEWSTUFF) << "Categories: " << categoryNames;

    q->connect(atticaHelper, SIGNAL(providersLoaded(QStringList)), q, SLOT(_k_providersLoaded(QStringList)));
    q->connect(atticaHelper, SIGNAL(loginChecked(bool)), q, SLOT(_k_checkCredentialsFinished(bool)));
    q->connect(atticaHelper, SIGNAL(licensesLoaded(Attica::License::List)), q, SLOT(_k_licensesLoaded(Attica::License::List)));
    q->connect(atticaHelper, SIGNAL(categoriesLoaded(Attica::Category::List)), q, SLOT(_k_categoriesLoaded(Attica::Category::List)));
    q->connect(atticaHelper, SIGNAL(contentByCurrentUserLoaded(Attica::Content::List)), q, SLOT(_k_contentByCurrentUserLoaded(Attica::Content::List)));
    q->connect(atticaHelper, SIGNAL(contentLoaded(Attica::Content)), q, SLOT(_k_updatedContentFetched(Attica::Content)));
    q->connect(atticaHelper, SIGNAL(detailsLinkLoaded(QUrl)), q, SLOT(_k_detailsLinkLoaded(QUrl)));
    q->connect(atticaHelper, SIGNAL(currencyLoaded(QString)), q, SLOT(_k_currencyLoaded(QString)));
    q->connect(atticaHelper, SIGNAL(previewLoaded(int, QImage)), q, SLOT(_k_previewLoaded(int, QImage)));
    atticaHelper->init();

    q->connect(ui.changePreview1Button, SIGNAL(clicked()), q, SLOT(_k_changePreview1()));
    q->connect(ui.changePreview2Button, SIGNAL(clicked()), q, SLOT(_k_changePreview2()));
    q->connect(ui.changePreview3Button, SIGNAL(clicked()), q, SLOT(_k_changePreview3()));

    q->connect(ui.providerComboBox, SIGNAL(currentIndexChanged(QString)), q, SLOT(_k_providerChanged(QString)));
    q->connect(ui.radioUpdate, SIGNAL(toggled(bool)), q, SLOT(_k_updateContentsToggled(bool)));

    q->connect(ui.registerNewAccountLabel, SIGNAL(linkActivated(QString)), q, SLOT(_k_openRegisterAccountWebpage(QString)));

    // Busy widget
    busyWidget = new KPixmapSequenceWidget();
    busyWidget->setSequence(KIconLoader::global()->loadPixmapSequence(QStringLiteral("process-working"), 22));
    busyWidget->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ui.busyWidget->setLayout(new QHBoxLayout());
    ui.busyWidget->layout()->addWidget(busyWidget);
    busyWidget->setVisible(false);

    WidgetQuestionListener::instance();

    return success;
}

void UploadDialogPrivate::setBusy(const QString &message)
{
    ui.busyLabel->setText(message);
    busyWidget->setVisible(true);
}

void UploadDialogPrivate::setIdle(const QString &message)
{
    ui.busyLabel->setText(message);
    busyWidget->setVisible(false);
}

void UploadDialogPrivate::_k_showPage(int page)
{
    ui.stackedWidget->setCurrentIndex(page);
    setIdle(QString());

    switch (ui.stackedWidget->currentIndex()) {
    case UserPasswordPage:
        ui.username->setFocus();
        setBusy(i18n("Fetching provider information..."));
        break;

    case FileNewUpdatePage:
        atticaHelper->loadLicenses();
        atticaHelper->loadCurrency();
        ui.uploadButton->setFocus();
        setBusy(i18n("Fetching license data from server..."));
        break;

    case Details1Page:
        if (ui.radioUpdate->isChecked()) {
            // Fetch
            atticaHelper->loadContent(ui.userContentList->currentItem()->data(Qt::UserRole).toString());
            setBusy(i18n("Fetching content data from server..."));
        }

        ui.mNameEdit->setFocus();
        break;

    case UploadFinalPage:
        if (previewFile1.isEmpty()) {
            ui.uploadPreview1ImageLabel->setVisible(false);
            ui.uploadPreview1Label->setVisible(false);
        }
        if (previewFile2.isEmpty()) {
            ui.uploadPreview2ImageLabel->setVisible(false);
            ui.uploadPreview2Label->setVisible(false);
        }
        if (previewFile3.isEmpty()) {
            ui.uploadPreview3ImageLabel->setVisible(false);
            ui.uploadPreview3Label->setVisible(false);
        }
        break;
    }

    _k_updatePage();
}

void UploadDialogPrivate::_k_updatePage()
{
    bool firstPage = ui.stackedWidget->currentIndex() == 0;
    backButton->setEnabled(!firstPage && !finished);

    bool nextEnabled = false;
    switch (ui.stackedWidget->currentIndex()) {
    case UserPasswordPage:
        if (ui.providerComboBox->count() > 0 && !ui.username->text().isEmpty() && !ui.password->text().isEmpty()) {
            nextEnabled = true;
        }
        break;

    case FileNewUpdatePage:
        // FIXME: check if the file requester contains a valid file
        if (!uploadFile.isEmpty() || ui.uploadFileRequester->url().isLocalFile()) {
            if (ui.radioNewUpload->isChecked() || ui.userContentList->currentRow() >= 0) {
                nextEnabled = true;
            }
        }
        break;

    case Details1Page:
        if (!ui.mNameEdit->text().isEmpty()) {
            nextEnabled = true;
        }
        break;

    case Details2Page:
        nextEnabled = true;
        break;

    case UploadFinalPage:
        break;
    }

    nextButton->setEnabled(nextEnabled);
    finishButton->setEnabled(finished);

    nextButton->setDefault(nextEnabled);
    finishButton->setDefault(!nextEnabled);

    if (nextEnabled && buttonBox->button(QDialogButtonBox::Cancel)->hasFocus()) {
        nextButton->setFocus();
    }
}

void UploadDialogPrivate::_k_providersLoaded(const QStringList &providers)
{
    if (providers.isEmpty()) {
        setIdle(i18n("Could not fetch provider information."));
        ui.stackedWidget->setEnabled(false);
        qWarning() << "Could not load providers.";
        return;
    }
    setIdle(QString());
    ui.providerComboBox->addItems(providers);
    ui.providerComboBox->setCurrentIndex(0);
    atticaHelper->setCurrentProvider(providers.at(0));

    QString user;
    QString pass;
    if (atticaHelper->loadCredentials(user, pass)) {
        ui.username->setText(user);
        ui.password->setText(pass);
    }
    _k_updatePage();
}

void UploadDialogPrivate::_k_providerChanged(const QString &providerName)
{
    atticaHelper->setCurrentProvider(providerName);
    QString registerUrl = atticaHelper->provider().getRegisterAccountUrl();
    if (!registerUrl.isEmpty()) {
        ui.registerNewAccountLabel->setText(QStringLiteral("<a href=\"register\">") + i18n("Register a new account") + QStringLiteral("</a>"));
    } else {
        ui.registerNewAccountLabel->setText(QString());
    }
    ui.username->clear();
    ui.password->clear();
    QString user;
    QString pass;
    if (atticaHelper->loadCredentials(user, pass)) {
        ui.username->setText(user);
        ui.password->setText(pass);
    }
    _k_updatePage();
}

void UploadDialogPrivate::_k_backPage()
{
    _k_showPage(ui.stackedWidget->currentIndex() - 1);
}

void UploadDialogPrivate::_k_nextPage()
{
    // TODO: validate credentials after user name/password have been entered
    if (ui.stackedWidget->currentIndex() == UserPasswordPage) {
        setBusy(i18n("Checking login..."));
        nextButton->setEnabled(false);
        ui.providerComboBox->setEnabled(false);
        ui.username->setEnabled(false);
        ui.password->setEnabled(false);
        atticaHelper->checkLogin(ui.username->text(), ui.password->text());
    } else {
        _k_showPage(ui.stackedWidget->currentIndex() + 1);
    }
}

void UploadDialogPrivate::_k_checkCredentialsFinished(bool success)
{
    ui.providerComboBox->setEnabled(true);
    ui.username->setEnabled(true);
    ui.password->setEnabled(true);

    if (success) {
        atticaHelper->saveCredentials(ui.username->text(), ui.password->text());
        _k_showPage(FileNewUpdatePage);

        atticaHelper->loadCategories(categoryNames);
        setBusy(i18n("Fetching your previously updated content..."));
    } else {
        // TODO check what the actual error is
        setIdle(i18n("Could not verify login, please try again."));
    }
}

void UploadDialogPrivate::_k_licensesLoaded(const Attica::License::List &licenses)
{
    ui.mLicenseCombo->clear();
    for (const Attica::License &license : licenses) {
        ui.mLicenseCombo->addItem(license.name(), license.id());
    }
}

void UploadDialogPrivate::_k_currencyLoaded(const QString &currency)
{
    ui.priceCurrency->setText(currency);
}

void UploadDialogPrivate::_k_contentByCurrentUserLoaded(const Attica::Content::List &contentList)
{
    setIdle(i18n("Fetching your previously updated content finished."));

    for (const Attica::Content &content : contentList) {
        QListWidgetItem *contentItem = new QListWidgetItem(content.name());
        contentItem->setData(Qt::UserRole, content.id());
        ui.userContentList->addItem(contentItem);
    }

    if (ui.userContentList->count() > 0) {
        ui.userContentList->setCurrentRow(0);
        ui.radioUpdate->setEnabled(true);
        _k_updatePage();
    }
}

void UploadDialogPrivate::_k_updatedContentFetched(const Attica::Content &content)
{
    setIdle(i18n("Fetching content data from server finished."));

    contentId = content.id();
    // fill in ui
    ui.mNameEdit->setText(content.name());
    ui.mSummaryEdit->setText(content.description());
    ui.mVersionEdit->setText(content.version());
    ui.changelog->setText(content.changelog());
    ui.priceCheckBox->setChecked(content.attribute(QStringLiteral("downloadbuy1")) == QLatin1Char('1'));
    ui.priceSpinBox->setValue(content.attribute(QStringLiteral("downloadbuyprice1")).toDouble());
    ui.priceReasonLineEdit->setText(content.attribute(QStringLiteral("downloadbuyreason1")));

    bool conversionOk = false;
    int licenseNumber = content.license().toInt(&conversionOk);
    if (conversionOk) {
        // check if that int is in list
        int row = ui.mLicenseCombo->findData(licenseNumber, Qt::UserRole);
        ui.mLicenseCombo->setCurrentIndex(row);
    } else {
        ui.mLicenseCombo->setEditText(content.license());
    }

    ui.contentWebsiteLink->setText(QLatin1String("<a href=\"") + content.detailpage().toString() + QLatin1String("\">")
                                   + i18nc("A link to the website where the get hot new stuff upload can be seen", "Visit website") + QLatin1String("</a>"));
    ui.fetchContentLinkImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
}

void UploadDialogPrivate::_k_previewLoaded(int index, const QImage &image)
{
    switch (index) {
    case 1:
        ui.previewImage1->setPixmap(QPixmap::fromImage(image));
        break;
    case 2:
        ui.previewImage2->setPixmap(QPixmap::fromImage(image));
        break;
    case 3:
        ui.previewImage3->setPixmap(QPixmap::fromImage(image));
        break;
    }
}

void UploadDialogPrivate::_k_updateContentsToggled(bool update)
{
    ui.userContentList->setEnabled(update);
}

UploadDialog::UploadDialog(QWidget *parent)
    : QDialog(parent)
    , d(new UploadDialogPrivate(this))
{
    const QString name = QCoreApplication::applicationName();
    init(name + QStringLiteral(".knsrc"));
}

UploadDialog::UploadDialog(const QString &configFile, QWidget *parent)
    : QDialog(parent)
    , d(new UploadDialogPrivate(this))
{
    init(configFile);
}

UploadDialog::~UploadDialog()
{
    delete d;
}

bool UploadDialog::init(const QString &configfile)
{
    bool success = d->init(configfile);

    setWindowTitle(i18n("Share Hot New Stuff"));

    d->_k_updatePage();

    connect(d->ui.username, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));

    connect(d->ui.password, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.mNameEdit, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.uploadFileRequester, SIGNAL(textChanged(QString)), this, SLOT(_k_updatePage()));
    connect(d->ui.priceCheckBox, SIGNAL(toggled(bool)), this, SLOT(_k_priceToggled(bool)));

    connect(d->ui.uploadButton, SIGNAL(clicked()), this, SLOT(_k_startUpload()));

    connect(d->backButton, SIGNAL(clicked()), this, SLOT(_k_backPage()));
    connect(d->nextButton, SIGNAL(clicked()), this, SLOT(_k_nextPage()));
    connect(d->buttonBox, &QDialogButtonBox::accepted, this, &UploadDialog::accept);
    connect(d->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QString displayName = QGuiApplication::applicationDisplayName();
    if (displayName.isEmpty()) {
        displayName = QCoreApplication::applicationName();
    }
    d->ui.mTitleWidget->setText(i18nc("Program name followed by 'Add On Uploader'", "%1 Add-On Uploader", displayName));

    if (success) {
        d->_k_showPage(0);
    }

    return success;
}

void UploadDialog::setUploadFile(const QUrl &payloadFile)
{
    d->uploadFile = payloadFile;

    d->ui.uploadFileLabel->setVisible(false);
    d->ui.uploadFileRequester->setVisible(false);

    QFile file(d->uploadFile.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("File not found: %1", d->uploadFile.url()), i18n("Upload Failed"));
    }
}

void UploadDialog::setUploadName(const QString &name)
{
    d->ui.mNameEdit->setText(name);
}

void UploadDialog::selectCategory(const QString &category)
{
    d->ui.mCategoryCombo->setCurrentIndex(d->ui.mCategoryCombo->findText(category, Qt::MatchFixedString));
}

void UploadDialog::setChangelog(const QString &changelog)
{
    d->ui.changelog->setText(changelog);
}

void UploadDialog::setDescription(const QString &description)
{
    d->ui.mSummaryEdit->setText(description);
}

void UploadDialog::setPriceEnabled(bool enabled)
{
    d->ui.priceCheckBox->setVisible(enabled);
    d->ui.priceGroupBox->setVisible(enabled);
}

void UploadDialog::setPrice(double price)
{
    d->ui.priceCheckBox->setEnabled(true);
    d->ui.priceSpinBox->setValue(price);
}

void UploadDialog::setPriceReason(const QString &reason)
{
    d->ui.priceReasonLineEdit->setText(reason);
}

void UploadDialog::setVersion(const QString &version)
{
    d->ui.mVersionEdit->setText(version);
}

void UploadDialog::setPreviewImageFile(uint number, const QUrl &file)
{
    QPixmap preview(file.toLocalFile());
    switch (number) {
    case 0:
        d->previewFile1 = file;
        d->ui.previewImage1->setPixmap(preview.scaled(d->ui.previewImage1->size()));
        break;
    case 1:
        d->previewFile2 = file;
        d->ui.previewImage2->setPixmap(preview.scaled(d->ui.previewImage2->size()));
        break;
    case 2:
        d->previewFile3 = file;
        d->ui.previewImage3->setPixmap(preview.scaled(d->ui.previewImage3->size()));
        break;
    default:
        qCCritical(KNEWSTUFF) << "Wrong preview image file number";
        break;
    }
}

void UploadDialogPrivate::_k_priceToggled(bool priceEnabled)
{
    ui.priceGroupBox->setEnabled(priceEnabled);
}

void UploadDialogPrivate::_k_categoriesLoaded(const Attica::Category::List &loadedCategories)
{
    categories = loadedCategories;

    // at least one category is needed
    if (categories.isEmpty()) {
        KMessageBox::error(q,
                           i18np("The server does not recognize the category %2 to which you are trying to upload.",
                                 "The server does not recognize any of the categories to which you are trying to upload: %2",
                                 categoryNames.size(),
                                 categoryNames.join(QLatin1String(", "))),
                           i18n("Error"));
        // close the dialog
        q->reject();
        return;
    }
    for (const Attica::Category &c : qAsConst(categories)) {
        ui.mCategoryCombo->addItem(c.name(), c.id());
    }
    atticaHelper->loadContentByCurrentUser();
}

void UploadDialog::accept()
{
    QDialog::accept();
}

void UploadDialogPrivate::_k_startUpload()
{
    // FIXME: this only works if categories are set in the .knsrc file
    // TODO: ask for confirmation when closing the dialog

    backButton->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);

    ui.uploadButton->setEnabled(false);

    // idle back and forth, we need a fix in attica to get at real progress values
    ui.uploadProgressBar->setMinimum(0);
    ui.uploadProgressBar->setMaximum(0);
    ui.uploadProgressBar->setValue(0);

    // check the category
    QString categoryName = ui.mCategoryCombo->currentText();
    QList<Attica::Category>::const_iterator iter = categories.constBegin();
    Attica::Category category;
    QList<Attica::Category>::const_iterator iterEnd = categories.constEnd();
    while (iter != iterEnd) {
        if (iter->name() == categoryName) {
            category = *iter;
            break;
        }
        ++iter;
    }
    if (!category.isValid()) {
        KMessageBox::error(q, i18n("The selected category \"%1\" is invalid.", categoryName), i18n("Upload Failed"));
        return;
    }

    // fill in the content object
    Attica::Content content;
    content.setName(ui.mNameEdit->text());
    QString summary = ui.mSummaryEdit->toPlainText();
    content.addAttribute(QStringLiteral("description"), summary);
    content.addAttribute(QStringLiteral("version"), ui.mVersionEdit->text());

    // for the license, if one of the licenses coming from the server was used, pass its id, otherwise the string
    QString licenseId = ui.mLicenseCombo->itemData(ui.mLicenseCombo->currentIndex()).toString();
    if (licenseId.isEmpty()) {
        // use other as type and add the string as text
        content.addAttribute(QStringLiteral("licensetype"), QStringLiteral("0"));
        content.addAttribute(QStringLiteral("license"), ui.mLicenseCombo->currentText());
    } else {
        content.addAttribute(QStringLiteral("licensetype"), licenseId);
    }

    content.addAttribute(QStringLiteral("changelog"), ui.changelog->toPlainText());

    // TODO: add additional attributes
    // content.addAttribute("downloadlink1", ui.link1->text());
    // content.addAttribute("downloadlink2", ui.link2->text());
    // content.addAttribute("homepage1", ui.homepage->text());
    // content.addAttribute("blog1", ui.blog->text());

    content.addAttribute(QStringLiteral("downloadbuy1"), ui.priceCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
    content.addAttribute(QStringLiteral("downloadbuyprice1"), QString::number(ui.priceSpinBox->value()));
    content.addAttribute(QStringLiteral("downloadbuyreason1"), ui.priceReasonLineEdit->text());

    if (ui.radioNewUpload->isChecked()) {
        // upload a new content
        Attica::ItemPostJob<Attica::Content> *job = currentProvider().addNewContent(category, content);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_contentAdded(Attica::BaseJob *)));
        job->start();
    } else {
        // update old content
        Attica::ItemPostJob<Attica::Content> *job =
            currentProvider().editContent(category, ui.userContentList->currentItem()->data(Qt::UserRole).toString(), content);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_contentAdded(Attica::BaseJob *)));
        job->start();
    }
}

void UploadDialogPrivate::_k_changePreview1()
{
    const QStringList filters = _supportedMimeTypes();
    QPointer<QFileDialog> dialog = new QFileDialog(q, i18n("Select preview image"));
    dialog->setMimeTypeFilters(filters);
    if (dialog->exec() == QDialog::Accepted) {
        QUrl url = dialog->selectedUrls().first();
        previewFile1 = url;
        qCDebug(KNEWSTUFF) << "preview is: " << url.url();
        QPixmap preview(url.toLocalFile());
        ui.previewImage1->setPixmap(preview.scaled(ui.previewImage1->size()));
    }
    delete dialog;
}

void UploadDialogPrivate::_k_changePreview2()
{
    const QStringList filters = _supportedMimeTypes();
    QPointer<QFileDialog> dialog = new QFileDialog(q, i18n("Select preview image"));
    dialog->setMimeTypeFilters(filters);
    if (dialog->exec() == QDialog::Accepted) {
        QUrl url = dialog->selectedUrls().first();
        previewFile2 = url;
        QPixmap preview(url.toLocalFile());
        ui.previewImage2->setPixmap(preview.scaled(ui.previewImage1->size()));
    }
    delete dialog;
}

void UploadDialogPrivate::_k_changePreview3()
{
    const QStringList filters = _supportedMimeTypes();
    QPointer<QFileDialog> dialog = new QFileDialog(q, i18n("Select preview image"));
    dialog->setMimeTypeFilters(filters);
    if (dialog->exec() == QDialog::Accepted) {
        QUrl url = dialog->selectedUrls().first();
        previewFile3 = url;
        QPixmap preview(url.toLocalFile());
        ui.previewImage3->setPixmap(preview.scaled(ui.previewImage1->size()));
    }
    delete dialog;
}

void UploadDialogPrivate::_k_contentAdded(Attica::BaseJob *baseJob)
{
    if (baseJob->metadata().error()) {
        if (baseJob->metadata().error() == Attica::Metadata::NetworkError) {
            KMessageBox::error(q, i18n("There was a network error."), i18n("Uploading Failed"));
            return;
        }
        if (baseJob->metadata().error() == Attica::Metadata::OcsError) {
            if (baseJob->metadata().statusCode() == 102) {
                KMessageBox::error(q, i18n("Authentication error."), i18n("Uploading Failed"));
            }
        }
        return;
    }

    ui.createContentImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));

    Attica::ItemPostJob<Attica::Content> *job = static_cast<Attica::ItemPostJob<Attica::Content> *>(baseJob);
    if (job->metadata().error() != Attica::Metadata::NoError) {
        KMessageBox::error(q, i18n("Upload failed: %1", job->metadata().message()));
        return;
    }

    // only when adding new content we get an id returned, otherwise stick with the old one
    QString id = job->result().id();
    if (!id.isEmpty()) {
        contentId = id;
    }

    if (!uploadFile.isEmpty()) {
        doUpload(QString(), uploadFile);
    } else {
        doUpload(QString(), ui.uploadFileRequester->url());
    }

    // FIXME: status labels need to accommodate 3 previews
    if (!previewFile1.isEmpty()) {
        doUpload(QStringLiteral("1"), previewFile1);
    }
    if (!previewFile2.isEmpty()) {
        doUpload(QStringLiteral("2"), previewFile2);
    }
    if (!previewFile3.isEmpty()) {
        doUpload(QStringLiteral("3"), previewFile3);
    }

    if (ui.radioNewUpload->isChecked()) {
        atticaHelper->loadDetailsLink(contentId);
    }
}

void UploadDialogPrivate::_k_openRegisterAccountWebpage(QString)
{
    KIO::OpenUrlJob *job = new KIO::OpenUrlJob(QUrl::fromUserInput(atticaHelper->provider().getRegisterAccountUrl()), QStringLiteral("text/html"));
    job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, q));
    job->start();
}

void UploadDialogPrivate::doUpload(const QString &index, const QUrl &path)
{
    QFile file(path.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        KMessageBox::error(q, i18n("File not found: %1", uploadFile.url()), i18n("Upload Failed"));
        q->reject();
        return;
    }

    QByteArray fileContents;
    fileContents.append(file.readAll());
    file.close();

    QString fileName = QFileInfo(path.toLocalFile()).fileName();

    Attica::PostJob *job = nullptr;
    if (index.isEmpty()) {
        job = currentProvider().setDownloadFile(contentId, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_fileUploadFinished(Attica::BaseJob *)));
    } else if (index == QLatin1Char('1')) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_preview1UploadFinished(Attica::BaseJob *)));
    } else if (index == QLatin1Char('2')) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_preview2UploadFinished(Attica::BaseJob *)));
    } else if (index == QLatin1Char('3')) {
        job = currentProvider().setPreviewImage(contentId, index, fileName, fileContents);
        q->connect(job, SIGNAL(finished(Attica::BaseJob *)), q, SLOT(_k_preview3UploadFinished(Attica::BaseJob *)));
    }
    if (job) {
        job->start();
    }
}

void UploadDialogPrivate::_k_fileUploadFinished(Attica::BaseJob *)
{
    ui.uploadContentImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
    finishedContents = true;
    uploadFileFinished();
}

void UploadDialogPrivate::_k_preview1UploadFinished(Attica::BaseJob *)
{
    ui.uploadPreview1ImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
    finishedPreview1 = true;
    uploadFileFinished();
}

void UploadDialogPrivate::_k_preview2UploadFinished(Attica::BaseJob *)
{
    ui.uploadPreview2ImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
    finishedPreview2 = true;
    uploadFileFinished();
}

void UploadDialogPrivate::_k_preview3UploadFinished(Attica::BaseJob *)
{
    ui.uploadPreview3ImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
    finishedPreview3 = true;
    uploadFileFinished();
}

void UploadDialogPrivate::uploadFileFinished()
{
    // FIXME multiple previews
    if (finishedContents && (previewFile1.isEmpty() || finishedPreview1) && (previewFile2.isEmpty() || finishedPreview2)
        && (previewFile3.isEmpty() || finishedPreview3)) {
        finished = true;
        ui.uploadProgressBar->setMinimum(0);
        ui.uploadProgressBar->setMaximum(100);
        ui.uploadProgressBar->setValue(100);
        _k_updatePage();
    }
}

void UploadDialogPrivate::_k_detailsLinkLoaded(const QUrl &url)
{
    ui.contentWebsiteLink->setText(QLatin1String("<a href=\"") + url.toString() + QLatin1String("\">")
                                   + i18nc("A link to the website where the get hot new stuff upload can be seen", "Visit website") + QLatin1String("</a>"));
    ui.fetchContentLinkImageLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(16));
}

QStringList UploadDialogPrivate::_supportedMimeTypes() const
{
    QStringList mimeTypes;
    const QList<QByteArray> supported = QImageReader::supportedMimeTypes();
    mimeTypes.reserve(supported.count());
    for (const QByteArray &mimeType : supported) {
        mimeTypes.append(QString::fromLatin1(mimeType));
    }
    return mimeTypes;
}

#include "moc_uploaddialog.cpp"
