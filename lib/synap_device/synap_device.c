// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2021 Synaptics Incorporated */

#include "synap_device.h"
#include "uapi/synap.h"

#include <linux/dma-buf.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define INIT_FD -255
static signed int gfd = INIT_FD;

bool synap_init()
{
    if (gfd >= 0 ) return true;
    gfd = open("/dev/synap", O_RDWR);
    return (gfd >= 0);
}

bool synap_set_input(
    uint32_t network_handle,
    uint32_t attachment_handle,
    uint32_t input_index)
{
    if (gfd < 0) return false;
    struct synap_set_network_io_data data = {
        .nid = network_handle,
        .aid = attachment_handle,
        .index = input_index
    };
    return ioctl(gfd, SYNAP_SET_NETWORK_INPUT, &data) == 0;
}

bool synap_set_output(
    uint32_t network_handle,
    uint32_t attachmetn_handle,
    uint32_t output_index)
{
    if (gfd < 0) return false;
    struct synap_set_network_io_data data = {
        .nid = network_handle,
        .aid = attachmetn_handle,
        .index = output_index
    };
    return ioctl(gfd, SYNAP_SET_NETWORK_OUTPUT, &data) == 0;
}

bool synap_allocate_io_buffer(
        uint32_t size,
        uint32_t *buffer_handle,
        uint32_t *mem_id,
        uint32_t *dmabuf_fd) {

    struct synap_create_io_buffer_data data = {
        .size = (__u32) size
    };

    if (ioctl(gfd, SYNAP_CREATE_IO_BUFFER, &data) < 0) {
        return false;
    }

    *dmabuf_fd = data.fd;
    *buffer_handle = data.bid;

    if (mem_id) {
        *mem_id = data.mem_id;
    }

    return true;
}

bool synap_lock_io_buffer(uint32_t dmabuf_fd) {
    struct dma_buf_sync syncData = { .flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW };

    return ioctl(dmabuf_fd, DMA_BUF_IOCTL_SYNC, &syncData) >= 0;
}

bool synap_unlock_io_buffer(uint32_t dmabuf_fd) {
    struct dma_buf_sync syncData = { .flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW };

    return ioctl(dmabuf_fd, DMA_BUF_IOCTL_SYNC, &syncData) >= 0;
}

bool synap_create_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id) {

    if (gfd < 0) return false;
    struct synap_create_io_buffer_from_dmabuf_data data = {
        .fd = dmabuf_fd,
        .size = size,
        .offset = offset,
    };

    if (ioctl(gfd, SYNAP_CREATE_IO_BUFFER_FROM_DMABUF, &data) == 0) {

        *buffer_handle = data.bid;

        if (mem_id) {
            *mem_id = data.mem_id;
        }

        return true;
    }
    return false;
}

bool synap_create_secure_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id) {

    if (gfd < 0) return false;
    struct synap_create_secure_io_buffer_from_dmabuf_data data = {
        .dmabuf_data = {
            .fd = dmabuf_fd,
            .size = size,
            .offset = offset,
        },
        .secure = 1
    };

    if (ioctl(gfd, SYNAP_CREATE_SECURE_IO_BUFFER_FROM_DMABUF, &data) == 0) {

        *buffer_handle = data.dmabuf_data.bid;

        if (mem_id) {
            *mem_id = data.dmabuf_data.mem_id;
        }

        return true;
    }
    return false;
}

bool synap_create_io_buffer_from_mem_id(
    uint32_t mem_id,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle) {

    if (gfd < 0) return false;
    struct synap_create_io_buffer_from_mem_id_data data = {
        .mem_id = mem_id,
        .size = size,
        .offset = offset,
    };

    if (ioctl(gfd, SYNAP_CREATE_IO_BUFFER_FROM_MEM_ID, &data) == 0) {
        *buffer_handle = data.bid;
        return true;
    }
    return false;
}


bool synap_destroy_io_buffer(
    uint32_t bid) {
    if (gfd < 0) return false;
    return ioctl(gfd, SYNAP_DESTROY_IO_BUFFER, &bid) == 0;
}

bool synap_attach_io_buffer(
    uint32_t network_handle,
    uint32_t buffer_handle,
    uint32_t *attachment_handle)
{
    if (gfd < 0) return false;
    struct synap_attach_io_buffer_data data = {
        .nid = network_handle,
        .bid = buffer_handle
    };

    if (ioctl(gfd, SYNAP_ATTACH_IO_BUFFER, &data) == 0) {
        *attachment_handle = data.aid;
        return true;
    }
    return false;
}


bool synap_detach_io_buffer(
    uint32_t network_handle,
    uint32_t attachment_handle)
{
    if (gfd < 0) return false;
    struct synap_attachment_data data = {
        .nid = network_handle,
        .aid = attachment_handle
    };
    return ioctl(gfd, SYNAP_DETACH_IO_BUFFER, &data) == 0;
}

bool synap_run_network(
    uint32_t network_handle)
{
    if (gfd < 0) return false;
    return ioctl(gfd, SYNAP_RUN_NETWORK, &network_handle) == 0;
}

bool synap_prepare_network(
    const void *buffer,
    size_t size,
    uint32_t *network_handle)
{
    if (gfd < 0) return gfd;
    struct synap_create_network_data data = {
        .start = (uintptr_t)buffer,
        .size = size
    };

    if (ioctl(gfd, SYNAP_CREATE_NETWORK, &data) == 0) {
        *network_handle = data.nid;
        return true;
    }
    return false;
}

bool synap_release_network(
    uint32_t network_handle)
{
    if (gfd < 0) return false;
    return ioctl(gfd, SYNAP_DESTROY_NETWORK, &network_handle) == 0;
}

bool synap_lock_npu()
{
    if (gfd < 0) return false;
    return ioctl(gfd, SYNAP_LOCK_HARDWARE, NULL) == 0;
}

bool synap_unlock_npu()
{
    if (gfd < 0) return false;
    return ioctl(gfd, SYNAP_UNLOCK_HARDWARE, NULL) == 0;
}

bool synap_deinit()
{
    if (gfd < 0) return false;
    int ret = close(gfd);
    gfd = INIT_FD;
    return ret == 0;
}
