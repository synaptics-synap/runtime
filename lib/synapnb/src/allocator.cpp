// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2020 Synaptics Incorporated. All rights reserved.


#include "synap/allocator.hpp"

using namespace std;

namespace synaptics {
namespace synap {


Allocator* std_allocator()
{
#if SYNAP_EBG_ENABLE
    return synap_allocator();
#else
    return malloc_allocator();
#endif
}



}  // namespace synap
}  // namespace synaptics
