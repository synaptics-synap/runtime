# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


set(name "synap_cli_ip")

add_executable(${name} main.cpp)

target_link_libraries(${name} PRIVATE synap_app synap_preprocess synapnb synap_postprocess)
if(ENABLE_PRIVATE_ALLOCATORS)
    target_link_libraries(${name} PRIVATE synap_allocators)
endif()

install(TARGETS ${name})


