set(knewstuff-dialog_SRCS
    main.cpp
    knsrcmodel.cpp
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(knewstuff-dialog ${knewstuff-dialog_SRCS} ${RESOURCES})

target_link_libraries(knewstuff-dialog
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Qml
    Qt5::Quick
    KF5::ConfigCore
    KF5::I18n
    KF5::NewStuffCore
)

install(TARGETS knewstuff-dialog ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
