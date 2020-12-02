/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
#define KNEWSTUFF_QTQUICKDIALOGWRAPPER_H

#include <QObject>
#include <KNSCore/ErrorCode>
#include <KNSCore/EntryInternal>

namespace KNSCore
{
class Engine;
}

namespace KNS3
{
class QtQuickDialogWrapperPrivate;
/**
 * This class is a wrapper around the QtQuick QML dialog. This dialog is loaded using the QQmlEngine.
 * The constructor will create the QML component, to show the dialog the show() method must be called.
 * It is recommended to reuse an instance of this class if it is expected that the user reopens the dialog.
 * @since 5.78
 */
class KNEWSTUFF_EXPORT QtQuickDialogWrapper : public QObject
{
    Q_OBJECT

public:
    QtQuickDialogWrapper(const QString &configFile, QObject *parent = nullptr);
    ~QtQuickDialogWrapper();

    /**
     * Opens the dialog
     */
    void open();

    /**
     * Similar to QDialog::exec. Shows the dialog and blocks until the user closes it.
     * @return changedEntries, useful if you want to refresh the UI after entries were changed
     * @see open
     */
    QList<KNSCore::EntryInternal> exec();

    /**
     * This signal gets emitted when the dialog is closed
     */
    Q_SIGNAL void closed();

    /**
     * Engine that is used by the dialog, might be null if the engine failed to initialize.
     * @return KNSCore::Engine used by the dialog
     */
    KNSCore::Engine* engine();

private:
    std::unique_ptr<QtQuickDialogWrapperPrivate> d;

    Q_DISABLE_COPY(QtQuickDialogWrapper)
};
}


#endif //KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
