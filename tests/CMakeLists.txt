# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets Gui Quick) # Widgets for KMoreTools and Quick for the interactive KNS test

configure_file(khotnewstuff_test.knsrc.in khotnewstuff_test.knsrc @ONLY)

macro(knewstuff_executable_tests)
   foreach(_testname ${ARGN})
       add_executable(${_testname} ${_testname}.cpp  ../src/knewstuff_debug.cpp  ../src/core/knewstuffcore_debug.cpp ../src/staticxml/staticxmlprovider.cpp)
       target_link_libraries(${_testname} KF5::NewStuffCore KF5::NewStuff KF5::I18n Qt${QT_MAJOR_VERSION}::Xml Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Quick Qt${QT_MAJOR_VERSION}::Gui)
       target_compile_definitions(${_testname} PRIVATE
           KNSSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
           KNSBUILDDIR="${CMAKE_CURRENT_BINARY_DIR}")
   endforeach()
endmacro()

knewstuff_executable_tests(
     khotnewstuff_upload
     khotnewstuff_test
)

# KMoreTools:
add_executable(kmoretoolstest_interactive kmoretools/kmoretoolstest_interactive.cpp ../src/knewstuff_debug.cpp)
ecm_mark_as_test(kmoretoolstest_interactive)
target_link_libraries(kmoretoolstest_interactive Qt${QT_MAJOR_VERSION}::Test KF5::NewStuff KF5::I18n Qt${QT_MAJOR_VERSION}::Widgets)
