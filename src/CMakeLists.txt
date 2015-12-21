project(KNewStuff)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=550)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

set(KNewStuff_SRCS
    downloaddialog.cpp
    downloadwidget.cpp
    downloadmanager.cpp
    entry.cpp
    button.cpp
    knewstuffaction.cpp
    core/author.cpp
    core/cache.cpp
    core/engine.cpp
    core/entryinternal.cpp
    core/installation.cpp
    core/provider.cpp
    core/security.cpp
    core/xmlloader.cpp
    kmoretools/kmoretools.cpp
    kmoretools/kmoretoolsconfigdialog_p.cpp
    kmoretools/kmoretoolsmenufactory.cpp
    kmoretools/kmoretoolspresets.cpp
    staticxml/staticxmlprovider.cpp
    ui/entrydetailsdialog.cpp
    ui/imageloader.cpp
    ui/imagepreviewwidget.cpp
    ui/itemsmodel.cpp
    ui/itemsview.cpp
    ui/itemsviewdelegate.cpp
    ui/itemsgridviewdelegate.cpp
    ui/itemsviewbasedelegate.cpp
    ui/progressindicator.cpp
    attica/atticaprovider.cpp

    upload/atticahelper.cpp
    uploaddialog.cpp
)

ki18n_wrap_ui(KNewStuff_SRCS
    downloadwidget.ui
    uploaddialog.ui
    kmoretools/ui/kmoretoolsconfigwidget.ui
)

add_library(KF5NewStuff ${KNewStuff_SRCS} )
add_library(KF5::NewStuff ALIAS KF5NewStuff)

generate_export_header(KF5NewStuff BASE_NAME KNewStuff EXPORT_FILE_NAME knewstuff_export.h)

target_include_directories(KF5NewStuff PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KF5NewStuff INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3>")

target_link_libraries(KF5NewStuff
  PUBLIC
    KF5::Attica                # For interacting with ocs providers, public for uploaddialog slots
    KF5::XmlGui                # For KActionCollection in knewstuffaction.h
    KF5::Service               # For KMoreTools
    Qt5::Widgets
  PRIVATE
    KF5::KIOCore               # For network fetching operations
    KF5::KIOWidgets            # For KUrlRequester
    KF5::Archive               # For decompressing archives
    KF5::Completion            # For KLineEdit in download dialog
    KF5::I18n                  # For translations
    KF5::ItemViews             # For buttons on download dialog
    KF5::IconThemes           # For KIcon
    KF5::TextWidgets          # For KTextEdit in upload dialog
)

set_target_properties(KF5NewStuff PROPERTIES
   VERSION ${KNEWSTUFF_VERSION_STRING}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuff
)

ecm_generate_headers(KNewStuff_CamelCase_HEADERS
  HEADER_NAMES
  DownloadDialog
  DownloadWidget
  DownloadManager
  UploadDialog
  Entry
  Button

  REQUIRED_HEADERS KNewStuff_HEADERS
  PREFIX KNS3
)

# For KMoreTools:
ecm_generate_headers(KNewStuff_CamelCase_HEADERS_KMoreTools
  HEADER_NAMES
  KMoreTools
  KMoreToolsMenuFactory
  KMoreToolsPresets

  RELATIVE kmoretools
  REQUIRED_HEADERS KNewStuff_HEADERS_KMoreTools
  PREFIX KNS3
)
install(FILES ${KNewStuff_CamelCase_HEADERS_KMoreTools} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNS3 COMPONENT Devel)
install(FILES ${KNewStuff_HEADERS_KMoreTools} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/kns3 COMPONENT Devel
)

install(FILES ${KNewStuff_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNS3 COMPONENT Devel)

install(TARGETS KF5NewStuff EXPORT KF5NewStuffTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_export.h
  knewstuffaction.h
  ${KNewStuff_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/kns3 COMPONENT Devel
)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuff LIB_NAME KF5NewStuff DEPS "widgets Attica KXmlGui" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
