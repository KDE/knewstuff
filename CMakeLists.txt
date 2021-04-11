cmake_minimum_required(VERSION 3.5)

set(KF_VERSION "5.82.0") # handled by release scripts
set(KF_DEP_VERSION "5.81.0") # handled by release scripts
project(KNewStuff VERSION ${KF_VERSION})

# Require at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FeatureSummary)
find_package(ECM 5.81.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

set(REQUIRED_QT_VERSION 5.15.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Widgets Xml)
find_package(Qt5 ${REQUIRED_QT_VERSION} NO_MODULE COMPONENTS Qml Quick)

find_package(KF5Archive ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Completion ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF5CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF_DEP_VERSION} REQUIRED)
find_package(KF5KIO ${KF_DEP_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Package ${KF_DEP_VERSION} REQUIRED)
find_package(KF5Service ${KF_DEP_VERSION} REQUIRED)
find_package(KF5TextWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF5XmlGui ${KF_DEP_VERSION} REQUIRED)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(KDEClangFormat)
include(KDEGitCommitHooks)

find_package(KF5Attica ${KF_DEP_VERSION} CONFIG REQUIRED)
set_package_properties(KF5Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in KNewStuff"
                       URL "https://commits.kde.org/attica"
                       TYPE REQUIRED
                      )
find_package(KF5Kirigami2 2.12.0)
set_package_properties(KF5Kirigami2 PROPERTIES
    DESCRIPTION "KDE's lightweight user interface framework for mobile and convergent applications"
    URL "https://techbase.kde.org/Kirigami"
    PURPOSE "Required by KNewStuffQuick's qml components"
    TYPE RUNTIME)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFF
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfigVersion.cmake"
                        SOVERSION 5)
ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFFCORE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuffcore_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffCoreConfigVersion.cmake"
                        SOVERSION 5)
ecm_setup_version(PROJECT VARIABLE_PREFIX KNEWSTUFFQUICK
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/knewstuffquick_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffQuickConfigVersion.cmake"
                        SOVERSION 5)

add_definitions(-DTRANSLATION_DOMAIN=\"knewstuff5\")
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055100)
add_definitions(-DQT_NO_FOREACH -DQT_NO_KEYWORDS)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()
add_subdirectory(data)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5NewStuff")
set(CMAKECONFIGCORE_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5NewStuffCore")
set(CMAKECONFIGQUICK_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5NewStuffQuick")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5NewStuff_QCH
        FILE KF5NewStuffQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5NewStuffQchTargets.cmake\")")
    ecm_install_qch_export(
        TARGETS KF5NewStuffCore_QCH
        FILE KF5NewStuffCoreQchTargets.cmake
        DESTINATION "${CMAKECONFIGCORE_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_CORE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5NewStuffCoreQchTargets.cmake\")")
    # TODO: KF5NewStuffQuick
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NewStuffConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NewStuffCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffCoreConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIGCORE_INSTALL_DIR}
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5NewStuffQuickConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffQuickConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIGQUICK_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffCoreConfigVersion.cmake"
  DESTINATION "${CMAKECONFIGCORE_INSTALL_DIR}"
  COMPONENT Devel
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffQuickConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5NewStuffQuickConfigVersion.cmake"
  DESTINATION "${CMAKECONFIGQUICK_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5NewStuffTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5NewStuffTargets.cmake NAMESPACE KF5:: )
install(EXPORT KF5NewStuffCoreTargets DESTINATION "${CMAKECONFIGCORE_INSTALL_DIR}" FILE KF5NewStuffCoreTargets.cmake NAMESPACE KF5:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuff_version.h
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuffcore_version.h
  ${CMAKE_CURRENT_BINARY_DIR}/knewstuffquick_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
