// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/file_utils.hpp"
#include "synap/logging.hpp"
#include "synap/bundle_parser_zip.hpp"
#include "synap/zip_tool.hpp"

using namespace std;

namespace synaptics {
namespace synap {


bool BundleParserZip::init(const void* data, const size_t size)
{
    ZipTool ztool;
    if (!ztool.open(data, size)) {
        LOGE << "Failed to open zip archive";
        return false;
    }

    vector<uint8_t> bundle_info = ztool.extract_archive("bundle.json");
    if (bundle_info.empty() || !BundleParser::init(bundle_info.data(), bundle_info.size())) {
        LOGE << "Failed to parse model bundle information";
        return false;
    }

    for (auto& gi: _graph_info) {
        vector<uint8_t> meta_data = ztool.extract_archive(gi.meta);
        gi.meta_data = string(meta_data.begin(), meta_data.end());
        if (gi.meta_data.empty()) {
            LOGE << "Failed to extract meta data for graph: " << gi.meta;
            return false;
        }

        gi.model_data = ztool.extract_archive(gi.model);
        if (gi.model_data.empty()) {
            LOGE << "Failed to extract model data for graph: " << gi.model;
            return false;
        }
    }

    return true;
}


}  // namespace synap
}  // namespace synaptics
