# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.

set(PKG_NAME "NNA")

set(INCLUDES VX vsi_nn_ops.h)
list(LENGTH INCLUDES count)
math(EXPR count "${count} - 1")
foreach(i RANGE ${count})
  list(GET INCLUDES ${i} inc)
  find_path(${PKG_NAME}_FOUND_PATH_${i}
    NAMES ${inc}
    PATHS ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES include 
  )
  message("found include ${subdir} ${inc} at ${${PKG_NAME}_FOUND_PATH_${i}}")
  list(APPEND ${PKG_NAME}_INCLUDE_DIRS ${${PKG_NAME}_FOUND_PATH_${i}})
endforeach()

set(LIBS ovxlib)
list(LENGTH LIBS count)
math(EXPR count "${count} - 1")
foreach(i RANGE ${count})
  list(GET LIBS ${i} lib)
  find_library(${PKG_NAME}_FOUND_LIB_${i}
    NAMES ${lib}
    PATHS ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES . lib nna-driver/usr/lib 
  )
  message("found lib ${lib} at ${${PKG_NAME}_FOUND_LIB_${i}}")
  list(APPEND ${PKG_NAME}_LIBRARIES ${${PKG_NAME}_FOUND_LIB_${i}})
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${PKG_NAME} DEFAULT_MSG
  ${PKG_NAME}_LIBRARIES ${PKG_NAME}_INCLUDE_DIRS)

mark_as_advanced(${PKG_NAME}_INCLUDE_DIRS ${PKG_NAME}_LIBRARIES)

