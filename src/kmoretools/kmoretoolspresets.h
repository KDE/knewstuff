/*
    Copyright 2015 by Gregor Mi <codestruct@posteo.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KMORETOOLSPRESETS_H
#define KMORETOOLSPRESETS_H

#include <QString>
#include <QUrl>
#include <QMenu>

#include "knewstuff_export.h"

class KMoreTools;
class KMoreToolsService;

/**
 * Provides static methods to make use of all desktop files provided by
 * KMoreTools.
 *
 * registerServiceByDesktopEntryName creates a KMoreToolsService instance
 * from a given desktopEntryName.
 *
 * registerServicesByGroupingNames takes a list of a set of predefined
 * grouping names and returns a list KMoreToolsService instances. Remember,
 * a KMoreToolsService represents a service which might or might not be
 * installed on the current maschine.
 *
 * The groupings defined here are used for the KMoreToolsMenuFactory.
 *
 * @since 5.10
 */
class KNEWSTUFF_EXPORT KMoreToolsPresets
{
public:
    /**
     * @Returns an _ordered_ list of KMoreToolsService instances
     *
     * Available grouping names (in alphabetical order):
     *
     * - "disk-usage"
     *      disk usage tools as currently used in dolphin
     *
     * - "disk-partitions"
     *      disk partition tools as currently used in dolphin
     *
     * - "git-clients"
     *      collection of git clients
     *
     * - "git-clients-and-actions"
     *      git clients and actions to be used in a file tree context menu
     *      (e.g. in kate's project plugin)
     *
     * - "screenshot-take"
     *      collection of screenshot-taking tools
     *
     * todo: handle overlapping groupings
     * todo later: additional groupings: "screenshot-edit", "textfile-edit" etc.
     */
    static QList<KMoreToolsService*> registerServicesByGroupingNames(
        KMoreTools* kmt, const QStringList& groupingNames);

    /**
     * Registers a service who's kmt-desktopfile is provided by the
     * KMoreTools library itself (see directory kmoretools-desktopfiles).
     * If the kmt-desktopfile is missing the service is still created
     * but with no translations and icon if the service is not installed.
     *
     * Associates a homepage URL because a regular .desktop file has got
     * no field for this information.
     *
     * Adds some corrections to faulty upstream .desktop files. Corrected
     * desktop filenames end with .kmt-edition.desktop.
     *
     * todo: how to avoid the "Do you trust this program?" question when a
     * non-installed kmt-edition desktopfile is used but the program is installed?
     * Possible solution: install all .kmt-edition files to proper desktop
     * file location.
     *
     * @returns the added KMoreToolsService
     */
    static KMoreToolsService* registerServiceByDesktopEntryName(
        KMoreTools* kmt, const QString& desktopEntryName);

    // todo later: add another method registerServiceByDesktopEntryNames (plural) that handles
    // a list of desktopEntryNames.
};

#endif
