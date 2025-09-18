// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2022 Synaptics Incorporated. All rights reserved.

///
/// Data [de]quantization and normalization.
///

#pragma once

#include "synap/metadata.hpp"


namespace synaptics {
namespace synap {

/// @return true if normalize_quantize operation is actually just a direct copy
/// This is the case if attr has no normalization and no quantization
/// or normalization and quantization simplify each other.
bool normalize_quantize_is_copy(const TensorAttributes* attr);

/// Convert, normalize and quantize uint8_t data to tensor
bool normalize_quantize(void* dst, const uint8_t* src, size_t size, const TensorAttributes* attr);

/// Convert, normalize and quantize int16_t data to tensor
bool normalize_quantize(void* dst, const int16_t* src, size_t size, const TensorAttributes* attr);

/// Convert, normalize and quantize float data to tensor
bool normalize_quantize(void* dst, const float* src, size_t size, const TensorAttributes* attr);

/// Convert quantized tensor data buffer to float
bool dequantize(float* dst, const uint8_t* src, size_t size, const TensorAttributes* src_attr);


}  // namespace synap
}  // namespace synaptics
