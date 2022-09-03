/*
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KMORETOOLS_P_H
#define KMORETOOLS_P_H

#include "kmoretools.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QUrl>

#include <KLocalizedString>

#define _ QStringLiteral

/**
 * Makes sure that if the same inputId is given more than once
 * we will get unique IDs.
 *
 * See KMoreToolsTest::testMenuItemIdGen().
 */
class KmtMenuItemIdGen
{
public:
    QString getId(const QString &inputId)
    {
        int postFix = desktopEntryNameUsageMap[inputId];
        desktopEntryNameUsageMap[inputId] = postFix + 1;
        return QStringLiteral("%1%2").arg(inputId).arg(postFix);
    }

    void reset()
    {
        desktopEntryNameUsageMap.clear();
    }

private:
    QMap<QString, int> desktopEntryNameUsageMap;
};

/**
 * A serializeable menu item
 */
class KmtMenuItemDto
{
public:
    QString id;

    /**
     * @note that is might contain an ampersand (&) which may be used for menu items.
     * Remove it with removeMenuAmpersand()
     */
    QString text;

    QIcon icon;

    KMoreTools::MenuSection menuSection;

    bool isInstalled = true;

    /**
     * only used if isInstalled == false
     */
    QUrl homepageUrl;

    QString appstreamId;

public:
    void jsonRead(const QJsonObject &json)
    {
        id = json[_("id")].toString();
        menuSection = json[_("menuSection")].toString() == _("main") ? KMoreTools::MenuSection_Main : KMoreTools::MenuSection_More;
        isInstalled = json[_("isInstalled")].toBool();
    }

    void jsonWrite(QJsonObject &json) const
    {
        json[_("id")] = id;
        json[_("menuSection")] = menuSection == KMoreTools::MenuSection_Main ? _("main") : _("more");
        json[_("isInstalled")] = isInstalled;
    }

    bool operator==(const KmtMenuItemDto rhs) const
    {
        return this->id == rhs.id;
    }

    /**
     * todo: is there a QT method that can be used instead of this?
     */
    static QString removeMenuAmpersand(const QString &str)
    {
        QString newStr = str;
        newStr.replace(QRegularExpression(QStringLiteral("\\&([^&])")), QStringLiteral("\\1")); // &Hallo --> Hallo
        newStr.replace(_("&&"), _("&")); // &&Hallo --> &Hallo
        return newStr;
    }
};

/**
 * The serializeable menu structure.
 * Used for working with user interaction for persisted configuration.
 */
class KmtMenuStructureDto
{
public:
    QList<KmtMenuItemDto> list;

public: // should be private but we would like to unit test
    /**
     * NOT USED
     */
    QList<const KmtMenuItemDto *> itemsBySection(KMoreTools::MenuSection menuSection) const
    {
        QList<const KmtMenuItemDto *> r;

        for (const auto &item : std::as_const(list)) {
            if (item.menuSection == menuSection) {
                r.append(&item);
            }
        }

        return r;
    }

    /**
     * don't store the returned pointer, but you can deref it which calls copy ctor
     */
    const KmtMenuItemDto *findInstalled(const QString &id) const
    {
        auto foundItem = std::find_if(list.begin(), list.end(), [id](const KmtMenuItemDto &item) {
            return item.id == id && item.isInstalled;
        });
        if (foundItem != list.end()) {
            // deref iterator which is a const MenuItemDto& from which we get the pointer
            // (todo: is this a good idea?)
            return &(*foundItem);
        }

        return nullptr;
    }

public:
    QString serialize() const
    {
        QJsonObject jObj;
        jsonWrite(jObj);
        QJsonDocument doc(jObj);
        auto jByteArray = doc.toJson(QJsonDocument::Compact);
        return QString::fromUtf8(jByteArray);
    }

    void deserialize(const QString &text)
    {
        QJsonParseError parseError;
        QJsonDocument doc(QJsonDocument::fromJson(text.toUtf8(), &parseError));
        jsonRead(doc.object());
    }

    void jsonRead(const QJsonObject &json)
    {
        list.clear();
        auto jArr = json[_("menuitemlist")].toArray();
        for (int i = 0; i < jArr.size(); ++i) {
            auto jObj = jArr[i].toObject();
            KmtMenuItemDto item;
            item.jsonRead(jObj);
            list.append(item);
        }
    }

    void jsonWrite(QJsonObject &json) const
    {
        QJsonArray jArr;
        for (const auto &item : std::as_const(list)) {
            QJsonObject jObj;
            item.jsonWrite(jObj);
            jArr.append(jObj);
        }
        json[_("menuitemlist")] = jArr;
    }

    /**
     * @returns true if there are any not-installed items
     */
    std::vector<KmtMenuItemDto> notInstalledServices() const
    {
        std::vector<KmtMenuItemDto> target;
        std::copy_if(list.begin(), list.end(), std::back_inserter(target), [](const KmtMenuItemDto &item) {
            return !item.isInstalled;
        });
        return target;
    }

public: // should be private but we would like to unit test
    /**
     * stable sorts:
     * 1. main items
     * 2. more items
     * 3. not installed items
     */
    void stableSortListBySection()
    {
        std::stable_sort(list.begin(), list.end(), [](const KmtMenuItemDto &i1, const KmtMenuItemDto &i2) {
            return (i1.isInstalled && i1.menuSection == KMoreTools::MenuSection_Main && i2.isInstalled && i2.menuSection == KMoreTools::MenuSection_More)
                || (i1.isInstalled && !i2.isInstalled);
        });
    }

public:
    /**
     * moves an item up or down respecting its category
     * @param direction: 1: down, -1: up
     */
    void moveWithinSection(const QString &id, int direction)
    {
        auto selItem = std::find_if(list.begin(), list.end(), [id](const KmtMenuItemDto &item) {
            return item.id == id;
        });

        if (selItem != list.end()) { // if found
            if (direction == 1) { // "down"
                auto itemAfter = std::find_if(selItem + 1,
                                              list.end(), // find item where to insert after in the same category
                                              [selItem](const KmtMenuItemDto &item) {
                                                  return item.menuSection == selItem->menuSection;
                                              });

                if (itemAfter != list.end()) {
                    int prevIndex = list.indexOf(*selItem);
                    list.insert(list.indexOf(*itemAfter) + 1, *selItem);
                    list.removeAt(prevIndex);
                }
            } else if (direction == -1) { // "up"
                // auto r_list = list;
                // std::reverse(r_list.begin(), r_list.end()); // we need to search "up"
                // auto itemBefore = std::find_if(selItem, list.begin(),// find item where to insert before in the same category
                //                               [selItem](const MenuItemDto& item) { return item.menuSection == selItem->menuSection; });

                // todo: can't std::find_if be used instead of this loop?
                QList<KmtMenuItemDto>::iterator itemBefore = list.end();
                auto it = selItem;
                while (it != list.begin()) {
                    --it;
                    if (it->menuSection == selItem->menuSection) {
                        itemBefore = it;
                        break;
                    }
                }

                if (itemBefore != list.end()) {
                    int prevIndex = list.indexOf(*selItem);
                    list.insert(itemBefore, *selItem);
                    list.removeAt(prevIndex + 1);
                }
            } else {
                Q_ASSERT(false);
            }
        } else {
            qWarning() << "selItem != list.end() == false";
        }

        stableSortListBySection();
    }

    void moveToOtherSection(const QString &id)
    {
        auto selItem = std::find_if(list.begin(), list.end(), [id](const KmtMenuItemDto &item) -> bool {
            return item.id == id;
        });

        if (selItem != list.end()) { // if found
            if (selItem->menuSection == KMoreTools::MenuSection_Main) {
                selItem->menuSection = KMoreTools::MenuSection_More;
            } else if (selItem->menuSection == KMoreTools::MenuSection_More) {
                selItem->menuSection = KMoreTools::MenuSection_Main;
            } else {
                Q_ASSERT(false);
            }
        }

        stableSortListBySection();
    }
};

/**
 * In menu structure consisting of main section items, more section items
 * and registered services which are not installed.
 * In contrast to KmtMenuStructureDto we are dealing here with
 * KMoreToolsMenuItem pointers instead of DTOs.
 */
class KmtMenuStructure
{
public:
    QList<KMoreToolsMenuItem *> mainItems;
    QList<KMoreToolsMenuItem *> moreItems;

    /**
     * contains each not installed registered service once
     */
    QList<KMoreToolsService *> notInstalledServices;

public:
    KmtMenuStructureDto toDto()
    {
        KmtMenuStructureDto result;

        for (auto item : std::as_const(mainItems)) {
            const auto a = item->action();
            KmtMenuItemDto dto;
            dto.id = item->id();
            dto.text = a->text(); // might be overridden, so we use directly from QAction
            dto.icon = a->icon();
            dto.isInstalled = true;
            dto.menuSection = KMoreTools::MenuSection_Main;
            result.list << dto;
        }

        for (auto item : std::as_const(moreItems)) {
            const auto a = item->action();
            KmtMenuItemDto dto;
            dto.id = item->id();
            dto.text = a->text(); // might be overridden, so we use directly from QAction
            dto.icon = a->icon();
            dto.isInstalled = true;
            dto.menuSection = KMoreTools::MenuSection_More;
            result.list << dto;
        }

        for (auto registeredService : std::as_const(notInstalledServices)) {
            KmtMenuItemDto dto;
            // dto.id = item->id(); // not used in this case
            dto.text = registeredService->formatString(_("$Name"));
            dto.icon = registeredService->icon();
            dto.isInstalled = false;
            // dto.menuSection = // not used in this case
            dto.homepageUrl = registeredService->homepageUrl();
            result.list << dto;
        }

        return result;
    }
};

/**
 * Helper class that deals with creating the menu where all the not-installed
 * services are listed.
 */
class KmtNotInstalledUtil
{
public:
    /**
     * For one given application/service which is named @p title a QMenu is
     * created with the given @p icon and @p homepageUrl.
     * It will be used as submenu for the menu that displays the not-installed
     * services.
     */
    static QMenu *createSubmenuForNotInstalledApp(const QString &title, QWidget *parent, const QIcon &icon, const QUrl &homepageUrl, const QString &appstreamId)
    {
        QMenu *submenuForNotInstalled = new QMenu(title, parent);
        submenuForNotInstalled->setIcon(icon);

        if (homepageUrl.isValid()) {
            auto websiteAction = submenuForNotInstalled->addAction(i18nc("@action:inmenu", "Visit homepage"));
            websiteAction->setIcon(QIcon::fromTheme(QStringLiteral("internet-services")));
            auto url = homepageUrl;
            // todo/review: is it ok to have sender and receiver the same object?
            QObject::connect(websiteAction, &QAction::triggered, websiteAction, [url](bool) {
                QDesktopServices::openUrl(url);
            });
        }

        QUrl appstreamUrl = QUrl(QStringLiteral("appstream://") % appstreamId);

        if (!appstreamId.isEmpty()) {
            auto installAction = submenuForNotInstalled->addAction(i18nc("@action:inmenu", "Install"));
            installAction->setIcon(QIcon::fromTheme(QStringLiteral("download")));
            QObject::connect(installAction, &QAction::triggered, installAction, [appstreamUrl](bool) {
                QDesktopServices::openUrl(appstreamUrl);
            });
        }

        if (!homepageUrl.isValid() && appstreamId.isEmpty()) {
            submenuForNotInstalled->addAction(i18nc("@action:inmenu", "No further information available."))->setEnabled(false);
        }

        return submenuForNotInstalled;
    }
};

/**
 * Url handling utils
 */
class KmtUrlUtil
{
public:
    /**
     * "file:///home/abc/hallo.txt" becomes "file:///home/abc"
     */
    static QUrl localFileAbsoluteDir(const QUrl &url)
    {
        if (!url.isLocalFile()) {
            qWarning() << "localFileAbsoluteDir: url must be local file";
        }
        QFileInfo fileInfo(url.toLocalFile());
        auto dir = QDir(fileInfo.absoluteDir()).absolutePath();
        return QUrl::fromLocalFile(dir);
    }
};

#endif
