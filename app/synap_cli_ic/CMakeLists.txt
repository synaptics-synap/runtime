# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


set(name "synap_cli_ic")

add_executable(${name} main.cpp)

target_link_libraries(${name} PRIVATE synap_app synap_preprocess synapnb synap_postprocess)

install(TARGETS ${name})
