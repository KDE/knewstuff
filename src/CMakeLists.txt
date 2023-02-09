# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_subdirectory(core)

ecm_qt_declare_logging_category(KNEWSTUFF_LOGGING_SRCS
    HEADER knewstuff_debug.h
    IDENTIFIER KNEWSTUFF
    CATEGORY_NAME kf.newstuff
    OLD_CATEGORY_NAMES org.kde.knewstuff
    DESCRIPTION "knewstuff (knewstuff)"
    EXPORT KNEWSTUFF
)

add_subdirectory(widgets)
ecm_qt_install_logging_categories(
    EXPORT KNEWSTUFF
    FILE knewstuff.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
