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

#ifndef KMORETOOLSCONFIGDIALOG_H
#define KMORETOOLSCONFIGDIALOG_H

#include <KPageDialog>

#include "kmoretools_p.h"

class KMoreToolsConfigDialogPrivate;

class KMoreToolsConfigDialog : public KPageDialog
{
public:
    /**
     * @param defaultStructure: as defined in calling code; also includes the not-installed items
     * @param configuredStructure: as loaded from config file
     * @param title: optional title
     */
    KMoreToolsConfigDialog(const MenuStructureDto& defaultStructure,
                           const MenuStructureDto& currentStructure,
                           const QString& title = QString());

    ~KMoreToolsConfigDialog();

    /**
     * result after ctor or after user used the dialog
     */
    MenuStructureDto currentStructure();

private:
    KMoreToolsConfigDialogPrivate* d;
};

#endif // KMORETOOLSCONFIGDIALOG_H
