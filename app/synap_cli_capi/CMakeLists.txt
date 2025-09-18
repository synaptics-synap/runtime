# Copyright 2025 Synaptics Incorporated
# SPDX-License-Identifier: Apache-2.0

set(name "synap_cli_capi")

add_executable(${name} main.c)

target_link_libraries(${name} PRIVATE synap_capi)

install(TARGETS ${name})
