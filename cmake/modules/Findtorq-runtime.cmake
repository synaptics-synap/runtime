# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.

add_library(torq-runtime INTERFACE)
target_include_directories(torq-runtime INTERFACE ${SYNAP_SYSROOT_INCLUDEDIR})
target_link_libraries(torq-runtime INTERFACE ${SYNAP_SYSROOT_LIBDIR}/libiree_runtime_unified.a ${SYNAP_SYSROOT_LIBDIR}/libdriver_torq_full.a)

