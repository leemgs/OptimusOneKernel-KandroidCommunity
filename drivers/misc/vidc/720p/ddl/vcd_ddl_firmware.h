
#ifndef _VCD_DDL_FIRMWARE_H_
#define _VCD_DDL_FIRMWARE_H_
#include "vcd_property.h"

#define VCD_FW_BIG_ENDIAN     0x0
#define VCD_FW_LITTLE_ENDIAN  0x1

struct vcd_fw_details_type {
	enum vcd_codec_type e_codec;
	u32 *p_fw_buffer_addr;
	u32 n_fw_size;
};

#define VCD_FW_PROP_BASE         0x0

#define VCD_FW_ENDIAN       (VCD_FW_PROP_BASE + 0x1)
#define VCD_FW_BOOTCODE     (VCD_FW_PROP_BASE + 0x2)
#define VCD_FW_DECODE     (VCD_FW_PROP_BASE + 0x3)
#define VCD_FW_ENCODE     (VCD_FW_PROP_BASE + 0x4)

extern unsigned char *vidc_command_control_fw;
extern u32 vidc_command_control_fw_size;
extern unsigned char *vidc_mpg4_dec_fw;
extern u32 vidc_mpg4_dec_fw_size;
extern unsigned char *vidc_h263_dec_fw;
extern u32 vidc_h263_dec_fw_size;
extern unsigned char *vidc_h264_dec_fw;
extern u32 vidc_h264_dec_fw_size;
extern unsigned char *vidc_mpg4_enc_fw;
extern u32 vidc_mpg4_enc_fw_size;
extern unsigned char *vidc_h264_enc_fw;
extern u32 vidc_h264_enc_fw_size;
extern unsigned char *vidc_vc1_dec_fw;
extern u32 vidc_vc1_dec_fw_size;

u32 vcd_fw_init(void);
u32 vcd_get_fw_property(u32 prop_id, void *prop_details);
u32 vcd_fw_transact(u32 b_add, u32 b_decoding, enum vcd_codec_type e_codec);
void vcd_fw_release(void);

#endif
