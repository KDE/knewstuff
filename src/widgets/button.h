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
/**
 * @class Button button.h <KNSWidgets/Button>
 *
 * QPushButton subclass that encapsulates the logic for showing the KNewStuff dialog.
 * If KNewStuff is disabled using KAuthorized, the button is hidden.
 * @see KAuthorized::GenericRestriction::GHNS
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
    explicit Button(const QString &text, const QString &configFile, QWidget *parent);

    /**
     * Constructor used when the code is generated from a .ui file
     *
     * @param parent the parent widget
     * @note When this constructor is used, the @p configFile property has to be set
     */
    explicit Button(QWidget *parent);

    ~Button() override;

Q_SIGNALS:
    /**
     * emitted when the dialog has been closed
     */
    void dialogFinished(const QList<KNSCore::Entry> &changedEntries);

private:
    // Only used by property that is set when generating C++ code from UI files
    void setConfigFile(const QString &configFile);
    QString configFile() const
    {
        return QString();
    }
    const std::unique_ptr<ButtonPrivate> d;
};

}

#endif // KNEWSTUFFBUTTON_H
