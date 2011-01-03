
#ifndef _VCD_DDL_METADATA_H_
#define _VCD_DDL_METADATA_H_

#define DDL_MAX_DEC_METADATATYPE  (8)
#define DDL_MAX_ENC_METADATATYPE  (3)

#define DDL_METADATA_EXTRAPAD_SIZE (256)
#define DDL_METADATA_HDR_SIZE (20)

#define DDL_METADATA_EXTRADATANONE_SIZE (24)

#define DDL_METADATA_ALIGNSIZE(x) ((x) = (((x) + 0x7) & ~0x7))

#define DDL_METADATA_MANDATORY (VCD_METADATA_DATANONE | \
				VCD_METADATA_QCOMFILLER)

#define DDL_METADATA_VC1_PAYLOAD_SIZE (38*4)

#define DDL_METADATA_SEI_PAYLOAD_SIZE (100)
#define DDL_METADATA_SEI_MAX (5)

#define DDL_METADATA_VUI_PAYLOAD_SIZE (256)

#define DDL_METADATA_PASSTHROUGH_PAYLOAD_SIZE  (68)

#define DDL_METADATA_CLIENT_INPUTBUFSIZE  (256)
#define DDL_METADATA_TOTAL_INPUTBUFSIZE \
	(DDL_METADATA_CLIENT_INPUTBUFSIZE * VCD_MAX_NO_CLIENT)

#define DDL_METADATA_CLIENT_INPUTBUF(p_main_buffer, p_client_buffer, \
		n_channel_id) \
{ \
  (p_client_buffer)->p_align_physical_addr = (u32 *)\
	((u8 *)(p_main_buffer)->p_align_physical_addr + \
	(DDL_METADATA_CLIENT_INPUTBUFSIZE * (n_channel_id)) \
	); \
  (p_client_buffer)->p_align_virtual_addr = (u32 *)\
	((u8 *)(p_main_buffer)->p_align_virtual_addr + \
	(DDL_METADATA_CLIENT_INPUTBUFSIZE * (n_channel_id)) \
	); \
  (p_client_buffer)->p_virtual_base_addr = 0; \
}

#define DDL_METADATA_HDR_VERSION_INDEX 0
#define DDL_METADATA_HDR_PORT_INDEX    1
#define DDL_METADATA_HDR_TYPE_INDEX    2


void ddl_set_default_meta_data_hdr(struct ddl_client_context_type *p_ddl);
u32 ddl_get_metadata_params(struct ddl_client_context_type	*p_ddl,
	struct vcd_property_hdr_type *p_property_hdr, void *p_property_value);
u32 ddl_set_metadata_params(struct ddl_client_context_type *p_ddl,
			    struct vcd_property_hdr_type *p_property_hdr,
			    void *p_property_value);
void ddl_set_default_metadata_flag(struct ddl_client_context_type *p_ddl);
void ddl_set_default_decoder_metadata_buffer_size
    (struct ddl_decoder_data_type *p_decoder,
	struct vcd_property_frame_size_type *p_frame_size,
	struct vcd_buffer_requirement_type *p_output_buf_req);
void ddl_set_default_encoder_metadata_buffer_size(struct ddl_encoder_data_type
						  *p_encoder);
void ddl_metadata_enable(struct ddl_client_context_type *p_ddl);
u32 ddl_encode_set_metadata_output_buf(struct ddl_client_context_type *p_ddl);
void ddl_decode_set_metadata_output(struct ddl_decoder_data_type *p_decoder);
void ddl_process_encoder_metadata(struct ddl_client_context_type *p_ddl);
void ddl_process_decoder_metadata(struct ddl_client_context_type *p_ddl);
#endif
