# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2023 Synaptics Incorporated. All rights reserved.


add_library(timvx INTERFACE)
target_include_directories(timvx INTERFACE ${SYNAP_SYSROOT_INCLUDEDIR})
target_link_libraries(timvx INTERFACE
    ${SYNAP_SYSROOT_LIBDIR}/libtim-vx.so
    ${SYNAP_SYSROOT_LIBDIR}/libovxlib.so
    ${SYNAP_SYSROOT_LIBDIR}/libsynapnb.so
    ${SYNAP_SYSROOT_LIBDIR}/libebg_utils.so
)