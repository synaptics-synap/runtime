# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


add_library(ncnn INTERFACE)
target_include_directories(ncnn INTERFACE ${SYNAP_SYSROOT_INCLUDEDIR})
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_link_libraries(ncnn INTERFACE ${SYNAP_SYSROOT_LIBDIR}/libncnnd.a)
else()
    target_link_libraries(ncnn INTERFACE ${SYNAP_SYSROOT_LIBDIR}/libncnn.a)
endif()

if(ANDROID)
    add_link_options(-static-openmp -fopenmp)
    target_link_libraries(ncnn INTERFACE vulkan
        ${SYNAP_SYSROOT_LIBDIR}/libGenericCodeGen.a
        ${SYNAP_SYSROOT_LIBDIR}/libglslang.a
        ${SYNAP_SYSROOT_LIBDIR}/libglslang-default-resource-limits.a
        ${SYNAP_SYSROOT_LIBDIR}/libMachineIndependent.a
        ${SYNAP_SYSROOT_LIBDIR}/libOSDependent.a
        ${SYNAP_SYSROOT_LIBDIR}/libOGLCompiler.a
        ${SYNAP_SYSROOT_LIBDIR}/libSPIRV.a
    )
endif()