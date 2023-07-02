/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "entrydetailsdialog_p.h"

#include <KLocalizedString>
#include <QMenu>
#include <knewstuff_debug.h>

#include "core/engine.h"
#include "core/imageloader_p.h"
#include <attica/provider.h>

using namespace KNS3;

EntryDetails::EntryDetails(KNSCore::Engine *engine, Ui::DownloadWidget *widget)
    : QObject(widget->m_listView)
    , m_engine(engine)
    , ui(widget)
{
    init();
}

EntryDetails::~EntryDetails()
{
}

void EntryDetails::init()
{
    connect(ui->preview1, &ImagePreviewWidget::clicked, this, &EntryDetails::preview1Selected);
    connect(ui->preview2, &ImagePreviewWidget::clicked, this, &EntryDetails::preview2Selected);
    connect(ui->preview3, &ImagePreviewWidget::clicked, this, &EntryDetails::preview3Selected);

    ui->ratingWidget->setMaxRating(10);
    ui->ratingWidget->setHalfStepsEnabled(true);

    updateButtons();
    connect(ui->installButton, &QAbstractButton::clicked, this, &EntryDetails::install);
    connect(ui->uninstallButton, &QAbstractButton::clicked, this, &EntryDetails::uninstall);
    // updating is the same as installing
    connect(ui->updateButton, &QAbstractButton::clicked, this, &EntryDetails::install);
    connect(ui->becomeFanButton, &QAbstractButton::clicked, this, &EntryDetails::becomeFan);

    ui->installButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    ui->updateButton->setIcon(QIcon::fromTheme(QStringLiteral("system-software-update")));
    ui->uninstallButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));

    connect(m_engine, &KNSCore::Engine::signalEntryEvent, this, [this](const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::EntryEvent event) {
        if (event == KNSCore::EntryInternal::DetailsLoadedEvent) {
            Q_EMIT entryChanged(entry);
        } else if (event == KNSCore::EntryInternal::StatusChangedEvent) {
            updateButtons();
        }
    });
    connect(m_engine, &KNSCore::Engine::signalEntryPreviewLoaded, this, &EntryDetails::slotEntryPreviewLoaded);
}

void EntryDetails::setEntry(const KNSCore::EntryInternal &entry)
{
    m_entry = entry;
    // immediately show something
    entryChanged(m_entry);
    // fetch more preview images
    m_engine->loadDetails(m_entry);
}

void EntryDetails::entryChanged(const KNSCore::EntryInternal &entry)
{
    if (ui->detailsStack->currentIndex() == 0) {
        return;
    }
    m_entry = entry;

    // FIXME
    // ui->ratingWidget->setEditable(m_engine->userCanVote(m_entry));

    if (!m_engine->userCanBecomeFan(m_entry)) {
        ui->becomeFanButton->setEnabled(false);
    }

    ui->m_titleWidget->setText(i18n("Details for %1", m_entry.name()));
    if (!m_entry.author().homepage().isEmpty()) {
        ui->authorLabel->setText(QLatin1String("<a href=\"") + m_entry.author().homepage() + QLatin1String("\">") + m_entry.author().name()
                                 + QLatin1String("</a>"));
    } else if (!m_entry.author().email().isEmpty()) {
        ui->authorLabel->setText(QLatin1String("<a href=\"mailto:") + m_entry.author().email() + QLatin1String("\">") + m_entry.author().name()
                                 + QLatin1String("</a>"));
    } else {
        ui->authorLabel->setText(m_entry.author().name());
    }

    QString summary = KNSCore::replaceBBCode(m_entry.summary()).replace(QLatin1Char('\n'), QLatin1String("<br/>"));
    QString changelog = KNSCore::replaceBBCode(m_entry.changelog()).replace(QLatin1Char('\n'), QLatin1String("<br/>"));

    QString description = QLatin1String("<html><body>") + summary;
    if (!changelog.isEmpty()) {
        description += QLatin1String("<br/><p><b>") + i18n("Changelog:") + QLatin1String("</b><br/>") + changelog + QLatin1String("</p>");
    }
    description += QLatin1String("</body></html>");
    ui->descriptionLabel->setText(description);

    QString homepageText(QLatin1String("<a href=\"") + m_entry.homepage().url() + QLatin1String("\">")
                         + i18nc("A link to the description of this Get Hot New Stuff item", "Homepage") + QStringLiteral("</a>"));

    if (!m_entry.donationLink().isEmpty()) {
        homepageText += QLatin1String("<br><a href=\"") + m_entry.donationLink() + QLatin1String("\">")
            + i18nc("A link to make a donation for a Get Hot New Stuff item (opens a web browser)", "Make a donation") + QLatin1String("</a>");
    }
    if (!m_entry.knowledgebaseLink().isEmpty()) {
        homepageText += QLatin1String("<br><a href=\"") + m_entry.knowledgebaseLink() + QLatin1String("\">")
            + i18ncp("A link to the knowledgebase (like a forum) (opens a web browser)",
                     "Knowledgebase (no entries)",
                     "Knowledgebase (%1 entries)",
                     m_entry.numberKnowledgebaseEntries())
            + QStringLiteral("</a>");
    }
    ui->homepageLabel->setText(homepageText);
    ui->homepageLabel->setToolTip(i18nc("Tooltip for a link in a dialog", "Opens in a browser window"));

    if (m_entry.rating() > 0) {
        ui->ratingWidget->setVisible(true);
        disconnect(ui->ratingWidget, &KRatingWidget::ratingChanged, this, &EntryDetails::ratingChanged);
        // Most of the voting is 20 - 80, so rate 20 as 0 stars and 80 as 5 stars
        int rating = qMax(0, qMin(10, (m_entry.rating() - 20) / 6));
        ui->ratingWidget->setRating(rating);
        connect(ui->ratingWidget, &KRatingWidget::ratingChanged, this, &EntryDetails::ratingChanged);
    } else {
        ui->ratingWidget->setVisible(false);
    }

    bool hideSmallPreviews =
        m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall2).isEmpty() && m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall3).isEmpty();

    ui->preview1->setVisible(!hideSmallPreviews);
    ui->preview2->setVisible(!hideSmallPreviews);
    ui->preview3->setVisible(!hideSmallPreviews);

    // in static xml we often only get a small preview, use that in details
    if (m_entry.previewUrl(KNSCore::EntryInternal::PreviewBig1).isEmpty() && !m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
        m_entry.setPreviewUrl(m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1), KNSCore::EntryInternal::PreviewBig1);
        m_entry.setPreviewImage(m_entry.previewImage(KNSCore::EntryInternal::PreviewSmall1), KNSCore::EntryInternal::PreviewBig1);
    }

    for (int type = KNSCore::EntryInternal::PreviewSmall1; type <= KNSCore::EntryInternal::PreviewBig3; ++type) {
        if (m_entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
            ui->previewBig->setVisible(false);
        } else

            if (!m_entry.previewUrl((KNSCore::EntryInternal::PreviewType)type).isEmpty()) {
            qCDebug(KNEWSTUFF) << "type: " << type << m_entry.previewUrl((KNSCore::EntryInternal::PreviewType)type);
            if (m_entry.previewImage((KNSCore::EntryInternal::PreviewType)type).isNull()) {
                m_engine->loadPreview(m_entry, (KNSCore::EntryInternal::PreviewType)type);
            } else {
                slotEntryPreviewLoaded(m_entry, (KNSCore::EntryInternal::PreviewType)type);
            }
        }
    }

    updateButtons();
}

void EntryDetails::updateButtons()
{
    if (ui->detailsStack->currentIndex() == 0) {
        return;
    }
    qCDebug(KNEWSTUFF) << "update buttons: " << m_entry.status();
    ui->installButton->setVisible(false);
    ui->uninstallButton->setVisible(false);
    ui->updateButton->setVisible(false);

    switch (m_entry.status()) {
    case Entry::Installed:
        ui->uninstallButton->setVisible(true);
        ui->uninstallButton->setEnabled(true);
        break;
    case Entry::Updateable:
        ui->updateButton->setVisible(true);
        ui->updateButton->setEnabled(true);
        ui->uninstallButton->setVisible(true);
        ui->uninstallButton->setEnabled(true);
        break;

    case Entry::Invalid:
    case Entry::Downloadable:
        ui->installButton->setVisible(true);
        ui->installButton->setEnabled(true);
        break;

    case Entry::Installing:
        ui->installButton->setVisible(true);
        ui->installButton->setEnabled(false);
        break;
    case Entry::Updating:
        ui->updateButton->setVisible(true);
        ui->updateButton->setEnabled(false);
        ui->uninstallButton->setVisible(true);
        ui->uninstallButton->setEnabled(false);
        break;
    case Entry::Deleted:
        ui->installButton->setVisible(true);
        ui->installButton->setEnabled(true);
        break;
    }

    if (QMenu *buttonMenu = ui->installButton->menu()) {
        buttonMenu->clear();
        ui->installButton->setMenu(nullptr);
        buttonMenu->deleteLater();
    }
    if (ui->installButton->isVisible() && m_entry.downloadLinkCount() > 1) {
        QMenu *installMenu = new QMenu(ui->installButton);
        const auto lst = m_entry.downloadLinkInformationList();
        for (const KNSCore::EntryInternal::DownloadLinkInformation &info : lst) {
            QString text = info.name;
            if (!info.distributionType.trimmed().isEmpty()) {
                text + QStringLiteral(" (") + info.distributionType.trimmed() + QLatin1Char(')');
            }
            QAction *installMenuAction = installMenu->addAction(QIcon::fromTheme(QStringLiteral("dialog-ok")), text);
            installMenuAction->setData(info.id);
            connect(installMenuAction, &QAction::triggered, this, [this, installMenuAction]() {
                installAction(installMenuAction);
            });
        }
        qCDebug(KNEWSTUFF) << "links: " << m_entry.downloadLinkInformationList().size();
        ui->installButton->setMenu(installMenu);
    }
}

void EntryDetails::install()
{
    m_engine->install(m_entry);
}

void EntryDetails::uninstall()
{
    m_engine->uninstall(m_entry);
}

void EntryDetails::slotEntryPreviewLoaded(const KNSCore::EntryInternal &entry, KNSCore::EntryInternal::PreviewType type)
{
    if (!(entry == m_entry)) {
        return;
    }

    switch (type) {
    case KNSCore::EntryInternal::PreviewSmall1:
        ui->preview1->setImage(entry.previewImage(KNSCore::EntryInternal::PreviewSmall1));
        break;
    case KNSCore::EntryInternal::PreviewSmall2:
        ui->preview2->setImage(entry.previewImage(KNSCore::EntryInternal::PreviewSmall2));
        break;
    case KNSCore::EntryInternal::PreviewSmall3:
        ui->preview3->setImage(entry.previewImage(KNSCore::EntryInternal::PreviewSmall3));
        break;
    case KNSCore::EntryInternal::PreviewBig1:
        m_currentPreview = entry.previewImage(KNSCore::EntryInternal::PreviewBig1);
        ui->previewBig->setImage(m_currentPreview);
        break;
    default:
        break;
    }
}

void EntryDetails::preview1Selected()
{
    previewSelected(0);
}

void EntryDetails::preview2Selected()
{
    previewSelected(1);
}

void EntryDetails::preview3Selected()
{
    previewSelected(2);
}

void EntryDetails::previewSelected(int current)
{
    KNSCore::EntryInternal::PreviewType type = static_cast<KNSCore::EntryInternal::PreviewType>(KNSCore::EntryInternal::PreviewBig1 + current);
    m_currentPreview = m_entry.previewImage(type);
    ui->previewBig->setImage(m_currentPreview);
}

void EntryDetails::ratingChanged(int rating)
{
    // engine expects values from 0..100
    qCDebug(KNEWSTUFF) << "rating: " << rating << " -> " << rating * 10;
    m_engine->vote(m_entry, rating * 10);
}

void EntryDetails::becomeFan()
{
    m_engine->becomeFan(m_entry);
}

void EntryDetails::installAction(QAction *action)
{
    m_engine->install(m_entry, action->data().toInt());
}

#include "moc_entrydetailsdialog_p.cpp"
