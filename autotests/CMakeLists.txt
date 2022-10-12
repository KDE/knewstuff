# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)
include(ECMAddTests)

find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED)
set_package_properties(Qt${QT_MAJOR_VERSION}Test PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for unit tests")

# src also removes -DQT_NO_CAST_FROM_ASCII
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../src ../src )

add_library(knewstuff_test_static STATIC
        ${CMAKE_BINARY_DIR}/src/knewstuff_debug.cpp
        ${CMAKE_BINARY_DIR}/src/core/knewstuffcore_debug.cpp)
target_link_libraries(knewstuff_test_static PUBLIC Qt${QT_MAJOR_VERSION}::Xml Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Gui KF5::KIOCore KF5::NewStuffCore KF5::NewStuff)
macro(knewstuff_unit_tests)
    foreach(_testmain ${ARGN})
       get_filename_component(_testname ${_testmain} NAME_WE)
       ecm_add_test(${_testmain}
                    TEST_NAME ${_testname}
                    NAME_PREFIX "knewstuff-")
       target_compile_definitions(${_testname} PUBLIC -DDATA_DIR="${CMAKE_CURRENT_BINARY_DIR}/data/")
       target_link_libraries(${_testname} knewstuff_test_static)
    endforeach()
endmacro()

knewstuff_unit_tests(
    knewstuffentrytest.cpp
)

# KMoreTools:
ecm_add_test(kmoretools/kmoretoolstest.cpp ../src/knewstuff_debug.cpp
             LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Widgets KF5::NewStuffCore KF5::NewStuff KF5::I18n KF5::KIOGui
             TEST_NAME kmoretoolstest)

# ===========

add_executable(kmoretoolstest2 kmoretools/kmoretoolstest2.cpp ../src/knewstuff_debug.cpp)
# The next line is commented out to prevent CI server to pick up those tests:
# add_test(kmoretoolstest2 kmoretoolstest2) # uncomment to run tests locally
ecm_mark_as_test(kmoretoolstest2)
target_link_libraries(kmoretoolstest2 Qt${QT_MAJOR_VERSION}::Test KF5::NewStuff KF5::I18n KF5::KIOGui)

add_subdirectory(core)
