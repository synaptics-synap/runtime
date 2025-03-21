// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#pragma once

#include <cstdint>
#include <string>
#include <vector>


namespace synaptics {
namespace synap {

/// @return filename extension
std::string filename_extension(const std::string& file_name);

/// @return filename without extension and path
std::string filename_without_extension(const std::string& file_name);

/// @return filename path
std::string filename_path(const std::string& file_name);

/// @return true if file exists
bool file_exists(const std::string& file_name);

/// Search if a file with a given name is present in the specifed path
/// or in any parent directory up to the specified number of levels
/// @return filename path if found, else empty string
std::string file_find_up(const std::string& file_name, const std::string& path, int levels = 2);

/// @return true if directory exists
bool directory_exists(const std::string& directory_name);

/// Read content of a file
std::string file_read(const std::string& file_name);

/// Read content of a binary file
std::vector<uint8_t> binary_file_read(const std::string& file_name);

/// Write data to a binary file
/// @return true if success
bool binary_file_write(const std::string& file_name, const void* data, size_t size);

/// wrapper for non-C++-17 systems std::filesystem::create_directory()
bool create_directory(const std::string& out_dir);

/// wrapper for non-C++-17 systems std::filesystem::create_directories()
bool create_directories(const std::string& out_dir);

}  // namespace synap
}  // namespace synaptics
