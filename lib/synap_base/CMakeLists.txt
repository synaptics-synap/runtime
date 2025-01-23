# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019 Synaptics Incorporated.

set(name synap_base)

file(GLOB SOURCES src/*.cpp)

add_library(${name} ${SOURCES})

target_link_libraries(${name}
    PUBLIC synap_utils
    PUBLIC nlohmann
    )

target_include_directories(${name} PUBLIC inc)

file(GLOB HEADERS inc/synap/*.hpp)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
