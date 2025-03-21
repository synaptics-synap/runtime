// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap Buffer cache
///

#pragma once

#include "synap/buffer.hpp"
#include <map>
#include <utility>

namespace synaptics {
namespace synap {

/// Maintains a set of Buffers.
/// This is nothing more than a small wrapper around a std::map but makes the use more explicit.
///
/// Example:
/// BufferCache<AMP_BD_HANDLE> buffers;
/// ...
/// AMP_BD_HANDLE bdh = ..;
/// Buffer* b = buffers.get(bdh);
/// if (!b) b = buffers.add(bdh,  get_bd_data(bdh), get_bd_size(bdh));
/// ...
template <typename Id>
class BufferCache {
public:
    typedef std::map<std::pair<Id,size_t>, Buffer> Map;

    /// Create Buffer set.
    /// @param allow_cpu_access: if true buffers will be created with CPU access enabled
    BufferCache(bool allow_cpu_access = true) {}

    /// Get Buffer associated to this id
    /// @param buffer_id: unique buffer id (typically a pointer or handle)
    /// @param offset: offset inside the buffer
    /// @return pointer to Buffer object associated to this id if present else nullptr
    Buffer* get(Id buffer_id, size_t offset = 0)
    {
        auto item = _buffers.find(std::make_pair(buffer_id, offset));
        return item == _buffers.end() ? nullptr : &item->second;
    }

    /// Add Buffer for the specified address and size.
    /// @param buffer_id: unique buffer id
    /// @param mem_id: mem_id of the buffer.
    /// @param data_size: size of buffer data. Must be a multiple of Allocator::align
    /// @param offset: offset inside the buffer
    /// @return pointer to a Buffer object referencing the specified address
    Buffer* add(Id buffer_id, uint32_t mem_id, size_t data_size, size_t offset = 0)
    {
        Buffer buffer(mem_id, offset, data_size);
        return &_buffers.emplace(std::make_pair(buffer_id, offset), std::move(buffer)).first->second;
    }

    /// Get Buffer associated to this id if it exists, else create a new Buffer
    Buffer* get(Id buffer_id, uint32_t mem_id, size_t data_size, size_t offset = 0)
    {
        Buffer* buffer = get(buffer_id, offset);
        return buffer ? buffer : add(buffer_id, mem_id, data_size, offset);
    }

    /// @return number of buffers in the cache
    size_t size() const { return _buffers.size(); }

    /// Clear the cache
    void clear() { _buffers.clear(); }

    /// Iterate buffers
    /// @return buffers map iterator
    typename Map::iterator begin() { return _buffers.begin(); }
    typename Map::iterator end() { return _buffers.end(); }

protected:
    Map _buffers;
};


}  // namespace synap
}  // namespace synaptics
