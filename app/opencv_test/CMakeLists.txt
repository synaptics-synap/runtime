# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


set(name "opencv_test")

file(GLOB SRC *.cpp)
add_executable(${name} ${SRC})

target_link_libraries(${name} opencv_imgcodecs)
if(ANDROID)
    target_link_libraries(${name} log)
endif()
message("install ${name}")
install(TARGETS ${name})
