// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 2021 Synaptics Incorporated */

#include <linux/types.h>
#include <linux/dma-buf.h>

struct synap_ca;

bool synap_ca_create(struct synap_ca** synap_ca);

bool synap_ca_load_ta(struct synap_ca* synap_ca, void *ta_data, size_t ta_size);

bool synap_ca_create_session(struct synap_ca* synap_ca,
                                    struct sg_table *secure_buf,
                                    struct sg_table *non_secure_buf);

bool synap_ca_set_input(struct synap_ca* synap_ca, u32 nid, u32 bid, u32 index);

bool synap_ca_set_output(struct synap_ca* synap_ca, u32 nid, u32 bid, u32 index);

bool synap_ca_activate_npu(struct synap_ca* synap_ca, u8 mode);

bool synap_ca_deactivate_npu(struct synap_ca* synap_ca);

bool synap_ca_create_network(struct synap_ca* synap_ca,
                                    struct sg_table *header,
                                    u32 header_offset, u32 header_size,
                                    struct sg_table *payload,
                                    u32 payload_offset, u32 payload_size,
                                    struct sg_table *code, struct sg_table *page_table,
                                    struct sg_table *pool, struct sg_table *profile_data,
                                    u32 *nid);

bool synap_ca_destroy_network(struct synap_ca* synap_ca, u32 nid);

bool synap_ca_start_network(struct synap_ca* synap_ca, u32 nid);

bool synap_ca_read_interrupt_register(struct synap_ca* synap_ca, volatile u32 *reg_value);

bool synap_ca_dump_state(struct synap_ca* synap_ca);

bool synap_ca_create_io_buffer_from_sg(struct synap_ca* synap_ca, struct sg_table *buf,
                                              u32 offset, u32 size, bool secure, u32 *bid,
                                              u32 *mem_id);

bool synap_ca_create_io_buffer_from_mem_id(struct synap_ca* synap_ca, u32 mem_id,
                                                  u32 offset, u32 size, u32 *bid);

bool synap_ca_destroy_io_buffer(struct synap_ca* synap_ca, u32 bid);

bool synap_ca_attach_io_buffer(struct synap_ca* synap_ca, u32 nid, u32 bid, u32 *aid);

bool synap_ca_detach_io_buffer(struct synap_ca* synap_ca, u32 nid, u32 aid);

bool synap_ca_destroy_session(struct synap_ca* synap_ca);

void synap_ca_destroy(struct synap_ca* context);