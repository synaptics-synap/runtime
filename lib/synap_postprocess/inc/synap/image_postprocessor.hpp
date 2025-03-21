// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once

///
/// Synap image postprocessor.
///

#include "synap/tensor.hpp"
#include <string>
#include <vector>

#include "synap/types.hpp"

namespace synaptics {
namespace synap {

/// Image postprocessor
class ImagePostprocessor {
public:

    /// Postprocessor result
    struct Result {
        /// Get extension type from tensor content.
        std::string ext;

        /// Image dimensions
        Dim2d dim;

        /// Converted image
        std::vector<uint8_t> data;
    };
    
    ///  Constructor.
    ///
    ImagePostprocessor() {}

    /// Perform processing of the image in the network output tensors.
    /// 
    /// @param tensors: output tensors of the network
    /// @return processing results    
    Result process(const Tensors& tensors);
    
};

}  // namespace synap
}  // namespace synaptics
