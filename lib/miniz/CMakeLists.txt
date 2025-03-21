# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


add_library(miniz STATIC miniz.c)

target_include_directories(miniz PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(miniz PUBLIC MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS)
