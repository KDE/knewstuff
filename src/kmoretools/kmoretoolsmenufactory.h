/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KMORETOOLSMENUFACTORY_H
#define KMORETOOLSMENUFACTORY_H

#include <QMenu>
#include <QString>
#include <QUrl>

#include <memory>

#include "KNS3/knewstuff_export.h"

class KMoreTools;
class KMoreToolsMenuFactoryPrivate;

/**
 * This is the class with the highest abstraction in KMoreTools.
 *
 * Creates a QMenu from a list of grouping names. For details on available
 * grouping names see KMoreToolsPresets::registerServicesByGroupingNames.
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreToolsMenuFactory
{
public:
    /**
     * @param uniqueId defines the config section name where the user
     * settings done by the Configure dialog will be stored.
     *
     * For more information about the parameter see KMoreTools::KMoreTools.
     */
    KMoreToolsMenuFactory(const QString &uniqueId);

    ~KMoreToolsMenuFactory();

    KMoreToolsMenuFactory(const KMoreToolsMenuFactory &) = delete;
    KMoreToolsMenuFactory &operator=(const KMoreToolsMenuFactory &) = delete;

    /**
     * For each grouping name menu items will be created an appended to a
     * lazy menu which is returned. The menu is lazy in a sense that the
     * items are not added until the menu is about to be shown.
     * NOTE: This means if the menu is not shown (as would be by e.g.
     * calling exec()) then the menu stays empty.
     *
     * For details on available grouping names see
     * KMoreToolsPresets::registerServicesByGroupingNames.
     *
     * For each grouping name there might be special handlings that take the
     * optional given @p url into account if needed. By default the url is
     * empty.
     *
     * Furthermore, some selected menu items will be put into the "More"
     * menu section by default.
     *
     * The "more:" grouping name
     * -------------------------
     * There is a special grouping name "more:" (note the colon). If this name
     * is given in the list, all further groupings are put into the More
     * section by default.
     *
     * NOTE that this method overrides a previously created QMenu* instance
     * of the same KMoreToolsMenuFactory instance. The reason is that the
     * internal KMoreTools pointer is reused.
     * (todo: solve this or rename the class?)
     *
     * @returns the created QMenu which includes a Main and (maybe) a More
     * section and an item that starts configure dialog where the user can
     * configure the menu (see KMoreTools).
     */
    QMenu *createMenuFromGroupingNames(const QStringList &groupingNames, const QUrl &url = QUrl());

    /**
     * See createMenuFromGroupingNames except that the menu is not created
     * but you have to provide one yourself. This is useful to create
     * lazy menus by connecting QMenu::aboutToShow.
     *
     * WARN 1: KMoreToolsMenuFactory must live as long as you would like to use
     * the menu.
     *
     * WARN 2: You must NOT reuse an existing KMoreToolsMenuFactory instance
     * to create a different menu.
     *
     * @since 5.11
     */
    void fillMenuFromGroupingNames(QMenu *menu, const QStringList &groupingNames, const QUrl &url = QUrl());

    /**
     * Set @p widget as the parent widget of the QMenu that will be created
     * by createMenuFromGroupingNames().
     * @see createMenuFromGroupingNames()
     * @since 5.37
     */
    void setParentWidget(QWidget *widget);

private:
    /*
     * TODO KF6: Not used, remove in the KF6 transition.
     * Preserves object size to counteract BIC introduced with
     * 3ecc3701f7e1aa83104b06fa90ea07eeca47f93d.
     */
    KMoreTools *m_off = nullptr;

    const std::unique_ptr<KMoreToolsMenuFactoryPrivate> d;
};

#endif
