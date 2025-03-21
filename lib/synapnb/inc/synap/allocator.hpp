// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap allocator.
///

#pragma once

#include <algorithm>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <vector>

namespace synaptics {
namespace synap {


/// Buffer allocator.
/// Allows to allocate aligned memory from different areas.
/// Memory must be allocated such that it completely includes all the cache-lines used for the
/// actual data. This ensures that no cache-line used for data is also used for something else.
class Allocator {
public:
    struct Memory {
        /// Aligned memory pointer
        void* address{};

        /// Memory block handle, allocator specific.
        uintptr_t handle{};

        /// file descriptor used to pass a reference to other drivers/processes that support
        /// dmabuf buffers
        int32_t fd{-1};

        /// handle used to attach this memory area to a network as input/output buffer
        uint32_t bid{};

        /// handle used to pass a reference to this memory area to TAs, invalid after bid is
        /// destroyed
        uint32_t mem_id{};

        /// memory size
        size_t size{};
    };

    /// Allocate memory.
    /// @param size: required memory size in bytes
    /// @return allocated memory information
    virtual Memory alloc(size_t size) = 0;

    /// Deallocate memory.
    /// @param mem: memory block information
    virtual void dealloc(const Memory& mem) = 0;

    /// Flush cache for the entire memory block.
    /// @param mem: memory block information
    /// @param size: memory block size
    /// @return true if success
    virtual bool cache_flush(const Memory& mem, size_t size) = 0;

    /// Invalidate cache for the entire memory block.
    /// @param mem: memory block information
    /// @param size: memory block size
    /// @return true if success
    virtual bool cache_invalidate(const Memory& mem, size_t size) = 0;

    /// The allocator can be used
    /// @return true : true the allocator is ready to allocate, false otherwise
    virtual bool available() const { return true; }

    virtual ~Allocator() {}

    /// Required alignment. This corresponds to the size of a NPU MMU page.
    static constexpr size_t alignment = 4096;

    /// @return val rounded upward to NPU MMU alignment
    static uintptr_t align(uintptr_t val) { return (val + alignment - 1) & ~(alignment - 1); }

    /// @return val rounded upward to alignment num
    static uintptr_t align(uintptr_t val, uint32_t num) { return (val + num - 1) & ~(num - 1); }

    /// @return addr rounded upward to alignment
    static void* align(void* addr) { return (void*)align((uintptr_t)addr); }
    static const void* align(const void* addr) { return (void*)align((uintptr_t)addr); }

protected:
    // Prevent explicit delete since we are using only global instances
    void operator delete(void*) {}
};


/// Get a pointer to the global standard (paged) allocator.
/// @return pointer to standard allocator
Allocator* std_allocator();

// Specific allocators
Allocator* synap_allocator();
Allocator* malloc_allocator();

}  // namespace synap
}  // namespace synaptics
