# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


set(name synap_app)

file(GLOB SRC src/*.cpp)
add_library(${name} STATIC ${SRC})

target_include_directories(${name}
    PUBLIC inc
)

target_link_libraries(${name} PUBLIC stb synap_utils nlohmann)

if(ANDROID)
    target_link_libraries(${name} PRIVATE log)
endif()

file(GLOB HEADERS inc/synap/*.hpp)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
