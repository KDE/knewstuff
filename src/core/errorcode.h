/*
    knewstuff3/errorcode.h.
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


#ifndef KNSCORE_ERRORCODE_H
#define KNSCORE_ERRORCODE_H

#include "knewstuffcore_export.h"
#include <qobjectdefs.h>

namespace KNSCore
{
    KNEWSTUFFCORE_EXPORT Q_NAMESPACE
    /**
     * An enumeration of specific error conditions which might occur and which
     * users of KNewStuff would want to react to. It is used by both the Engine and
     * Provider classes.
     * @since 5.53
     */
    enum ErrorCode {
        UnknownError, ///< An unknown error (this should not be used, an error report of this nature should be considered a bug)
        NetworkError, ///< A network error. In signalErrorCode, this will be accompanied by the QtNetwork error code in the metadata
        OcsError, ///< An error reported by the OCS API server. In signalErrorCode, this will be accompanied by the OCS error code in the metadata
        ConfigFileError, ///< The configuration file is missing or somehow incorrect. The configuration file filename will be held in the metadata
        ProviderError, ///< A provider has failed to load or initialize. The provider file URL or provider URL will be held in the metadata
        InstallationError, ///< Installation of a content item has failed
        ImageError ///< Loading an image has failed. The entry name and preview type which failed will be held in the metadata as a QVariantList
    };
    Q_ENUM_NS(ErrorCode)
}
#endif//KNSCORE_ERRORCODE_H
