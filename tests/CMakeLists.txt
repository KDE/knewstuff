
include(ECMMarkAsTest)

find_package(Qt5 5.2.0 CONFIG REQUIRED Test)

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

