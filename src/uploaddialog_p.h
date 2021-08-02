/*
    SPDX-FileCopyrightText: 2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_UI_UPLOADDIALOG_P_H
#define KNEWSTUFF3_UI_UPLOADDIALOG_P_H

#include "ui_uploaddialog.h"

#include <attica/category.h>
#include <attica/content.h>
#include <attica/license.h>
#include <attica/listjob.h>
#include <attica/postjob.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include <QUrl>

namespace KNS3
{
class UploadDialogPrivate
{
public:
    UploadDialogPrivate(UploadDialog *q)
        : q(q)
    {
    }

    UploadDialog *q;

    bool init(const QString &configfile);
};
}

#endif
