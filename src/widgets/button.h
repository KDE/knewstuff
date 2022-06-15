/*
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFFWIDGETSBUTTON_H
#define KNEWSTUFFWIDGETSBUTTON_H

#include <QPushButton>
#include <memory>

#include "KNSCore/EntryInternal"
#include "knewstuffwidgets_export.h"

namespace KNSCore
{
using Entry = KNSCore::EntryInternal;
}

namespace KNSWidgets
{
class ButtonPrivate;
/**
 * @class Button button.h <KNSWidgets/Button>
 *
 * KHotNewStuff push button that makes using KHNS in an application
 * more convenient by encapsulating most of the details involved in
 * using KHotNewStuff in the button itself.
 *
 * @since 5.91
 */
class KNEWSTUFFWIDGETS_EXPORT Button : public QPushButton
{
    Q_OBJECT

    // This way the configFile can be set as a property with QtCreator.
    Q_PROPERTY(QString configFile WRITE setConfigFile READ configFile)

public:
    /**
     * Constructor used when the details of the KHotNewStuff
     * download is known when the button is created.
     *
     * @param text describing what is being downloaded.
     *        It should be a text beginning with "Download New ..." for consistency
     * @param configFile the name of the .knsrc file
     * @param parent the parent widget
     */
    Button(const QString &text, const QString &configFile, QWidget *parent);

    /**
     * Constructor used when the details of the KHotNewStuff
     * download is not known in advance of the button being created.
     *
     * @param parent the parent widget
     * @note When this constructor is used, the @p configFile property has to be set
     */
    explicit Button(QWidget *parent);

    ~Button() override;

    /**
     * set the name of the .knsrc file to use
     */
    void setConfigFile(const QString &configFile);

Q_SIGNALS:
    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    void aboutToShowDialog();

    /**
     * emitted when the Hot New Stuff dialog has been closed
     */
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);

private:
    QString configFile();
    Q_SLOT void showDialog();
    friend class ButtonPrivate;
    const std::unique_ptr<ButtonPrivate> d;
};

}

#endif // KNEWSTUFFBUTTON_H
