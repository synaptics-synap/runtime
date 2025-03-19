// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/detector.hpp"
#include "synap/classifier.hpp"
#include "synap/logging.hpp"
#include "json.hpp"


using namespace std;

namespace synaptics {
namespace synap {

static constexpr int json_dump_indent = 2;

using json = nlohmann::json;

void to_json(json& j, const Dim2d& p);
void to_json(json& j, const Landmark& p);
void to_json(json& j, const Mask& p);
void to_json(json& j, const Rect& p);
void to_json(json& j, const Detector::Result::Item& p);
void to_json(json& j, const Detector::Result& p);
void to_json(json& j, const Classifier::Result& p);


void to_json(json& j, const Dim2d& p)
{
    j = json{{"x", p.x}, {"y", p.y}};
}


void to_json(json& j, const Landmark& p)
{
    j = json{{"x", p.x}, {"y", p.y}, {"z", p.z}, {"visibility", p.visibility}};
}

void to_json(json& j, const Mask& mask)
{
    j = json{
        {"width", mask.width()},
        {"height", mask.height()},
        {"data", mask.buffer()}
    };
}

void to_json(json& j, const Rect& p)
{
    j = json{
        {"origin", p.origin},
        {"size", p.size}
    };
}


void to_json(json& j, const Detector::Result::Item& p)
{
    json lms = json{ { "points", p.landmarks } };
    j = json{
        {"class_index", p.class_index},
        {"confidence", p.confidence},
        {"bounding_box", p.bounding_box},
        {"landmarks", lms },
        {"mask", p.mask}
    };
}

void to_json(json& j, const Detector::Result& p)
{
    j = json{
        {"success", p.success},
        {"items", p.items}
    };
}


void to_json(json& j, const Classifier::Result::Item& p)
{
    j = json{
        {"class_index", p.class_index},
        {"confidence", p.confidence}
    };
}


void to_json(json& j, const Classifier::Result& p)
{
    j = json{
        {"success", p.success},
        {"items", p.items}
    };
}


std::string to_json_str(const Detector::Result& p) {
    json j;
    to_json(j, p);
    return j.dump(json_dump_indent);
}


std::string to_json_str(const Classifier::Result& p)
{
    json j;
    to_json(j, p);
    return j.dump(json_dump_indent);
}


}  // namespace synap
}  // namespace synaptics
