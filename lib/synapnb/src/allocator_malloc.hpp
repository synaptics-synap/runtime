// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/allocator.hpp"


namespace synaptics {
namespace synap {


class AllocatorMalloc: public Allocator {
public:
    AllocatorMalloc() {}
    ~AllocatorMalloc() {}
    Memory alloc(size_t size) override;
    void dealloc(const Memory& mem) override;
    bool cache_flush(const Memory& mem, size_t size) override;
    bool cache_invalidate(const Memory& mem, size_t size) override;
};


}  // namespace synap
}  // namespace synaptics
