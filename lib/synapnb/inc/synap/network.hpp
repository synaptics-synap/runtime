// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap Neural Network.
///

#pragma once
#include "synap/tensor.hpp"
#include <memory>
#include <string>

namespace synaptics {
namespace synap {

class NetworkPrivate;

/// Load and execute a neural network on the NPU accelerator.
class Network {
    // Implementation details
    std::unique_ptr<NetworkPrivate> d;

public:
    Network();
    Network(Network&& rhs);
    Network(const Network& rhs) = delete;
    Network& operator=(Network&& rhs);
    Network& operator=(const Network& rhs) = delete;
    ~Network();


    /// Load model.
    /// In case another model was previously loaded it is disposed before loading the one specified.
    ///
    /// @param model_file      path to .synap model file.
    ///                        Can also be the path to a legacy .nb model file.
    /// @param meta_file       for legacy .nb models must be the path to the model's metadata file
    ///                        (JSON-formatted). In all other cases must be the empty string.
    /// @return                true if success
    bool load_model(const std::string& model_file, const std::string& meta_file = "");


    /// Load model.
    /// In case another model was previously loaded it is disposed before loading the one specified.
    ///
    /// @param model_data      model data, as from e.g. fread() of model.synap
    ///                        The caller retains ownership of the model data and can delete them
    ///                        at the end of this method.
    /// @param model_size      model size in bytes
    /// @param meta_data       for legacy .nb models must be the model's metadata (JSON-formatted).
    ///                        In all other cases must be nullptr.
    /// @return                true if success
    bool load_model(const void* model_data, size_t model_size, const char* meta_data = nullptr);


    /// Run inference.
    /// Input data to be processed are read from input tensor(s).
    /// Inference results are generated in output tensor(s).
    ///
    /// @return true if success, false if inference failed or network not correctly initialized.
    bool predict();


    /// Collection of input tensors that can be accessed by index and iterated.
    Tensors inputs;

    /// Collection of output tensors that can be accessed by index and iterated.
    Tensors outputs;
};


/// Get synap version.
///
/// @return version number
SynapVersion synap_version();


}  // namespace synap
}  // namespace synaptics
