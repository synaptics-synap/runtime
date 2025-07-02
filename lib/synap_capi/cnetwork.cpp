// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2025 Synaptics Incorporated */
///
/// Synap C API for Network.
///

#include "cnetwork.h"
#include "synap/network.hpp"
#include "synap/logging.hpp"

using namespace synaptics::synap;

struct CNetwork {
    Network* impl;
};

#ifdef __cplusplus
extern "C" {
#endif

CNetwork* network_create() {
    Network* net = new Network();
    if (!net) {
        LOGE << "Failed to create Network instance";
        return nullptr;
    }
    CNetwork* wrapper = new CNetwork{ net };
    return wrapper;
}

void network_destroy(CNetwork* network) {
    if (network) {
        delete network->impl;
        delete network;
    }
}

bool network_load(CNetwork* network, const char* filename) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return false;
    }
    return network->impl->load_model(filename);
}

bool network_predict(
    CNetwork* network,
    const CNetworkInput* inputs, size_t input_count,
    CNetworkOutput* outputs, size_t output_count
) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return false;
    }
    Network* net = network->impl;

    // check inputs and outputs
    if (!inputs) { LOGE << "Inputs pointer is null"; return false; }
    if (input_count != net->inputs.size()) {
        LOGE << "Input count mismatch: expected " << net->inputs.size() << ", got " << input_count;
        return false;
    }
    if (!outputs) { LOGE << "Outputs pointer is null"; return false; }
    if (output_count != net->outputs.size()) {
        LOGE << "Output count mismatch: expected " << net->outputs.size() << ", got " << output_count;
        return false;
    }

    // assign inputs
    for (size_t i = 0; i < input_count; ++i) {
        const void* input_data;
        switch(inputs[i].type) {
            case INPUT_DTYPE_UINT8:
                input_data = inputs[i].data.u8;
                break;
            case INPUT_DTYPE_INT16:
                input_data = inputs[i].data.i16;
                break;
            case INPUT_DTYPE_FLOAT:
                input_data = inputs[i].data.f32;
                break;
            case INPUT_DTYPE_RAW:
                input_data = inputs[i].data.raw;
                break;
            default:
                LOGE << "Unsupported input data type for input " << i;
                return false;
        }
        if (!net->inputs[inputs[i].index].assign(input_data, inputs[i].size)) {
            LOGE << "Failed to assign data to input tensor " << inputs[i].index;
            return false;
        }
    }

    // run prediction
    if (!net->predict()) return false;

    // assign outputs
    for (size_t i = 0; i < net->outputs.size(); ++i) {
        outputs[i].data = net->outputs[i].as_float();
        outputs[i].size = net->outputs[i].size();
    }

    return true;
}

size_t network_get_input_size(CNetwork* network, size_t index) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return 0;
    }
    Network* net = network->impl;

    if (index >= net->inputs.size()) {
        LOGE << "Input index out of range: " << index << ", size: " << net->inputs.size();
        return 0;
    }

    return net->inputs[index].size();
}

size_t network_get_input_count(CNetwork* network) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return 0;
    }
    return network->impl->inputs.size();
}

size_t network_get_output_count(CNetwork* network) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return 0;
    }
    return network->impl->outputs.size();
}

#ifdef __cplusplus
}
#endif