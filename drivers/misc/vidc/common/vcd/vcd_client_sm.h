
#ifndef _VCD_CLIENT_SM_H_
#define _VCD_CLIENT_SM_H_
#include "vcd_api.h"
#include "vcd_ddl_api.h"

struct vcd_clnt_state_table_type_t;
struct vcd_clnt_state_ctxt_type_t;
struct vcd_clnt_ctxt_type_t;

enum vcd_clnt_state_enum_type {
	VCD_CLIENT_STATE_NULL = 0,
	VCD_CLIENT_STATE_OPEN,
	VCD_CLIENT_STATE_STARTING,
	VCD_CLIENT_STATE_RUN,
	VCD_CLIENT_STATE_FLUSHING,
	VCD_CLIENT_STATE_PAUSING,
	VCD_CLIENT_STATE_PAUSED,
	VCD_CLIENT_STATE_STOPPING,
	VCD_CLIENT_STATE_EOS,
	VCD_CLIENT_STATE_INVALID,
	VCD_CLIENT_STATE_MAX,
	VCD_CLIENT_STATE_32BIT = 0x7FFFFFFF
};

#define   CLIENT_STATE_EVENT_NUMBER(ppf) \
    ((u32 *) (&(((struct vcd_clnt_state_table_type_t*)0)->ev_hdlr.ppf)) -  \
    (u32 *) (&(((struct vcd_clnt_state_table_type_t*)0)->ev_hdlr.pf_close)) \
	+ 1)

struct vcd_clnt_state_table_type_t {
	struct {
		u32(*pf_close) (struct vcd_clnt_ctxt_type_t *p_cctxt);
		u32(*pf_encode_start) (struct vcd_clnt_ctxt_type_t *p_cctxt);
		u32(*pf_encode_frame) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_frame_data_type *p_input_frame);
		u32(*pf_decode_start) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_sequence_hdr_type *p_seq_hdr);
		u32(*pf_decode_frame) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_frame_data_type *p_input_frame);
		u32(*pf_pause) (struct vcd_clnt_ctxt_type_t *p_cctxt);
		u32(*pf_resume) (struct vcd_clnt_ctxt_type_t *p_cctxt);
		u32(*pf_flush) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				u32 n_mode);
		u32(*pf_stop) (struct vcd_clnt_ctxt_type_t *p_cctxt);
		u32(*pf_set_property) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_property_hdr_type *p_prop_hdr,
				void *p_prop);
		u32(*pf_get_property) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_property_hdr_type *p_prop_hdr,
				void *p_prop);
		u32(*pf_set_buffer_requirements) (struct vcd_clnt_ctxt_type_t *
						  p_cctxt,
						  enum vcd_buffer_type e_buffer,
						  struct
						  vcd_buffer_requirement_type *
						  p_buffer_req);
		u32(*pf_get_buffer_requirements) (struct vcd_clnt_ctxt_type_t *
						  p_cctxt,
						  enum vcd_buffer_type e_buffer,
						  struct
						  vcd_buffer_requirement_type *
						  p_buffer_req);
		u32(*pf_set_buffer) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				enum vcd_buffer_type e_buffer, u8 *p_buffer,
				u32 n_buf_size);
		u32(*pf_allocate_buffer) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				enum vcd_buffer_type e_buffer, u32 n_buf_size,
				u8 **pp_vir_buf_addr, u8 **pp_phy_buf_addr);
		u32(*pf_free_buffer) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				enum vcd_buffer_type e_buffer, u8 *p_buffer);
		u32(*pf_fill_output_buffer) (
				struct vcd_clnt_ctxt_type_t *p_cctxt,
				struct vcd_frame_data_type *p_buffer);
		void (*pf_clnt_cb) (struct vcd_clnt_ctxt_type_t *p_cctxt,
				u32 event, u32 status, void *p_payload,
				u32 n_size, u32 *ddl_handle,
				void *const p_client_data);
	} ev_hdlr;

	void (*pf_entry) (struct vcd_clnt_ctxt_type_t *p_cctxt,
			s32 n_state_event_type);
	void (*pf_exit) (struct vcd_clnt_ctxt_type_t *p_cctxt,
			s32 n_state_event_type);
};

struct vcd_clnt_state_ctxt_type_t {
	const struct vcd_clnt_state_table_type_t *p_state_table;
	enum vcd_clnt_state_enum_type e_state;
};

extern void vcd_do_client_state_transition
    (struct vcd_clnt_ctxt_type_t *p_cctxt,
     enum vcd_clnt_state_enum_type e_to_state, u32 n_ev_code);

extern const struct vcd_clnt_state_table_type_t *vcd_get_client_state_table(
		enum vcd_clnt_state_enum_type e_state);

#endif
