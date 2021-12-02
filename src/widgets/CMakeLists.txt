# SPDX-FileCopyrightText: Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(NewStuffWidgets)
add_library(KF5::NewStuffWidgets ALIAS NewStuffWidgets)

target_sources(NewStuffWidgets PRIVATE action.cpp)
target_link_libraries(NewStuffWidgets PUBLIC KF5::NewStuff)
target_link_libraries(NewStuffWidgets
  PUBLIC
    KF5::NewStuffCore
    KF5::NewStuff
  PRIVATE
    KF5::I18n
)
target_include_directories(NewStuffWidgets INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff/KNSWidgets>")
target_include_directories(NewStuffWidgets
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff/KNSWidgets;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/knscore>")

set(KNewStuffWidgets_HEADERS
  action.h
)
ecm_generate_headers(KNewStuffWidgets_CamelCase_HEADERS
  HEADER_NAMES
  Action

  REQUIRED_HEADERS KNewStuffWidgets_HEADERS
  PREFIX KNSWidgets
)

install(FILES ${KNewStuffWidgets_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff/KNSWidgets COMPONENT Devel)
install(FILES ${KNewStuffWidgets_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff/knswidgets COMPONENT Devel)
install(TARGETS NewStuffWidgets EXPORT KF5NewStuffTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
