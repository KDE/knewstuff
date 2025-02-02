/*
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFFWIDGETSBUTTON_H
#define KNEWSTUFFWIDGETSBUTTON_H

#include <KNSCore/Entry>
#include <QPushButton>
#include <memory>

#include "knewstuffwidgets_export.h"

namespace KNSWidgets
{
class ButtonPrivate;
/*!
 * \class KNSWidgets::Button
 * \inmodule KNewStuffWidgets
 *
 * \brief QPushButton subclass that encapsulates the logic for showing the KNewStuff dialog.
 *
 * If KNewStuff is disabled using KAuthorized, the button is hidden.
 *
 * \sa KAuthorized::GenericRestriction::GHNS
 *
 * \since 5.91
 */
class KNEWSTUFFWIDGETS_EXPORT Button : public QPushButton
{
    Q_OBJECT

public:
    /*!
     * Constructor used when the details of the KHotNewStuff
     * download is known when the button is created.
     *
     * \a text describing what is being downloaded.
     * It should be a text beginning with "Download New ..." for consistency
     *
     * \a configFile the name of the .knsrc file
     *
     * \a parent the parent widget
     *
     */
    explicit Button(const QString &text, const QString &configFile, QWidget *parent);

    /*!
     * Constructor used when the code is generated from a .ui file
     * After the UI is set up, you must call setConfigFile(QString)
     */
    explicit Button(QWidget *parent);

    ~Button() override;

    /*!
     * \note This should only be used when creating the button from a UI-file.
     */
    void setConfigFile(const QString &configFile);

Q_SIGNALS:
    /*!
     * Emitted when the dialog has been closed.
     *
     * \a changedEntries contains the entries that were changed
     *
     */
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);

private:
    const std::unique_ptr<ButtonPrivate> d;
};

}

#endif // KNEWSTUFFBUTTON_H
