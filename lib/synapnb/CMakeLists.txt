# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2023 Synaptics Incorporated. All rights reserved.


add_library(synapnb SHARED)

set_target_properties(synapnb PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/synapnb.map)
target_link_libraries(synapnb PRIVATE -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/synapnb.map -Wl,--gc-sections)

if(CMAKE_CROSSCOMPILING)
    # Some code may benefit from NEON instruction for ARM
    target_compile_definitions(synapnb PRIVATE SYNAP_NB_NEON=1)
endif()

find_package(Threads REQUIRED)
target_link_libraries(synapnb PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_sources(synapnb PRIVATE
    src/allocator.cpp
    src/buffer.cpp
    src/network.cpp
    src/predictor_bundle.cpp
    src/quantization.cpp
    src/tensor.cpp
)

if(ENABLE_EBGRUNTIME)
    message("EBG runtime is enable")
    target_sources(synapnb PRIVATE
        src/allocator_synap.cpp
        src/npu.cpp
        src/predictor_ebg.cpp
        src/synap_driver.cpp
    )
    target_compile_definitions(synapnb PUBLIC SYNAP_EBG_ENABLE=1)
    target_link_libraries(synapnb PUBLIC synap_device)
else()
    # Alloc-based allocator is not available when EBG is also supported since it does not
    # provide aligned allocations
    target_sources(synapnb PRIVATE src/allocator_malloc.cpp)
endif()


if(ENABLE_ONNXRUNTIME)
    target_sources(synapnb PRIVATE src/predictor_onnx.cpp)
    target_link_libraries(synapnb PUBLIC onnxruntime)
    target_compile_definitions(synapnb PRIVATE SYNAP_ONNX_ENABLE=1)
endif()


if(ENABLE_TFLITERUNTIME)
    target_sources(synapnb PRIVATE src/predictor_tflite.cpp)
    target_link_libraries(synapnb PUBLIC tensorflow-lite)
    target_compile_definitions(synapnb PRIVATE SYNAP_TFLITE_ENABLE=1)
endif()

if(ENABLE_TORQRUNTIME)
    target_sources(synapnb PRIVATE
       src/predictor_torq.cpp
    )
    target_link_libraries(synapnb PUBLIC torq-runtime)
    target_compile_definitions(synapnb PRIVATE SYNAP_TORQ_ENABLE=1)
endif()

target_link_libraries(synapnb
    PUBLIC synap_utils
    PUBLIC synap_base
)

set(VSI_DIR ${CMAKE_SOURCE_DIR}/external/vsi_acuity/cmdtools)

target_include_directories(synapnb
    PUBLIC inc
    PRIVATE android
)

file(GLOB HEADERS inc/synap/*.hpp)

set_target_properties(synapnb PROPERTIES PUBLIC_HEADER "${HEADERS}")

if(ENABLE_SECURE_DEBUG AND EXISTS ${CMAKE_CURRENT_LIST_DIR}/src/allocator_secure_debug.cmake)
    # Use debug allocator instead of standard one
    include(${CMAKE_CURRENT_LIST_DIR}/src/allocator_secure_debug.cmake)
endif()

install(TARGETS synapnb)
