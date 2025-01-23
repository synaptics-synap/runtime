// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2022 Synaptics Incorporated */

// Dummy implementation for host compilation.

#include "synap_device.h"

// for ftruncate
#include <unistd.h>
// for memfd_create
#include <sys/mman.h>

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

using namespace std;

typedef uint32_t Fd;
typedef uint32_t Bid;
typedef uint32_t Memid;
typedef uint32_t BufferAttachment;

struct BufferSimulator {
    uint32_t size;
    mutex m;
    Fd fd;
    Memid mem_id;
    bool secure;
};

class MemorySimulator {
public:
    MemorySimulator()
    {
    }

    bool allocate_io_buffer(
            uint32_t size,
            Bid *bid,
            Memid *mem_id,
            Fd *fd)
    {
        const string fd_name = to_string(last_bid_ + 1);
        const int newfd = memfd_create(fd_name.c_str(), MFD_ALLOW_SEALING);
        if (newfd == -1) {
            cerr << "memfd_create() failure" << endl;
            return false;
        }

        if (ftruncate(newfd, size) == -1) {
            cerr << "ftruncate() failure" << endl;
            close(newfd);
            return false;
        }
        *fd = newfd;

        // Insert new BufferSimulator
        last_bid_++;
        BufferSimulator & buf = buffers_[last_bid_];
        *bid = last_bid_;
        buf.fd = newfd;
        buf.mem_id = last_bid_; // arbitrary
        *mem_id = buf.mem_id;
        buf.secure = false;
        return true;
    }

    BufferSimulator * get_buffer(Bid bid)
    {
        auto res = buffers_.find(bid);
        if (res == buffers_.end()) {
            cerr << "Buffer ID not found" << bid << endl;
            return nullptr;
        }
        return &(res->second);
    }

    bool create_io_buffer(
        Fd fd,
        uint32_t offset,
        uint32_t size,
        Bid *bid,
        uint32_t *mem_id,
        bool secure)
    {
        cout << "create_io_buffer fd: " << fd << endl;
        last_bid_++;
        BufferSimulator & buf = buffers_[last_bid_];
        *bid = last_bid_;
        buf.fd = fd;
        buf.size = size;
        buf.mem_id = last_bid_; // arbitrary
        buf.secure = secure;
        return true;
    }

    bool create_io_buffer_from_mem_id(
        uint32_t mem_id,
        uint32_t offset,
        uint32_t size,
        Bid *bid)
    {
        cout << "create_io_buffer_from_mem_id fd: " << mem_id << endl;
        return false;
    }


    bool destroy_io_buffer(
        Bid bid)
    {
        auto res = buffers_.find(bid);
        if (res == buffers_.end()) {
            cerr << "Buffer ID not found" << bid << endl;
            return false;
        }
        BufferSimulator & buf = res->second;
        buffers_.erase(res);
        return true;
    }

private:
    int dma_heap_h_{-1};
    Bid last_bid_{0};
    map<Bid, BufferSimulator> buffers_;
};

// Singleton memory simulator
static MemorySimulator mem;

// NPU Network simulator
class NetworkSimulator {
public:

    struct BufferDescription {
        // Arbitrary
        enum {
            BAD_VALUE = UINT32_MAX
        };
        enum Type {
            UNDEFINED,
            INPUT,
            OUTPUT
        };
        Type type;
        uint32_t index;
        Bid bid;
    };

    NetworkSimulator() {}

    BufferDescription * get_buffer_description(BufferAttachment buffer_attachment)
    {
        auto res = buffers_.find(buffer_attachment);
        if (res == buffers_.end()) {
            cerr << "set_input: Cannot find buffer attachment " << buffer_attachment << endl;
            return nullptr;
        }
        return &res->second;
    }

    bool set_input(
        BufferAttachment buffer_attachment,
        uint32_t input_index)
    {
        auto buf_desc = get_buffer_description(buffer_attachment);
        if (!buf_desc) return false;
        buf_desc->type = BufferDescription::INPUT;
        buf_desc->index = input_index;
        return true;
    }

    bool set_output(
        BufferAttachment buffer_attachment,
        uint32_t output_index)
    {
        auto buf_desc = get_buffer_description(buffer_attachment);
        if (!buf_desc) return false;
        buf_desc->type = BufferDescription::OUTPUT;
        buf_desc->index = output_index;
        return true;
    }

    bool attach_io_buffer(
        Bid bid,
        BufferAttachment *buffer_attachment)
    {
        // TODO: we should look for bid and make sure it was not set before.
        last_buffer_attachment_++;
        BufferDescription bufdesc{BufferDescription::UNDEFINED, BufferDescription::BAD_VALUE, bid};
        bufdesc.bid = bid;
        buffers_[last_buffer_attachment_] = bufdesc;
        *buffer_attachment = last_buffer_attachment_;
        return true;
    }


    bool detach_io_buffer(
        uint32_t buffer_attachment)
    {
        auto res = buffers_.find(buffer_attachment);
        if (res == buffers_.end()) {
            cerr << "detach_io_buffer: buffer attachment not found " << buffer_attachment << endl;
            return false;
        }
        buffers_.erase(res);
        return true;
    }

    bool run()
    {
        // TODO: check that all input/output are set correctly and are coherent

        // Idea: For a given test case with input, network and expected output we could compute
        // the hash of input and of the network and then retrieve the associated output for that
        // test case

        // Nothing to do for now.
        return true;
    }

private:
    BufferAttachment last_buffer_attachment_{0};
    map<BufferAttachment, BufferDescription> buffers_;
};


// NPU simulator
class NpuSimulator {
public:
    NpuSimulator()
    {
    }

    NetworkSimulator * get_network(uint32_t network_handle)
    {
        auto res = networks_.find(network_handle);
        if (res == networks_.end()) {
            cerr << "Network handle not found " << network_handle << endl;
            return nullptr;
        }
        return &(res->second);
    }

    bool set_input(
        uint32_t network_handle,
        BufferAttachment buffer_attachment,
        uint32_t input_index)
    {
        /*
        cout << "set_input() with network_handle=" << network_handle
             << " buffer_attachment=" << buffer_attachment
             << " input_index=" << input_index << endl;
        */
        auto network = get_network(network_handle);
        if (!network) return false;
        return network->set_input(buffer_attachment, input_index);
    }

    bool set_output(
        uint32_t network_handle,
        BufferAttachment buffer_attachment,
        uint32_t output_index)
    {
        /*
        cout << "set_output() with network_handle=" << network_handle
             << " buffer_attachment=" << buffer_attachment
             << " output_index=" << output_index << endl;
        */
        auto network = get_network(network_handle);
        if (!network) return false;
        return network->set_output(buffer_attachment, output_index);
    }


    bool attach_io_buffer(
        uint32_t network_handle,
        Bid bid,
        BufferAttachment * buffer_attachment)
    {
        //cout << "attach_io_buffer network_handle: " << network_handle << " bid: " << bid << endl;
        auto network = get_network(network_handle);
        if (!network) return false;
        return network->attach_io_buffer(bid, buffer_attachment);
    }


    bool detach_io_buffer(
        uint32_t network_handle,
        BufferAttachment buffer_attachment)
    {
        auto network = get_network(network_handle);
        if (!network) return false;
        return network->detach_io_buffer(buffer_attachment);
    }

    bool run_network(
        uint32_t network_handle)
    {
        auto network = get_network(network_handle);
        if (!network) return false;
        return network->run();
    }

    bool prepare_network(
        const void *buffer,
        size_t size,
        uint32_t *network_handle)
    {
        last_network_handle_++;
        networks_[last_network_handle_] = NetworkSimulator();
        //cout << "New Network handle: " << last_network_handle_ << endl;
        *network_handle = last_network_handle_;
        return true;
    }

    bool release_network(
        uint32_t network_handle)
    {
        auto network = get_network(network_handle);
        if (!network) return false;
        networks_.erase(network_handle);
        return true;
    }

    bool lock_npu()
    {
        m_.lock();
        return true;
    }

    bool unlock_npu()
    {
        m_.unlock();
        return true;
    }

private:
    mutex m_;
    uint32_t last_network_handle_{0};
    map<uint32_t, NetworkSimulator> networks_;
};

static unique_ptr<NpuSimulator> npu;

bool synap_init()
{
    npu.reset(new NpuSimulator);
    return true;
}

bool synap_set_input(
    uint32_t network_handle,
    uint32_t attachment_handle,
    uint32_t input_index)
{
    return npu->set_input(network_handle, attachment_handle, input_index);
}

bool synap_set_output(
    uint32_t network_handle,
    uint32_t attachment_handle,
    uint32_t output_index)
{
    return npu->set_output(network_handle, attachment_handle, output_index);
}

bool synap_allocate_io_buffer(
        uint32_t size,
        uint32_t *buffer_handle,
        uint32_t *mem_id,
        uint32_t *dmabuf_fd)
{
    return mem.allocate_io_buffer(size, buffer_handle, mem_id, dmabuf_fd);
}

bool synap_lock_io_buffer(uint32_t dmabuf_fd)
{
    // Note: if we had NPU execution this is the place where we copy the data from the NPU output
    // buffer to the user buffer memory
    return true;
}

bool synap_unlock_io_buffer(uint32_t dmabuf_fd)
{
    // Note: if we had NPU execution this is the place where we copy the data from the user
    // buffer to the memory actually used to perform inference
    return true;
}

bool synap_create_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id)
{
    return mem.create_io_buffer(dmabuf_fd, offset, size, buffer_handle, mem_id, false);
}

bool synap_create_secure_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id)
{
    return mem.create_io_buffer(dmabuf_fd, offset, size, buffer_handle, mem_id, true);
}

bool synap_create_io_buffer_from_mem_id(
    uint32_t mem_id,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle)
{
    return mem.create_io_buffer_from_mem_id(mem_id, offset, size, buffer_handle);
}


bool synap_destroy_io_buffer(uint32_t bid)
{
    return mem.destroy_io_buffer(bid);
}

bool synap_attach_io_buffer(
    uint32_t network_handle,
    uint32_t buffer_handle,
    uint32_t *attachment_handle)
{
    return npu->attach_io_buffer(network_handle, buffer_handle, attachment_handle);
}


bool synap_detach_io_buffer(
    uint32_t network_handle,
    uint32_t attachment_handle)
{
    return npu->detach_io_buffer(network_handle, attachment_handle);
}

bool synap_run_network(
    uint32_t network_handle)
{
    return npu->run_network(network_handle);
}

bool synap_prepare_network(
    const void *buffer,
    size_t size,
    uint32_t *network_handle)
{
    return npu->prepare_network(buffer, size, network_handle);
}

bool synap_release_network(uint32_t network_handle)
{
    return npu->release_network(network_handle);
}

bool synap_lock_npu()
{
    return npu->lock_npu();
}

bool synap_unlock_npu()
{
    return npu->unlock_npu();
}

bool synap_deinit()
{
    npu.reset();
    return true;
}
