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

/// Represents an input to the network
/// @note The data is considered as raw data, so no normalization or conversion is done
typedef struct {
    const void* data;
    size_t size;
    size_t index;
} CNetworkInput;

/// Represents an output from the network
typedef struct {
    const float* data;
    size_t size;
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


/// Load a network model from a .synap model file.
/// @param [in] network Pointer to the CNetwork instance
/// @param [in] filename Path to the .synap model file
/// @return true if the model was loaded successfully
bool network_load(CNetwork* network, const char* filename);


/// Run inference on the network.
/// @note Inputs are considered as raw data so no normalization or conversion is done.
/// @param [in] network A CNetwork instance.
/// @param [in] inputs Collection of inputs to the network
/// @param [in] input_count Number of inputs in the collection, must match the number of inputs in the network
/// @param [out] outputs Collection of outputs to be filled by the network
/// @param [in] output_count Number of outputs in the collection, must match the number of outputs in the network
/// @return true if inference was successful
bool network_predict(
    CNetwork* network,
    const CNetworkInput* inputs, size_t input_count,
    CNetworkOutput* outputs, size_t output_count
);

#ifdef __cplusplus
}
#endif