/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2010 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "itemsviewdelegate_p.h"

#include <QApplication>
#include <QMenu>
#include <QPainter>
#include <QProcess>
#include <QToolButton>
#include <knewstuff_debug.h>

#include <KFormat>
#include <KLocalizedString>
#include <KRatingWidget>
#include <KShell>

#include "core/itemsmodel.h"

#include "entrydetailsdialog_p.h"

namespace KNS3
{
enum { DelegateLabel, DelegateInstallButton, DelegateDetailsButton, DelegateRatingWidget };

ItemsViewDelegate::ItemsViewDelegate(QAbstractItemView *itemView, KNSCore::Engine *engine, QObject *parent)
    : ItemsViewBaseDelegate(itemView, engine, parent)
{
}

ItemsViewDelegate::~ItemsViewDelegate()
{
}

QList<QWidget *> ItemsViewDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> list;

    QLabel *infoLabel = new QLabel();
    infoLabel->setOpenExternalLinks(true);
    // not so nice - work around constness to install the event filter
    ItemsViewDelegate *delegate = const_cast<ItemsViewDelegate *>(this);
    infoLabel->installEventFilter(delegate);
    list << infoLabel;

    QToolButton *installButton = new QToolButton();
    installButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    list << installButton;
    setBlockedEventTypes(installButton, QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(installButton, &QAbstractButton::clicked, this, &ItemsViewDelegate::slotInstallClicked);
    connect(installButton, &QToolButton::triggered, this, &ItemsViewDelegate::slotInstallActionTriggered);

    QToolButton *detailsButton = new QToolButton();
    detailsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    list << detailsButton;
    setBlockedEventTypes(detailsButton, QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick);
    connect(detailsButton, &QToolButton::clicked, this, static_cast<void (ItemsViewDelegate::*)()>(&ItemsViewDelegate::slotDetailsClicked));

    KRatingWidget *rating = new KRatingWidget();
    rating->setMaxRating(10);
    rating->setHalfStepsEnabled(true);
    list << rating;
    const KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
    connect(rating, &KRatingWidget::ratingChanged, this, [this, entry](int newRating) {
        m_engine->vote(entry, newRating * 10);
    });

    return list;
}

void ItemsViewDelegate::updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    const KNSCore::ItemsModel *model = qobject_cast<const KNSCore::ItemsModel *>(index.model());
    if (!model) {
        qCDebug(KNEWSTUFF) << "WARNING - INVALID MODEL!";
        return;
    }

    const KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();

    // setup the install button
    int margin = option.fontMetrics.height() / 2;
    int right = option.rect.width();

    QToolButton *installButton = qobject_cast<QToolButton *>(widgets.at(DelegateInstallButton));
    if (installButton) {
        if (installButton->menu()) {
            QMenu *buttonMenu = installButton->menu();
            buttonMenu->clear();
            installButton->setMenu(nullptr);
            buttonMenu->deleteLater();
        }

        bool installable = false;
        bool enabled = true;
        QString text;
        QIcon icon;

        switch (entry.status()) {
        case Entry::Installed:
            text = i18n("Uninstall");
            icon = m_iconDelete;
            break;
        case Entry::Updateable:
            text = i18n("Update");
            icon = m_iconUpdate;
            installable = true;
            break;
        case Entry::Installing:
            text = i18n("Installing");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case Entry::Updating:
            text = i18n("Updating");
            enabled = false;
            icon = m_iconUpdate;
            break;
        case Entry::Downloadable:
            text = i18n("Install");
            icon = m_iconInstall;
            installable = true;
            break;
        case Entry::Deleted:
            text = i18n("Install Again");
            icon = m_iconInstall;
            installable = true;
            break;
        default:
            text = i18n("Install");
        }
        installButton->setText(text);
        installButton->setEnabled(enabled);
        installButton->setIcon(icon);
        installButton->setPopupMode(QToolButton::InstantPopup);

        // If there are multiple files we want to show a dropdown, but not if it is just an update
        if (installable && entry.downloadLinkCount() > 1 && entry.status() != Entry::Updateable) {
            QMenu *installMenu = new QMenu(installButton);
            const auto lst = entry.downloadLinkInformationList();
            for (const KNSCore::EntryInternal::DownloadLinkInformation &info : lst) {
                QString text = info.name;
                if (!info.distributionType.trimmed().isEmpty()) {
                    text += QLatin1String(" (") + info.distributionType.trimmed() + QLatin1Char(')');
                }
                QAction *installAction = installMenu->addAction(m_iconInstall, text);
                installAction->setData(QPoint(index.row(), info.id));
            }
            installButton->setMenu(installMenu);
        } else if (entry.status() == Entry::Installed && m_engine->hasAdoptionCommand()) {
            QMenu *m = new QMenu(installButton);
            // Add icon to use dropdown, see also BUG: 385858
            QAction *action = m->addAction(QIcon::fromTheme(QStringLiteral("checkmark")), m_engine->useLabel());
            connect(action, &QAction::triggered, m, [this, entry](bool) {
                m_engine->adoptEntry(entry);
            });
            installButton->setPopupMode(QToolButton::MenuButtonPopup);
            installButton->setMenu(m);
        }
        // Add uninstall option for updatable entries, BUG: 422047
        if (entry.status() == Entry::Updateable) {
            QMenu *m = installButton->menu();
            if (!m) {
                m = new QMenu(installButton);
            }
            QAction *action = m->addAction(m_iconDelete, i18n("Uninstall"));
            connect(action, &QAction::triggered, m, [this, entry](bool) {
                m_engine->uninstall(entry);
            });
            installButton->setPopupMode(QToolButton::MenuButtonPopup);
            installButton->setMenu(m);
        }
    }

    QToolButton *detailsButton = qobject_cast<QToolButton *>(widgets.at(DelegateDetailsButton));
    if (detailsButton) {
        detailsButton->setText(i18n("Details"));
        detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("documentinfo")));
    }

    if (installButton && detailsButton) {
        if (m_buttonSize.width() < installButton->sizeHint().width()) {
            const_cast<QSize &>(m_buttonSize) =
                QSize(qMax(option.fontMetrics.height() * 7, qMax(installButton->sizeHint().width(), detailsButton->sizeHint().width())),
                      installButton->sizeHint().height());
        }
        installButton->resize(m_buttonSize);
        installButton->move(right - installButton->width() - margin, option.rect.height() / 2 - installButton->height() * 1.5);
        detailsButton->resize(m_buttonSize);
        detailsButton->move(right - installButton->width() - margin, option.rect.height() / 2 - installButton->height() / 2);
    }

    QLabel *infoLabel = qobject_cast<QLabel *>(widgets.at(DelegateLabel));
    if (infoLabel != nullptr) {
        infoLabel->setWordWrap(true);
        if (model->hasPreviewImages()) {
            // move the text right by kPreviewWidth + margin pixels to fit the preview
            infoLabel->move(KNSCore::PreviewWidth + margin * 2, 0);
            infoLabel->resize(QSize(option.rect.width() - KNSCore::PreviewWidth - (margin * 6) - m_buttonSize.width(), option.fontMetrics.height() * 7));

        } else {
            infoLabel->move(margin, 0);
            infoLabel->resize(QSize(option.rect.width() - (margin * 4) - m_buttonSize.width(), option.fontMetrics.height() * 7));
        }

        QString text = QStringLiteral(
            "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">p, li { white-space: pre-wrap; margin:0 0 0 0;}\n"
            "</style></head><body><p><b>");

        QUrl link = qvariant_cast<QUrl>(entry.homepage());
        if (!link.isEmpty()) {
            text += QLatin1String("<p><a href=\"") + link.url() + QLatin1String("\">") + entry.name() + QLatin1String("</a></p>\n");
        } else {
            text += entry.name();
        }

        const auto downloadInfo = entry.downloadLinkInformationList();
        if (!downloadInfo.isEmpty() && downloadInfo.at(0).size > 0) {
            QString sizeString = KFormat().formatByteSize(downloadInfo.at(0).size * 1000);
            text += i18nc("Show the size of the file in a list", "<p>Size: %1</p>", sizeString);
        }

        text += QLatin1String("</b></p>\n");

        QString authorName = entry.author().name();
        QString email = entry.author().email();
        QString authorPage = entry.author().homepage();

        if (!authorName.isEmpty()) {
            if (!authorPage.isEmpty()) {
                text += QLatin1String("<p>")
                    + i18nc("Show the author of this item in a list",
                            "By <i>%1</i>",
                            QLatin1String(" <a href=\"") + authorPage + QLatin1String("\">") + authorName + QLatin1String("</a>"))
                    + QLatin1String("</p>\n");
            } else if (!email.isEmpty()) {
                text += QLatin1String("<p>") + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + QLatin1String(" <a href=\"mailto:")
                    + email + QLatin1String("\">") + email + QLatin1String("</a></p>\n");
            } else {
                text += QLatin1String("<p>") + i18nc("Show the author of this item in a list", "By <i>%1</i>", authorName) + QLatin1String("</p>\n");
            }
        }

        QString summary =
            QLatin1String("<p>") + option.fontMetrics.elidedText(entry.summary(), Qt::ElideRight, infoLabel->width() * 3) + QStringLiteral("</p>\n");
        text += summary;

        unsigned int fans = entry.numberFans();
        unsigned int downloads = entry.downloadCount();

        QString fanString;
        QString downloadString;
        if (fans > 0) {
            fanString = i18ncp("fan as in supporter", "1 fan", "%1 fans", fans);
        }
        if (downloads > 0) {
            downloadString = i18np("1 download", "%1 downloads", downloads);
        }
        if (downloads > 0 || fans > 0) {
            text += QLatin1String("<p>") + downloadString;
            if (downloads > 0 && fans > 0) {
                text += QLatin1String(", ");
            }
            text += fanString + QLatin1String("</p>\n");
        }

        text += QLatin1String("</body></html>");
        // use simplified to get rid of newlines etc
        text = KNSCore::replaceBBCode(text).simplified();
        infoLabel->setText(text);
    }

    KRatingWidget *rating = qobject_cast<KRatingWidget *>(widgets.at(DelegateRatingWidget));
    if (rating) {
        if (entry.rating() > 0) {
            rating->setToolTip(i18n("Rating: %1%", entry.rating()));
            // Don't attempt to send a rating to the server if we're just updating the UI
            rating->blockSignals(true);
            // assume all entries come with rating 0..100 but most are in the range 20 - 80, so 20 is 0 stars, 80 is 5 stars
            rating->setRating((entry.rating() - 20) * 10 / 60);
            rating->blockSignals(false);
            // put the rating label below the install button
            rating->move(right - installButton->width() - margin, option.rect.height() / 2 + installButton->height() / 2);
            rating->resize(m_buttonSize);
        } else {
            rating->setVisible(false);
        }
    }
}

// draws the preview
void ItemsViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int margin = option.fontMetrics.height() / 2;

    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->setPen(QPen(option.palette.highlightedText().color()));
    } else {
        painter->setPen(QPen(option.palette.text().color()));
    }

    const KNSCore::ItemsModel *realmodel = qobject_cast<const KNSCore::ItemsModel *>(index.model());

    if (realmodel->hasPreviewImages()) {
        int height = option.rect.height();
        QPoint point(option.rect.left() + margin, option.rect.top() + ((height - KNSCore::PreviewHeight) / 2));

        KNSCore::EntryInternal entry = index.data(Qt::UserRole).value<KNSCore::EntryInternal>();
        if (entry.previewUrl(KNSCore::EntryInternal::PreviewSmall1).isEmpty()) {
            // paint the no preview icon
            // point.setX((PreviewWidth - m_noImage.width())/2 + 5);
            // point.setY(option.rect.top() + ((height - m_noImage.height()) / 2));
            // painter->drawPixmap(point, m_noImage);
        } else {
            QImage image = entry.previewImage(KNSCore::EntryInternal::PreviewSmall1);
            if (!image.isNull()) {
                point.setX((KNSCore::PreviewWidth - image.width()) / 2 + 5);
                point.setY(option.rect.top() + ((height - image.height()) / 2));
                painter->drawImage(point, image);

                QPoint framePoint(point.x() - 5, point.y() - 5);
                if (m_frameImage.isNull()) {
                    painter->drawPixmap(framePoint, m_frameImage);
                } else {
                    painter->drawPixmap(framePoint, m_frameImage.scaled(image.width() + 10, image.height() + 10));
                }
            } else {
                QRect rect(point, QSize(KNSCore::PreviewWidth, KNSCore::PreviewHeight));
                painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, i18n("Loading Preview"));
            }
        }
    }
    painter->restore();
}

QSize ItemsViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QSize size;

    size.setWidth(option.fontMetrics.height() * 4);
    size.setHeight(qMax(option.fontMetrics.height() * 7, KNSCore::PreviewHeight)); // up to 6 lines of text, and two margins
    return size;
}

} // namespace

#include "moc_itemsviewdelegate_p.cpp"
