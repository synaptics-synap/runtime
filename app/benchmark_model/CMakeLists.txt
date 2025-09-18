# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.

cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

set(projectname "benchmark_model")

PROJECT(${projectname})

ExternalProject_Add(${projectname}
  GIT_REPOSITORY    https://github.com/VeriSilicon/tensorflow.git
  GIT_TAG           nnapi-delegate-2.0
  GIT_SHALLOW       ON
  SOURCE_DIR        ""
  BUILD_ALWAYS      OFF
  CONFIGURE_COMMAND <SOURCE_DIR>/tensorflow/lite/tools/make/download_dependencies.sh
  COMMAND           cp ${CMAKE_CURRENT_SOURCE_DIR}/as371_makefile.inc <SOURCE_DIR>/tensorflow/lite/tools/make/targets
  BUILD_COMMAND     $(MAKE) TARGET_TOOLCHAIN_ROOT=/opt/bin/toolchain/$ENV{TOOLCHAIN_NAME} TARGET_TOOLCHAIN_PREFIX=${TOOLCHAIN_PREFIX} TARGET=${TARGET_OS} BUILD_WITH_NNAPI=true -C <SOURCE_DIR> -f tensorflow/lite/tools/make/Makefile
  INSTALL_COMMAND   mkdir -p /opt/target/bin
  COMMAND           cp <SOURCE_DIR>/tensorflow/lite/tools/make/gen/as371_armv7-a/bin/benchmark_model /opt/target/bin
  TEST_COMMAND      ""
  )

install(CODE "MESSAGE(\"benchmark_model installed.\")")
