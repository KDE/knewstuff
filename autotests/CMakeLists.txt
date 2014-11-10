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
       add_executable(${_testname} ${_testname}.cpp)
       add_test("knewstuff-${_testname}" ${_testname})
       ecm_mark_as_test(${_testname})
       target_link_libraries(${_testname} KF5::NewStuff Qt5::Xml Qt5::Test Qt5::Gui KF5::KIOCore)
    endforeach()
endmacro()

knewstuff_unit_tests(
    knewstuffauthortest
    knewstuffentrytest
)

