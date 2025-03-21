# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019 Synaptics Incorporated.

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/modules)
cmake_minimum_required(VERSION 3.19)
project(SyNAP_RUNTIME)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_CROSSCOMPILING)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie  -Wl,--no-undefined -Wl,--no-allow-shlib-undefined")
endif()
# Release build: Strip shared libraries
add_link_options($<$<CONFIG:RELEASE>:-s>)
if(ENABLE_CPU_PROFILING)
    add_compile_options(-pg)
endif()
message("CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

option(ENABLE_EBGRUNTIME "Enable EBGRUNTIME compilation and associated SyNAP support" ON)
option(ENABLE_ONNXRUNTIME "Enable ONNXRUNTIME compilation and associated SyNAP support" OFF)
option(ENABLE_TFLITERUNTIME "Enable TFLITE runtime compilation and associated SyNAP support" OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    message("Old GCC version detected, disabling C++-17 standard filesystem library")
    set(ENABLE_CXX_STD_FILESYSTEM OFF)
else()
    set(ENABLE_CXX_STD_FILESYSTEM ON)
endif()

if(CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Enable file-based bundle models
    set(ENABLE_FILE_BASED_BUNDLE ON)
    add_compile_definitions(SYNAP_FILE_BASED_BUNDLE=1)
endif()

# add common dmabuf allocator options here for lib/app
option(ENABLE_DMA_HEAP_ALLOCATOR "Enable synap DMA-BUF heap allocator" ${VSSDK_KERNEL_DMABUF_ENABLED})
message("ENABLE_DMA_HEAP_ALLOCATOR: ${ENABLE_DMA_HEAP_ALLOCATOR}")

message("ENABLE_EBGRUNTIME: ${ENABLE_EBGRUNTIME}")
message("ENABLE_ONNXRUNTIME: ${ENABLE_ONNXRUNTIME}")
message("ENABLE_TFLITERUNTIME: ${ENABLE_TFLITERUNTIME}")

if(ENABLE_ONNXRUNTIME)
    find_package(Onnxruntime REQUIRED)
endif()

message("ENABLE_NCNN: ${ENABLE_NCNN}")
if(ENABLE_NCNN)
    find_package(NCNN REQUIRED)
endif()

message("ENABLE_TFLITERUNTIME: ${ENABLE_TFLITERUNTIME}")
if(ENABLE_TFLITERUNTIME)
    find_package(tensorflow_lite REQUIRED)
endif()

add_subdirectory(lib)

add_subdirectory(app)
