// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2021 Synaptics Incorporated */

#include <linux/string.h>
#include <linux/slab.h>
#include <linux/tee_drv.h>

#include "synap_kernel_ca.h"
#include "synap_kernel_log.h"
#include <ca/synap_ta_cmd.h>

struct synap_ca {
    struct tee_context* teec_context;
    u32 teec_session;
};

enum {
/*! The operation was successful. */
	TEEC_SUCCESS			= 0x00000000,
/*! 0x00000001 - 0xFFFEFFFF are implementation defined */
/*! Non-specific cause. */
	TEEC_ERROR_GENERIC		= 0xFFFF0000,
/*! Access privileges are not sufficient. */
	TEEC_ERROR_ACCESS_DENIED	= 0xFFFF0001,
/*! The operation was cancelled. */
	TEEC_ERROR_CANCEL		= 0xFFFF0002,
/*! Concurrent accesses caused conflict. */
	TEEC_ERROR_ACCESS_CONFLICT	= 0xFFFF0003,
/*! Too much data for the requested operation was passed. */
	TEEC_ERROR_EXCESS_DATA		= 0xFFFF0004,
/*! Input data was of invalid format. */
	TEEC_ERROR_BAD_FORMAT		= 0xFFFF0005,
/*! Input parameters were invalid. */
	TEEC_ERROR_BAD_PARAMETERS	= 0xFFFF0006,
/*! Operation is not valid in the current state. */
	TEEC_ERROR_BAD_STATE		= 0xFFFF0007,
/*! The requested data item is not found. */
	TEEC_ERROR_ITEM_NOT_FOUND	= 0xFFFF0008,
/*! The requested operation should exist but is not yet implemented. */
	TEEC_ERROR_NOT_IMPLEMENTED	= 0xFFFF0009,
/*! The requested operation is valid but is not supported in this
 * Implementation. */
	TEEC_ERROR_NOT_SUPPORTED	= 0xFFFF000A,
/*! Expected data was missing. */
	TEEC_ERROR_NO_DATA		= 0xFFFF000B,
/*! System ran out of resources. */
	TEEC_ERROR_OUT_OF_MEMORY	= 0xFFFF000C,
/*! The system is busy working on something else. */
	TEEC_ERROR_BUSY			= 0xFFFF000D,
/*! Communication with a remote party failed. */
	TEEC_ERROR_COMMUNICATION	= 0xFFFF000E,
/*! A security fault was detected. */
	TEEC_ERROR_SECURITY		= 0xFFFF000F,
/*! The supplied buffer is too short for the generated output. */
	TEEC_ERROR_SHORT_BUFFER		= 0xFFFF0010,
/*! 0xFFFF0011 - 0xFFFFFFFF are reserved for future use */
/*! The MAC value supplied is different from the one calculated */
	TEEC_ERROR_MAC_INVALID		= 0xFFFF3071,
};


static const char *synap_ta_log_code_to_str(u32 code) {
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

static int synap_ca_optee_ctx_match(struct tee_ioctl_version_data *ver, const void *data)
{
    return (ver->impl_id == TEE_IMPL_ID_OPTEE);
}


bool synap_ca_create(struct synap_ca **synap_ca) {

    struct tee_context *ctx;

    *synap_ca = (struct synap_ca *) kzalloc(sizeof(struct synap_ca), GFP_KERNEL);

    if (!*synap_ca) {
        KLOGE("cannot allocate memory for synap_ca");
        return false;
    }

    ctx = tee_client_open_context(NULL, synap_ca_optee_ctx_match, NULL, NULL);

    if (IS_ERR(ctx)) {
        kfree(*synap_ca);
        KLOGE("cannot create context for synap_ca");
        return false;
    }

    (*synap_ca)->teec_context = ctx;

    return true;
}

bool synap_ca_load_ta(struct synap_ca *synap_ca, void *ta_data, size_t ta_size) {
    // TAs are automatically loaded by the tee supplicant
    return true;
}

bool synap_ca_create_session(struct synap_ca *synap_ca,
                                    struct sg_table *secure_buf,
                                    struct sg_table *non_secure_buf)
{
    int result;
    struct page *pg = NULL;
    struct tee_param params[1] = {0};
    struct synap_ta_memory_area* areas;
    uuid_t uuid = UUID_INIT(0x1316a183, 0x894d, 0x43fe, 
        0x98, 0x93, 0xbb, 0x94, 0x6a, 0xe1, 0x04, 0x2f);

    struct tee_shm *shm = NULL;
    struct tee_ioctl_open_session_arg sess_arg = {0};

    LOG_ENTER();

    if (secure_buf->nents != 1 || non_secure_buf->nents != 1) {
        KLOGE("only contiguous driver buffers are supported");
        return false;
    }

    shm = tee_shm_alloc_kernel_buf(synap_ca->teec_context, 2 * sizeof(struct synap_ta_memory_area));
    if (!shm) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    areas = (struct synap_ta_memory_area *) shm->kaddr;

    pg = sg_page(non_secure_buf->sgl);
    areas[0].addr = PFN_PHYS(page_to_pfn(pg));
    areas[0].npage = non_secure_buf->sgl->length / PAGE_SIZE;

    pg = sg_page(secure_buf->sgl);
    areas[1].addr = PFN_PHYS(page_to_pfn(pg));
    areas[1].npage = secure_buf->sgl->length / PAGE_SIZE;

    memset(&sess_arg, 0, sizeof(sess_arg));
    memset(&params, 0, sizeof(params));
    
    memcpy(sess_arg.uuid, uuid.b, TEE_IOCTL_UUID_LEN);

    sess_arg.clnt_login = TEE_IOCTL_LOGIN_REE_KERNEL;
    sess_arg.num_params = 1;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;

    params[0].u.memref.size = shm->size;
    params[0].u.memref.shm = shm;

    result = tee_client_open_session(synap_ca->teec_context, &sess_arg, params);

    tee_shm_free(shm);

    if (result < 0 || sess_arg.ret != TEEC_SUCCESS) {
        KLOGE("Error while opening session Error=0x%08x Return=0x%08x (%s)", result, sess_arg.ret, synap_ta_log_code_to_str(sess_arg.ret));
        return false;
    }

    synap_ca->teec_session = sess_arg.session;

    return true;

}

bool synap_ca_activate_npu(struct synap_ca *synap_ca, u8 mode)
{    
    int result;
    struct tee_param params[1] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_ACTIVATE_NPU;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = mode > 0 ? 1 : 0;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}


bool synap_ca_deactivate_npu(struct synap_ca *synap_ca)
{
    int result;
    struct tee_ioctl_invoke_arg arg = {0};

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_DEACTIVATE_NPU;
    arg.num_params = 0;
    arg.session = synap_ca->teec_session;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, NULL);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;

}



bool synap_ca_set_input(struct synap_ca *synap_ca, u32 nid, u32 aid, u32 index)
{

    int result;
    struct tee_param params[2] = {0};
    struct tee_ioctl_invoke_arg arg = {0};

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_SET_INPUT;
    arg.num_params = 2;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = nid;
    params[0].u.value.b = aid;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[1].u.value.a = index;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;

}

bool synap_ca_set_output(struct synap_ca *synap_ca, u32 nid, u32 aid, u32 index)
{

    int result;
    struct tee_param params[2] = {0};
    struct tee_ioctl_invoke_arg arg = {0};

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_SET_OUTPUT;
    arg.num_params = 2;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = nid;
    params[0].u.value.b = aid;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[1].u.value.a = index;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;

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

static bool synap_ca_wrap_user_buffer(struct synap_ca *synap_ca, struct sg_table *ub_sg,
                                             u32 ub_offset, u32 ub_size,
                                             struct tee_shm **shm,
                                             struct synap_ta_user_buffer **ub) {

    struct synap_ta_user_buffer *res;

    size_t shm_size = sizeof(struct synap_ta_user_buffer) + sizeof(struct synap_ta_memory_area) * ub_sg->nents;

    LOG_ENTER();

    *shm = tee_shm_alloc_kernel_buf(synap_ca->teec_context, shm_size);

    if (!*shm) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    res = (*shm)->kaddr;

    res->areas_count = ub_sg->nents;

    res->offset = ub_offset;
    res->size = ub_size;

    synap_ca_copy_to_memory_areas(&res->areas[0], ub_sg);

    *ub = res;

    return true;
}

static bool synap_ca_create_network_resources(
        struct synap_ca *synap_ca, struct sg_table *code,
        struct sg_table *page_table, struct sg_table *pool, struct sg_table *profile_data,
        struct tee_shm **shm, struct synap_ta_network_resources **resources) {

    u32 offset = 0;
    size_t shm_size;

    struct synap_ta_network_resources *res;

    LOG_ENTER();

    shm_size = sizeof(struct synap_ta_network_resources);
    shm_size += sizeof(struct synap_ta_memory_area) * code->nents;
    shm_size += sizeof(struct synap_ta_memory_area) * page_table->nents;

    if (pool != NULL) {
        shm_size += sizeof(struct synap_ta_memory_area) * pool->nents;
    }

    if (profile_data != NULL) {
        shm_size += sizeof(struct synap_ta_memory_area) * profile_data->nents;
    }

    *shm = tee_shm_alloc_kernel_buf(synap_ca->teec_context, shm_size);

    if (!*shm) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    res = (*shm)->kaddr;

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

    struct tee_shm* resources_shm;
    struct tee_shm* header_shm;
    struct tee_shm* payload_shm;

    struct tee_param params[4] = {0};
    struct tee_ioctl_invoke_arg arg = {0};

    int result;

    struct synap_ta_user_buffer *header_ub;
    struct synap_ta_user_buffer *payload_ub;
    struct synap_ta_network_resources *res;

    LOG_ENTER();

    if (!synap_ca_create_network_resources(synap_ca, code, page_table, pool, profile_data,
                                           &resources_shm, &res)) {
        return false;
    }

    if (!synap_ca_wrap_user_buffer(synap_ca, header, header_offset, header_size,
                                   &header_shm, &header_ub)) {
        tee_shm_free(resources_shm);
        return false;
    }

    if (!synap_ca_wrap_user_buffer(synap_ca, payload, payload_offset, payload_size,
                                   &payload_shm, &payload_ub)) {
        tee_shm_free(resources_shm);
        tee_shm_free(header_shm);
        return false;
    }

    arg.func = SYNAP_TA_CMD_CREATE_NETWORK;
    arg.num_params = 4;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    params[0].u.memref.shm = header_shm;
    params[0].u.memref.size = header_shm->size;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    params[1].u.memref.shm = payload_shm;
    params[1].u.memref.size = payload_shm->size;

    params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    params[2].u.memref.shm = resources_shm;
    params[2].u.memref.size =  resources_shm->size;

    params[3].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        // SECURITY error here normally means that the network has been crypted/signed
        // using a different key set
        KLOGE("Error=0x%08x Return=0x%08x (%s) "
              "header.phyAddr=0x%08x size %d area count %d "
              "payload.phyAddr=0x%08x size %d area count %d "
              "resources.phyAddr=0x%08x size %d "
              "code count %d pt count %d pool count %d",
              result, arg.ret, synap_ta_log_code_to_str(arg.ret),
              header_shm->paddr, header_shm->size, header_ub->areas_count,
              payload_shm->paddr, payload_shm->size, payload_ub->areas_count,
              resources_shm->paddr, resources_shm->size,
              code->nents, page_table->nents, pool ? pool->nents: -1);
    }

    tee_shm_free(resources_shm);
    tee_shm_free(header_shm);
    tee_shm_free(payload_shm);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        return false;
    }

    *nid = params[3].u.value.a;
    
    return true;
}

bool synap_ca_destroy_network(struct synap_ca *synap_ca, u32 nid)
{
    int result;
    struct tee_param params[1] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_DESTROY_NETWORK;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = nid;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}

bool synap_ca_start_network(struct synap_ca *synap_ca, u32 nid)
{
    int result;
    struct tee_param params[1] = {0};
    struct tee_ioctl_invoke_arg arg = {0};

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_START_NETWORK;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = nid;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS ) {
        // BAD_FORMAT here normally means that the network has been compiled for an incompatible HW
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}

static bool synap_ca_create_areas(struct synap_ca *synap_ca,
                                  struct sg_table *buf, u32 offset, u32 size,
                                  size_t *count,
                                  struct tee_shm** shm) {

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

    *shm = tee_shm_alloc_kernel_buf(synap_ca->teec_context, 
                                    areas_count * sizeof(struct synap_ta_memory_area));

    if (!*shm) {
        KLOGE("cannot allocate shared memory");
        return false;
    }

    areas = (struct synap_ta_memory_area *) (*shm)->kaddr;

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
    int result;
    struct tee_param params[3] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_CREATE_IO_BUFFER_FROM_MEM_ID;
    arg.num_params = 3;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = mem_id;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[1].u.value.a = offset;
    params[1].u.value.b = size;

    params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    *bid = params[2].u.value.a;

    if (result < 0 || arg.ret != TEEC_SUCCESS ) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}


bool synap_ca_create_io_buffer_from_sg(struct synap_ca *synap_ca, struct sg_table *buf,
                                              u32 offset, u32 size, bool secure, u32 *bid,
                                              u32 *mem_id) {
    int result;
    struct tee_param params[3] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };
    struct tee_shm *shm = NULL;

    size_t areas_count;

    LOG_ENTER();

    if (!synap_ca_create_areas(synap_ca, buf, offset, size, &areas_count, &shm)) {
        return false;
    }

    arg.func = SYNAP_TA_CMD_CREATE_IO_BUFFER_FROM_SG;
    arg.num_params = 3;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT;
    params[0].u.memref.shm = shm;
    params[0].u.memref.size = shm->size;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[1].u.value.a = secure ? 1 : 0;
    params[1].u.value.b = areas_count;

    params[2].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    tee_shm_free(shm);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s), shm.phyAddr=0x%08x", result, arg.ret, synap_ta_log_code_to_str(arg.ret), shm->paddr);
        return false;
    }

    *bid = params[2].u.value.a;
    *mem_id = params[2].u.value.b;

    return true;
}

bool synap_ca_attach_io_buffer(struct synap_ca *synap_ca, u32 nid,
                                      u32 bid, u32 *aid) {
    int result;
    struct tee_param params[2] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_ATTACH_IO_BUFFER;
    arg.num_params = 2;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = bid;
    params[0].u.value.b = nid;

    params[1].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    *aid = params[1].u.value.a;

    return true;
}



bool synap_ca_detach_io_buffer(struct synap_ca *synap_ca, u32 nid, u32 aid)
{

    int result;
    struct tee_param params[1] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_DETACH_IO_BUFFER;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = nid;
    params[0].u.value.b = aid;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}


bool synap_ca_destroy_io_buffer(struct synap_ca *synap_ca, u32 bid)
{

    int result;
    struct tee_param params[1] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_DESTROY_IO_BUFFER;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT;
    params[0].u.value.a = bid;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}


bool synap_ca_read_interrupt_register(struct synap_ca *synap_ca, volatile u32 *reg_value)
{

    int result;
    struct tee_param params[1] = { 0 };
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_READ_INTERRUPT_REGISTER;
    arg.num_params = 1;
    arg.session = synap_ca->teec_session;

    params[0].attr = TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, params);

    // TEEC_ERROR_BAD_STATE may be raised if the execution was aborted
    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    *reg_value = params[0].u.value.a;

    return true;
}

bool synap_ca_dump_state(struct synap_ca *synap_ca) {

    int result;
    struct tee_ioctl_invoke_arg arg = { 0 };

    LOG_ENTER();

    arg.func = SYNAP_TA_CMD_DUMP_STATE;
    arg.num_params = 0;
    arg.session = synap_ca->teec_session;

    result = tee_client_invoke_func(synap_ca->teec_context, &arg, NULL);

    if (result < 0 || arg.ret != TEEC_SUCCESS) {
        KLOGE("Error=0x%08x Return=0x%08x (%s)", result, arg.ret, synap_ta_log_code_to_str(arg.ret));
        return false;
    }

    return true;
}


bool synap_ca_destroy_session(struct synap_ca *synap_ca)
{
    LOG_ENTER();

    tee_client_close_session(synap_ca->teec_context, synap_ca->teec_session);

    return true;
}


void synap_ca_destroy(struct synap_ca* synap_ca) {

    tee_client_close_context(synap_ca->teec_context);

    kfree(synap_ca);
}