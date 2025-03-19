// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once
#include <memory>
#include <string>
#include <vector>


namespace synaptics {
namespace synap {

enum class ImageType { invalid, grayscale, rgb, rgba };

std::vector<uint8_t> image_read(const uint8_t* input, size_t inputSize, int32_t* width,
                                int32_t* height, ImageType* type);

std::vector<uint8_t> image_file_read(const std::string& file_name, int32_t* width, int32_t* height,
                                     ImageType* type);

bool bmp_file_write(const uint8_t* data, const std::string& file_name, int32_t width,
                    int32_t height, ImageType type);

bool jpg_file_write(const uint8_t* data, const std::string& file_name, int32_t width,
                    int32_t height, ImageType type, int quality = 99);


bool png_file_write(const uint8_t* data, const std::string& file_name, int32_t width,
                    int32_t height, ImageType type);


}  // namespace synap
}  // namespace synaptics
