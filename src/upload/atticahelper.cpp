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

#include "atticahelper_p.h"
#include "jobs/httpjob.h"

#include <QImage>

#include <attica/listjob.h>
#include <attica/postjob.h>
#include <attica/accountbalance.h>

using namespace KNSCore;

AtticaHelper::AtticaHelper(QObject *parent) :
    QObject(parent)
{
}

void AtticaHelper::init()
{
    connect(&providerManager, &Attica::ProviderManager::defaultProvidersLoaded, this, &AtticaHelper::defaultProvidersLoaded);
    providerManager.loadDefaultProviders();
}

void AtticaHelper::addProviderFile(const QUrl &file)
{
    if (! providerManager.providerFiles().contains(file)) {
        // If a custom provider file is added, remove all the default ones.
        const auto lstUrl = providerManager.defaultProviderFiles();
        for (const QUrl &url : lstUrl) {
            providerManager.removeProviderFileFromDefaultProviders(url);
        }
        providerManager.addProviderFile(file);
    }
}

void AtticaHelper::defaultProvidersLoaded()
{
    QStringList providers;
    const auto lst = providerManager.providers();
    for (const Attica::Provider &p : lst) {
        if (p.isEnabled()) {
            providers.append(p.name());
        }
    }
    emit providersLoaded(providers);
}

void AtticaHelper::setCurrentProvider(const QString &provider)
{
    const auto lst = providerManager.providers();
    for (const Attica::Provider &p : lst) {
        if (p.name() == provider) {
            currentProvider = p;
            break;
        }
    }
}

Attica::Provider AtticaHelper::provider()
{
    return currentProvider;
}

void AtticaHelper::checkLogin(const QString &name, const QString &password)
{
    Attica::PostJob *checkLoginJob = currentProvider.checkLogin(name, password);
    connect(checkLoginJob, &Attica::BaseJob::finished, this, &AtticaHelper::checkLoginFinished);
    checkLoginJob->start();
}

void AtticaHelper::checkLoginFinished(Attica::BaseJob *baseJob)
{
    emit loginChecked(baseJob->metadata().error() == Attica::Metadata::NoError);
}

bool AtticaHelper::loadCredentials(QString &name, QString &password)
{
    if (currentProvider.isValid() && currentProvider.hasCredentials()) {
        if (currentProvider.loadCredentials(name, password)) {
            m_username = name;
            return true;
        }
    }
    return false;
}

bool AtticaHelper::saveCredentials(const QString &name, const QString &password)
{
    return currentProvider.saveCredentials(name, password);
}

void AtticaHelper::loadCategories(const QStringList &configuredCategories)
{
    m_configuredCategories = configuredCategories;
    Attica::ListJob<Attica::Category> *job = currentProvider.requestCategories();
    connect(job, &Attica::ListJob<Attica::Category>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::categoriesLoaded));
    job->start();
}

void AtticaHelper::categoriesLoaded(Attica::BaseJob *baseJob)
{
    Attica::ListJob<Attica::Category> *listJob = static_cast<Attica::ListJob<Attica::Category>*>(baseJob);
    const Attica::Category::List newCategories = listJob->itemList();

    if (m_configuredCategories.isEmpty()) {
        qWarning() << "No category was set in knsrc file. Adding all categories.";
        for (const Attica::Category &category : newCategories) {
            m_validCategories.append(category);
        }
    } else {
        for (const Attica::Category &category : newCategories) {
            if (m_configuredCategories.contains(category.name())) {
                m_validCategories.append(category);
            }
        }
    }
    emit categoriesLoaded(m_validCategories);
}

void AtticaHelper::loadContentByCurrentUser()
{
    // in case of updates we need the list of stuff that has been uploaded by the user before
    Attica::ListJob<Attica::Content> *userContent = currentProvider.searchContentsByPerson(m_validCategories, m_username);
    connect(userContent, &Attica::ListJob<Attica::Content>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::contentByCurrentUserLoaded));

    userContent->start();
}

void AtticaHelper::contentByCurrentUserLoaded(Attica::BaseJob *baseJob)
{
    Attica::ListJob<Attica::Content> *contentList = static_cast<Attica::ListJob<Attica::Content>*>(baseJob);
    m_userCreatedContent = contentList->itemList();
    emit contentByCurrentUserLoaded(m_userCreatedContent);
}

void AtticaHelper::loadLicenses()
{
    Attica::ListJob<Attica::License> *licenseJob = currentProvider.requestLicenses();
    connect(licenseJob, &Attica::ListJob<Attica::License>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::licensesLoaded));
    licenseJob->start();
}

void AtticaHelper::licensesLoaded(Attica::BaseJob *baseJob)
{
    Attica::ListJob<Attica::License> *licenseList = static_cast<Attica::ListJob<Attica::License>*>(baseJob);
    emit licensesLoaded(licenseList->itemList());
}

void AtticaHelper::loadDetailsLink(const QString &contentId)
{
    Attica::ItemJob<Attica::Content> *contentJob = currentProvider.requestContent(contentId);
    connect(contentJob, &Attica::ItemJob<Attica::Content>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::detailsLinkLoaded));
    contentJob->start();
}

void AtticaHelper::detailsLinkLoaded(Attica::BaseJob *baseJob)
{
    Attica::ItemJob<Attica::Content> *contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);
    Attica::Content content = contentItemJob->result();

    emit detailsLinkLoaded(content.detailpage());
}

void AtticaHelper::loadContent(const QString &contentId)
{
    Attica::ItemJob<Attica::Content> *contentJob = currentProvider.requestContent(contentId);
    connect(contentJob, &Attica::ItemJob<Attica::Content>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::contentLoaded));
    contentJob->start();
}

void AtticaHelper::loadCurrency()
{
    Attica::ItemJob<Attica::AccountBalance> *job = currentProvider.requestAccountBalance();
    connect(job, &Attica::ItemJob<Attica::AccountBalance>::finished,
            this, static_cast<void(AtticaHelper::*)(Attica::BaseJob*)>(&AtticaHelper::currencyLoaded));
    job->start();
}

void AtticaHelper::currencyLoaded(Attica::BaseJob *baseJob)
{
    Attica::ItemJob<Attica::AccountBalance> *balanceJob = static_cast<Attica::ItemJob<Attica::AccountBalance>* >(baseJob);
    Attica::AccountBalance balance = balanceJob->result();
    emit currencyLoaded(balance.currency());
}

void AtticaHelper::contentLoaded(Attica::BaseJob *baseJob)
{
    Attica::ItemJob<Attica::Content> *contentItemJob = static_cast<Attica::ItemJob<Attica::Content>* >(baseJob);

    const Attica::Content content(contentItemJob->result());
    emit contentLoaded(content);

    for (int previewNum = 1; previewNum <= 3; ++previewNum) {
        QUrl url = QUrl::fromUserInput(content.smallPreviewPicture(QString::number(previewNum)));
        if (! url.isEmpty()) {
            m_previewJob[previewNum - 1] = HTTPJob::get(url, KNSCore::NoReload, KNSCore::HideProgressInfo);
            connect(m_previewJob[previewNum - 1], &KJob::result, this, &AtticaHelper::slotPreviewDownload);
            connect(m_previewJob[previewNum - 1], &HTTPJob::data, this, &AtticaHelper::slotPreviewData);
        }
    }
}

void AtticaHelper::slotPreviewData(KJob* job, const QByteArray& buf)
{
    if (job == m_previewJob[0]) {
        m_previewBuffer[0].append(buf);
    } else if (job == m_previewJob[1]) {
        m_previewBuffer[1].append(buf);
    } else if (job == m_previewJob[2]) {
        m_previewBuffer[2].append(buf);
    }
}

void AtticaHelper::slotPreviewDownload(KJob *job)
{
    int previewNum = -1;
    if (job == m_previewJob[0]) {
        previewNum = 1;
    } else if (job == m_previewJob[1]) {
        previewNum = 2;
    } else if (job == m_previewJob[2]) {
        previewNum = 3;
    }
    Q_ASSERT(previewNum != -1);
    if (job->error()) {
        m_previewBuffer[previewNum - 1].clear();
        return;
    }
    QImage image;
    image.loadFromData(m_previewBuffer[previewNum - 1]);
    m_previewBuffer[previewNum - 1].clear();

    emit previewLoaded(previewNum, image);
}

