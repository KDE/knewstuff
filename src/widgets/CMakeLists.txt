# SPDX-FileCopyrightText: Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF5NewStuffWidgets)
add_library(KF5::NewStuffWidgets ALIAS KF5NewStuffWidgets)

target_sources(KF5NewStuffWidgets PRIVATE action.cpp)
target_link_libraries(KF5NewStuffWidgets PUBLIC KF5::NewStuff)
target_link_libraries(KF5NewStuffWidgets
  PUBLIC
    KF5::NewStuffCore
    KF5::NewStuff
  PRIVATE
    KF5::I18n
)
target_include_directories(KF5NewStuffWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNSWidgets>")
target_include_directories(KF5NewStuffWidgets
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNSWidgets;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/knscore>")

set_target_properties(KF5NewStuffWidgets PROPERTIES
   VERSION ${KNEWSTUFF_VERSION}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuffWidgets
)

set(KNewStuffWidgets_HEADERS
  action.h
)
ecm_generate_headers(KNewStuffWidgets_CamelCase_HEADERS
  HEADER_NAMES
  Action

  REQUIRED_HEADERS KNewStuffWidgets_HEADERS
  PREFIX KNSWidgets
)

install(FILES ${KNewStuffWidgets_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNSWidgets COMPONENT Devel)
install(FILES ${KNewStuffWidgets_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/knswidgets COMPONENT Devel)
install(TARGETS KF5NewStuffWidgets EXPORT KF5NewStuffTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
