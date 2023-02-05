# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

set(KNewStuffCore_SRCS
    author.cpp
    commentsmodel.cpp
    cache.cpp
    engine.cpp
    entryinternal.cpp
    entrywrapper.cpp
    imageloader.cpp
    installation.cpp
    itemsmodel.cpp
    provider.cpp
    providersmodel.cpp
    tagsfilterchecker.cpp
    xmlloader.cpp
    errorcode.cpp

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

    # A simple wrapper around KPackage operations, which allows for asynchronous interaction
    jobs/kpackagejob.cpp

    ../attica/atticaprovider.cpp
    ../staticxml/staticxmlprovider.cpp
)
if(KF6Syndication_FOUND)
    set(KNewStuffCore_syndication_SRCS
        ../opds/opdsprovider.cpp
        )
endif()

ecm_qt_declare_logging_category(KNewStuffCore_SRCS
    HEADER knewstuffcore_debug.h
    IDENTIFIER KNEWSTUFFCORE
    CATEGORY_NAME kf.newstuff.core
    OLD_CATEGORY_NAMES org.kde.knewstuff.core
    DESCRIPTION "knewstuff (Core Lib)"
    EXPORT KNEWSTUFF
)

add_library(KF6NewStuffCore ${KNewStuffCore_SRCS} ${KNewStuffCore_syndication_SRCS})
add_library(KF6::NewStuffCore ALIAS KF6NewStuffCore )

ecm_generate_export_header(KF6NewStuffCore
    EXPORT_FILE_NAME knewstuffcore_export.h
    BASE_NAME KNewStuffCore
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS 5.31 5.36 5.53 5.71 5.74 5.77 5.83
)

# The src/ dir is needed for the entry.h header. This only happens because some
# code in Core uses an enum from KNS3::Entry
set(KNewStuffCore_BUILD_INCLUDE_DIRS
    ${KNewStuff_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

target_include_directories(KF6NewStuffCore
    PUBLIC "$<BUILD_INTERFACE:${KNewStuffCore_BUILD_INCLUDE_DIRS}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3>"
)

target_link_libraries(KF6NewStuffCore
  PUBLIC
    KF6::Attica                # For interacting with ocs providers, public for uploaddialog slots
    KF6::CoreAddons
    Qt6::Xml
  PRIVATE
    KF6::Archive               # For decompressing archives
    KF6::I18n                  # For translations
    KF6::ConfigCore
    KF6::Package
    Qt6::Gui                   # For QImage
)

if(KF6Syndication_FOUND)
    target_compile_definitions(KF6NewStuffCore PRIVATE -DSYNDICATION_FOUND="${KF6Syndication_FOUND}")
    target_link_libraries(KF6NewStuffCore
        PRIVATE
        KF6::Syndication #OPDS
    )
endif()

set_target_properties(KF6NewStuffCore PROPERTIES
   VERSION ${KNEWSTUFF_VERSION}
   SOVERSION ${KNEWSTUFF_SOVERSION}
   EXPORT_NAME NewStuffCore
)

ecm_generate_headers(KNewStuffCore_CamelCase_HEADERS
  HEADER_NAMES
  Author
  Cache
  Engine
  EntryInternal
  EntryWrapper
  ErrorCode
  Installation
  ItemsModel
  Provider
  ProvidersModel
  Question
  QuestionListener
  QuestionManager
  TagsFilterChecker
  XmlLoader

  REQUIRED_HEADERS KNewStuffCore_HEADERS
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/KNSCore
)

install(TARGETS KF6NewStuffCore EXPORT KF6NewStuffCoreTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${KNewStuffCore_CamelCase_HEADERS}
        ${KNewStuffCore_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/knewstuffcore_export.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3/KNSCore
    COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6NewStuffCore_QCH
        NAME KNewStuffCore
        BASE_NAME KF6NewStuffCore
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES ${KNewStuffCore_HEADERS}
        LINK_QCHS
            KF6Attica_QCH
            KF6CoreAddons_QCH
        INCLUDE_DIRS
            ${KNewStuffCore_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KNEWSTUFFCORE_EXPORT
            KNEWSTUFFCORE_DEPRECATED
            KNEWSTUFFCORE_DEPRECATED_EXPORT
            "KNEWSTUFFCORE_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KNewStuffCore LIB_NAME KF6NewStuffCore DEPS "Attica" FILENAME_VAR COREPRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KNewStuff3)
install(FILES ${COREPRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

