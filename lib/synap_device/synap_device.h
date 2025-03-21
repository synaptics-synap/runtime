// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2021 Synaptics Incorporated */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Initialize synap instance
///
/// @return true if success
bool synap_init();


/// Set the specified input for the next network execution to the specified buffer
///
/// @param [in] network_handle: handle of the network
/// @param [in] attachment_handle: handle of the buffer
/// @param [in] input_index: index of the network input
/// @return true if success
bool synap_set_input(
    uint32_t network_handle,
    uint32_t attachment_handle,
    uint32_t input_index);


/// Set the specified output for the next network execution to the specified buffer
///
/// @param [in] network_handle: handle of the network
/// @param [in] attachment_handle: handle of the buffer
/// @param [in] output_index: index of the network output
/// @return true if success
bool synap_set_output(
    uint32_t network_handle,
    uint32_t attachment_handle,
    uint32_t output_index);


/// Allocates a dmabuf memory area and registers it as a io buffer
///
/// @param [in] offset: offset of the shm buffer
/// @param [in] size: size of the shm buffer will be used
/// @param [out] buffer_handle: handle of the buffer
/// @param [out] mem_id: handle of the buffer (NULL can be passed if not interested)
/// @param [out] dmabuf_fd: fd to the dmabuf memory (used to sync access to the memory)
/// @return true if success
bool synap_allocate_io_buffer(
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id,
    uint32_t *dmabuf_fd);

/// Helper function to start accessing a dmabuf
///
/// @param [in] dmabuf_fd:  fd to the dmabuf memory
/// @return true if success
bool synap_lock_io_buffer(uint32_t dmabuf_fd);


/// Helper function to call when finished accessing a dmabuf
///
/// @param [in] dmabuf_fd:  fd to the dmabuf memory
/// @return true if success
bool synap_unlock_io_buffer(uint32_t dmabuf_fd);

/// Registers dmabuf shared memory area as a io buffer
///
/// @param [in] dmabuf_fd: fd to the dmabuf memory
/// @param [in] offset: offset of the shm buffer
/// @param [in] size: size of the shm buffer will be used
/// @param [out] buffer_handle: handle of the buffer
/// @param [out] mem_id: handle of the buffer (NULL can be passed)
/// @return true if success
bool synap_create_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id);


/// Registers dmabuf shared memory area as a secure io buffer
///
/// @param [in] dmabuf_fd: fd to the dmabuf memory
/// @param [in] offset: offset of the shm buffer
/// @param [in] size: size of the shm buffer will be used
/// @param [out] buffer_handle: handle of the buffer
/// @param [out] mem_id: handle of the buffer (NULL can be passed)
/// @return true if success
bool synap_create_secure_io_buffer(
    uint32_t dmabuf_fd,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle,
    uint32_t *mem_id);

/// Registers an existing mem_id as a io buffer
///
/// @param [in] mem_id: mem_id of the buffer to register
/// @param [in] offset: offset of the mem_id buffer
/// @param [in] size: size of the mem_id buffer will be used
/// @param [out] buffer_handle: handle of the buffer
/// @return true if success
bool synap_create_io_buffer_from_mem_id(
    uint32_t mem_id,
    uint32_t offset,
    uint32_t size,
    uint32_t *buffer_handle);


/// Releases a shared memory area created previously
///
/// @param [in] mem_id: id of the buffer
/// @return true if success
bool synap_destroy_io_buffer(
    uint32_t buffer_handle);

/// Attach a given io buffer identified by mem_id to
/// a given network
///
/// @param [in] network_handle: handle of the network
/// @param [in] mem_id: TZ mem_id handle of the memory
/// @param [out] buffer_handle: handle of the buffer
/// @return true if success
bool synap_attach_io_buffer(
    uint32_t network_handle,
    uint32_t mem_id,
    uint32_t *buffer_handle);


/// Detach a the specified buffer attachment
///
/// @param [in] network_handle: handle of the network
/// @param [in] attachment_handle: handle to the attachment
/// @return true if success
bool synap_detach_io_buffer(
    uint32_t network_handle,
    uint32_t attachment_handle);


/// Starts the execution of the specified network and block until execution
///
/// @param [in] network_handle: handle of the network
/// @return true if success
bool synap_run_network(
    uint32_t network_handle);


/// Loads the specified network for execution
///
/// @param [in] data: buffer containing the network binary
/// @param [in] size: size of the network binary
/// @param [out] network_handle: handle of the network
/// @return true if success
bool synap_prepare_network(
    const void *data,
    size_t size,
    uint32_t *network_handle);


/// Release all resources associated with a network, including all the buffers
///
/// @param [in] network_handle: handle of the network
/// @return true if success
bool synap_release_network(
    uint32_t network_handle);


/// Locks NPU for the calling process.
/// Running inference from any other process (including NNAPI) will fail.
///
/// @return true if success, false if NPU already locked by another process
bool synap_lock_npu();


/// Unlocks NPU.
///
/// @return true if success, false if NPU lock not owned by this process.
bool synap_unlock_npu();


/// Deinitialize the synap instance
///
/// @return true if success
bool synap_deinit();

#ifdef __cplusplus
}
#endif
