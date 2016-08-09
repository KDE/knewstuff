set(qmlplugin_SRCS
    qmlplugin.cpp

    engine.cpp
    itemsmodel.cpp
)

add_library (newstuffqmlplugin SHARED ${qmlplugin_SRCS})

target_link_libraries (newstuffqmlplugin
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    KF5::NewStuffCore
)

install (TARGETS newstuffqmlplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)
install (DIRECTORY qml DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)
install (FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)