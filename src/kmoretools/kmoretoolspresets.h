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
 * TODO: doc
 */
class KNEWSTUFF_EXPORT KMoreToolsPresets
{
public:
    /**
     * Registers a service who's kmt-desktopfile is provided by the
     * KMoreTools library itself. If the kmt-desktopfile is missing the
     * service is still created but with no translations and icon if the service
     * is not installed.
     *
     * Sets the homepage URL for your convenience. And add some corrections.
     *
     * todo: how to avoid the "Do you trust this program?" question when a
     * non-installed desktop file is used? Possible solution: install all
     * .kmt-edition files to proper desktop file location.
     *
     *
     * @returns the added KMoreToolsService
     */
    static KMoreToolsService* registerServiceByDesktopEntryName(KMoreTools* kmt, const QString& desktopEntryName);

    /**
     * Makes use of registerServiceByDesktopEntryName.
     *
     * Available groupings:
     * "git-clients"
     * "disk-usage"
     * "disk-partitions"
     * "screenshot-take"
     *
     * todo: handle overlapping groupings
     * todo later: additional groupings: "screenshot-edit, textfile-edit"
     * (todo?: rename "category" to something more suitable like "task", "intent", "purpose" or similar?)
     */
    static QList<KMoreToolsService*> registerServicesByGroupingName(KMoreTools* kmt, const QStringList& groupingNames);
};

#endif
