/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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
    KMoreToolsConfigDialog(const KmtMenuStructureDto &defaultStructure, const KmtMenuStructureDto &currentStructure, const QString &title = QString());

    ~KMoreToolsConfigDialog() override;

    /**
     * result after ctor or after user used the dialog
     */
    KmtMenuStructureDto currentStructure();

private:
    KMoreToolsConfigDialogPrivate *d;
};

#endif // KMORETOOLSCONFIGDIALOG_H
