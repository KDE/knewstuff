# SPDX-FileCopyrightText: Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6NewStuffWidgets)
add_library(KF6::NewStuffWidgets ALIAS KF6NewStuffWidgets)

ecm_generate_export_header(KF6NewStuffWidgets
    EXPORT_FILE_NAME knewstuffwidgets_export.h
    BASE_NAME KNewStuffWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_sources(KF6NewStuffWidgets PRIVATE
    action.cpp button.cpp
    ../ui/widgetquestionlistener.cpp)

target_link_libraries(KF6NewStuffWidgets PUBLIC KF6::NewStuff)
target_link_libraries(KF6NewStuffWidgets
  PUBLIC
    KF6::NewStuffCore
    KF6::NewStuff
  PRIVATE
    KF6::I18n
    KF6::ConfigCore
    KF6::WidgetsAddons
)

target_include_directories(KF6NewStuffWidgets
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3>")

set_target_properties(KF6NewStuffWidgets PROPERTIES
   VERSION ${KNEWSTUFF_VERSION}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuffWidgets
)

set(KNewStuffWidgets_HEADERS
  action.h
  button.h
)
ecm_generate_headers(KNewStuffWidgets_CamelCase_HEADERS
  HEADER_NAMES
  Action
  Button

  REQUIRED_HEADERS KNewStuffWidgets_HEADERS
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/KNSWidgets
)

install(
    FILES
        ${KNewStuffWidgets_CamelCase_HEADERS}
        ${KNewStuffWidgets_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/knewstuffwidgets_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/KNSWidgets
    COMPONENT Devel
)

# Kept for backwards compatibility, in KF6 all headers will be installed in KNewStuff3/KNSWidgets only
if(QT_MAJOR_VERSION STREQUAL "5")
    install(
        FILES
            ${KNewStuffWidgets_HEADERS}
            ${CMAKE_CURRENT_BINARY_DIR}/knewstuffwidgets_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/knswidgets
        COMPONENT Devel
    )
endif()

install(TARGETS KF6NewStuffWidgets EXPORT KF6NewStuffTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if (BUILD_DESIGNERPLUGIN)
    include(ECMAddQtDesignerPlugin)
    ecm_qtdesignerplugin_widget(KNSWidgets::Button
        INCLUDE_FILE "KNSWidgets/Button"
        TOOLTIP "KHotNewStuff push button that encapsulats most of the details involved in using KHotNewStuff in it."
        GROUP "Buttons (KF6)"
    )

    ecm_add_qtdesignerplugin(knewstuffwidgets
        NAME KNewStuffWidgets
        OUTPUT_NAME knewstuff6widgets
        WIDGETS
            KNSWidgets::Button
        LINK_LIBRARIES
            KF6::NewStuffWidgets
        INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
        COMPONENT Devel
    )
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF6NewStuffWidgets_QCH
        NAME KNewStuffWidgets
        BASE_NAME KF6NewStuffWidgets
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES ${KNewStuffWidgets_HEADERS}
        LINK_QCHS
            KF6NewStuffCore_QCH
            KF6NewStuff_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${KNewStuff_BINARY_DIR}
        BLANK_MACROS
            KNEWSTUFFWIDGETS_EXPORT
            "KNEWSTUFFWIDGETS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

