# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


function(parse_config cfg_file key_prefix debug)

    file(STRINGS ${cfg_file} cfg REGEX "^[ ]*[^#].+$")
    if(debug)
#        string(REPLACE ";"  "\n" str "${cfg}")
#        message("VSSDK_BUILD_CONFIG ${str}")
    endif()

    foreach(item IN LISTS cfg)
        string(FIND ${item} "=" pos)
        string(SUBSTRING ${item} 0 ${pos} key)
        math(EXPR pos2 "${pos} + 1")
        string(SUBSTRING ${item} ${pos2} -1 val)
        set(key "${key_prefix}_${key}")
        if(val STREQUAL "y")
            set("${key}" ON CACHE INTERNAL "" FORCE)
        else()
            string(REGEX REPLACE "\"([^\"]+)\"" "\\1" val ${val})
            set("${key}" ${val} CACHE INTERNAL "" FORCE)
        endif()
        if(debug)
            message("${key}\t\t${val}")
        endif()
    endforeach()
endfunction()
