// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Application-level information for AI model.
///

#pragma once

#include <string>
#include <vector>


namespace synaptics {
namespace synap {


/// Application-level information for Classification and Object Detection
class LabelInfo {
public:
    ///  Default constructor.
    LabelInfo();

    ///  Construct with a file.
    ///
    ///  @param filename: name of json file containing application-level information
    LabelInfo(const std::string& filename);

    ///  Init with a file.
    ///
    ///  @param filename: name of json file containing application-level information
    bool init(const std::string& filename);

    ///  Init with a file.
    ///  (Avoid relying on C++ STL types as arguments for compatibility with other lib versions)
    ///
    ///  @param filename: name of json file containing application-level information
    bool init(const char* filename);

    ///  Get label.
    ///
    ///  @param class_index: index of the class
    ///  @return class label if available else empty string
    const std::string& label(int class_index) const;

    ///  Get label as char*.
    ///  (Avoid relying on C++ STL types as arguments for compatibility with other lib versions)
    ///
    ///  @param class_index: index of the class
    ///  @return class label if available else empty string
    const char* label_ptr(int class_index) const;

private:
    std::vector<std::string> _labels;
};

}  // namespace synap
}  // namespace synaptics
