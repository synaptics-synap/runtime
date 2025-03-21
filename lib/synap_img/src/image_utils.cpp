// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/image_utils.hpp"
#include "synap/file_utils.hpp"
#include "synap/logging.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>


using namespace std;

namespace synaptics {
namespace synap {


static inline int get_stbi_comp(ImageType type)
{
    // 1=Y, 2=YA, 3=RGB, 4=RGBA
    int comp;
    switch (type) {
    case ImageType::invalid:
        comp = -1;
        break;
    case ImageType::grayscale:
        comp = 1;
        break;
    case ImageType::rgb:
        comp = 3;
        break;
    case ImageType::rgba:
        comp = 4;
        break;
    }
    return comp;
}


static inline ImageType from_stbi_comp(int comp)
{
    // 1=Y, 2=YA, 3=RGB, 4=RGBA
    ImageType type = ImageType::invalid;
    switch (comp) {
    case 1:
        type = ImageType::grayscale;
        break;
    case 3:
        type = ImageType::rgb;
        break;
    case 4:
        type = ImageType::rgba;
        break;
    }
    return type;
}

vector<uint8_t> image_file_read(const string& file_name, int32_t* width, int32_t* height,
                                ImageType* type)
{
    int comp;
    uint8_t* data = stbi_load(file_name.c_str(), width, height, &comp, 0);
    if (data == nullptr) {
        LOGE << "Error reading file " << file_name;
        *type = ImageType::invalid;
        return vector<uint8_t>();
    }
    *type = from_stbi_comp(comp);
    vector<uint8_t> v{data, data + (*width) * (*height) * comp};
    stbi_image_free(data);
    return v;
}

vector<uint8_t> image_read(const uint8_t* input, size_t inputSize, int32_t* width, int32_t* height,
                           ImageType* type)
{
    int comp;
    *type = ImageType::invalid;
    uint8_t* data = stbi_load_from_memory(input, inputSize, width, height, &comp, 0);
    if (data == nullptr) {
        LOGE << "Error reading data";
        return vector<uint8_t>();
    }
    *type = from_stbi_comp(comp);
    vector<uint8_t> v{data, data + (*width) * (*height) * comp};
    stbi_image_free(data);
    return v;
}

bool bmp_file_write(const uint8_t* data, const string& file_name, int32_t width, int32_t height,
                    ImageType type)
{
    const auto comp = get_stbi_comp(type);
    if (!stbi_write_bmp(file_name.c_str(), width, height, comp, data)) {
        LOGE << "bmp_write failed: " << file_name;
        return false;
    }
    return true;
}

bool jpg_file_write(const uint8_t* data, const string& file_name, int32_t width, int32_t height,
                    ImageType type, int quality)
{
    const auto comp = get_stbi_comp(type);
    if (!stbi_write_jpg(file_name.c_str(), width, height, comp, data, quality)) {
        LOGE << "jpg_write failed: " << file_name;
        return false;
    }
    return true;
}

bool png_file_write(const uint8_t* data, const string& file_name, int32_t width, int32_t height,
                    ImageType type)
{
    const auto comp = get_stbi_comp(type);
    const int stride_in_bytes = width * comp;
    if (!stbi_write_png(file_name.c_str(), width, height, comp, data, stride_in_bytes)) {
        LOGE << "png_write failed: " << file_name;
        return false;
    }
    return true;
}


}  // namespace synap
}  // namespace synaptics
