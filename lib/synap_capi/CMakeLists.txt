# Copyright 2025 Synaptics Incorporated
# SPDX-License-Identifier: Apache-2.0

set(name synap_capi)

set(SOURCES synap_cnetwork.cpp)
set(HEADERS synap_cnetwork.h)

add_library(${name} STATIC ${SOURCES})
target_link_libraries(${name} PRIVATE synapnb)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})