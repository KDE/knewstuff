/*
 * Copyright (C) 2020 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ENTRYWRAPPER_H
#define ENTRYWRAPPER_H

#include <QObject>

#include "entryinternal.h"
#include "knewstuffcore_export.h"

namespace KNSCore
{
// This is for passing an entryinternal through qml, particularly useful for lists
// such as changedEntries. This is supposed to closely approximate the current
// codepaths used in client code (which expects a list of entries), but for KF6
// we will want to turn this into a model instead, probably with some handy
// iteration assistance for use in places which would previously use the lists.
/// TODO KF6 see above (in short, make this class irrelevant so it can be removed)

/**
 * @short Wraps a KNSCore::EntryInternal in a QObject for passing through Qt Quick
 *
 * For those places where we need to pass a KNSCore::EntryInternal through Qt Quick,
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
    explicit EntryWrapper(const EntryInternal& entry, QObject *parent = nullptr);
    virtual ~EntryWrapper();

    /**
     * Get a copy of the wrapped-up entry
     * @return A copy of the entry
     */
    EntryInternal entry() const;
private:
    class Private;
    Private *d;
};
}
Q_DECLARE_METATYPE(KNSCore::EntryWrapper*)

#endif//ENTRYWRAPPER_H
