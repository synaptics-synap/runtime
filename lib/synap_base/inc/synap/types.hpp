// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap base types.
///

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

namespace synaptics {
namespace synap {


/// Synap framework version
struct SynapVersion {
    uint32_t major;
    uint32_t minor;
    uint32_t subminor;
};


/// Data layout
enum class Layout { none, nchw, nhwc };


/// Data types
enum class DataType {
    invalid,
    byte,
    int8,
    uint8,
    int16,
    uint16,
    int32,
    uint32,
    float16,
    float32
};


/// Quantization scheme
enum class QuantizationScheme {
    none,
    dynamic_fixed_point,
    affine_asymmetric
};

/// Security attributes for tensors
enum class Security {
    ///  No security specification
    none,
    /// The input can be either in secure or non-secure memory
    any,
    /// The input/output must be in secure memory
    secure,
    /// The input must be in non secure memory
    non_secure,
    /// The output buffer must be in secure memory if at least one input is in secure memory
    secure_if_input_secure
};

enum class ImageTransformType {
    two_points_align,
    five_points_align
};


/// Two-dim size
struct Dim2d {
    int32_t x;
    int32_t y;
};

inline Dim2d operator+(const Dim2d& lhs, const Dim2d& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Dim2d operator-(const Dim2d& lhs, const Dim2d& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

/// Landmark
struct Landmark {
    int32_t x{};
    int32_t y{};
    int32_t z{};
    float visibility = -1.0f;
};


/// Segment mask
struct Mask {
    Mask();
    Mask(uint32_t width, uint32_t height);
    inline uint32_t width() const { return _width; }
    inline uint32_t height() const { return _height; }
    inline operator bool() const { return !(_data.empty()); }
    const std::vector<float>& buffer() const { return _data; }
    const float* data() const;
    void set_value(uint32_t row, uint32_t col, float& val);
private:
    std::vector<float> _data;
    uint32_t _width {0};
    uint32_t _height {0};
};


/// Bounding box rectangle
struct Rect {
    Dim2d origin;  /// Top-left corner of the bounding box
    Dim2d size;

    bool empty() const { return size.x <= 0 || size.y <= 0; }
};


/// Tensor dimensions.
/// The order of the dimensions is given by the Layout
struct Shape : public std::vector<int32_t> {
    using vector::vector;

    /// @return number of elements in a tensor with this shape
    size_t item_count() const;

    /// @return true if shape is valid
    bool valid() const;
};


/// Tensor dimensions as named fields for 4D tensors.
struct Dimensions {
    Dimensions() {}

    /// Create from NHWC dimensions.
    Dimensions(int32_t n_, int32_t h_, int32_t w_, int32_t c_) : n{n_}, h{h_}, w{w_}, c{c_} {}

    /// Create from shape and layout.
    /// Only meaningful for shape of rank 4 (all fields initialized at 0 otherwise)
    /// @param s: tensor shape
    /// @param layout: tensor layout
    Dimensions(const Shape& s, Layout layout);
    
    bool empty() const { return n == 0 && h == 0 && w == 0 && c == 0; }
    bool operator!=(const Dimensions& rhs) const { return !(*this == rhs); }
    bool operator==(const Dimensions& rhs) const {
        return n == rhs.n && h == rhs.h && w == rhs.w && c == rhs.c;
    }

    int32_t n{};
    int32_t h{};
    int32_t w{};
    int32_t c{};
};


/// @return size in byte of a DataType
size_t synap_type_size(DataType synap_type);

/// @return true if DataType is integral (int8, uint8, int16,..)
bool synap_type_is_integral(DataType synap_type);


// String conversions
std::ostream& operator<<(std::ostream& os, const SynapVersion& v);
std::ostream& operator<<(std::ostream& os, const Shape& v);
std::ostream& operator<<(std::ostream& os, const Layout& layout);
std::ostream& operator<<(std::ostream& os, const DataType& type);
std::ostream& operator<<(std::ostream& os, const QuantizationScheme& qs);
std::ostream& operator<<(std::ostream& os, const Dimensions& d);
std::ostream& operator<<(std::ostream& os, const Dim2d& d);
std::ostream& operator<<(std::ostream& os, const Rect& r);
std::ostream& operator<<(std::ostream& os, const Landmark& l);
bool from_string(Rect& r, const std::string& str);


}  // namespace synap
}  // namespace synaptics
