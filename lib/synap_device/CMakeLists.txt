# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


if(CMAKE_CROSSCOMPILING)
    add_library(synap_device STATIC synap_device.c)
elseif(ENABLE_SYNAP_DEVICE_SIMULATOR)
    add_library(synap_device STATIC synap_device_simulator.cpp)
else()
    return()
endif()

target_include_directories(synap_device
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ../../kernel/inc
)

set_target_properties(synap_device PROPERTIES PUBLIC_HEADER synap_device.h)
install(TARGETS synap_device)

if("${SYNAP_BUILD_TYPE}" STREQUAL "VSSDK_REL")
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/ # Warning: Trailing slash is important
        DESTINATION vsi_npu_driver/synap/user)
endif()
