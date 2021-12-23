# SPDX-FileCopyrightText: Dan Leinir Turthra Jensen <admin@leinir.dk>
# SPDX-License-Identifier: BSD-2-Clause

set(knewstuff-dialog_SRCS
    main.cpp
    knsrcmodel.cpp
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(knewstuff-dialog ${knewstuff-dialog_SRCS} ${RESOURCES})

target_link_libraries(knewstuff-dialog
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Qml
    Qt${QT_MAJOR_VERSION}::Quick
    KF5::ConfigCore
    KF5::I18n
    KF5::NewStuffCore
)

install(TARGETS knewstuff-dialog ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
