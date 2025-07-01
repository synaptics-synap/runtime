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
        if (!net->inputs[inputs[i].index].assign(inputs[i].data, inputs[i].size)) {
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

#ifdef __cplusplus
}
#endif