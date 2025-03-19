// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/allocator.hpp"


namespace synaptics {
namespace synap {


class AllocatorSynap: public Allocator {
public:
    AllocatorSynap() {}
    ~AllocatorSynap() {}
    Memory alloc(size_t size) override;
    void dealloc(const Memory& mem) override;
    bool cache_flush(const Memory& mem, size_t size) override;
    bool cache_invalidate(const Memory& mem, size_t size) override;

protected:
    static bool suspend_cpu_access(int fd);
    static bool resume_cpu_access(int fd);
};


}  // namespace synap
}  // namespace synaptics
