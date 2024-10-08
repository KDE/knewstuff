# SPDX-FileCopyrightText: Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

if (WIN32)
    set(DATA_DIR "/${CMAKE_CURRENT_BINARY_DIR}/data/")
else()
    set(DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data/")
endif()
configure_file(data/entry.xml.in data/entry.xml)
configure_file(data/provider.xml.in data/provider.xml)
configure_file(data/enginetest.knsrc.in data/enginetest.knsrc)
configure_file(data/installationtest.knsrc.in data/installationtest.knsrc)

knewstuff_unit_tests(
    knewstuffauthortest.cpp
    knewstuffenginetest.cpp
    installationtest.cpp
)

if(LINUX) # fails on freebsd for unknown reasons
    knewstuff_unit_tests(atticaprovidertest.cpp)
endif()

target_link_libraries(knewstuffenginetest knewstuff_qml_STATIC)
