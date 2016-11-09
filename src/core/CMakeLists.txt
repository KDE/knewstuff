project(KNewStuffCore)

set(KNewStuffCore_SRCS
    author.cpp
    cache.cpp
    downloadmanager.cpp
    engine.cpp
    entryinternal.cpp
    imageloader.cpp
    installation.cpp
    itemsmodel.cpp
    provider.cpp
    security.cpp
    xmlloader.cpp

    # A system by which queries can be passed to the user, and responses
    # gathered, depending on implementation. See question.h for details.
    question.cpp
    questionmanager.cpp
    questionlistener.cpp

    # A set of minimal KJob based classes, designed to replace the
    # more powerful KIO based system in places where KIO is not available
    # for one reason or another.
    jobs/downloadjob.cpp
    jobs/filecopyjob.cpp
    jobs/filecopyworker.cpp
    jobs/httpjob.cpp
    jobs/httpworker.cpp

    ../attica/atticaprovider.cpp
    ../staticxml/staticxmlprovider.cpp

    ../upload/atticahelper.cpp
)

ecm_qt_declare_logging_category(KNewStuffCore_SRCS HEADER knewstuffcore_debug.h IDENTIFIER KNEWSTUFFCORE CATEGORY_NAME org.kde.knewstuff.core)

add_library(KF5NewStuffCore ${KNewStuffCore_SRCS} )
add_library(KF5::NewStuffCore ALIAS KF5NewStuffCore )

generate_export_header(KF5NewStuffCore BASE_NAME KNewStuffCore EXPORT_FILE_NAME knewstuffcore_export.h)

target_include_directories(KF5NewStuffCore PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_include_directories(KF5NewStuffCore INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3>")

target_link_libraries(KF5NewStuffCore
  PUBLIC
    KF5::Attica                # For interacting with ocs providers, public for uploaddialog slots
    KF5::Archive               # For decompressing archives
    KF5::CoreAddons
  PRIVATE
    KF5::I18n                  # For translations
    KF5::ConfigCore
    Qt5::Gui                   # For QImage
    Qt5::Xml
)

set_target_properties(KF5NewStuffCore PROPERTIES
   VERSION ${KNEWSTUFF_VERSION_STRING}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuffCore
)

install(TARGETS KF5NewStuffCore EXPORT KF5NewStuffCoreTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuffCore LIB_NAME KF5NewStuffCore DEPS "Attica" FILENAME_VAR COREPRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff)