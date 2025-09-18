# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2023 Synaptics Incorporated. All rights reserved.


cmake_minimum_required(VERSION 3.19)

project(SyNAP_KO LANGUAGES NONE)

if(NOT ${CMAKE_CROSSCOMPILING})
    message(FATAL_ERROR "Cannot compile for host")
endif()

list(APPEND CMAKE_MODULE_PATH ${FRAMEWORK_DIR}/cmake/modules)

find_package(VSSDK REQUIRED)

set(synap_ko_intermediate ${VSSDK_BUILD_DIR}/../target/synap/intermediate/ko)
set(synap_ko ${synap_ko_intermediate}/linux/synap.ko)
file(GLOB src src/*.c src/*.h ../common/inc/*.h ../common/src/*.c)

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(MOD_OPTIONS DEBUG=1)
endif()

add_custom_command(
    OUTPUT ${synap_ko}
    # This is just to make sure the module will be rebuilt if the source is changed.
    DEPENDS ${src} Kbuild build.sh
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${VSSDK_CONFIG_FILE} ${MOD_OPTIONS}
    WORKING_DIRECTORY ${VSSDK_DIR}
)

add_custom_target(synapko ALL DEPENDS ${synap_ko})
add_custom_target(synapko_clean COMMAND rm -rf ${synap_ko_intermediate})

if(${VSSDK_CONFIG_ANDROID_OS})
    set(install_subdir "android")
elseif(${CONFIG_LINUX_OS})
    set(install_subdir "linux")
endif()

install(FILES ${synap_ko} DESTINATION kernel/${install_subdir}/${VSSDK_CONFIG_LINUX_SRC_PATH})
