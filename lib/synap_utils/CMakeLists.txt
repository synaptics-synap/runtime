# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


option(SYNAP_LOGS_TO_STDERR "Enable synap logging messages to stderr" OFF)

set(name synap_utils)

file(GLOB SRC
    src/file_utils.cpp
    src/string_utils.cpp
    src/zip_tool.cpp
    src/bundle_parser_zip.cpp
    src/bundle_parser.cpp
    )

if(ENABLE_FILE_BASED_BUNDLE)
    list(APPEND SRC src/bundle_parser_dir.cpp)
endif()

add_library(${name} STATIC ${SRC})

target_include_directories(${name}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(${name} PRIVATE miniz nlohmann)

if(ENABLE_CXX_STD_FILESYSTEM)
    target_compile_definitions(${name} PUBLIC ENABLE_STD_FILESYSTEM)
endif()

if(SYNAP_LOGS_TO_STDERR)
    target_compile_definitions(${name} PUBLIC SYNAP_LOGS_TO_STDERR)
endif()

if(ANDROID)
    target_link_libraries(${name} PRIVATE log)
endif()

file(GLOB HEADERS inc/synap/*.hpp)
set_target_properties(${name} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${name})
