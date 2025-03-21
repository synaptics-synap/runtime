set(TARGET_NAME synap)
set(PYBIND11_DIR ${CMAKE_SOURCE_DIR}/extern/pybind11)
set(EXPORT_SRC
    export_preprocessor.cpp
    export_postprocessor.cpp
    export_tensor.cpp
    export_types.cpp
    export.cpp
)

if(NOT EXISTS ${PYBIND11_DIR}/CMakeLists.txt)
    message(FATAL_ERROR "Pybind11 submodule not initialized correctly: ${PYBIND11_DIR}/CMakeLists.txt not found")
endif()
add_subdirectory(${PYBIND11_DIR} ${CMAKE_BINARY_DIR}/pybind11)
pybind11_add_module(${TARGET_NAME} ${EXPORT_SRC})

# manual target naming necessary as Pybind11 infers architecture from host machine python installation instead of aarch64
string(REPLACE "." ";" VERSION_LIST ${PYTHON_VERSION})
list(GET VERSION_LIST 0 PYTHON_MAJOR)
list(GET VERSION_LIST 1 PYTHON_MINOR)
set(PYTHON_COMPACT_VERSION "${PYTHON_MAJOR}${PYTHON_MINOR}")
set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${TARGET_NAME}.cpython-${PYTHON_COMPACT_VERSION}-aarch64-linux-gnu"
    SUFFIX ".so"
)

target_link_libraries(${TARGET_NAME} PRIVATE
    synapnb
    synap_base
    synap_preprocess
    synap_postprocess
)

install(TARGETS ${TARGET_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
)

# copy built library for building pip pacakge
set(TARGET_OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/synap)
add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${TARGET_OUTPUT_DIR}
    COMMENT "Copying built shared object to ${TARGET_OUTPUT_DIR}"
)
