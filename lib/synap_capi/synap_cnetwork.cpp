// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2025 Synaptics Incorporated */
///
/// Synap C API for Network.
///

#include "synap_cnetwork.h"
#include "synap/network.hpp"
#include "synap/logging.hpp"

using namespace synaptics::synap;

struct CNetwork {
    Network* impl;
};

// anonymous namespace for helpers
namespace {
    bool check_network(CNetwork* network) {
    if (!network || !network->impl) {
        LOGE << "Invalid network";
        return false;
    }
    return true;
}

    bool check_tensor_index_and_type(CNetwork* network, size_t index, CNetworkTensorType type) {
        if (!check_network(network)) return false;
    if (type == TENSOR_TYPE_INPUT) {
        if (index >= network->impl->inputs.size()) {
            LOGE << "Input index out of range: " << index << ", size: " << network->impl->inputs.size();
            return false;
        }
    } else if (type == TENSOR_TYPE_OUTPUT) {
        if (index >= network->impl->outputs.size()) {
            LOGE << "Output index out of range: " << index << ", size: " << network->impl->outputs.size();
            return false;
        }
    } else {
        LOGE << "Invalid tensor type";
        return false;
    }
    return true;
}
}


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
    if (!check_network(network)) return false;
    return network->impl->load_model(filename);
}

bool network_predict(
    CNetwork* network,
    const CNetworkInput* inputs, size_t input_count,
    CNetworkOutput* outputs, size_t output_count
) {
    if (!check_network(network)) return false;
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
        bool assigned = false;
        switch(inputs[i].type) {
            case INPUT_DTYPE_UINT8:
                assigned = net->inputs[inputs[i].index].assign((const uint8_t*) inputs[i].data, inputs[i].size);
                break;
            case INPUT_DTYPE_INT16:
                assigned = net->inputs[inputs[i].index].assign((const uint16_t*) inputs[i].data, inputs[i].size);
                break;
            case INPUT_DTYPE_FLOAT:
                assigned = net->inputs[inputs[i].index].assign((const float*) inputs[i].data, inputs[i].size);
                break;
            case INPUT_DTYPE_RAW:
                assigned = net->inputs[inputs[i].index].assign(inputs[i].data, inputs[i].size);
                break;
            default:
                LOGE << "Unsupported input data type for input " << i;
                return false;
        }
        if (!assigned) {
            LOGE << "Failed to assign data to input tensor " << inputs[i].index;
            return false;
        }
    }

    // run prediction
    if (!net->predict()) return false;

    // assign outputs
    for (size_t i = 0; i < net->outputs.size(); ++i) {
        outputs[i].data_float = net->outputs[i].as_float();
        outputs[i].data = net->outputs[i].data();
        outputs[i].size = net->outputs[i].size();
        switch(net->outputs[i].data_type()) {
            case DataType::invalid:
                outputs[i].type = TENSOR_DTYPE_INVALID;
                LOGW << "Invalid output data type for output " << i;
                break;
            case DataType::byte:
                outputs[i].type = TENSOR_DTYPE_BYTE;
                break;
            case DataType::int8:
                outputs[i].type = TENSOR_DTYPE_INT8;
                break;
            case DataType::uint8:
                outputs[i].type = TENSOR_DTYPE_UINT8;
                break;
            case DataType::int16:
                outputs[i].type = TENSOR_DTYPE_INT16;
                break;
            case DataType::uint16:
                outputs[i].type = TENSOR_DTYPE_UINT16;
                break;
            case DataType::int32:
                outputs[i].type = TENSOR_DTYPE_INT32;
                break;
            case DataType::uint32:
                outputs[i].type = TENSOR_DTYPE_UINT32;
                break;
            case DataType::float16:
                outputs[i].type = TENSOR_DTYPE_FLOAT16;
                break;
            case DataType::float32:
                outputs[i].type = TENSOR_DTYPE_FLOAT32;
                break;
            default:
                LOGE << "Unsupported output data type for output " << i;
                return false;
        }
    }

    return true;
}

size_t network_get_tensor_count(CNetwork* network, CNetworkTensorType type) {
    if (!check_network(network))
        return 0;
    if (type == TENSOR_TYPE_INPUT)
        return network->impl->inputs.size();
    else if (type == TENSOR_TYPE_OUTPUT)
        return network->impl->outputs.size();
    else {
        LOGE << "Invalid tensor type";
        return 0;
    }
}

size_t network_get_tensor_size(CNetwork* network, size_t index, CNetworkTensorType type) {
    if (!check_tensor_index_and_type(network, index, type)) return 0;

    if (type == TENSOR_TYPE_INPUT)
        return network->impl->inputs[index].size();
    else
        return network->impl->outputs[index].size();
}

size_t network_get_tensor_item_count(CNetwork* network, size_t index, CNetworkTensorType type) {
    if (!check_tensor_index_and_type(network, index, type)) return 0;

    if (type == TENSOR_TYPE_INPUT)
        return network->impl->inputs[index].item_count();
    else
        return network->impl->outputs[index].item_count();
}

CTensorDataType network_get_tensor_data_type(CNetwork* network, size_t index, CNetworkTensorType type) {
    if (!check_tensor_index_and_type(network, index, type)) return TENSOR_DTYPE_INVALID;

    Tensor* tensor_ptr;
    if (type == TENSOR_TYPE_INPUT)
        tensor_ptr = &(network->impl->inputs[index]);
    else
        tensor_ptr = &(network->impl->outputs[index]);
    
    switch (tensor_ptr->data_type()) {
        case DataType::invalid: return TENSOR_DTYPE_INVALID;
        case DataType::byte:    return TENSOR_DTYPE_BYTE;
        case DataType::int8:    return TENSOR_DTYPE_INT8;
        case DataType::uint8:   return TENSOR_DTYPE_UINT8;
        case DataType::int16:   return TENSOR_DTYPE_INT16;
        case DataType::uint16:  return TENSOR_DTYPE_UINT16;
        case DataType::int32:   return TENSOR_DTYPE_INT32;
        case DataType::uint32:  return TENSOR_DTYPE_UINT32;
        case DataType::float16: return TENSOR_DTYPE_FLOAT16;
        case DataType::float32: return TENSOR_DTYPE_FLOAT32;
        default:
            LOGE << "Unknown tensor data type for tensor " << tensor_ptr->name();
            return TENSOR_DTYPE_INVALID;
    }
}

const float* network_get_tensor_data(CNetwork* network, size_t index, CNetworkTensorType type) {
    if (!check_tensor_index_and_type(network, index, type)) return nullptr;

    if (type == TENSOR_TYPE_INPUT)
        return network->impl->inputs[index].as_float();
    else
        return network->impl->outputs[index].as_float();
}

void* network_get_tensor_data_raw(CNetwork* network, size_t index, CNetworkTensorType type) {
    if (!check_tensor_index_and_type(network, index, type)) return nullptr;

    if (type == TENSOR_TYPE_INPUT)
        return network->impl->inputs[index].data();
    else
        return network->impl->outputs[index].data();
}

#ifdef __cplusplus
}
#endif