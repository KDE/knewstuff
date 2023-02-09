/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
#define KNEWSTUFF_QTQUICKDIALOGWRAPPER_H

#include <KNSCore/Entry>
#include <KNSCore/ErrorCode>
#include <QObject>

#include "knewstuffwidgets_export.h"

namespace KNSCore
{
class Engine;
};

namespace KNSWidgets
{
class QtQuickDialogWrapperPrivate;
/**
 * This class is a wrapper around the QtQuick QML dialog. This dialog is loaded using the QQmlEngine.
 * The constructor will create the QML component, to show the dialog the show() method must be called.
 * It is recommended to reuse an instance of this class if it is expected that the user reopens the dialog.
 * @since 5.78
 */
class KNEWSTUFFWIDGETS_EXPORT QtQuickDialogWrapper : public QObject
{
    Q_OBJECT

public:
    explicit QtQuickDialogWrapper(const QString &configFile, QObject *parent = nullptr);
    ~QtQuickDialogWrapper() override;

    /**
     * Opens the dialog
     */
    void open();

    /**
     * This signal gets emitted when the dialog is closed
     */
    Q_SIGNAL void closed();

    /**
     * Engine that is used by the dialog, might be null if the engine failed to initialize.
     * @return KNSCore::Engine used by the dialog
     */
    KNSCore::Engine *engine();

    /**
     * Entries that were changed while the user interacted with the dialog
     * @since 5.94
     */
    QList<KNSCore::Entry> changedEntries() const;

private:
    const std::unique_ptr<QtQuickDialogWrapperPrivate> d;

    Q_DISABLE_COPY(QtQuickDialogWrapper)
};
}

#endif // KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
