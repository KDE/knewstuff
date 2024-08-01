# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: Harald Sitter <sitter@kde.org>

ecm_add_qml_module(newstuffqmlpluginprivate URI "org.kde.newstuff.private" GENERATE_PLUGIN_SOURCE VERSION 1.0)
target_sources(newstuffqmlpluginprivate PRIVATE transientmagicianassistant.cpp)
ecm_qt_declare_logging_category(newstuffqmlpluginprivate
    HEADER knewstuffquickprivate_debug.h
    IDENTIFIER KNEWSTUFFQUICKPRIVATE
    CATEGORY_NAME kf.newstuff.quick.private
    OLD_CATEGORY_NAMES org.kde.knewstuff.quick.private
    DESCRIPTION "knewstuff (qtquick private)"
    EXPORT KNEWSTUFF
)
target_link_libraries(newstuffqmlpluginprivate PRIVATE Qt::Quick)
ecm_finalize_qml_module(newstuffqmlpluginprivate DESTINATION ${KDE_INSTALL_QMLDIR})
