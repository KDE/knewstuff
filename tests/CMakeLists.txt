# SPDX-FileCopyrightText: KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets Gui Quick) # Widgets for KMoreTools and Quick for the interactive KNS test

configure_file(khotnewstuff_test.knsrc.in khotnewstuff_test.knsrc @ONLY)

macro(knewstuff_executable_tests)
   foreach(_testname ${ARGN})
       add_executable(${_testname} ${_testname}.cpp  ../src/knewstuff_debug.cpp  ../src/core/knewstuffcore_debug.cpp ../src/staticxml/staticxmlprovider.cpp ../src/core/xmlloader.cpp ../src/core/jobs/httpjob.cpp  ../src/core/jobs/httpworker.cpp)
       target_link_libraries(${_testname} KF6::NewStuffCore KF6::NewStuff KF6::I18n KF6::ConfigCore Qt6::Xml Qt6::Test Qt6::Quick Qt6::Gui)
       target_compile_definitions(${_testname} PRIVATE
           KNSSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
           KNSBUILDDIR="${CMAKE_CURRENT_BINARY_DIR}")
   endforeach()
endmacro()

knewstuff_executable_tests(
     khotnewstuff_test
)

# KMoreTools:
add_executable(kmoretoolstest_interactive kmoretools/kmoretoolstest_interactive.cpp)
ecm_mark_as_test(kmoretoolstest_interactive)
target_link_libraries(kmoretoolstest_interactive Qt6::Test KF6::NewStuff KF6::I18n Qt6::Widgets KF6::KIOGui)
