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

# The src/ dir is needed for the entry.h header. This only happens because some
# code in Core uses an enum from KNS3::Entry
target_include_directories(KF5NewStuffCore
    PUBLIC "$<BUILD_INTERFACE:${KNewStuff_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR};${CMAKE_BINARY_DIR}/src;${CMAKE_SOURCE_DIR}/src;>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/kns3;${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNSCore>")

target_link_libraries(KF5NewStuffCore
  PUBLIC
    KF5::Attica                # For interacting with ocs providers, public for uploaddialog slots
    KF5::CoreAddons
    Qt5::Xml
  PRIVATE
    KF5::Archive               # For decompressing archives
    KF5::I18n                  # For translations
    KF5::ConfigCore
    Qt5::Gui                   # For QImage
)

set_target_properties(KF5NewStuffCore PROPERTIES
   VERSION ${KNEWSTUFF_VERSION_STRING}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuffCore
)

ecm_generate_headers(KNewStuffCore_CamelCase_HEADERS
  HEADER_NAMES
  Author
  Cache
  DownloadManager
  Engine
  EntryInternal
  Installation
  ItemsModel
  Provider
  Question
  QuestionListener
  QuestionManager
  Security
  XmlLoader

  REQUIRED_HEADERS KNewStuffCore_HEADERS
  PREFIX KNSCore
)

install(TARGETS KF5NewStuffCore EXPORT KF5NewStuffCoreTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${KNewStuffCore_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/KNSCore COMPONENT Devel)
install(FILES
    ${KNewStuffCore_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/knewstuffcore_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3/knscore COMPONENT Devel)

if(BUILD_QCH)
    ecm_add_qch(
        KF5NewStuffCore_QCH
        NAME KNewStuffCore
        BASE_NAME KF5NewStuffCore
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES ${KNewStuffCore_HEADERS}
        LINK_QCHS
            KF5Attica_QCH
            KF5CoreAddons_QCH
        BLANK_MACROS
            KNEWSTUFFCORE_EXPORT
            KNEWSTUFFCORE_DEPRECATED
            KNEWSTUFFCORE_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuffCore LIB_NAME KF5NewStuffCore DEPS "Attica" FILENAME_VAR COREPRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KNewStuff3)
install(FILES ${COREPRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

