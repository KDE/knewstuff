/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KMORETOOLSPRESETS_P_H
#define KMORETOOLSPRESETS_P_H

class KMoreToolsPresetsPrivate
{
public:
    /**
     * Same as KMoreToolsPresets::registerServicesByGroupingNames
     * but sets @p firstMoreSectionDesktopEntryName to the desktop entry name
     * of the first service that should be in the more section by default.
     * If there is no such service @p firstMoreSectionDesktopEntryName
     * will not be changed.
     */
    static QList<KMoreToolsService *>
    registerServicesByGroupingNames(QString *firstMoreSectionDesktopEntryName, KMoreTools *kmt, const QStringList &groupingNames);
};

#endif
