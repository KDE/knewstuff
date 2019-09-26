set(qmlplugin_SRCS
    qmlplugin.cpp

    quickengine.cpp
    quickitemsmodel.cpp
    quickquestionlistener.cpp
    author.cpp
    categoriesmodel.cpp
    commentsmodel.cpp
    downloadlinkinfo.cpp
)

ecm_qt_declare_logging_category(qmlplugin_SRCS HEADER knewstuffquick_debug.h IDENTIFIER KNEWSTUFFQUICK CATEGORY_NAME org.kde.knewstuff.quick)

add_library (newstuffqmlplugin ${qmlplugin_SRCS})

target_link_libraries (newstuffqmlplugin
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    Qt5::Xml
    KF5::ConfigCore
    KF5::I18n
    KF5::NewStuffCore
)

install (TARGETS newstuffqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
install (DIRECTORY qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
