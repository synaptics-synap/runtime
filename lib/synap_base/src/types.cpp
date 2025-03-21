// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap tensor.
///

#include "synap/types.hpp"
#include "synap/logging.hpp"
#include "synap/string_utils.hpp"

using namespace std;

namespace synaptics {
namespace synap {


Mask::Mask() = default;

Mask::Mask(uint32_t _mask_w, uint32_t _mask_h)
: _width(_mask_w), _height(_mask_h)
{
    _data.resize(_mask_w * _mask_h);
    LOGV << "Reserved " << _data.capacity() * sizeof(float) << " bytes for mask";
}

const float* Mask::data() const
{
    if (_data.empty()) {
        LOGE << "Mask has no data";
        return nullptr;
    }
    return _data.data();
}


void Mask::set_value(uint32_t row, uint32_t col, float& val)
{
    _data[row * _width + col] = val;
}


size_t Shape::item_count() const
{
    size_t c = 1;
    for (const auto& d : *this) {
        c *= d;
    }
    return c;
}

bool Shape::valid() const
{
    for (const auto& d : *this) {
        if (d <= 0) {
            return false;
        }
    }
    return true;
}

size_t synap_type_size(DataType synap_type)
{
    switch (synap_type) {
    case DataType::byte:
    case DataType::int8:
    case DataType::uint8:
        return 1;
    case DataType::int16:
    case DataType::uint16:
    case DataType::float16:
        return 2;
    case DataType::int32:
    case DataType::uint32:
    case DataType::float32:
        return 4;
    case DataType::invalid:
        break;
    }

    LOGE << "Invalid data type";
    return 0;
}


bool synap_type_is_integral(DataType synap_type)
{
    switch (synap_type) {
    case DataType::int8:
    case DataType::uint8:
    case DataType::int16:
    case DataType::uint16:
    case DataType::int32:
    case DataType::uint32:
        return true;
    case DataType::byte:
    case DataType::float16:
    case DataType::float32:
        return false;
    case DataType::invalid:
        break;
    }

    LOGE << "Invalid data type";
    return false;
}


Dimensions::Dimensions(const Shape& s, Layout layout)
{
    if (s.size() == 4) {
        *this = layout == Layout::nchw? Dimensions{s[0], s[2], s[3], s[1]} :
                layout == Layout::nhwc? Dimensions{s[0], s[1], s[2], s[3]} : Dimensions{};
    }
}


std::ostream& operator<<(std::ostream& os, const SynapVersion& v)
{
    os << v.major << "."<< v.minor << "."<< v.subminor;
    return os;
}


std::ostream& operator<<(std::ostream& os, const Shape& v)
{
    os << '[';
    for (int i = 0; i < v.size(); ++i) {
        os << (i ? ", " : "") << v[i];
    }
    os << ']';
    return os;
}


std::ostream& operator<<(std::ostream& os, const Dimensions& d)
{
    os << "{'n':" << d.n << ", 'h':" << d.h << ", 'w':" << d.w << ", 'c':" << d.c << '}';
    return os;
}


std::ostream& operator<<(std::ostream& os, const Layout& layout)
{
    switch (layout) {
    case Layout::none:
        os << "none";
        return os;
    case Layout::nchw:
        os << "nchw";
        return os;
    case Layout::nhwc:
        os << "nhwc";
        return os;
    }
    os << "Layout?";
    return os;
}


std::ostream& operator<<(std::ostream& os, const DataType& type)
{
    switch (type) {
    case DataType::invalid:
        os << "invalid";
        return os;
    case DataType::byte:
        os << "byte";
        return os;
    case DataType::int8:
        os << "int8";
        return os;
    case DataType::uint8:
        os << "uint8";
        return os;
    case DataType::int16:
        os << "int16";
        return os;
    case DataType::uint16:
        os << "uint16";
        return os;
    case DataType::int32:
        os << "int32";
        return os;
    case DataType::uint32:
        os << "uint32";
        return os;
    case DataType::float16:
        os << "float16";
        return os;
    case DataType::float32:
        os << "float32";
        return os;
    }
    os << "DataType?(" << static_cast<int>(type) << ")";
    return os;
}


std::ostream& operator<<(std::ostream& os, const QuantizationScheme& qs) {
    switch (qs) {
    case QuantizationScheme::none:
        os << "none";
        return os;
    case QuantizationScheme::dynamic_fixed_point:
        os << "dynamic_fixed_point";
        return os;
    case QuantizationScheme::affine_asymmetric:
        os << "affine_asymmetric";
        return os;
    }
    os << "QuantizationScheme?(" << static_cast<int>(qs) << ")";
    return os;
}


std::ostream& operator<<(std::ostream& os, const Dim2d& d)
{
    os << d.x << ',' << d.y;
    return os;
}


std::ostream& operator<<(std::ostream& os, const Landmark& l)
{
    os << l.x << ',' << l.y << ',' << l.z << ',' << l.visibility;
    return os;
}


std::ostream& operator<<(std::ostream& os, const Rect& r)
{
    os << r.origin << ',' << r.size;
    return os;
}


bool from_string(Rect& r, const std::string& str)
{
    auto v = format_parse::get_ints(str, nullptr);
    if (v.size() != 4) {
        LOGE << "Error converting string: " << str << "to Rect";
        return false;
    }
    r = {v[0], v[1], v[2], v[3]};
    return true;
}


}  // namespace synap
}  // namespace synaptics
