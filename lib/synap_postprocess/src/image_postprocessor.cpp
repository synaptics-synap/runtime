// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/image_postprocessor.hpp"
#include "synap/file_utils.hpp"
#include "synap/string_utils.hpp"
#include "synap/image_convert.hpp"
#include "synap/logging.hpp"
#include "synap/tensor.hpp"

#include <cctype>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;

namespace synaptics {
namespace synap {


ImagePostprocessor::Result ImagePostprocessor::process(const Tensors& tensors)
{
    Result result;

    if (tensors.size() == 2) {
        const Tensor* y{};
        const Tensor* uv{};
        if (tensors[0].format() == "y8" && tensors[1].format() == "uv8") {
            y = &tensors[0];
            uv = &tensors[1];
        }
        else if (tensors[1].format() == "y8" && tensors[0].format() == "uv8") {
            y = &tensors[1];
            uv = &tensors[0];
        }
        else {
            LOGE << "Unsupported formats: " << tensors[0].format() << ", " << tensors[1].format();
            return {};
        }
        
        Dimensions dim = y->dimensions();
        result.dim.x = dim.w;
        result.dim.y = dim.h;
        result.ext = "nv12";
        result.data.resize(y->size() * 3 / 2);
        image_convert_to_yuv420sp((uint8_t*)y->data(), (uint8_t*)uv->data(),
                                  uv->layout() == Layout::nchw, dim.h, dim.w, result.data.data());
    }
    else if (tensors.size() == 1) {
        const Tensor* t = &tensors[0];
        result.ext = format_parse::get_type(t->format());
        if (result.ext.empty()) {
            Dimensions dim = t->dimensions();
            result.dim.x = dim.w;
            result.dim.y = dim.h;
            // Try to deduce image type from the number of channels
            switch(dim.c) {
            case 1: result.ext = "gray"; break;
            case 3: result.ext = "rgb"; break;
            case 4: result.ext = "bgra"; break;
            default: result.ext = "dat"; break;
            }
        }

        result.data.resize(t->size());
        bool interleaved_fmt = result.ext == "rgb" || result.ext == "bgr" || result.ext == "bgra";
        if (interleaved_fmt && t->layout() == Layout::nchw) {
            switch(synap_type_size(t->data_type())) {
            case 1:
                if (!nchw_to_nhwc(t->shape(), static_cast<const uint8_t*>(t->data()), result.data.data())) {
                    LOGE << "Error converting image to nhwc";
                    return {};
                }
                break;
            case 2:
                if (!nchw_to_nhwc(t->shape(), static_cast<const uint16_t*>(t->data()), reinterpret_cast<uint16_t*>(result.data.data()))) {
                    LOGE << "Error converting image to nhwc";
                    return {};
                }
            case 4:
                if (!nchw_to_nhwc(t->shape(), static_cast<const uint32_t*>(t->data()), reinterpret_cast<uint32_t*>(result.data.data()))) {
                    LOGE << "Error converting image to nhwc";
                    return {};
                }
                break;
            default:
                LOGE << "Error converting image to nhwc, item size: " << synap_type_size(t->data_type());
                return {};
            }
        }
        else {
            memcpy(result.data.data(), t->data(), result.data.size());
        }
    }
    else {
        LOGE << "Unsupported number of tensors: " << tensors.size();
    }

    return result;
}


}  // namespace synap
}  // namespace synaptics
