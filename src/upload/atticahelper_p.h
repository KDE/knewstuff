/*
    Copyright (C) 2010 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef KNEWSTUFF3_ATTICAHELPER_P_H
#define KNEWSTUFF3_ATTICAHELPER_P_H

#include <QStringList>

#include <attica/providermanager.h>
#include <attica/provider.h>

#include <attica/category.h>
#include <attica/content.h>
#include <attica/license.h>

#include "knewstuffcore_export.h"

class KJob;

namespace KNSCore
{
class HTTPJob;
/**
 * @short Upload helper for Attica based providers
 *
 * Use this class to help you build upload functionality into applications
 * which do not fit the KNS3::UploadDialog use case, such as situations where
 * you have to fit the uploading into the middle of another workflow.
 *
 * As uploading is not entirely trivial, we suggest you look at the code in
 * uploaddialog.cpp to see an example of an actual implementation.
 *
 * @see KNS3::UploadDialog
 */
class KNEWSTUFFCORE_EXPORT AtticaHelper : public QObject
{
    Q_OBJECT
public:
    explicit AtticaHelper(QObject *parent = nullptr);
    void init();

    void setCurrentProvider(const QString &provider);
    void addProviderFile(const QUrl &file);

    Attica::Provider provider();

    void checkLogin(const QString &name, const QString &password);
    bool loadCredentials(QString &name, QString &password);
    bool saveCredentials(const QString &name, const QString &password);
    void loadCategories(const QStringList &configuredCategories);
    void loadContentByCurrentUser();
    void loadLicenses();
    void loadDetailsLink(const QString &contentId);
    void loadContent(const QString &contentId);
    void loadCurrency();
    void loadPreviews(const QString &contentId);

Q_SIGNALS:
    void loginChecked(bool);
    void providersLoaded(const QStringList &);
    void categoriesLoaded(Attica::Category::List);
    void contentByCurrentUserLoaded(const Attica::Content::List &);
    void licensesLoaded(const Attica::License::List &);
    void detailsLinkLoaded(const QUrl &);
    void contentLoaded(const Attica::Content &);
    void currencyLoaded(const QString &);
    void previewLoaded(int index, const QImage &image);

private Q_SLOTS:
    void checkLoginFinished(Attica::BaseJob *baseJob);
    void defaultProvidersLoaded();
    void categoriesLoaded(Attica::BaseJob *baseJob);
    void contentByCurrentUserLoaded(Attica::BaseJob *baseJob);
    void licensesLoaded(Attica::BaseJob *baseJob);
    void detailsLinkLoaded(Attica::BaseJob *baseJob);
    void contentLoaded(Attica::BaseJob *baseJob);
    void currencyLoaded(Attica::BaseJob *baseJob);

    void slotPreviewData(KJob *job, const QByteArray &buf);
    void slotPreviewDownload(KJob *job);

private:
    Attica::ProviderManager providerManager;
    Attica::Provider currentProvider;
    Attica::Category::List m_validCategories;

    QString m_username;
    QStringList m_configuredCategories;
    Attica::Content::List m_userCreatedContent;

    QByteArray m_previewBuffer[3];
    HTTPJob *m_previewJob[3];

    Q_DISABLE_COPY(AtticaHelper)
};
}

#endif
