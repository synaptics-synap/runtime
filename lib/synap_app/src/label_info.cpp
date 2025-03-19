// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/file_utils.hpp"
#include "synap/logging.hpp"
#include "synap/label_info.hpp"

#include "json.hpp"

using namespace std;

namespace synaptics {
namespace synap {

using json = nlohmann::json;

LabelInfo::LabelInfo()
{
}

LabelInfo::LabelInfo(const std::string& filename)
{
    if (!filename.empty())
        init(filename);
}


bool LabelInfo::init(const std::string& filename)
{
    if (!file_exists(filename)) {
        LOGE << "Info file not found: " << filename;
        return false;
    }
    std::string info_string = file_read(filename);
    if (info_string.empty()) {
        LOGE << "Failed to read info file: " << filename;
        return false;
    }

    try {
        json j = json::parse(info_string);
        j["labels"].get_to(_labels);
    }
    catch (json::parse_error& e)
    {
        LOGE << filename << ": " << e.what();
        return false;
    }
    return true;
}


bool LabelInfo::init(const char* filename)
{
    return init(string(filename));
}


const std::string& LabelInfo::label(int class_index) const
{
    static const string no_label;
    return class_index >= 0 && class_index < _labels.size() ? _labels[class_index] : no_label;
}


const char* LabelInfo::label_ptr(int class_index) const
{
    return label(class_index).c_str();
}


}  // namespace synap
}  // namespace synaptics
