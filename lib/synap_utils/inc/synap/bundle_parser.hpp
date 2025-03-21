// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Bundle information
///

#pragma once

#include <string>
#include <vector>


namespace synaptics {
namespace synap {


/// Bundle information
class BundleParser {
public:

    struct TensorInfo {
        int subgraph_index{};
        bool is_input{};
        size_t tensor_index{};
    };

    struct SubGraphInfo {
        std::vector<TensorInfo> inputs;

        // graph and meta file path
        std::string model;
        std::string meta;

        // graph data and meta data in memory
        std::vector<uint8_t> model_data;
        std::string meta_data;
    };

    struct GraphBundle {
        std::vector<TensorInfo> inputs;
        std::vector<TensorInfo> outputs;
        std::vector<SubGraphInfo> items;
    };

    ///  Constructor.
    ///
    BundleParser() {}
    virtual ~BundleParser() {}

    ///  init with file.
    ///
    ///  @param data: data containing bundle information
    virtual bool init(const std::string& filename);

    ///  init with a data buffer.
    ///
    ///  @param data: data containing bundle information
    virtual bool init(const void* data, const size_t size);

    const std::vector<SubGraphInfo>& graph_info() const;

    const std::vector<TensorInfo>& inputs() const;

    const std::vector<TensorInfo>& outputs() const;

    int parallel_limit() const;

protected:
    std::vector<SubGraphInfo> _graph_info;
    std::vector<TensorInfo> _inputs;
    std::vector<TensorInfo> _outputs;
    int _parallel_limit{};

private:
    bool parse(const std::string& info);
};

}  // namespace synap
}  // namespace synaptics
