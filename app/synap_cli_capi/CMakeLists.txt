# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2025 Synaptics Incorporated

set(name "synap_cli_capi")

add_executable(${name} main.c)

target_link_libraries(${name} PRIVATE synap_capi)

install(TARGETS ${name})
