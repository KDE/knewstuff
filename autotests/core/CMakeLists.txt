if (WIN32)
set(DATA_DIR "/${CMAKE_CURRENT_BINARY_DIR}/data/")
else()
set(DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data/")
endif()
configure_file(data/entry.xml.in data/entry.xml)
configure_file(data/provider.xml.in data/provider.xml)
configure_file(data/enginetest.knsrc.in data/enginetest.knsrc)

knewstuff_unit_tests(
        knewstuffauthortest.cpp
        knewstuffenginetest.cpp
)
