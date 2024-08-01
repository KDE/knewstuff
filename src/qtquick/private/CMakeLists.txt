# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Harald Sitter <sitter@kde.org>

add_library(knewstuff_qml_private_STATIC STATIC)
target_sources(knewstuff_qml_private_STATIC PRIVATE
    transientmagicianassistant.cpp
)
ecm_qt_declare_logging_category(knewstuff_qml_private_STATIC
    HEADER knewstuffquickprivate_debug.h
    IDENTIFIER KNEWSTUFFQUICKPRIVATE
    CATEGORY_NAME kf.newstuff.quick.private
    OLD_CATEGORY_NAMES org.kde.knewstuff.quick.private
    DESCRIPTION "knewstuff (qtquick private)"
    # EXPORT KNEWSTUFF
)

set_property(TARGET knewstuff_qml_private_STATIC PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(knewstuff_qml_private_STATIC PUBLIC
    Qt6::Quick
)

ecm_add_qml_module(newstuffqmlpluginprivate URI "org.kde.newstuff.private" VERSION 1.0)
target_sources(newstuffqmlpluginprivate PRIVATE plugin.cpp)
target_link_libraries(newstuffqmlpluginprivate PRIVATE knewstuff_qml_private_STATIC)
ecm_finalize_qml_module(newstuffqmlpluginprivate DESTINATION ${KDE_INSTALL_QMLDIR})
