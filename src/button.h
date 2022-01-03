/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3BUTTON_H
#define KNEWSTUFF3BUTTON_H

#include <QPushButton>

#include "entry.h"
#include "knewstuff_export.h"

#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 91)

#include <memory>

namespace KNS3
{
class ButtonPrivate;
/**
 * KHotNewStuff push button that makes using KHNS in an application
 * more convenient by encapsulating most of the details involved in
 * using KHotNewStuff in the button itself.
 *
 * @since 4.4
 * @deprecated Since 5.91, use KNSWidgets::Button instead
 */
class KNEWSTUFF_EXPORT Button : public QPushButton
{
    Q_OBJECT

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
    KNEWSTUFF_DEPRECATED_VERSION(5, 91, "use KNSWidgets::Button instead")
    Button(const QString &text, const QString &configFile, QWidget *parent);

    /**
     * Constructor used when the details of the KHotNewStuff
     * download is not known in advance of the button being created.
     *
     * @param parent the parent widget
     */
    KNEWSTUFF_DEPRECATED_VERSION(5, 91, "use KNSWidgets::Button instead")
    explicit Button(QWidget *parent);

    ~Button() override;

    /**
     * set the name of the .knsrc file to use
     */
    void setConfigFile(const QString &configFile);

#if KNEWSTUFF_ENABLE_DEPRECATED_SINCE(5, 76)
    /**
     * Set the text that should appear on the button.
     * @deprecated Since 5.0, use setText(const QString&) instead
     */
    KNEWSTUFF_DEPRECATED_VERSION_BELATED(5, 76, 5, 0, "Use setText(const QString&) instead")
    void setButtonText(const QString &text);
#endif

Q_SIGNALS:
    /**
     * emitted when the Hot New Stuff dialog is about to be shown, usually
     * as a result of the user having click on the button
     */
    void aboutToShowDialog();

    /**
     * emitted when the Hot New Stuff dialog has been closed
     */
    void dialogFinished(const KNS3::Entry::List &changedEntries);

protected Q_SLOTS:
    void showDialog();

private:
    void init();

    const std::unique_ptr<ButtonPrivate> d;
};

}

#endif
#endif // KNEWSTUFFBUTTON_H
