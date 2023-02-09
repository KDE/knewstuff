# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_subdirectory(core)

set(KNewStuff_SRCS
    qtquickdialogwrapper.cpp
)
ecm_qt_declare_logging_category(KNewStuff_SRCS
    HEADER knewstuff_debug.h
    IDENTIFIER KNEWSTUFF
    CATEGORY_NAME kf.newstuff
    OLD_CATEGORY_NAMES org.kde.knewstuff
    DESCRIPTION "knewstuff (knewstuff)"
    EXPORT KNEWSTUFF
)

add_library(KF6NewStuff ${KNewStuff_SRCS})
add_library(KF6::NewStuff ALIAS KF6NewStuff)

ecm_generate_export_header(KF6NewStuff
    EXPORT_FILE_NAME knewstuff_export.h
    BASE_NAME KNewStuff
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

target_include_directories(KF6NewStuff
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3>")

target_link_libraries(KF6NewStuff
  PUBLIC
    KF6::NewStuffCore
    KF6::Service               # For KMoreTools
    Qt6::Widgets
  PRIVATE
    KF6::KIOCore               # For network fetching operations
    KF6::KIOWidgets            # For KUrlRequester
    KF6::Completion            # For KLineEdit in download dialog
    KF6::I18n                  # For translations
    KF6::ItemViews             # For buttons on download dialog
    KF6::ConfigCore # KAuthorized
    Qt6::Qml
    Qt6::Quick
)

set_target_properties(KF6NewStuff PROPERTIES
   VERSION ${KNEWSTUFF_VERSION}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuff
)

ecm_generate_headers(KNewStuff_CamelCase_HEADERS
  HEADER_NAMES
  QtQuickDialogWrapper

  REQUIRED_HEADERS KNewStuff_HEADERS
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/KNS3
)

install(
    FILES
        ${KNewStuff_CamelCase_HEADERS}
        ${KNewStuff_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/KNS3
    COMPONENT Devel
)

install(TARGETS KF6NewStuff EXPORT KF6NewStuffTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if(BUILD_QCH)
    ecm_add_qch(
        KF6NewStuff_QCH
        NAME KNewStuff
        BASE_NAME KF6NewStuff
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KNewStuff_HEADERS}
            ${KMoreTools_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            KF6NewStuffCore_QCH
            KF6Service_QCH
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${KNewStuff_BINARY_DIR}
        BLANK_MACROS
            KNEWSTUFF_EXPORT
            KNEWSTUFF_DEPRECATED
            KNEWSTUFF_DEPRECATED_EXPORT
            "KNEWSTUFF_DEPRECATED_VERSION(x, y, t)"
            "KNEWSTUFF_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

# after ecm_add_qch, due to KF6NewStuffWidgets_QCH dependency on KF6NewStuff_QCH being declared
add_subdirectory(widgets)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuff LIB_NAME KF6NewStuff DEPS "widgets KNewStuffCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

add_subdirectory(qtquick)
add_subdirectory(tools)

ecm_qt_install_logging_categories(
    EXPORT KNEWSTUFF
    FILE knewstuff.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
