/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kmoretoolsconfigdialog_p.h"

#include "ui_kmoretoolsconfigwidget.h"

#include <QDebug>
#include <QStandardItemModel>

#include <KLocalizedString>

class KMoreToolsConfigDialogPrivate
{
public:
    /**
     * menu defined by code
     */
    KmtMenuStructureDto defaultStructure;

    /**
     * resulting menu (default merged with configured) and then maybe edited via GUI
     */
    KmtMenuStructureDto currentStructure;

    Ui::KMoreToolsConfigWidget* configUi = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    QAction* moveToMoreSectionAction = nullptr;
    QAction* moveToMainSectionAction = nullptr;

public:
    QAction* createActionForButton(QAbstractButton* button, QObject* parent)
    {
        auto action = new QAction(button->icon(), button->text(), parent);
        return action;
    }

    QListWidgetItem* selectedItemMainSection()
    {
        auto items = configUi->listMainSection->selectedItems();
        if (items.isEmpty()) {
            return nullptr;
        } else {
            return items[0];
        }
    }

    QListWidgetItem* selectedItemMoreSection()
    {
        auto items = configUi->listMoreSection->selectedItems();
        if (items.isEmpty()) {
            return nullptr;
        } else {
            return items[0];
        }
    }

    /**
     * Only one section has a selection at a time.
     * @return the id of the item in one of the sections (main or more) or empty string
     */
    QString uiSelectedItemId()
    {
        auto mainItem = selectedItemMainSection();
        auto moreItem = selectedItemMoreSection();
        if (mainItem) {
            return mainItem->data(Qt::UserRole).toString();
        } else if (moreItem) {
            return moreItem->data(Qt::UserRole).toString();
        } else {
            return QString();
        }
    }

    void updateMoveButtonsState()
    {
        bool hasSelectedMain = selectedItemMainSection();
        if (hasSelectedMain) {
            auto listMain = configUi->listMainSection;
            moveUpAction->setEnabled(hasSelectedMain && listMain->currentRow() > 0);
            moveDownAction->setEnabled(hasSelectedMain && listMain->currentRow() < listMain->count() - 1);
        }

        bool hasSelectedMore = selectedItemMoreSection();
        if (hasSelectedMore) {
            auto listMore = configUi->listMoreSection;
            moveUpAction->setEnabled(hasSelectedMore && listMore->currentRow() > 0);
            moveDownAction->setEnabled(hasSelectedMore && listMore->currentRow() < listMore->count() - 1);
        }

        moveToMoreSectionAction->setEnabled(hasSelectedMain);
        moveToMainSectionAction->setEnabled(hasSelectedMore);
    }

    /**
     * refill lists and restore selection
     */
    void updateListViews(const QString &idToSelect = QString())
    {
        configUi->listMainSection->clear();
        configUi->listMoreSection->clear();

        // restore item selection
        QListWidgetItem* mainSelItem = nullptr;
        QListWidgetItem* moreSelItem = nullptr;

        for (const auto &item : qAsConst(currentStructure.list)) {
            QIcon icon = item.icon;
            if (icon.isNull()) {
                QPixmap pix(16, 16); // TODO: should same size as other icons in the listview
                pix.fill(QColor(0, 0, 0, 0)); // transparent
                icon = QIcon(pix);
            }

            if (item.isInstalled) {
                auto listItem = new QListWidgetItem(icon, KmtMenuItemDto::removeMenuAmpersand(item.text) /*+ " - " + item.id*/);
                listItem->setData(Qt::UserRole, item.id);
                if (item.menuSection == KMoreTools::MenuSection_Main) {
                    configUi->listMainSection->addItem(listItem);
                    if (item.id == idToSelect) {
                        mainSelItem = listItem;
                    }
                } else if (item.menuSection == KMoreTools::MenuSection_More) {
                    configUi->listMoreSection->addItem(listItem);
                    if (item.id == idToSelect) {
                        moreSelItem = listItem;
                    }
                } else {
                    Q_ASSERT(false);
                }
            }
        }

        //
        // restore selection
        // "current vs. selected?" see http://doc.qt.digia.com/4.6/model-view-selection.html
        //
        if (mainSelItem) {
            mainSelItem->setSelected(true);
            configUi->listMainSection->setCurrentItem(mainSelItem); // for focus and keyboard handling
            configUi->listMainSection->setFocus();
        }

        if (moreSelItem) {
            moreSelItem->setSelected(true);
            configUi->listMoreSection->setCurrentItem(moreSelItem); // for focus and keyboard handling
            configUi->listMoreSection->setFocus();
        }

        updateMoveButtonsState();
    }
};

/**
 * for merging strategy see KMoreToolsMenuBuilderPrivate::createMenuStructure(mergeWithUserConfig=true)
 */
KMoreToolsConfigDialog::KMoreToolsConfigDialog(const KmtMenuStructureDto& defaultStructure,
        const KmtMenuStructureDto& currentStructure,
        const QString& title)
    : d(new KMoreToolsConfigDialogPrivate())
{
    d->defaultStructure = defaultStructure;
    d->currentStructure = currentStructure;

    QWidget *configPage = new QWidget();
    if (title.isEmpty()) {
        addPage(configPage, i18n("Configure menu"));
    } else {
        addPage(configPage, i18n("Configure menu - %1", title));
    }
    d->configUi = new Ui::KMoreToolsConfigWidget();
    d->configUi->setupUi(configPage);

    //
    // show or don't show not-installed section depending if there are any
    //
    auto notInstalledServices = defaultStructure.notInstalledServices();
    d->configUi->frameNotInstalledTools->setVisible(!notInstalledServices.empty());
    if (!notInstalledServices.empty()) {
        auto menu = new QMenu(this);
        for (const KmtMenuItemDto& registeredService : notInstalledServices) {

            QMenu* submenuForNotInstalled = KmtNotInstalledUtil::createSubmenuForNotInstalledApp(
                registeredService.text, menu, registeredService.icon, registeredService.homepageUrl, registeredService.appstreamId);
            menu->addMenu(submenuForNotInstalled);
        }
        d->configUi->buttonNotInstalledTools->setMenu(menu);
    }

    //
    // connect signals
    //
    {
        auto configUi = d->configUi;

        //
        // actions
        //
        d->moveUpAction = d->createActionForButton(configUi->buttonMoveUp, this);
        d->moveUpAction->setEnabled(false);
        configUi->buttonMoveUp->setDefaultAction(d->moveUpAction);
        connect(d->moveUpAction, &QAction::triggered, this, [this]() {
            QString selectedItemId = d->uiSelectedItemId();
            if (!selectedItemId.isEmpty()) {
                d->currentStructure.moveWithinSection(selectedItemId, -1);
                d->updateListViews(selectedItemId);
            }
        });

        d->moveDownAction = d->createActionForButton(configUi->buttonMoveDown, this);
        d->moveDownAction->setEnabled(false);
        configUi->buttonMoveDown->setDefaultAction(d->moveDownAction);
        connect(d->moveDownAction, &QAction::triggered, this, [this]() {
            QString selectedItemId = d->uiSelectedItemId();
            if (!selectedItemId.isEmpty()) {
                d->currentStructure.moveWithinSection(selectedItemId, 1);
                d->updateListViews(selectedItemId);
            }
        });

        d->moveToMoreSectionAction = d->createActionForButton(configUi->buttonMoveToMore, this);
        d->moveToMoreSectionAction->setEnabled(false);
        configUi->buttonMoveToMore->setDefaultAction(d->moveToMoreSectionAction);
        connect(d->moveToMoreSectionAction, &QAction::triggered, this, [this]() {
            QString selectedItemId = d->selectedItemMainSection()->data(Qt::UserRole).toString();
            d->currentStructure.moveToOtherSection(selectedItemId);
            d->selectedItemMainSection()->setSelected(false);
            d->updateListViews(selectedItemId);
        });

        d->moveToMainSectionAction = d->createActionForButton(configUi->buttonMoveToMain, this);
        d->moveToMainSectionAction->setEnabled(false);
        configUi->buttonMoveToMain->setDefaultAction(d->moveToMainSectionAction);
        connect(d->moveToMainSectionAction, &QAction::triggered, this, [this]() {
            QString selectedItemId = d->selectedItemMoreSection()->data(Qt::UserRole).toString();
            d->currentStructure.moveToOtherSection(selectedItemId);
            d->selectedItemMoreSection()->setSelected(false);
            d->updateListViews(selectedItemId);
        });

        connect(configUi->buttonReset, &QAbstractButton::clicked, this, [this]() {
            d->currentStructure = d->defaultStructure;
            d->updateListViews();
        });

        //
        // widgets enabled or not
        //
        connect(configUi->listMainSection, &QListWidget::itemSelectionChanged, this,
        [this]() {
            if (!d->selectedItemMainSection()) {
                d->moveToMoreSectionAction->setEnabled(false);
                d->moveUpAction->setEnabled(false);
                d->moveDownAction->setEnabled(false);
                return;
            } else {
                d->moveToMoreSectionAction->setEnabled(true);
            }

            d->updateMoveButtonsState();
        });

        connect(configUi->listMainSection, &QListWidget::currentItemChanged, this,
        [this, configUi](QListWidgetItem* current, QListWidgetItem* previous) {
            Q_UNUSED(previous)
            if (current && d->selectedItemMoreSection()) {
                d->selectedItemMoreSection()->setSelected(false);
                configUi->listMoreSection->setCurrentItem(nullptr);
            }
            d->updateMoveButtonsState();
        });

        connect(configUi->listMoreSection, &QListWidget::itemSelectionChanged, this,
        [this]() {
            if (!d->selectedItemMoreSection()) {
                d->moveToMainSectionAction->setEnabled(false);
                d->moveUpAction->setEnabled(false);
                d->moveDownAction->setEnabled(false);
                return;
            } else {
                d->moveToMainSectionAction->setEnabled(true);
            }

            d->updateMoveButtonsState();
        });

        connect(configUi->listMoreSection, &QListWidget::currentItemChanged, this,
        [this, configUi](QListWidgetItem* current, QListWidgetItem* previous) {
            Q_UNUSED(previous)
            if (current && d->selectedItemMainSection()) {
                d->selectedItemMainSection()->setSelected(false);
                configUi->listMainSection->setCurrentItem(nullptr);
            }
            d->updateMoveButtonsState();
        });
    }

    d->updateListViews();
}

KMoreToolsConfigDialog::~KMoreToolsConfigDialog()
{
    delete d;
}

KmtMenuStructureDto KMoreToolsConfigDialog::currentStructure()
{
    return d->currentStructure;
}
