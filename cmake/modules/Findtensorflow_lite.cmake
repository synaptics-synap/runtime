# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.


add_library(tensorflow-lite INTERFACE)

target_include_directories(tensorflow-lite INTERFACE ${SYNAP_SYSROOT_INCLUDEDIR})

set(USE_TFLITE_SHARED ON)

if(USE_TFLITE_SHARED)
    target_link_libraries(tensorflow-lite INTERFACE ${SYNAP_SYSROOT_LIBDIR}/libtensorflow-lite.so)
    return()
endif()

# order of libraries is important
set(TFLITE_LIBS
    ${SYNAP_SYSROOT_LIBDIR}/libtensorflow-lite.a
    XNNPACK farmhash flatbuffers
    fft2d_fftsg fft2d_fftsg2d.a
    eight_bit_int_gemm cpuinfo_internals cpuinfo pthreadpool)

# order of libraries is important

set(ABSL_LIBS
 absl_log_severity
 absl_throw_delegate absl_scoped_set_env absl_strerror absl_hash absl_city
 absl_low_level_hash absl_graphcycles_internal absl_raw_hash_set
 absl_hashtablez_sampler absl_status absl_statusor absl_time
 absl_time_zone absl_civil_time absl_crc_cord_state absl_crc_internal absl_crc32c
 absl_cordz_functions absl_cordz_info absl_strings absl_cord_internal absl_str_format_internal
 absl_cordz_sample_token absl_cordz_handle absl_cord absl_stacktrace absl_symbolize
 absl_synchronization absl_spinlock_wait absl_int128 absl_base absl_malloc_internal absl_raw_logging_internal
)

set(RUY_LIBS ruy_ctx ruy_context ruy_cpuinfo ruy_prepacked_cache ruy_thread_pool
    ruy_wait ruy_allocator ruy_system_aligned_alloc
    ruy_context_get_ctx ruy_denormal  ruy_tune ruy_blocking_counter
    ruy_apply_multiplier ruy_frontend
    ruy_prepare_packed_matrices ruy_trmul ruy_block_map)

if(CMAKE_CROSSCOMPILING)
    list(APPEND RUY_LIBS ruy_pack_arm ruy_kernel_arm)
else()
    list(APPEND RUY_LIBS
        ruy_kernel_avx ruy_pack_avx ruy_have_built_path_for_avx
        ruy_kernel_avx512 ruy_pack_avx512 ruy_have_built_path_for_avx512
        ruy_kernel_avx2_fma ruy_pack_avx2_fma ruy_have_built_path_for_avx2_fma
        )
endif()

target_link_libraries(tensorflow-lite INTERFACE ${TFLITE_LIBS} ${ABSL_LIBS} ${RUY_LIBS})
