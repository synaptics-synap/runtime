// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Preprocessor for synap network.
///

#pragma once

#include <string>
#include <vector>

#include "synap/input_data.hpp"

namespace synaptics {
namespace synap {

class Tensor;
class Tensors;

/// Preprocessor to convert an InputData and assign it to a Tensor
class Preprocessor {
public:
    ///  Construct with default preprocessing options.
    /// There are no customizable preprocessing options at the moment
    Preprocessor() {}

    /// Write InputData to tensor.
    /// Perform conversions to adapt the data to the tensor layout and format if possible.
    /// The tensor format can be specified in the conversion metafile when the network is compiled.
    /// The following formats are currently supported:
    /// - rgb: 8-bits RGB image
    /// - bgr: 8-bits BGR image
    /// - y8: 8-bits y component of yuv image
    /// - uv8: 8-bits uv interleaved components of yuv image
    /// - vu8: 8-bits vu interleaved components of yuv image
    ///
    /// @param t: destination tensor
    /// @param data: input data to be assigned
    /// @param[out] assigned_rect: if not nullptr will contain the coordinates of the part of the
    /// input image actually assigned to the tensor, keeping into account the aspect ratio used
    /// @return true if success
    bool assign(Tensor& t, const InputData& data, Rect* assigned_rect = nullptr) const;

    /// Write InputData to tensors.
    /// Perform conversions to adapt the data to the tensors layout and format if possible.
    /// Equivalent to assigning the InputData to all destination tensors.
    ///
    /// @param ts: destination tensors
    /// @param data: input data to be assigned
    /// @param start_index: index of the first tensor to assign
    /// @param[out] assigned_rect: if not nullptr will contain the coordinates of the part of the
    /// input image actually assigned to the tensor, keeping into account the aspect ratio used
    /// @return true if success
    bool assign(Tensors& ts, const InputData& data, size_t start_index = 0, Rect* assigned_rect = nullptr) const;

    /// Set region of interest.
    /// The input images will be cropped to the specified rectangle if not empty.
    /// Note: requires the model to have been compiled with cropping enabled,
    /// otherwise assignment will fail.
    /// 
    /// @param roi: cropping rectangle (no cropping if empty)
    void set_roi(const Rect& roi);

private:
    // Region of interest in the input image
    Rect _roi{};
};


}  // namespace synap
}  // namespace synaptics
