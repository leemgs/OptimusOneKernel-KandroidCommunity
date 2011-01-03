
#ifndef _VCD_API_H_
#define _VCD_API_H_
#include "vcd_property.h"
#include "vcd_status.h"

#define VCD_FRAME_FLAG_EOS 0x00000001
#define VCD_FRAME_FLAG_DECODEONLY   0x00000004
#define VCD_FRAME_FLAG_ENDOFFRAME 0x00000010
#define VCD_FRAME_FLAG_SYNCFRAME 0x00000020
#define VCD_FRAME_FLAG_EXTRADATA 0x00000040
#define VCD_FRAME_FLAG_CODECCONFIG  0x00000080

#define VCD_FLUSH_INPUT   0x0001
#define VCD_FLUSH_OUTPUT  0x0002
#define VCD_FLUSH_ALL     0x0003

#define VCD_FRAMETAG_INVALID  0xffffffff

struct vcd_handle_container_type {
	void *handle;
};
struct vcd_flush_cmd_type {
	u32 n_mode;
};

enum vcd_frame_type {
	VCD_FRAME_YUV = 1,
	VCD_FRAME_I,
	VCD_FRAME_P,
	VCD_FRAME_B,
	VCD_FRAME_NOTCODED,
	VCD_FRAME_32BIT = 0x7fffffff
};

enum vcd_power_state_type {
	VCD_PWR_STATE_ON = 1,
	VCD_PWR_STATE_SLEEP,
};

struct vcd_frame_data_type {
	u8 *p_virtual;
	u8 *p_physical;
	u32 n_alloc_len;
	u32 n_data_len;
	u32 n_offset;
	s64 time_stamp;
	u32 n_flags;
	u32 n_frm_clnt_data;
	struct vcd_property_dec_output_buffer_type dec_op_prop;
	u32 b_interlaced;
	enum vcd_frame_type e_frame_type;
	u32 n_ip_frm_tag;
	u32 n_intrlcd_ip_frm_tag;
};

struct vcd_sequence_hdr_type {
	u8 *p_sequence_header;
	u32 n_sequence_header_len;

};

enum vcd_buffer_type {
	VCD_BUFFER_INPUT = 0x1,
	VCD_BUFFER_OUTPUT = 0x2,
	VCD_BUFFER_INVALID = 0x3,
	VCD_BUFFER_32BIT = 0x7FFFFFFF
};

struct vcd_buffer_requirement_type {
	u32 n_min_count;
	u32 n_actual_count;
	u32 n_max_count;
	u32 n_size;
	u32 n_align;
	u32 n_buf_pool_id;
};

struct vcd_init_config_type {
	void *p_device_name;
	void *(*pf_map_dev_base_addr) (void *p_device_name);
	void (*pf_un_map_dev_base_addr) (void);
	void (*pf_interrupt_clr) (void);
	void (*pf_register_isr) (void *p_device_name);
	void (*pf_deregister_isr) (void);
	u32  (*pf_timer_create) (void (*pf_timer_handler)(void *),
		void *p_user_data, void **pp_timer_handle);
	void (*pf_timer_release) (void *p_timer_handle);
	void (*pf_timer_start) (void *p_timer_handle, u32 n_time_out);
	void (*pf_timer_stop) (void *p_timer_handle);
};

u32 vcd_init(struct vcd_init_config_type *p_config, s32 *p_driver_handle);
u32 vcd_term(s32 driver_handle);
u32 vcd_open(s32 driver_handle, u32 b_decoding,
	void (*callback) (u32 event, u32 status, void *p_info, u32 n_size,
	void *handle, void *const p_client_data), void *p_client_data);
u32 vcd_close(void *handle);
u32 vcd_encode_start(void *handle);
u32 vcd_encode_frame(void *handle, struct vcd_frame_data_type *p_input_frame);
u32 vcd_decode_start(void *handle, struct vcd_sequence_hdr_type *p_seq_hdr);
u32 vcd_decode_frame(void *handle, struct vcd_frame_data_type *p_input_frame);
u32 vcd_pause(void *handle);
u32 vcd_resume(void *handle);
u32 vcd_flush(void *handle, u32 n_mode);
u32 vcd_stop(void *handle);
u32 vcd_set_property(void *handle, struct vcd_property_hdr_type *p_prop_hdr,
					void *p_prop_val);
u32 vcd_get_property(void *handle, struct vcd_property_hdr_type *p_prop_hdr,
					 void *p_prop_val);
u32 vcd_set_buffer_requirements(void *handle, enum vcd_buffer_type e_buffer,
		struct vcd_buffer_requirement_type *p_buffer_req);
u32 vcd_get_buffer_requirements(void *handle, enum vcd_buffer_type e_buffer,
		struct vcd_buffer_requirement_type *p_buffer_req);
u32 vcd_set_buffer(void *handle, enum vcd_buffer_type e_buffer,
		u8 *p_buffer, u32 n_buf_size);
u32 vcd_allocate_buffer(void *handle, enum vcd_buffer_type e_buffer,
		u32 n_buf_size, u8 **pp_vir_buf_addr, u8 **pp_phy_buf_addr);

u32 vcd_free_buffer(void *handle, enum vcd_buffer_type e_buffer, u8 *p_buffer);
u32 vcd_fill_output_buffer(void *handle, struct vcd_frame_data_type *p_buffer);
u32 vcd_set_device_power(s32 driver_handle,
		enum vcd_power_state_type e_pwr_state);
void vcd_read_and_clear_interrupt(void);
void vcd_response_handler(void);
u8 vcd_get_num_of_clients(void);

#endif
