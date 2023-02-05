# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(newstuffqmlplugin URI "org.kde.newstuff" VERSION 1.0)

target_sources(newstuffqmlplugin PRIVATE
    qmlplugin.cpp

    quickengine.cpp
    quickitemsmodel.cpp
    quickquestionlistener.cpp
    quicksettings.cpp
    author.cpp
    categoriesmodel.cpp
    commentsmodel.cpp
    downloadlinkinfo.cpp
    searchpresetmodel.cpp
)

ecm_qt_declare_logging_category(newstuffqmlplugin
    HEADER knewstuffquick_debug.h
    IDENTIFIER KNEWSTUFFQUICK
    CATEGORY_NAME kf.newstuff.quick
    OLD_CATEGORY_NAMES org.kde.knewstuff.quick
    DESCRIPTION "knewstuff (qtquick)"
    EXPORT KNEWSTUFF
)

ecm_target_qml_sources(newstuffqmlplugin SOURCES
    qml/NewStuffList.qml
    qml/NewStuffItem.qml
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.1 SOURCES
    qml/Button.qml
    qml/Dialog.qml
    qml/DialogContent.qml
    qml/DownloadItemsSheet.qml
    qml/EntryDetails.qml
    qml/Page.qml
    qml/QuestionAsker.qml
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.81 SOURCES
    qml/Action.qml
)

ecm_target_qml_sources(newstuffqmlplugin VERSION 1.85 SOURCES
    qml/UploadPage.qml
)

ecm_target_qml_sources(newstuffqmlplugin PRIVATE PATH private SOURCES
    qml/private/ConditionalLoader.qml
    qml/private/EntryCommentDelegate.qml
    qml/private/EntryCommentsPage.qml
    qml/private/EntryScreenshots.qml
    qml/private/ErrorDisplayer.qml
    qml/private/GridTileDelegate.qml
    qml/private/MessageBoxSheet.qml
    qml/private/Rating.qml
    qml/private/Shadow.qml
)

ecm_target_qml_sources(newstuffqmlplugin PRIVATE PATH private/entrygriddelegates SOURCES
    qml/private/entrygriddelegates/BigPreviewDelegate.qml
    qml/private/entrygriddelegates/FeedbackOverlay.qml
    qml/private/entrygriddelegates/ThumbDelegate.qml
    qml/private/entrygriddelegates/TileDelegate.qml
)

target_link_libraries (newstuffqmlplugin
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Xml
    KF6::ConfigCore
    KF6::I18n
    KF6::NewStuffCore
)

# This is used only for the deprecation system - we don't export things from the QML plugin, that's not how they work
ecm_generate_export_header(newstuffqmlplugin
    EXPORT_FILE_NAME knewstuffquick_export.h
    BASE_NAME KNewStuffQuick
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

ecm_finalize_qml_module(newstuffqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})
