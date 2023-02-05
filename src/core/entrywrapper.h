/*
    SPDX-FileCopyrightText: 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ENTRYWRAPPER_H
#define ENTRYWRAPPER_H

#include <QObject>

#include "entry.h"
#include "knewstuffcore_export.h"

#include <memory>

namespace KNSCore
{
// This is for passing an entry through qml, particularly useful for lists
// such as changedEntries. This is supposed to closely approximate the current
// codepaths used in client code (which expects a list of entries), but for KF6
// we will want to turn this into a model instead, probably with some handy
// iteration assistance for use in places which would previously use the lists.
/// TODO KF6 see above (in short, make this class irrelevant so it can be removed)

class EntryWrapperPrivate;
/**
 * @short Wraps a KNSCore::Entry in a QObject for passing through Qt Quick
 *
 * For those places where we need to pass a KNSCore::Entry through Qt Quick,
 * this class wraps such objects in a QObject, and provides the ability to interact
 * with the data through that.
 *
 * Since that class is not a QObject (and can't easily be turned into one), until major
 * upheaval becomes possible in Frameworks 6, this class will have to do.
 * @since 5.67
 */
class KNEWSTUFFCORE_EXPORT EntryWrapper : public QObject
{
    Q_OBJECT
public:
    explicit EntryWrapper(const Entry &entry, QObject *parent = nullptr);
    ~EntryWrapper() override;

    /**
     * Get a copy of the wrapped-up entry
     * @return A copy of the entry
     */
    Entry entry() const;

private:
    const std::unique_ptr<EntryWrapperPrivate> d;
};
}
Q_DECLARE_METATYPE(KNSCore::EntryWrapper *)

#endif // ENTRYWRAPPER_H
