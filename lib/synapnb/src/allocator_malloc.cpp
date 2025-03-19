// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "allocator_malloc.hpp"
#include "synap/logging.hpp"


using namespace std;

namespace synaptics {
namespace synap {


Allocator::Memory AllocatorMalloc::alloc(size_t size)
{
    void* ptr = aligned_alloc(64, Allocator::align(size, 64));
    LOGV << "Allocated memory of size: " << size << " at address: " << ptr;
    // Use an invalid BID since this memory doesn't come from synap allocator
    return Memory{ptr, 1, -1, 0, 0, (uint32_t)size};
}


void AllocatorMalloc::dealloc(const Memory& mem)
{
    if (mem.address) {
        LOGV << "Releasing memory at address: " << mem.address;
        free(mem.address);
    }
}


bool AllocatorMalloc::cache_flush(const Memory& mem, size_t size)
{
    // This allocator is used only when the entire inference is done in SW
    // so there is no need to flush the cache
    LOGV << "cache_flush ignored";
    return true;
}


bool AllocatorMalloc::cache_invalidate(const Memory& mem, size_t size)
{
    // This allocator is used only when the entire inference is done in SW
    // so there is no need to invalidate the cache
    LOGV << "cache_invalidate ignored";
    return true;
}


Allocator* malloc_allocator()
{
    static AllocatorMalloc allocator{};
    return &allocator;
}

}  // namespace synap
}  // namespace synaptics
