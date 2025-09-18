# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.


set(SYNAP_USED_ONNXRT_SHARED_LIB ON)

add_library(onnxruntime INTERFACE)
target_include_directories(onnxruntime INTERFACE ${SYNAP_SYSROOT_INCLUDEDIR}/onnxruntime)

if(SYNAP_USED_ONNXRT_SHARED_LIB)
    target_link_libraries(onnxruntime INTERFACE ${SYNAP_SYSROOT_LIBDIR}/libonnxruntime.so)
else()

    # Warning not tested yet and will conflict with tflite

    target_link_libraries(onnxruntime INTERFACE
        libonnx.a
        libonnx_proto.a
        libprotobuf.a
        libabsl_base.a
        libabsl_log_severity.a
        libabsl_malloc_internal.a
        libabsl_raw_logging_internal.a
        libabsl_spinlock_wait.a
        libabsl_throw_delegate.a
        libabsl_city.a
        libabsl_hash.a
        libabsl_low_level_hash.a
        libabsl_raw_hash_set.a
        libabsl_hashtablez_sampler.a
        libre2.a
        libpthreadpool.a
        libXNNPACK.a
        libcpuinfo.a
        libclog.a
        libnsync_cpp.a

        libonnxruntime_flatbuffers.a
        libonnxruntime_common.a
        libonnxruntime_mlas.a
        libonnxruntime_graph.a
        libonnxruntime_framework.a
        libonnxruntime_util.a
        libonnxruntime_providers.a
        libonnxruntime_providers_xnnpack.a
        libonnxruntime_optimizer.a
        libonnxruntime_session.a
        )

        if(ANDROID)
            target_link_libraries(onnxruntime INTERFACE
                libonnxruntime_providers_nnapi.a
            )
        endif()
endif()

