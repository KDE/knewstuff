include(ECMMarkAsTest)

find_package(Qt5Test ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED)
set_package_properties(Qt5Test PROPERTIES
    TYPE REQUIRED
    PURPOSE "Required for unit tests")

# src also removes -DQT_NO_CAST_FROM_ASCII
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../src ../src )

macro(knewstuff_unit_tests)
    foreach(_testname ${ARGN})
       add_executable(${_testname} ${_testname}.cpp ../src/core/author.cpp ../src/core/entryinternal.cpp ../src/entry.cpp ../src/core/xmlloader.cpp ../src/core/jobs/httpjob.cpp ../src/core/jobs/httpworker.cpp  ../src/knewstuff_debug.cpp  ../src/knewstuffcore_debug.cpp)
       # fake static linking to prevent the export macros on windows to kick in.
       set_target_properties(${_testname} PROPERTIES COMPILE_FLAGS -DKNEWSTUFF_STATIC_DEFINE)
       add_test("knewstuff-${_testname}" ${_testname})
       ecm_mark_as_test(${_testname})
       target_link_libraries(${_testname} Qt5::Xml Qt5::Test Qt5::Gui KF5::KIOCore KF5::NewStuffCore)
    endforeach()
endmacro()

knewstuff_unit_tests(
    knewstuffauthortest
    knewstuffentrytest
)

# KMoreTools:
add_executable(kmoretoolstest kmoretools/kmoretoolstest.cpp ../src/knewstuff_debug.cpp)
add_test(kmoretoolstest kmoretoolstest)
ecm_mark_as_test(kmoretoolstest)
target_link_libraries(kmoretoolstest Qt5::Test Qt5::Widgets KF5::NewStuffCore KF5::NewStuff KF5::I18n)
install(FILES kmoretools/1/a.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/1
        RENAME a.desktop)
install(FILES kmoretools/1/b.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/1
        RENAME b.desktop)
install(FILES kmoretools/1/c.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/1
        RENAME c.desktop)
