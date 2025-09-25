# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.

set(PKG_NAME "ebg_utils")

find_path(EBG_UTILS_INC_DIR synap/ebg_utils.h
    PATHS ${SYNAP_SYSROOT_INCLUDEDIR} NO_CMAKE_FIND_ROOT_PATH)

find_library(EBG_UTILS_LIB NAMES ebg_utils
    PATHS ${SYNAP_SYSROOT_LIBDIR} NO_CMAKE_FIND_ROOT_PATH)

message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR}")
message("EBG_UTILS_INC_DIR ${EBG_UTILS_INC_DIR}")
message("EBG_UTILS_LIB ${EBG_UTILS_LIB}")

add_library(ebg_utils INTERFACE)
target_include_directories(ebg_utils INTERFACE ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
target_link_libraries(ebg_utils INTERFACE ${EBG_UTILS_LIB})
