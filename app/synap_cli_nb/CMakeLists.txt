# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


set(name "synap_cli_nb")

add_executable(${name} main.cpp)

target_link_libraries(${name} PRIVATE synap_app synap_base)

target_link_libraries(${name} PRIVATE ebg_utils)

install(TARGETS ${name})

