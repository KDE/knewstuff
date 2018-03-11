set(qmlplugin_SRCS
    qmlplugin.cpp

    quickengine.cpp
    quickitemsmodel.cpp
    downloadlinkinfo.cpp
)

add_library (newstuffqmlplugin ${qmlplugin_SRCS})

target_link_libraries (newstuffqmlplugin
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::Xml
    KF5::NewStuffCore
)

install (TARGETS newstuffqmlplugin EXPORT KF5NewStuffQuickTargets DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)
install (DIRECTORY qml DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)
install (FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/newstuff)
