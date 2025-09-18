# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.


set(PKG_NAME "synap_device")

find_path(SYNAP_DEVICE_INC_DIR synap_device.h
    PATHS ${SYNAP_SYSROOT_INCLUDEDIR} NO_CMAKE_FIND_ROOT_PATH)

find_library(SYNAP_DEVICE_LIB_DIR NAMES synap_device
    PATHS ${SYNAP_SYSROOT_LIBDIR} NO_CMAKE_FIND_ROOT_PATH)

message("SYNAP_DEVICE_INC_DIR: ${SYNAP_DEVICE_INC_DIR}")
message("SYNAP_DEVICE_LIB_DIR: ${SYNAP_DEVICE_LIB_DIR}")

add_library(synap_device INTERFACE)
target_include_directories(synap_device INTERFACE ${SYNAP_DEVICE_INC_DIR})
target_link_libraries(synap_device INTERFACE ${SYNAP_DEVICE_LIB_DIR})
