# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

add_subdirectory(core)

set(KNewStuff_SRCS
    entry.cpp
    button.cpp
    knewstuffaction.cpp
    kmoretools/kmoretools.cpp
    kmoretools/kmoretoolsconfigdialog_p.cpp
    kmoretools/kmoretoolsmenufactory.cpp
    kmoretools/kmoretoolspresets.cpp
    ui/widgetquestionlistener.cpp
    uploaddialog.cpp
    qtquickdialogwrapper.cpp
)
# Exclude deprecated classes from the build
if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
        EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.80.0)
list(APPEND KNewStuff_SRCS
        downloaddialog.cpp
        downloadwidget.cpp
        downloadmanager.cpp
        ui/entrydetailsdialog.cpp
        ui/imagepreviewwidget.cpp
        ui/itemsview.cpp
        ui/itemsviewdelegate.cpp
        ui/itemsgridviewdelegate.cpp
        ui/itemsviewbasedelegate.cpp
        ui/progressindicator.cpp
        )
endif()

qt_add_resources(KNewStuff_RESOURCES uploaddialog.qrc)

ecm_qt_declare_logging_category(KNewStuff_SRCS
    HEADER knewstuff_debug.h
    IDENTIFIER KNEWSTUFF
    CATEGORY_NAME kf.newstuff
    OLD_CATEGORY_NAMES org.kde.knewstuff
    DESCRIPTION "knewstuff (knewstuff)"
    EXPORT KNEWSTUFF
)

ki18n_wrap_ui(KNewStuff_SRCS
    downloadwidget.ui
    kmoretools/ui/kmoretoolsconfigwidget.ui
)

add_library(KF5NewStuff ${KNewStuff_SRCS} ${KNewStuff_RESOURCES})
add_library(KF5::NewStuff ALIAS KF5NewStuff)

ecm_generate_export_header(KF5NewStuff
    EXPORT_FILE_NAME knewstuff_export.h
    BASE_NAME KNewStuff
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS 5.29 5.76 5.77 5.78 5.79 5.80 5.82 5.85
)

target_include_directories(KF5NewStuff
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNS3;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/knscore>")

target_link_libraries(KF5NewStuff
  PUBLIC
    KF5::NewStuffCore
    KF5::XmlGui                # For KActionCollection in knewstuffaction.h
    KF5::Service               # For KMoreTools
    Qt5::Widgets
  PRIVATE
    KF5::KIOCore               # For network fetching operations
    KF5::KIOWidgets            # For KUrlRequester
    KF5::Completion            # For KLineEdit in download dialog
    KF5::I18n                  # For translations
    KF5::ItemViews             # For buttons on download dialog
    KF5::IconThemes           # For KIcon
    Qt5::Qml
    Qt5::Quick
    KF5::TextWidgets          # For KTextEdit in upload dialog
)

set_target_properties(KF5NewStuff PROPERTIES
   VERSION ${KNEWSTUFF_VERSION}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuff
)

set(KNewStuff_HEADERS
  knewstuffaction.h
)

ecm_generate_headers(KNewStuff_CamelCase_HEADERS
  HEADER_NAMES
  DownloadDialog
  DownloadWidget
  DownloadManager
  UploadDialog
  Entry
  Button
  QtQuickDialogWrapper

  REQUIRED_HEADERS KNewStuff_HEADERS
  PREFIX KNS3
)

# For KMoreTools:
ecm_generate_headers(KNewStuff_CamelCase_HEADERS_KMoreTools
  HEADER_NAMES
  KMoreTools
  KMoreToolsMenuFactory
  KMoreToolsPresets

  RELATIVE kmoretools
  REQUIRED_HEADERS KNewStuff_HEADERS_KMoreTools
  PREFIX KNS3
)
install(FILES ${KNewStuff_CamelCase_HEADERS_KMoreTools} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNS3 COMPONENT Devel)
install(FILES ${KNewStuff_HEADERS_KMoreTools} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/kns3 COMPONENT Devel
)

install(FILES ${KNewStuff_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNS3 COMPONENT Devel)

install(TARGETS KF5NewStuff EXPORT KF5NewStuffTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
  ${KNewStuff_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/kns3 COMPONENT Devel
)

add_subdirectory(widgets)

if(BUILD_QCH)
    ecm_add_qch(
        KF5NewStuff_QCH
        NAME KNewStuff
        BASE_NAME KF5NewStuff
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KNewStuff_HEADERS}
            ${KNewStuff_HEADERS_KMoreTools}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            KF5NewStuffCore_QCH
            KF5XmlGui_QCH
            KF5Service_QCH
            Qt5Widgets_QCH
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

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuff LIB_NAME KF5NewStuff DEPS "widgets KXmlGui KNewStuffCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

add_subdirectory(qtquick)
add_subdirectory(tools)

ecm_qt_install_logging_categories(
    EXPORT KNEWSTUFF
    FILE knewstuff.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
