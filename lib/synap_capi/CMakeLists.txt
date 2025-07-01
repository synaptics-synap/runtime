# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2025 Synaptics Incorporated

set(name synap_capi)

set(SOURCES cnetwork.cpp)
set(HEADERS cnetwork.h)

add_library(${name} STATIC ${SOURCES})
target_link_libraries(${name} PRIVATE synapnb)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})