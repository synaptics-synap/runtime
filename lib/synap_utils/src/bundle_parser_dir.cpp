// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/bundle_parser_dir.hpp"

#include "synap/file_utils.hpp"
#include "synap/logging.hpp"

#include <filesystem>

using namespace std;

namespace synaptics {
namespace synap {

namespace fs = std::filesystem;

static string get_absolute_path(const string& parent_path, const string& filename)
{
    fs::path tmp(filename);
    if (tmp.is_relative()) {
        return parent_path + "/" + filename;
    }
    return filename;
}

bool BundleParserDir::init(const std::string& filename)
{
    if (!BundleParser::init(filename)) {
        LOGE << "Failed to parse info file: " << filename;
        return false;
    }

    auto absolute_json_path = fs::canonical(filename);
    string cur_parent_path = absolute_json_path.parent_path();

    for (auto& i : _graph_info) {
        i.model = get_absolute_path(cur_parent_path, i.model);
        i.meta = get_absolute_path(cur_parent_path, i.meta);

        LOGV << " model: " << i.model << " meta: " << i.meta;
    }

    return true;
}


}  // namespace synap
}  // namespace synaptics
