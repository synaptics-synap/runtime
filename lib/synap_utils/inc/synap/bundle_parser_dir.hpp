// SPDX-License-Identifier: Apache-2.0
// Copyright (C) 2013-2022 Synaptics Incorporated. All rights reserved.

///
/// Bundle information
///

#pragma once

#include "bundle_parser.hpp"

#include <string>
#include <vector>


namespace synaptics {
namespace synap {


/// Bundle information
class BundleParserDir : public BundleParser {
public:

    ///  Construct.
    ///
    BundleParserDir() {}
    ~BundleParserDir() {}

    ///  init with a file.
    ///
    ///  @param filename: name of json file containing bundle information
    bool init(const std::string& filename) override;


};

}  // namespace synap
}  // namespace synaptics
