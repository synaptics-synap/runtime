// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once

#include <stdint.h>
#include "synap/types.hpp"

namespace synaptics {
namespace synap {

/// YUV420SP (NV12) to y:
/// YYY..UVUV... -> YYY
bool image_convert_yuv420sp_to_y(const uint8_t* in_data, int32_t height, int32_t width,
                                 uint8_t* out_data);

/// YUV420SP (NV12) to uv:
/// YYY..UVUV... -> UVUVUV...
/// YYY..UVUV... -> UUU...VVV... (planar)
bool image_convert_yuv420sp_to_uv(const uint8_t* in_data, int32_t height, int32_t width,
                                  uint8_t* out_data, bool planar);

/// YUV420SP (NV12) to planar:
/// YYY..UVUV... -> YYY...UUU...VVV...
bool image_convert_yuv420sp_to_planar(const uint8_t* in_data, int32_t height, int32_t width,
                                      uint8_t* out_data);

/// planar to YUV420SP (NV12):
/// YYY...UUU...VVV... -> YYY..UVUV...
bool image_convert_to_yuv420sp(const uint8_t* y, const uint8_t* uv, bool planar, int32_t height,
                                      int32_t width, uint8_t* out_data);


template<typename T>
bool nhwc_to_nchw(const Shape& in_shape, const T* in_data, T* out_data, bool swap = false)
{
    if (in_shape.size() != 4) {
        return false;
    }
    constexpr int32_t N = 1;
    const int32_t hh = in_shape[1];
    const int32_t ww = in_shape[2];
    const int32_t cc = in_shape[3];

    for (int n = 0; n < N; n++) {
        const size_t in_batch_offset = n * hh * ww * cc;
        const size_t out_batch_offset = n * cc * hh * ww;
        for (int h = 0; h < hh; ++h) {
            size_t in_row_offset = h * ww * cc + in_batch_offset;
            size_t out_row_offset = h * ww + out_batch_offset;
            for (int w = 0; w < ww; ++w) {
                size_t in_col_offset = w * cc + in_row_offset;
                size_t out_col_offset = w + out_row_offset;
                for (int c = 0; c < cc; ++c) {
                    size_t in_addr = (swap ? cc - 1 - c : c) + in_col_offset;
                    size_t out_addr = c * hh * ww + out_col_offset;
                    out_data[out_addr] = in_data[in_addr];
                }
            }
        }
    }
    
    return true;
}


template<typename T>
bool nchw_to_nhwc(const Shape& in_shape, const T* in_data, T* out_data, bool swap = false)
{
    if (in_shape.size() != 4) {
        return false;
    }
    constexpr int32_t N = 1;
    const int32_t hh = in_shape[2];
    const int32_t ww = in_shape[3];
    const int32_t cc = in_shape[1];

    for (int n = 0; n < N; n++) {
        const size_t out_batch_offset = n * hh * ww * cc;
        const size_t in_batch_offset = n * cc * hh * ww;
        for (int h = 0; h < hh; ++h) {
            size_t out_row_offset = h * ww * cc + out_batch_offset;
            size_t in_row_offset = h * ww + in_batch_offset;
            for (int w = 0; w < ww; ++w) {
                size_t out_col_offset = w * cc + out_row_offset;
                size_t in_col_offset = w + in_row_offset;
                for (int c = 0; c < cc; ++c) {
                    size_t out_addr = (swap ? cc - 1 - c : c) + out_col_offset;
                    size_t in_addr = c * hh * ww + in_col_offset;
                    out_data[out_addr] = in_data[in_addr];
                }
            }
        }
    }
    return true;
}


}  // namespace synap
}  // namespace synaptics
