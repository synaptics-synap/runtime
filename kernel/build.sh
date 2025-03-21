#!/bin/bash

# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2021 Synaptics Incorporated

# directory where this script is located
mod_dir=$(readlink -f "$(dirname "${BASH_SOURCE}")")

# kernel module name
mod_name=synap

# include the common file to build a kernel kernel_module
source build/kernel_module.rc
