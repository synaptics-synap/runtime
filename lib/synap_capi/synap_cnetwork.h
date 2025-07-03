// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2025 Synaptics Incorporated */
///
/// Synap C API for Network.
///

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct CNetwork;
typedef struct CNetwork CNetwork;


/// Network tensor type
typedef enum {
    TENSOR_TYPE_INPUT,
    TENSOR_TYPE_OUTPUT,
} CNetworkTensorType;


/// Network input data type
typedef enum {
    INPUT_DTYPE_UINT8,
    INPUT_DTYPE_INT16,
    INPUT_DTYPE_FLOAT,
    INPUT_DTYPE_RAW
} CNetworkInputType;


/// Network output data type
typedef enum {
    OUTPUT_DTYPE_INVALID,
    OUTPUT_DTYPE_BYTE,
    OUTPUT_DTYPE_INT8,
    OUTPUT_DTYPE_UINT8,
    OUTPUT_DTYPE_INT16,
    OUTPUT_DTYPE_UINT16,
    OUTPUT_DTYPE_INT32,
    OUTPUT_DTYPE_UINT32,
    OUTPUT_DTYPE_FLOAT16,
    OUTPUT_DTYPE_FLOAT32
} CNetworkOutputType;


/// Represents an input to the network
/// @note Input data is owned and managed by the caller
typedef struct {

    /// Pointer to the input data, later converted to specified type
    /// @note No normalization or conversion is done for `INPUT_DTYPE_RAW`
    const void* data;

    /// Size of the input data in bytes
    /// @note For `INPUT_DTYPE_RAW`, the size is the number of bytes
    /// @note For other types, the size is the number of elements of the type
    size_t size;

    /// Index of the input tensor
    size_t index;

    /// Type of the input data
    CNetworkInputType type;

} CNetworkInput;


/// Represents an output from the network
/// @note Output data is owned by the network and must not be freed by the caller
typedef struct {

    /// Pointer to the denormalized output data
    const float* data_float;

    /// @note No de-normalization or conversion is done for `data_raw`
    void* data;

    /// Size of the output data in bytes
    /// @note The size is always number of elements * sizeof(data type)
    size_t size;

    /// Type of the output data
    CNetworkOutputType type;

} CNetworkOutput;


#ifdef __cplusplus
extern "C" {
#endif

/// Create a new CNetwork instance
/// @return CNetwork pointer, or NULL on failure
CNetwork* network_create();


/// Free a CNetwork instance
/// @param network Pointer to the CNetwork instance to be destroyed
void network_destroy(CNetwork* network);


/// Load a network model from a .synap model file
/// @param network Pointer to the CNetwork instance
/// @param filename Path to the .synap model file
/// @return true if the model was loaded successfully
bool network_load(CNetwork* network, const char* filename);


/// Run inference on the network and get floating point outputs
/// @note No normalization or conversion is done for `INPUT_DTYPE_RAW` inputs
/// @param network A CNetwork instance
/// @param inputs Collection of inputs to the network
/// @param input_count Number of inputs in the collection, must match the number of inputs in the network
/// @param [out] outputs Collection of outputs to be filled by the network
/// @param output_count Number of outputs in the collection, must match the number of outputs in the network
/// @return true if inference was successful
bool network_predict(
    CNetwork* network,
    const CNetworkInput* inputs, size_t input_count,
    CNetworkOutput* outputs, size_t output_count
);


/// Get the number of tensors in the network inputs or outputs
/// @param network A CNetwork instance
/// @return Number of tensors in the collection
size_t network_get_tensor_count(CNetwork* network, CNetworkTensorType type);


/// Get the size of a specific input or output tensor in the network
/// @param network A CNetwork instance
/// @param index Index of the tensor in the collection
/// @param type Type of the tensor (input or output)
/// @return tensor size in bytes
size_t network_get_tensor_size(CNetwork* network, size_t index, CNetworkTensorType type);


/// Get pointer to denormalized tensor data for a specific input or output tensor
/// @param network A CNetwork instance
/// @param index Index of the tensor in the collection
/// @param type Type of the tensor (input or output)
/// @return float pointer to tensor data
const float* network_get_tensor_data(CNetwork* network, size_t index, CNetworkTensorType type);


/// Get pointer to raw input data for a specific input or output tensor
/// @param network A CNetwork instance
/// @param index Index of the tensor in the collection
/// @param type Type of the tensor (input or output)
/// @return void pointer to tensor data, must be manually cast to the appropriate type
void* network_get_tensor_data_raw(CNetwork* network, size_t index, CNetworkTensorType type);

#ifdef __cplusplus
}
#endif