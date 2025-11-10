// Copyright 2025 Synaptics Incorporated
// SPDX-License-Identifier: Apache-2.0

#include "predictor_torq.hpp"

namespace synaptics {
namespace synap {

PredictorTORQ::PredictorTORQ()
    : vm_instance_(nullptr),
      vm_session_(nullptr),
      next_attachment_id_(1),
      model_metadata_(nullptr),
      runtime_initialized_(false),
      model_loaded_(false) {

    memset(&main_call_, 0, sizeof(main_call_));
}

PredictorTORQ::~PredictorTORQ() {
    cleanup_runtime();
}

bool PredictorTORQ::load_model(const void* model, size_t size, NetworkMetadata* meta) {
    if (!model || size == 0 || !meta) {
        LOGE << "Invalid model parameters";
        return false;
    }

    const bool enable_npu = format_parse::get_bool(meta->delegate, "npu");
    const bool enable_cpu = format_parse::get_bool(meta->delegate, "cpu", !enable_npu);
    if (enable_npu) {
        device_name_ = TORQ_ID;
        if (enable_cpu) {
            LOGI << "Both NPU and CPU delegates specified; using NPU only.";
        }
    } else if (enable_cpu) {
        device_name_ = LOCAL_TASK_ID;
    } else {
        LOGW << "No delegate specified.";
        return false;
    }

    // Copy model data since we will need it later to perform inference
    const uint8_t* model_data = static_cast<const uint8_t*>(model);
    _model.assign(model_data, model_data + size);

    model_metadata_ = std::make_unique<NetworkMetadata>(*meta);

    LOGI << "Loading TORQ VMFB model, size: " << size << " bytes";
    LOGI << "TORQ VMFB meta, input size: " << model_metadata_->inputs.size() << " entries";

    // Initialize TORQ runtime if not already done
    if (!runtime_initialized_) {
        iree_status_t status = initialize_runtime();
        if (!iree_status_is_ok(status)) {
            LOGE << "Failed to initialize TORQ runtime";
            iree_status_fprint(stderr, status);
            iree_status_ignore(status);
            return false;
        }
    }

    // Create bytecode module from VMFB data
    iree_const_byte_span_t module_data = iree_make_const_byte_span(
        static_cast<const uint8_t*>(_model.data()), _model.size());

    iree_status_t status =
        iree_runtime_session_append_bytecode_module_from_memory(
            vm_session_, module_data, iree_allocator_null());

    if (!iree_status_is_ok(status)) {
        LOGE << "Failed to create IREE bytecode module";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return false;
    }

    status = iree_runtime_call_initialize_by_name(
        vm_session_, iree_make_cstring_view(ENTRY_FN_NAME), &main_call_);
    if (!iree_status_is_ok(status)) {
        LOGE << "Unable to resolve entry function from vm session";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return false;
    }

    model_loaded_ = true;
    LOGI << "TORQ VMFB model loaded successfully";

    return true;
}

bool PredictorTORQ::predict() {
    if (!model_loaded_ || !vm_session_) {
        LOGE << "Model not loaded or session not initialized";
        return false;
    }

    LOGI << "Running TORQ inference";

    iree_runtime_call_reset(&main_call_);

    iree_status_t status = prepare_invocation_inputs();
    if (!iree_status_is_ok(status)) {
        LOGE << "Failed to prepare invocation inputs";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return false;
    }

    status = iree_runtime_call_invoke(&main_call_, /*flags=*/0);

    if (!iree_status_is_ok(status)) {
        LOGE << "TORQ inference failed";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return false;
    }

    status = update_output_buffer_pointers();
    if (!iree_status_is_ok(status)) {
        LOGE << "Failed to update output buffer";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return false;
    }

    LOGI << "TORQ inference completed successfully";
    return true;
}

BufferAttachment PredictorTORQ::attach_buffer(Buffer* buffer, int32_t index, bool is_input) {
    if (!buffer) {
        LOGE << "Null buffer provided";
        return 0;
    }

    if (!vm_session_) {
        LOGE << "VM session not initialized";
        return 0;
    }

    // Create buffer info
    auto buffer_info = std::make_unique<BufferInfo>();
    buffer_info->synap_buffer = buffer;
    buffer_info->tensor_index = index;
    buffer_info->is_input = is_input;
    buffer_info->hal_buffer = nullptr;
    buffer_info->buffer_view = nullptr;

    // Create HAL buffer from Synap buffer
    iree_status_t status = create_hal_buffer(buffer, &buffer_info->hal_buffer);
    if (!iree_status_is_ok(status)) {
        LOGE << "Failed to create HAL buffer";
        iree_status_fprint(stderr, status);
        iree_status_ignore(status);
        return 0;
    }

    // Get tensor attributes for buffer view creation
    if (model_metadata_) {
        const auto& tensors = is_input ? model_metadata_->inputs : model_metadata_->outputs;
        if (index >= 0 && index < static_cast<int32_t>(tensors.size())) {
            status = create_buffer_view(
                buffer_info->hal_buffer,
                tensors[index],
                &buffer_info->buffer_view);

            if (!iree_status_is_ok(status)) {
                LOGE << "Failed to create buffer view";
                iree_hal_buffer_release(buffer_info->hal_buffer);
                iree_status_fprint(stderr, status);
                iree_status_ignore(status);
                return 0;
            }
        }
    }

    BufferAttachment attachment_id = next_attachment_id_++;
    attached_buffers_[attachment_id] = std::move(buffer_info);

    LOGI << "Buffer attached with ID: " << attachment_id;

    return attachment_id;
}

bool PredictorTORQ::set_buffer(Buffer* buffer, int32_t index, bool is_input, BufferAttachment handle) {
    auto it = attached_buffers_.find(handle);
    if (it == attached_buffers_.end()) {
        LOGE << "Invalid buffer attachment handle: " << handle;
        return false;
    }

    BufferInfo* buffer_info = it->second.get();
    if (buffer_info->synap_buffer != buffer ||
        buffer_info->tensor_index != index ||
        buffer_info->is_input != is_input) {
        LOGE << "Buffer parameters mismatch for attachment " << handle;
        return false;
    }

    // Buffer is already set during attachment for TORQ
    LOGI << "Buffer set for tensor " << index << " (input: " << is_input << ")";
    return true;
}

bool PredictorTORQ::detach_buffer(BufferAttachment handle) {
    auto it = attached_buffers_.find(handle);
    if (it == attached_buffers_.end()) {
        LOGE << "Invalid buffer attachment handle: " << handle;
        return false;
    }

    BufferInfo* buffer_info = it->second.get();

    // Release IREE resources
    if (buffer_info->buffer_view) {
        iree_hal_buffer_view_release(buffer_info->buffer_view);
        buffer_info->buffer_view = nullptr;
    }

    if (buffer_info->hal_buffer) {
        iree_hal_buffer_release(buffer_info->hal_buffer);
        buffer_info->hal_buffer = nullptr;
    }

    attached_buffers_.erase(it);

    LOGI << "Buffer detached: " << handle;
    return true;
}

Tensor* PredictorTORQ::get_tensor(int32_t index, bool is_input) {
    // Use default tensor creation
    return nullptr;
}

iree_status_t PredictorTORQ::initialize_runtime() {
    LOGI << "Initializing TORQ runtime";

    iree_runtime_instance_options_t instance_options;
    iree_runtime_instance_options_initialize(&instance_options);
    iree_runtime_instance_options_use_all_available_drivers(&instance_options);

    // Create VM instance
    IREE_RETURN_IF_ERROR(iree_runtime_instance_create(
        &instance_options, iree_allocator_system(), &vm_instance_));

    // Create torq device
    iree_hal_device_t* hal_device;
    IREE_RETURN_IF_ERROR(iree_runtime_instance_try_create_default_device(
        vm_instance_, iree_make_cstring_view(device_name_.c_str()), &hal_device));
    if (!hal_device) {
        return iree_make_status(IREE_STATUS_INTERNAL, "Failed to get TORQ device");
    }

    // Create session
    iree_runtime_session_options_t session_options;
    iree_runtime_session_options_initialize(&session_options);

    iree_status_t status = iree_runtime_session_create_with_device(
        vm_instance_, &session_options, hal_device,
        iree_runtime_instance_host_allocator(vm_instance_),
        &vm_session_);

    iree_hal_device_release(hal_device);
    if (iree_status_is_ok(status)) {
        runtime_initialized_ = true;
        LOGI << "TORQ runtime initialized successfully";
    }

    return status;
}

void PredictorTORQ::cleanup_runtime() {

    // Release runtime call
    iree_runtime_call_deinitialize(&main_call_);

    // Detach all buffers
    for (auto& pair : attached_buffers_) {
        BufferInfo* buffer_info = pair.second.get();
        if (buffer_info->buffer_view) {
            iree_hal_buffer_view_release(buffer_info->buffer_view);
            buffer_info->buffer_view = nullptr;
        }
        if (buffer_info->hal_buffer) {
            iree_hal_buffer_release(buffer_info->hal_buffer);
            buffer_info->hal_buffer = nullptr;
        }
    }
    attached_buffers_.clear();

    if (vm_session_) {
        iree_runtime_session_release(vm_session_);
        vm_session_ = nullptr;
    }

    runtime_initialized_ = false;
    model_loaded_ = false;

    LOGI << "TORQ runtime cleaned up";
}

iree_status_t PredictorTORQ::create_hal_buffer(Buffer* buffer, iree_hal_buffer_t** out_buffer) {
    if (!buffer || !out_buffer) {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "Invalid arguments");
    }

    // Get buffer properties
    void* data = buffer->data();
    size_t size = buffer->size();

    if (!data || size == 0) {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "Invalid buffer data");
    }

    iree_hal_device_t* device = iree_runtime_session_device(vm_session_);
    if (!device) {
        return iree_make_status(IREE_STATUS_FAILED_PRECONDITION, "No device available from session");
    }

    iree_hal_allocator_t* allocator = iree_hal_device_allocator(device);
    if (!allocator) {
        return iree_make_status(IREE_STATUS_FAILED_PRECONDITION, "No allocator available from device");
    }

    // Set up buffer parameters
    iree_hal_buffer_params_t params;
    params.type = IREE_HAL_MEMORY_TYPE_HOST_LOCAL | IREE_HAL_MEMORY_TYPE_DEVICE_VISIBLE;
    params.access = IREE_HAL_MEMORY_ACCESS_ALL;
    params.usage = IREE_HAL_BUFFER_USAGE_DEFAULT;
    params.queue_affinity = IREE_HAL_QUEUE_AFFINITY_ANY;

    iree_hal_external_buffer_t external_buffer;
    external_buffer.type = IREE_HAL_EXTERNAL_BUFFER_TYPE_HOST_ALLOCATION;
    external_buffer.flags = IREE_HAL_EXTERNAL_BUFFER_FLAG_NONE;
    external_buffer.size = static_cast<iree_device_size_t>(size);
    external_buffer.handle.host_allocation.ptr = data;

    IREE_RETURN_IF_ERROR(iree_hal_allocator_import_buffer(
        allocator,
        params,
        &external_buffer,
        iree_hal_buffer_release_callback_null(),
        out_buffer));
    LOGI << "Import buffer " << std::hex << buffer << "with data " << data << " as hal buffer " << out_buffer << "\n";

    return iree_ok_status();
}

iree_status_t PredictorTORQ::create_buffer_view(
    iree_hal_buffer_t* buffer,
    const TensorAttributes& tensor_attr,
    iree_hal_buffer_view_t** out_buffer_view) {

    if (!buffer || !out_buffer_view) {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "Invalid arguments");
    }

    // Convert shape
    std::vector<iree_hal_dim_t> dims;
    for (int32_t dim : tensor_attr.shape) {
        dims.push_back(static_cast<iree_hal_dim_t>(dim));
    }

    // Convert data type
    iree_hal_element_type_t element_type = convert_data_type(tensor_attr.dtype);

    // Create buffer view
    IREE_RETURN_IF_ERROR(iree_hal_buffer_view_create(
        buffer,
        dims.size(),
        dims.data(),
        element_type,
        IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR,
        iree_runtime_session_host_allocator(vm_session_),
        out_buffer_view));
    LOGI << "Created buffer view " << std::hex << out_buffer_view << " for hal buffer " << buffer << "\n";

    return iree_ok_status();
}

iree_hal_element_type_t PredictorTORQ::convert_data_type(DataType dtype) {
    switch (dtype) {
        case DataType::float32:
            return IREE_HAL_ELEMENT_TYPE_FLOAT_32;
        case DataType::float16:
            return IREE_HAL_ELEMENT_TYPE_FLOAT_16;
        case DataType::int32:
            return IREE_HAL_ELEMENT_TYPE_SINT_32;
        case DataType::int16:
            return IREE_HAL_ELEMENT_TYPE_SINT_16;
        case DataType::int8:
            return IREE_HAL_ELEMENT_TYPE_SINT_8;
        case DataType::uint32:
            return IREE_HAL_ELEMENT_TYPE_UINT_32;
        case DataType::uint16:
            return IREE_HAL_ELEMENT_TYPE_UINT_16;
        case DataType::uint8:
            return IREE_HAL_ELEMENT_TYPE_UINT_8;
        case DataType::byte:
            return IREE_HAL_ELEMENT_TYPE_UINT_8;
        default:
            LOGW << "Unsupported data type, defaulting to float32";
            return IREE_HAL_ELEMENT_TYPE_FLOAT_32;
    }
}

iree_status_t PredictorTORQ::prepare_invocation_inputs() {
    if (!model_metadata_) {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "not configured");
    }

    size_t input_count = model_metadata_->inputs.size();

    // Add input buffer views to the list
    for (size_t i = 0; i < input_count; ++i) {
        // Find attached buffer for this input
        iree_hal_buffer_view_t* buffer_view = nullptr;
        for (const auto& pair : attached_buffers_) {
            const BufferInfo* info = pair.second.get();
            if (info->is_input && info->tensor_index == static_cast<int32_t>(i)) {
                buffer_view = info->buffer_view;
                break;
            }
        }

        if (!buffer_view) {
            return iree_make_status(IREE_STATUS_FAILED_PRECONDITION,
                                  "Input buffer %zu not attached", i);
        }
        IREE_RETURN_IF_ERROR(iree_runtime_call_inputs_push_back_buffer_view(&main_call_, buffer_view));
    }

    return iree_ok_status();
}

iree_status_t PredictorTORQ::update_output_buffer_pointers() {
    if (!model_metadata_) {
        return iree_make_status(IREE_STATUS_INVALID_ARGUMENT, "not configured");
    }

    size_t expected_count = model_metadata_->outputs.size();

    for (size_t i = 0; i < expected_count; ++i) {
        // Get output buffer view from runtime call
        iree_hal_buffer_view_t* iree_output_view = nullptr;
        IREE_RETURN_IF_ERROR(iree_runtime_call_outputs_pop_front_buffer_view(&main_call_, &iree_output_view));

        if (!iree_output_view) {
            return iree_make_status(IREE_STATUS_FAILED_PRECONDITION,
                                  "Output %zu is null", i);
        }

        // Find the corresponding attached output buffer
        BufferInfo* target_buffer_info = nullptr;
        for (const auto& pair : attached_buffers_) {
            BufferInfo* info = pair.second.get();
            if (!info->is_input && info->tensor_index == static_cast<int32_t>(i)) {
                target_buffer_info = info;
                break;
            }
        }
        if (!target_buffer_info) {
            return iree_make_status(IREE_STATUS_FAILED_PRECONDITION,
                                  "No attached buffer found for output %zu", i);
        }

        // Get the underlying HAL buffer from IREE's output
        iree_hal_buffer_t* iree_output_buffer = iree_hal_buffer_view_buffer(iree_output_view);

        iree_hal_buffer_mapping_t temp_mapping;
        IREE_RETURN_IF_ERROR(iree_hal_buffer_map_range(
            iree_output_buffer,
            IREE_HAL_MAPPING_MODE_SCOPED,
            IREE_HAL_MEMORY_ACCESS_READ,
            0,  // offset
            IREE_WHOLE_BUFFER,
            &temp_mapping));

        // Copy data to existing Synap buffer
        bool assign_success = target_buffer_info->synap_buffer->assign(
            temp_mapping.contents.data,
            temp_mapping.contents.data_length);

        iree_hal_buffer_unmap_range(&temp_mapping);
        if (!assign_success) {
            return iree_make_status(IREE_STATUS_INTERNAL,
                                  "Failed to copy TORQ output data to Synap buffer %zu", i);
        }

        iree_hal_buffer_view_release(iree_output_view);
        LOGI << "Copied TORQ output data to Synap buffer " << i << " ("
             << temp_mapping.contents.data_length << " bytes)";
    }

    return iree_ok_status();
}

}  // namespace synap
}  // namespace synaptics
