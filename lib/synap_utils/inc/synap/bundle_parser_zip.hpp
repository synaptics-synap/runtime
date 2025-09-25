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
class BundleParserZip : public BundleParser {
public:

    ///  Construct.
    ///
    BundleParserZip() {}
    ~BundleParserZip() {}

    ///  init with a buffer.
    ///
    ///  @param data: buffer containing bundle information
    bool init(const void* data, const size_t size) override;

};

}  // namespace synap
}  // namespace synaptics
