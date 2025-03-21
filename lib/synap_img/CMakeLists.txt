# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


set(name synap_img)

file(GLOB SRC src/*.cpp)
add_library(${name} STATIC ${SRC})

target_include_directories(${name} PUBLIC inc)

if(CMAKE_CROSSCOMPILING)
    target_compile_definitions(${name} PRIVATE STBI_NEON)
endif()

target_link_libraries(${name} PUBLIC stb PRIVATE synap_utils synap_base)

if(ANDROID)
    target_link_libraries(${name} PRIVATE log)
endif()

file(GLOB HEADERS inc/synap/*.hpp)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
