set(qmlplugin_SRCS
    qmlplugin.cpp

    quickengine.cpp
    quickitemsmodel.cpp
    quickquestionlistener.cpp
    quicksettings.cpp
    author.cpp
    categoriesmodel.cpp
    commentsmodel.cpp
    downloadlinkinfo.cpp
)

ecm_qt_declare_logging_category(qmlplugin_SRCS
    HEADER knewstuffquick_debug.h
    IDENTIFIER KNEWSTUFFQUICK
    CATEGORY_NAME kf.newstuff.quick
    OLD_CATEGORY_NAMES org.kde.knewstuff.quick
    DESCRIPTION "knewstuff (qtquick)"
    EXPORT KNEWSTUFF
)

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

# This is used only for the deprecation system - we don't export things from the QML plugin, that's not how they work
ecm_generate_export_header(newstuffqmlplugin
    EXPORT_FILE_NAME knewstuffquick_export.h
    BASE_NAME KNewStuffQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS 5.81
)

install (TARGETS newstuffqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
install (DIRECTORY qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/newstuff)
