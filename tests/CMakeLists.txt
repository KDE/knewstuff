
include(ECMMarkAsTest)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets) # Widgets for KMoreTools

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
add_executable(kmoretoolstest2 kmoretools/kmoretoolstest2.cpp)
# The next line is commented out to prevent CI server to pick up those tests:
# add_test(kmoretoolstest2 kmoretoolstest2) # uncomment to run tests locally
ecm_mark_as_test(kmoretoolstest2)
target_link_libraries(kmoretoolstest2 Qt5::Test Qt5::Widgets KF5::NewStuff KF5::I18n)
install(FILES kmoretools/2/org.kde.kate.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/2
        RENAME org.kde.kate.desktop)
install(FILES kmoretools/2/org.kde.kate.png
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/2)
install(FILES kmoretools/2/mynotinstalledapp.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/2
        RENAME mynotinstalledapp.desktop)
install(FILES kmoretools/2/mynotinstalledapp.png
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/2)
install(FILES kmoretools/2/mynotinstapp2.desktop.notranslate
        DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kmoretools/unittest-kmoretools/2
        RENAME mynotinstapp2.desktop)
