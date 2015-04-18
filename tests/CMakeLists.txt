
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test Widgets) # Widgets for KMoreTools

macro(knewstuff_executable_tests)
   foreach(_testname ${ARGN})
       add_executable(${_testname} ${_testname}.cpp)
       target_link_libraries(${_testname} KF5::NewStuff KF5::I18n Qt5::Xml Qt5::Test)
       target_compile_definitions(${_testname} PRIVATE
           KNSSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\""
           KNSBUILDDIR="\\"${CMAKE_CURRENT_BINARY_DIR}\\"")
   endforeach()
endmacro()

knewstuff_executable_tests(
     khotnewstuff
     khotnewstuff_upload
)
# FIXME: port to new API
#knewstuff_executable_tests(
#    knewstuff2_test
#    knewstuff2_download
#    knewstuff2_standard
#    knewstuff2_cache
#)

# KMoreTools:
add_executable(kmoretoolstest kmoretools/kmoretoolstest.cpp)
add_test(kmoretoolstest kmoretoolstest)
ecm_mark_as_test(kmoretoolstest)
target_link_libraries(kmoretoolstest Qt5::Test Qt5::Widgets KF5::NewStuff KF5::I18n)
install(FILES kmoretools/1/a.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/1 RENAME a.desktop)
install(FILES kmoretools/1/b.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/1 RENAME b.desktop)
install(FILES kmoretools/1/c.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/1 RENAME c.desktop)
install(FILES kmoretools/2/kate.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/2 RENAME kate.desktop)
install(FILES kmoretools/2/mynotinstalledapp.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/2 RENAME mynotinstalledapp.desktop)
install(FILES kmoretools/2/mynotinstapp2.desktop.notranslate DESTINATION ${DATA_INSTALL_DIR}/kmoretools/unittest-kmoretools/2 RENAME mynotinstapp2.desktop)
