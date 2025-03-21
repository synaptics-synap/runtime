# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

set(name synap_postprocess)

file(GLOB SOURCES src/*.cpp)
add_library(${name} ${SOURCES})
target_include_directories(${name} PUBLIC inc)
target_link_libraries(${name} PRIVATE synapnb synap_img nlohmann)

if(CMAKE_CROSSCOMPILING)
    target_compile_definitions(${name} PRIVATE SYNAP_NB_NEON=1)
endif()

file(GLOB HEADERS inc/synap/*.hpp)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
