// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2021 Synaptics Incorporated */

#include <linux/string.h>
#include <linux/slab.h>

#include "synap_kernel_ca.h"
#include "synap_kernel_log.h"
#include <tee_client_api.h>
#include "tee_client_const.h"
#include <ca/synap_ta_cmd.h>

struct synap_ca {
    TEEC_Context teec_context;
    TEEC_Session teec_session;
};

static const char *synap_ta_log_code_to_str(TEEC_Result code) {
    switch (code) {
        case TEEC_SUCCESS: return "SUCCESS";
        case TEEC_ERROR_GENERIC: return "GENERIC";
        case TEEC_ERROR_ACCESS_DENIED: return "ACCESS_DENIED";
        case TEEC_ERROR_CANCEL: return "CANCEL";
        case TEEC_ERROR_ACCESS_CONFLICT: return "ACCESS_CONFLICT";
        case TEEC_ERROR_EXCESS_DATA: return "EXCESS_DATA";
        case TEEC_ERROR_BAD_FORMAT: return "BAD_FORMAT/INCOMPATIBLE_HW";
        case TEEC_ERROR_BAD_PARAMETERS: return "BAD_PARAMETERS";
        case TEEC_ERROR_BAD_STATE: return "BAD_STATE";
        case TEEC_ERROR_ITEM_NOT_FOUND: return "ITEM_NOT_FOUND";
        case TEEC_ERROR_NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
        case TEEC_ERROR_NOT_SUPPORTED: return "NOT_SUPPORTED";
        case TEEC_ERROR_NO_DATA: return "NO_DATA";
        case TEEC_ERROR_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
        case TEEC_ERROR_BUSY: return "BUSY";
        case TEEC_ERROR_COMMUNICATION: return "COMMUNICATION";
        case TEEC_ERROR_SECURITY: return "SECURITY";
        case TEEC_ERROR_SHORT_BUFFER: return "SHORT_BUFFER";
        default: return "Unknown";
    }
}


bool synap_ca_create(struct synap_ca **synap_ca) {
    *synap_ca = (struct synap_ca *) kzalloc(sizeof(struct synap_ca), GFP_KERNEL);

    if (!*synap_ca) {
        KLOGE("cannot allocate memory for synap_ca");
        return false;
    }

    return TEEC_InitializeContext(NULL, &((*synap_ca)->teec_context)) == TEEC_SUCCESS;
}

bool synap_ca_load_ta(struct synap_ca *synap_ca, void *ta_data, size_t ta_size) {

    TEEC_Result ret = TEEC_SUCCESS;
    TEEC_SharedMemory teeShm;
    TEEC_Parameter parameter;

    teeShm.size = ta_size;
    teeShm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

    ret = TEEC_AllocateSharedMemory(&synap_ca->teec_context, &teeShm);

    if (ret != TEEC_SUCCESS || (NULL == teeShm.buffer)) {
        KLOGE("alloc teeshm failed ret=0x%x", ret);
        return false;
    }

    memcpy(teeShm.buffer, ta_data, teeShm.size);

    parameter.memref.parent = &teeShm;
    parameter.memref.size = teeShm.size;
    parameter.memref.offset = 0;

    ret = TEEC_RegisterTA(&synap_ca->teec_context, &parameter, TEEC_MEMREF_PARTIAL_INPUT);

    if (TEEC_SUCCESS == ret) {
        KLOGH("SyNAP TA loaded successfully");
    } else if (TEEC_ERROR_ACCESS_CONFLICT == ret) {
        KLOGE("SyNAP TA has been registered");
        ret = TEEC_SUCCESS;
    } else {
        KLOGE("SyNAP TA loading failed, ret=0x%x", ret);
    }

    TEEC_ReleaseSharedMemory(&teeShm);

    return ret == TEEC_SUCCESS;
}

bool synap_ca_create_session(struct synap_ca *synap_ca,
                                    struct sg_table *secure_buf,
                                    struct sg_table *non_secure_buf)
{
    TEEC_SharedMemory shm;
    TEEC_Result result;
    struct page *pg = NULL;
    TEEC_Operation operation;
    struct synap_ta_memory_area* areas;
    const TEEC_UUID uuid = SYNAP_TA_UUID;

    LOG_ENTER();

    memset(&operation, 0, sizeof(TEEC_Operation));

    if (secure_buf->nents != 1 || non_secure_buf->nents != 1) {
        KLOGE("only contiguous driver buffers are supported");
        return false;
    }

    shm.size = 2 * sizeof(struct synap_ta_memory_area);
    shm.flags = TEEC_MEM_INPUT;

    if ((result = TEEC_AllocateSharedMemory(&synap_ca->teec_context, &shm)) != TEEC_SUCCESS) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    areas = (struct synap_ta_memory_area *) shm.buffer;

    pg = sg_page(non_secure_buf->sgl);
    areas[0].addr = PFN_PHYS(page_to_pfn(pg));
    areas[0].npage = non_secure_buf->sgl->length / PAGE_SIZE;

    pg = sg_page(secure_buf->sgl);
    areas[1].addr = PFN_PHYS(page_to_pfn(pg));
    areas[1].npage = secure_buf->sgl->length / PAGE_SIZE;

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_NONE,
                                            TEEC_NONE, TEEC_NONE);

    operation.params[0].memref.parent = &shm;
    operation.params[0].memref.size = shm.size;
    operation.params[0].memref.offset = 0;

    result = TEEC_OpenSession(&synap_ca->teec_context, &synap_ca->teec_session, &uuid, TEEC_LOGIN_USER, NULL,
                           &operation, (uint32_t *) NULL);

    if (result != TEEC_SUCCESS) {
        KLOGE("error while opening session ret=0x%08x, shm.phyAddr=0x%08x", result, shm.phyAddr);
    }

    TEEC_ReleaseSharedMemory(&shm);

    return result == TEEC_SUCCESS;
}

bool synap_ca_activate_npu(struct synap_ca *synap_ca, u8 mode)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = mode > 0 ? 1 : 0;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_ACTIVATE_NPU, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }

    return result == TEEC_SUCCESS;
}


bool synap_ca_deactivate_npu(struct synap_ca *synap_ca)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_DEACTIVATE_NPU, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }

    return result == TEEC_SUCCESS;
}



bool synap_ca_set_input(struct synap_ca *synap_ca, u32 nid, u32 aid, u32 index)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                            TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = nid;
    operation.params[0].value.b = aid;
    operation.params[1].value.a = index;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_SET_INPUT, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }

    return result == TEEC_SUCCESS;
}

bool synap_ca_set_output(struct synap_ca *synap_ca, u32 nid, u32 aid, u32 index)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                            TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = nid;
    operation.params[0].value.b = aid;
    operation.params[1].value.a = index;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_SET_OUTPUT, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }

    return result == TEEC_SUCCESS;
}

static void synap_ca_copy_to_memory_areas(struct synap_ta_memory_area *areas,
                                          struct sg_table *sgt) {
    u32 i;
    struct scatterlist *sg = NULL;
    struct page *pg = NULL;

    for_each_sg(sgt->sgl, sg, sgt->nents, i) {
        pg = sg_page(sg);
        areas[i].addr = PFN_PHYS(page_to_pfn(pg));
        areas[i].npage = sg->length / PAGE_SIZE;
        KLOGD("sg 0x%08x %d", areas[i].addr, areas[i].npage);
    }
}

static TEEC_Result synap_ca_wrap_user_buffer(struct synap_ca *synap_ca, struct sg_table *ub_sg,
                                             u32 ub_offset, u32 ub_size,
                                             TEEC_SharedMemory *shm,
                                             struct synap_ta_user_buffer **ub) {
    TEEC_Result result;

    struct synap_ta_user_buffer *res;

    LOG_ENTER();

    shm->size = sizeof(struct synap_ta_user_buffer);
    shm->size += sizeof(struct synap_ta_memory_area) * ub_sg->nents;

    shm->flags = TEEC_MEM_INPUT;

    if ((result = TEEC_AllocateSharedMemory(synap_ca->teec_session.device, shm)) != TEEC_SUCCESS) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    res = shm->buffer;

    res->areas_count = ub_sg->nents;

    res->offset = ub_offset;
    res->size = ub_size;

    synap_ca_copy_to_memory_areas(&res->areas[0], ub_sg);

    *ub = res;

    return true;
}

static TEEC_Result synap_ca_create_network_resources(
        struct synap_ca *synap_ca, struct sg_table *code,
        struct sg_table *page_table, struct sg_table *pool, struct sg_table *profile_data,
        TEEC_SharedMemory *shm, struct synap_ta_network_resources **resources) {

    TEEC_Result result;
    u32 offset = 0;

    struct synap_ta_network_resources *res;

    LOG_ENTER();

    shm->size = sizeof(struct synap_ta_network_resources);
    shm->size += sizeof(struct synap_ta_memory_area) * code->nents;
    shm->size += sizeof(struct synap_ta_memory_area) * page_table->nents;

    if (pool != NULL) {
        shm->size += sizeof(struct synap_ta_memory_area) * pool->nents;
    }

    if (profile_data != NULL) {
        shm->size += sizeof(struct synap_ta_memory_area) * profile_data->nents;
    }

    shm->flags = TEEC_MEM_INPUT;

    if ((result = TEEC_AllocateSharedMemory(synap_ca->teec_session.device, shm)) != TEEC_SUCCESS) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    res = shm->buffer;

    res->code_areas_count = code->nents;
    res->page_table_areas_count = page_table->nents;

    if (pool != NULL) {
        res->pool_areas_count = pool->nents;
    } else {
        res->pool_areas_count = 0;
    }

    if (profile_data != NULL) {
        res->profile_operation_areas_count = profile_data->nents;
    } else {
        res->profile_operation_areas_count = 0;
    }

    synap_ca_copy_to_memory_areas(&res->areas[0], code);

    offset = res->code_areas_count;

    synap_ca_copy_to_memory_areas(&res->areas[offset], page_table);

    offset += res->page_table_areas_count;

    if (pool != NULL) {
        synap_ca_copy_to_memory_areas(&res->areas[offset], pool);

        offset += res->pool_areas_count;
    }

    if (profile_data != NULL) {
        synap_ca_copy_to_memory_areas(&res->areas[offset], profile_data);
    }

    *resources = res;

    return true;
}

bool synap_ca_create_network(struct synap_ca *synap_ca,
                                    struct sg_table *header,
                                    u32 header_offset, u32 header_size,
                                    struct sg_table *payload,
                                    u32 payload_offset, u32 payload_size,
                                    struct sg_table *code, struct sg_table *page_table,
                                    struct sg_table *pool, struct sg_table *profile_data,
                                    u32 *nid)
{

    TEEC_SharedMemory resources_shm;
    TEEC_SharedMemory header_shm;
    TEEC_SharedMemory payload_shm;

    TEEC_Operation operation;
    TEEC_Result result;
    struct synap_ta_user_buffer *header_ub;
    struct synap_ta_user_buffer *payload_ub;
    struct synap_ta_network_resources *res;

    LOG_ENTER();

    memset(&operation, 0, sizeof(TEEC_Operation));

    if (!synap_ca_create_network_resources(synap_ca, code, page_table, pool, profile_data,
                                           &resources_shm, &res)) {
        return false;
    }

    if (!synap_ca_wrap_user_buffer(synap_ca, header, header_offset, header_size,
                                   &header_shm, &header_ub)) {
        TEEC_ReleaseSharedMemory(&resources_shm);
        return false;
    }

    if (!synap_ca_wrap_user_buffer(synap_ca, payload, payload_offset, payload_size,
                                   &payload_shm, &payload_ub)) {
        TEEC_ReleaseSharedMemory(&resources_shm);
        TEEC_ReleaseSharedMemory(&header_shm);
        return false;
    }

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_MEMREF_PARTIAL_INPUT,
                                            TEEC_MEMREF_PARTIAL_INPUT, TEEC_VALUE_OUTPUT);

    operation.params[0].memref.parent = &header_shm;
    operation.params[0].memref.size = header_shm.size;

    operation.params[1].memref.parent = &payload_shm;
    operation.params[1].memref.size = payload_shm.size;

    operation.params[2].memref.parent = &resources_shm;
    operation.params[2].memref.size =  resources_shm.size;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_CREATE_NETWORK, &operation,
                                (uint32_t *) NULL);

    if (result != TEEC_SUCCESS) {
        // SECURITY error here normally means that the network has been crypted/signed
        // using a different key set
        KLOGE("Error=0x%08x (%s) "
              "header.phyAddr=0x%08x size %d area count %d "
              "payload.phyAddr=0x%08x size %d area count %d "
              "resources.phyAddr=0x%08x size %d "
              "code count %d pt count %d pool count %d",
              result, synap_ta_log_code_to_str(result),
              header_shm.phyAddr, header_shm.size, header_ub->areas_count,
              payload_shm.phyAddr, payload_shm.size, payload_ub->areas_count,
              resources_shm.phyAddr, resources_shm.size,
              code->nents, page_table->nents, pool ? pool->nents: -1);
    }

    TEEC_ReleaseSharedMemory(&resources_shm);
    TEEC_ReleaseSharedMemory(&header_shm);
    TEEC_ReleaseSharedMemory(&payload_shm);

    *nid = operation.params[3].value.a;
    return result == TEEC_SUCCESS;
}

bool synap_ca_destroy_network(struct synap_ca *synap_ca, u32 nid)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = nid;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_DESTROY_NETWORK, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}

bool synap_ca_start_network(struct synap_ca *synap_ca, u32 nid)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = nid;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_START_NETWORK, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        // BAD_FORMAT here normally means that the network has been compiled for an incompatible HW
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}

static TEEC_Result synap_ca_create_areas(struct synap_ca *synap_ca,
                                        struct sg_table *buf, u32 offset, u32 size,
                                        size_t *count,
                                        TEEC_SharedMemory *shm) {

    struct scatterlist *sg = NULL;
    struct page *pg = NULL;
    u32 i = 0;

    struct synap_ta_memory_area* areas;

    u64 mem_start, mem_end;
    size_t area_idx = 0;
    u32 areas_count = 0;
    u32 current_offset = 0;

    LOG_ENTER();

    if (offset % PAGE_SIZE != 0) {
        KLOGE("offset is not a multiple of PAGE_SIZE");
        return false;
    }

    mem_start = offset;
    mem_end = offset + size;

    if (buf->nents == 0) {
        KLOGE("sg list has no entries");
        return false;
    }

    // Count the number of segments that fall (completely or partially)
    // in the range [offset, offset + size)
    for_each_sg(buf->sgl, sg, buf->nents, i) {

        if (current_offset < mem_end && current_offset + sg->length > mem_start) {
            areas_count++;
        }

        current_offset += sg->length;
    }

    shm->size = areas_count * sizeof(struct synap_ta_memory_area);
    shm->flags = TEEC_MEM_INPUT;

    if (TEEC_AllocateSharedMemory(synap_ca->teec_session.device, shm) != TEEC_SUCCESS) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    areas = (struct synap_ta_memory_area *) shm->buffer;

    current_offset = 0;

    // Select and cut the areas so that they don't go outside the range [offset, offset + size)
    for_each_sg(buf->sgl, sg, buf->nents, i) {

        u64 sg_start;
        u64 sg_end;

        pg = sg_page(sg);

        KLOGI("processing sg addr 0x%08x pages %d", (uint32_t) PFN_PHYS(page_to_pfn(pg)),
              sg->length / PAGE_SIZE);

        sg_start = current_offset;
        sg_end = current_offset + sg->length;

        if (sg_end > mem_start && sg_start < mem_end ) {

            u64 range_start = sg_start > mem_start ? sg_start: mem_start;
            u64 range_end = sg_end < mem_end ? sg_end : mem_end;

            areas[area_idx].addr = PFN_PHYS(page_to_pfn(pg)) + (range_start - current_offset);
            areas[area_idx].npage = (range_end - range_start) / PAGE_SIZE;

            if ((range_end - range_start) % PAGE_SIZE != 0) {
                areas[area_idx].npage += 1;
            }

            KLOGI("created sg addr=0x%x len=%d", areas[area_idx].addr, areas[area_idx].npage);

            area_idx++;
        }

        current_offset += sg->length;

    }

    *count = areas_count;

    return true;

}


bool synap_ca_create_io_buffer_from_mem_id(struct synap_ca *synap_ca, u32 mem_id,
                                                  u32 offset, u32 size, u32 *bid) {
    TEEC_Operation operation;
    TEEC_Result result;

    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
                                            TEEC_VALUE_OUTPUT, TEEC_NONE);

    operation.params[0].value.a = mem_id;

    operation.params[1].value.a = offset;
    operation.params[1].value.b = size;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_CREATE_IO_BUFFER_FROM_MEM_ID,
                                &operation, (uint32_t *) NULL);

    *bid = operation.params[2].value.a;

    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}


bool synap_ca_create_io_buffer_from_sg(struct synap_ca *synap_ca, struct sg_table *buf,
                                              u32 offset, u32 size, bool secure, u32 *bid,
                                              u32 *mem_id) {
    TEEC_SharedMemory shm;
    TEEC_Result result;

    TEEC_Operation operation;

    size_t areas_count;

    LOG_ENTER();

    memset(&operation, 0, sizeof(TEEC_Operation));

    if (!synap_ca_create_areas(synap_ca, buf, offset, size, &areas_count, &shm)) {
        return false;
    }

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INPUT, TEEC_VALUE_INPUT,
                                            TEEC_VALUE_OUTPUT, TEEC_NONE);
    operation.params[0].memref.parent = &shm;
    operation.params[0].memref.size = shm.size;

    operation.params[1].value.a = secure ? 1 : 0;
    operation.params[1].value.b = areas_count;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_CREATE_IO_BUFFER_FROM_SG,
                             &operation, (uint32_t *) NULL);

    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s), shm.phyAddr=0x%08x", result, synap_ta_log_code_to_str(result), shm.phyAddr);
    }

    TEEC_ReleaseSharedMemory(&shm);

    if (result == TEEC_SUCCESS) {
        *bid = operation.params[2].value.a;
        *mem_id = operation.params[2].value.b;
    }

    return result == TEEC_SUCCESS;
}

bool synap_ca_attach_io_buffer(struct synap_ca *synap_ca, u32 nid,
                                      u32 bid, u32 *aid) {
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(TEEC_Operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT,
                                            TEEC_NONE, TEEC_NONE);

    operation.params[0].value.a = bid;
    operation.params[0].value.b = nid;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_ATTACH_IO_BUFFER,
                                &operation, (uint32_t *) NULL);
    *aid = operation.params[1].value.a;

    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}



bool synap_ca_detach_io_buffer(struct synap_ca *synap_ca, u32 nid, u32 aid)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = nid;
    operation.params[0].value.b = aid;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_DETACH_IO_BUFFER, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}


bool synap_ca_destroy_io_buffer(struct synap_ca *synap_ca, u32 bid)
{
    TEEC_Operation operation;
    TEEC_Result result;
    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);
    operation.params[0].value.a = bid;

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_DESTROY_IO_BUFFER, &operation, NULL);
    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}


bool synap_ca_read_interrupt_register(struct synap_ca *synap_ca, volatile u32 *reg_value)
{
    TEEC_Operation operation;
    TEEC_Result result;

    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_READ_INTERRUPT_REGISTER, &operation, NULL);
    *reg_value = operation.params[0].value.a;

    // TEEC_ERROR_BAD_STATE may be raised if the execution was aborted
    if (result == TEEC_ERROR_BAD_STATE) {
        KLOGI("Info=0x%08x (%s) while reading npu status register", result, synap_ta_log_code_to_str(result));
    } else if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s) while reading npu status register", result, synap_ta_log_code_to_str(result));
    }

    return result == TEEC_SUCCESS;
}

bool synap_ca_dump_state(struct synap_ca *synap_ca) {
    TEEC_Operation operation;
    TEEC_Result result;

    LOG_ENTER();

    memset(&operation, 0, sizeof(operation));

    operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    result = TEEC_InvokeCommand(&synap_ca->teec_session, SYNAP_TA_CMD_DUMP_STATE, &operation, NULL);

    if (result != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x (%s)", result, synap_ta_log_code_to_str(result));
    }
    return result == TEEC_SUCCESS;
}


bool synap_ca_destroy_session(struct synap_ca *synap_ca)
{
    LOG_ENTER();

    TEEC_CloseSession(&synap_ca->teec_session);

    return true;
}


void synap_ca_destroy(struct synap_ca* synap_ca) {
    TEEC_FinalizeContext(&synap_ca->teec_context);
    kfree(synap_ca);
}