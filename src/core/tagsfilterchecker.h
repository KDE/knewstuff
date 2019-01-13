/*
    Copyright (c) 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

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

#ifndef KNSCORE_TAGSFILTERCHECKER_H
#define KNSCORE_TAGSFILTERCHECKER_H

#include <QStringList>
#include "knewstuffcore_export.h"

namespace KNSCore {

/**
 * @brief Apply simple filtering logic to a list of tags
 *
 * == Examples of specifying tag filters ==
 * Value for tag "tagname" must be exactly "tagdata":
 * tagname==tagdata
 *
 * Value for tag "tagname" must be different from "tagdata":
 * tagname!=tagdata
 *
 * == Tag filter list ==
 * A tag filter list is a string list of filters as shown above, and a combination
 * of which might look like:
 *
 * - ghns_excluded!=1
 * - data##mimetype==application/cbr+zip
 * - data##mimetype==application/cbr+rar
 * 
 * which would filter out anything which has ghns_excluded set to 1, and
 * anything where the value of data##mimetype does not equal either
 * "application/cbr+zip" or "application/cbr+rar".
 * Notice in particular the two data##mimetype entries. Use this
 * for when a tag may have multiple values.
 *
 * The value does not current suppport wildcards. The list should be considered
 * a binary AND operation (that is, all filter entries must match for the data
 * entry to be included in the return data)
 * @since 5.51
 */
class KNEWSTUFFCORE_EXPORT TagsFilterChecker {
public:
    /**
     * Constructs an instance of the tags filter checker, prepopulated
     * with the list of tag filters in the tagFilter parameter.
     *
     * @param tagFilter The list of tag filters
     * @since 5.51
     */
    TagsFilterChecker(const QStringList &tagFilter);
    ~TagsFilterChecker();

    TagsFilterChecker(const TagsFilterChecker &) = delete;
    TagsFilterChecker& operator=(const TagsFilterChecker &) = delete;

    /**
     * Check whether the filter list accepts the passed list of tags
     *
     * @param tags A list of tags in the form of key=value strings
     * @return True if the filter accepts the list, false if not
     * @since 5.51
     */
    bool filterAccepts(const QStringList &tags);

private:
    class Private;
    Private *d;
};

}

#endif//KNSCORE_TAGSFILTERCHECKER_H
