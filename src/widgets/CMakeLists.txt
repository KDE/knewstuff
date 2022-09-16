# SPDX-FileCopyrightText: Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF5NewStuffWidgets)
add_library(KF5::NewStuffWidgets ALIAS KF5NewStuffWidgets)

# Flag to hint this is compilation of the library itself
# Needed because the overloaded signal Action::dialogFinished(QList<KNSCore::Entry>) was only added in a later version,
# but SC requirement needs to keep usages of plain &Action::dialogFinished working.
# So the flag is used to add all signals to the library itself,
# while consumers will only see one variant depending on KNEWSTUFFWIDGETS_DISABLE_DEPRECATED_BEFORE_AND_AT settings
target_compile_definitions(KF5NewStuffWidgets PRIVATE -DKNEWSTUFFWIDGETS_PRIVATE_BUILDING)

ecm_generate_export_header(KF5NewStuffWidgets
    EXPORT_FILE_NAME knewstuffwidgets_export.h
    BASE_NAME KNewStuffWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.91
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_sources(KF5NewStuffWidgets PRIVATE
    action.cpp button.cpp
    ../ui/widgetquestionlistener.cpp)

target_link_libraries(KF5NewStuffWidgets PUBLIC KF5::NewStuff)
target_link_libraries(KF5NewStuffWidgets
  PUBLIC
    KF5::NewStuffCore
    KF5::NewStuff
  PRIVATE
    KF5::I18n
)

target_include_directories(KF5NewStuffWidgets
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3>")

set_target_properties(KF5NewStuffWidgets PROPERTIES
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

install(TARGETS KF5NewStuffWidgets EXPORT KF5NewStuffTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

if (BUILD_DESIGNERPLUGIN)
    include(ECMAddQtDesignerPlugin)
    ecm_qtdesignerplugin_widget(KNSWidgets::Button
        INCLUDE_FILE "KNSWidgets/Button"
        TOOLTIP "KHotNewStuff push button that encapsulats most of the details involved in using KHotNewStuff in it."
        GROUP "Buttons (KF5)"
    )

    ecm_add_qtdesignerplugin(knewstuffwidgets
        NAME KNewStuffWidgets
        WIDGETS
            KNSWidgets::Button
        LINK_LIBRARIES
            KF5::NewStuffWidgets
        INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
        COMPONENT Devel
    )
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF5NewStuffWidgets_QCH
        NAME KNewStuffWidgets
        BASE_NAME KF5NewStuffWidgets
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES ${KNewStuffWidgets_HEADERS}
        LINK_QCHS
            KF5NewStuffCore_QCH
            KF5NewStuff_QCH
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

