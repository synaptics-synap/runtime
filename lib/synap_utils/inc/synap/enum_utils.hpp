// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once
#include <string>


namespace synaptics {
namespace synap {

template <typename E>
struct EnumStr {
    const char* tag;
    E value;
};

/// Convert a string to an enum value
/// @return true if conversion successful
template <typename E, size_t N>
bool get_enum(const std::string& str, const EnumStr<E> (&enum_table)[N], E& out)
{
    for (const auto& enum_entry : enum_table) {
        if (str == enum_entry.tag) {
            out = enum_entry.value;
            return true;
        }
    }
    return false;
}

/// @return string with all enum tags separated by '|'
template <typename E, size_t N>
std::string enum_tags(const EnumStr<E> (&enum_table)[N])
{
    std::string enum_tags;
    std::string sep;
    for (const auto& enum_entry : enum_table) {
        if (enum_entry.tag[0]) {
            enum_tags += sep + enum_entry.tag;
            sep  = '|';
        }
    }
    return enum_tags;
}


}  // namespace synap
}  // namespace synaptics
