// Copyright 2025 Synaptics Incorporated
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "predictor.hpp"
#include "synap/logging.hpp"
#include "synap/tensor.hpp"
#include "synap/string_utils.hpp"

// IREE Runtime includes
#include "iree/base/api.h"
#include "iree/hal/api.h"
#include "iree/modules/hal/module.h"
#include "iree/vm/api.h"
#include "iree/vm/bytecode/module.h"
#include "iree/runtime/api.h"

#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>

#define TORQ_ID "torq"
#define LOCAL_TASK_ID "local-task"
#define TORQ_HW_TYPE "astra_machina"
#define ENTRY_FN_NAME "module.main"

namespace synaptics {
namespace synap {

/// TORQ Predictor
/// This predictor uses TORQ runtime with TORQ HAL driver to execute
/// compiled VMFB models on the NPU via /dev/torq device node.
class PredictorTORQ : public Predictor {
public:
    PredictorTORQ();
    ~PredictorTORQ() override;

    PredictorTORQ(const PredictorTORQ&) = delete;
    PredictorTORQ& operator=(const PredictorTORQ&) = delete;

    /// Load VMFB model using TORQ runtime
    bool load_model(const void* model, size_t size, NetworkMetadata* meta) override;

    /// Run inference using IREE VM
    bool predict() override;

    /// Attach buffer for IREE HAL
    BufferAttachment attach_buffer(Buffer* buffer, int32_t index, bool is_input) override;

    /// Set buffer for IREE tensor
    bool set_buffer(Buffer* buffer, int32_t index, bool is_input, BufferAttachment handle) override;

    /// Detach buffer from IREE HAL
    bool detach_buffer(BufferAttachment handle) override;

    /// Get tensor information
    Tensor* get_tensor(int32_t index, bool is_input) override;

private:
    /// Buffer attachment information
    struct BufferInfo {
        iree_hal_buffer_t* hal_buffer;
        iree_hal_buffer_view_t* buffer_view;
        Buffer* synap_buffer;
        int32_t tensor_index;
        bool is_input;
    };

    /// Initialize TORQ runtime components
    iree_status_t initialize_runtime();

    /// Cleanup TORQ runtime resources
    void cleanup_runtime();

    /// Create HAL buffer from Synap buffer
    iree_status_t create_hal_buffer(Buffer* buffer, iree_hal_buffer_t** out_buffer);

    /// Create buffer view for tensor
    iree_status_t create_buffer_view(iree_hal_buffer_t* buffer,
                                   const TensorAttributes& tensor_attr,
                                   iree_hal_buffer_view_t** out_buffer_view);

    /// Convert Synap data type to IREE element type
    iree_hal_element_type_t convert_data_type(DataType dtype);

    /// Prepare input/output lists for VM invocation
    iree_status_t prepare_invocation_inputs();
    iree_status_t update_output_buffer_pointers();

    // IREE Runtime components
    iree_runtime_instance_t* vm_instance_;
    iree_runtime_session_t* vm_session_;
    iree_runtime_call_t main_call_;

    // Buffer management
    std::map<BufferAttachment, std::unique_ptr<BufferInfo>> attached_buffers_;
    BufferAttachment next_attachment_id_;
    std::unique_ptr<NetworkMetadata> model_metadata_;
    std::vector<uint8_t> _model{};

    // Initialization state
    bool runtime_initialized_;
    bool model_loaded_;

    std::string device_name_;
};

}  // namespace synap
}  // namespace synaptics
