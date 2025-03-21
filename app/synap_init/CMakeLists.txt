# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


set(name "synap_init")

add_executable(${name} main.cpp)

target_link_libraries(${name} PRIVATE synap_app synapnb)

install(TARGETS ${name})
