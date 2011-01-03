
#ifndef _VCD_DDL_API_H_
#define _VCD_DDL_API_H_
#include "vcd_ddl_internal_property.h"

struct ddl_init_config_type {
	u8 *p_core_virtual_base_addr;
	void (*pf_interrupt_clr) (void);
	void (*ddl_callback) (u32 event, u32 status, void *payload, u32 size,
		u32 *p_ddl_handle, void *const p_client_data);
};

struct ddl_frame_data_type_tag {
	struct vcd_frame_data_type vcd_frm;
	u32 b_frm_trans_end;
	u32 n_frm_delta;
};

u32 ddl_device_init(struct ddl_init_config_type *p_ddl_init_config,
					void *p_client_data);
u32 ddl_device_release(void *p_client_data);
u32 ddl_open(u32 **p_ddl_handle, u32 b_decoding);
u32 ddl_close(u32 **p_ddl_handle);
u32 ddl_encode_start(u32 *ddl_handle, void *p_client_data);
u32 ddl_encode_frame(u32 *ddl_handle,
	struct ddl_frame_data_type_tag *p_input_frame,
	struct ddl_frame_data_type_tag *p_output_bit, void *p_client_data);
u32 ddl_encode_end(u32 *ddl_handle, void *p_client_data);
u32 ddl_decode_start(u32 *ddl_handle, struct vcd_sequence_hdr_type *p_header,
					void *p_client_data);
u32 ddl_decode_frame(u32 *ddl_handle,
	struct ddl_frame_data_type_tag *p_input_bits, void *p_client_data);
u32 ddl_decode_end(u32 *ddl_handle, void *p_client_data);
u32 ddl_set_property(u32 *ddl_handle,
	struct vcd_property_hdr_type *p_property_hdr, void *p_property_value);
u32 ddl_get_property(u32 *ddl_handle,
	struct vcd_property_hdr_type *p_property_hdr, void *p_property_value);
void ddl_read_and_clear_interrupt(void);
u32 ddl_process_core_response(void);
u32 ddl_reset_hw(u32 n_mode);
#endif
