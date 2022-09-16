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
    DEPRECATION_VERSIONS 5.29 5.76 5.77 5.78 5.79 5.80 5.82 5.85 5.91 5.94
)

target_include_directories(KF5NewStuff
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3>")

target_link_libraries(KF5NewStuff
  PUBLIC
    KF5::NewStuffCore
    KF5::XmlGui                # For KActionCollection in knewstuffaction.h
    KF5::Service               # For KMoreTools
    Qt${QT_MAJOR_VERSION}::Widgets
  PRIVATE
    KF5::KIOCore               # For network fetching operations
    KF5::KIOWidgets            # For KUrlRequester
    KF5::Completion            # For KLineEdit in download dialog
    KF5::I18n                  # For translations
    KF5::ItemViews             # For buttons on download dialog
    KF5::IconThemes           # For KIcon
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
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

# For KMoreTools:
if(QT_MAJOR_VERSION EQUAL 5)
    ecm_generate_headers(KMoreTools_CamelCase_HEADERS
        HEADER_NAMES
        KMoreTools
        KMoreToolsMenuFactory
        KMoreToolsPresets

        RELATIVE kmoretools
        REQUIRED_HEADERS KMoreTools_HEADERS
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/KNS3
    )
    install(
        FILES ${KMoreTools_CamelCase_HEADERS}
        FILES ${KMoreTools_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/KNS3
        COMPONENT Devel
    )
    install(
        FILES ${KMoreTools_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/kns3
        COMPONENT Devel
    )
endif()

# In KF6 all KMoreTools headers will be only in ${KDE_INSTALL_INCLUDEDIR_KF}/KMoreTools
ecm_generate_headers(KMoreTools_CamelCase_HEADERS
    HEADER_NAMES
    KMoreTools
    KMoreToolsMenuFactory
    KMoreToolsPresets

    RELATIVE kmoretools
    REQUIRED_HEADERS KMoreTools_HEADERS
)
install(
    FILES
        ${KMoreTools_CamelCase_HEADERS}
        ${KMoreTools_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KMoreTools
    COMPONENT Devel
)
target_include_directories(KF5NewStuff
    INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KMoreTools>"
)
# So that '#include "KNS3/knewstuff_export.h"' works in KMoreTools headers during the build
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/KNS3
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

install(TARGETS KF5NewStuff EXPORT KF5NewStuffTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

# Kept for backwards compatibility, in KF6 all headers will be in KNewStuff3/KNS3 only
if(QT_MAJOR_VERSION STREQUAL "5")
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
        ${KNewStuff_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/kns3 COMPONENT Devel
    )
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF5NewStuff_QCH
        NAME KNewStuff
        BASE_NAME KF5NewStuff
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KNewStuff_HEADERS}
            ${KMoreTools_HEADERS}
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

# after ecm_add_qch, due to KF5NewStuffWidgets_QCH dependency on KF5NewStuff_QCH being declared
add_subdirectory(widgets)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuff LIB_NAME KF5NewStuff DEPS "widgets KXmlGui KNewStuffCore" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

add_subdirectory(qtquick)
add_subdirectory(tools)

ecm_qt_install_logging_categories(
    EXPORT KNEWSTUFF
    FILE knewstuff.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
