


#ifndef REGMACH64_H
#define REGMACH64_H




#define CRTC_H_TOTAL_DISP	0x0000	
#define CRTC2_H_TOTAL_DISP	0x0000	
#define CRTC_H_SYNC_STRT_WID	0x0004	
#define CRTC2_H_SYNC_STRT_WID	0x0004	
#define CRTC_H_SYNC_STRT	0x0004
#define CRTC2_H_SYNC_STRT	0x0004
#define CRTC_H_SYNC_DLY		0x0005
#define CRTC2_H_SYNC_DLY	0x0005
#define CRTC_H_SYNC_WID		0x0006
#define CRTC2_H_SYNC_WID	0x0006
#define CRTC_V_TOTAL_DISP	0x0008	
#define CRTC2_V_TOTAL_DISP	0x0008	
#define CRTC_V_TOTAL		0x0008
#define CRTC2_V_TOTAL		0x0008
#define CRTC_V_DISP		0x000A
#define CRTC2_V_DISP		0x000A
#define CRTC_V_SYNC_STRT_WID	0x000C	
#define CRTC2_V_SYNC_STRT_WID	0x000C	
#define CRTC_V_SYNC_STRT	0x000C
#define CRTC2_V_SYNC_STRT	0x000C
#define CRTC_V_SYNC_WID		0x000E
#define CRTC2_V_SYNC_WID	0x000E
#define CRTC_VLINE_CRNT_VLINE	0x0010	
#define CRTC2_VLINE_CRNT_VLINE	0x0010	
#define CRTC_OFF_PITCH		0x0014	
#define CRTC_OFFSET		0x0014
#define CRTC_PITCH		0x0016
#define CRTC_INT_CNTL		0x0018	
#define CRTC_GEN_CNTL		0x001C	
#define CRTC_PIX_WIDTH		0x001D
#define CRTC_FIFO		0x001E
#define CRTC_EXT_DISP		0x001F


#define DSP_CONFIG		0x0020	
#define PM_DSP_CONFIG		0x0020	
#define DSP_ON_OFF		0x0024	
#define PM_DSP_ON_OFF		0x0024	
#define TIMER_CONFIG		0x0028	
#define MEM_BUF_CNTL		0x002C	
#define MEM_ADDR_CONFIG		0x0034	


#define CRT_TRAP		0x0038	

#define I2C_CNTL_0		0x003C	

#define DSTN_CONTROL_LG		0x003C	


#define OVR_CLR			0x0040	
#define OVR2_CLR		0x0040	
#define OVR_WID_LEFT_RIGHT	0x0044	
#define OVR2_WID_LEFT_RIGHT	0x0044	
#define OVR_WID_TOP_BOTTOM	0x0048	
#define OVR2_WID_TOP_BOTTOM	0x0048	


#define VGA_DSP_CONFIG		0x004C	
#define PM_VGA_DSP_CONFIG	0x004C	
#define VGA_DSP_ON_OFF		0x0050	
#define PM_VGA_DSP_ON_OFF	0x0050	
#define DSP2_CONFIG		0x0054	
#define PM_DSP2_CONFIG		0x0054	
#define DSP2_ON_OFF		0x0058	
#define PM_DSP2_ON_OFF		0x0058	


#define CRTC2_OFF_PITCH		0x005C	


#define CUR_CLR0		0x0060	
#define CUR2_CLR0		0x0060	
#define CUR_CLR1		0x0064	
#define CUR2_CLR1		0x0064	
#define CUR_OFFSET		0x0068	
#define CUR2_OFFSET		0x0068	
#define CUR_HORZ_VERT_POSN	0x006C	
#define CUR2_HORZ_VERT_POSN	0x006C	
#define CUR_HORZ_VERT_OFF	0x0070	
#define CUR2_HORZ_VERT_OFF	0x0070	

#define CNFG_PANEL_LG		0x0074	


#define GP_IO			0x0078	


#define HW_DEBUG		0x007C	


#define SCRATCH_REG0		0x0080	
#define SCRATCH_REG1		0x0084	
#define SCRATCH_REG2		0x0088	
#define SCRATCH_REG3		0x008C	


#define CLOCK_CNTL			0x0090	

#define CLOCK_SEL			0x0f
#define CLOCK_SEL_INTERNAL		0x03
#define CLOCK_SEL_EXTERNAL		0x0c
#define CLOCK_DIV			0x30
#define CLOCK_DIV1			0x00
#define CLOCK_DIV2			0x10
#define CLOCK_DIV4			0x20
#define CLOCK_STROBE			0x40


#define CLOCK_BIT			0x04	
#define CLOCK_PULSE			0x08	

#define CLOCK_DATA			0x80


#define CLOCK_CNTL_ADDR			CLOCK_CNTL + 1
#define PLL_WR_EN			0x02
#define PLL_ADDR			0xfc
#define CLOCK_CNTL_DATA			CLOCK_CNTL + 2
#define PLL_DATA			0xff


#define CLOCK_SEL_CNTL		0x0090	


#define CNFG_STAT1		0x0094	
#define CNFG_STAT2		0x0098	


#define BUS_CNTL		0x00A0	

#define LCD_INDEX		0x00A4	
#define LCD_DATA		0x00A8	

#define HFB_PITCH_ADDR_LG	0x00A8	


#define EXT_MEM_CNTL		0x00AC	
#define MEM_CNTL		0x00B0	
#define MEM_VGA_WP_SEL		0x00B4	
#define MEM_VGA_RP_SEL		0x00B8	

#define I2C_CNTL_1		0x00BC	

#define LT_GIO_LG		0x00BC	


#define DAC_REGS		0x00C0	
#define DAC_W_INDEX		0x00C0	
#define DAC_DATA		0x00C1	
#define DAC_MASK		0x00C2	
#define DAC_R_INDEX		0x00C3	
#define DAC_CNTL		0x00C4	

#define EXT_DAC_REGS		0x00C8	

#define HORZ_STRETCHING_LG	0x00C8	
#define VERT_STRETCHING_LG	0x00CC	


#define GEN_TEST_CNTL		0x00D0	


#define CUSTOM_MACRO_CNTL	0x00D4	

#define LCD_GEN_CNTL_LG		0x00D4	
#define POWER_MANAGEMENT_LG	0x00D8	


#define CNFG_CNTL		0x00DC	
#define CNFG_CHIP_ID		0x00E0	
#define CNFG_STAT0		0x00E4	


#define CRC_SIG			0x00E8	
#define CRC2_SIG		0x00E8	





#define DST_OFF_PITCH		0x0100	
#define DST_X			0x0104	
#define DST_Y			0x0108	
#define DST_Y_X			0x010C	
#define DST_WIDTH		0x0110	
#define DST_HEIGHT		0x0114	
#define DST_HEIGHT_WIDTH	0x0118	
#define DST_X_WIDTH		0x011C	
#define DST_BRES_LNTH		0x0120	
#define DST_BRES_ERR		0x0124	
#define DST_BRES_INC		0x0128	
#define DST_BRES_DEC		0x012C	
#define DST_CNTL		0x0130	
#define DST_Y_X__ALIAS__	0x0134	
#define TRAIL_BRES_ERR		0x0138	
#define TRAIL_BRES_INC		0x013C	
#define TRAIL_BRES_DEC		0x0140	
#define LEAD_BRES_LNTH		0x0144	
#define Z_OFF_PITCH		0x0148	
#define Z_CNTL			0x014C	
#define ALPHA_TST_CNTL		0x0150	
#define SECONDARY_STW_EXP	0x0158	
#define SECONDARY_S_X_INC	0x015C	
#define SECONDARY_S_Y_INC	0x0160	
#define SECONDARY_S_START	0x0164	
#define SECONDARY_W_X_INC	0x0168	
#define SECONDARY_W_Y_INC	0x016C	
#define SECONDARY_W_START	0x0170	
#define SECONDARY_T_X_INC	0x0174	
#define SECONDARY_T_Y_INC	0x0178	
#define SECONDARY_T_START	0x017C	


#define SRC_OFF_PITCH		0x0180	
#define SRC_X			0x0184	
#define SRC_Y			0x0188	
#define SRC_Y_X			0x018C	
#define SRC_WIDTH1		0x0190	
#define SRC_HEIGHT1		0x0194	
#define SRC_HEIGHT1_WIDTH1	0x0198	
#define SRC_X_START		0x019C	
#define SRC_Y_START		0x01A0	
#define SRC_Y_X_START		0x01A4	
#define SRC_WIDTH2		0x01A8	
#define SRC_HEIGHT2		0x01AC	
#define SRC_HEIGHT2_WIDTH2	0x01B0	
#define SRC_CNTL		0x01B4	

#define SCALE_OFF		0x01C0	
#define SECONDARY_SCALE_OFF	0x01C4	

#define TEX_0_OFF		0x01C0	
#define TEX_1_OFF		0x01C4	
#define TEX_2_OFF		0x01C8	
#define TEX_3_OFF		0x01CC	
#define TEX_4_OFF		0x01D0	
#define TEX_5_OFF		0x01D4	
#define TEX_6_OFF		0x01D8	
#define TEX_7_OFF		0x01DC	

#define SCALE_WIDTH		0x01DC	
#define SCALE_HEIGHT		0x01E0	

#define TEX_8_OFF		0x01E0	
#define TEX_9_OFF		0x01E4	
#define TEX_10_OFF		0x01E8	
#define S_Y_INC			0x01EC	

#define SCALE_PITCH		0x01EC	
#define SCALE_X_INC		0x01F0	

#define RED_X_INC		0x01F0	
#define GREEN_X_INC		0x01F4	

#define SCALE_Y_INC		0x01F4	
#define SCALE_VACC		0x01F8	
#define SCALE_3D_CNTL		0x01FC	


#define HOST_DATA0		0x0200	
#define HOST_DATA1		0x0204	
#define HOST_DATA2		0x0208	
#define HOST_DATA3		0x020C	
#define HOST_DATA4		0x0210	
#define HOST_DATA5		0x0214	
#define HOST_DATA6		0x0218	
#define HOST_DATA7		0x021C	
#define HOST_DATA8		0x0220	
#define HOST_DATA9		0x0224	
#define HOST_DATAA		0x0228	
#define HOST_DATAB		0x022C	
#define HOST_DATAC		0x0230	
#define HOST_DATAD		0x0234	
#define HOST_DATAE		0x0238	
#define HOST_DATAF		0x023C	
#define HOST_CNTL		0x0240	


#define BM_HOSTDATA		0x0244	
#define BM_ADDR			0x0248	
#define BM_DATA			0x0248	
#define BM_GUI_TABLE_CMD	0x024C	


#define PAT_REG0		0x0280	
#define PAT_REG1		0x0284	
#define PAT_CNTL		0x0288	


#define SC_LEFT			0x02A0	
#define SC_RIGHT		0x02A4	
#define SC_LEFT_RIGHT		0x02A8	
#define SC_TOP			0x02AC	
#define SC_BOTTOM		0x02B0	
#define SC_TOP_BOTTOM		0x02B4	


#define USR1_DST_OFF_PITCH	0x02B8	
#define USR2_DST_OFF_PITCH	0x02BC	
#define DP_BKGD_CLR		0x02C0	
#define DP_FOG_CLR		0x02C4	
#define DP_FRGD_CLR		0x02C4	
#define DP_WRITE_MASK		0x02C8	
#define DP_CHAIN_MASK		0x02CC	
#define DP_PIX_WIDTH		0x02D0	
#define DP_MIX			0x02D4	
#define DP_SRC			0x02D8	
#define DP_FRGD_CLR_MIX		0x02DC	
#define DP_FRGD_BKGD_CLR	0x02E0	


#define DST_X_Y			0x02E8	
#define DST_WIDTH_HEIGHT	0x02EC	


#define USR_DST_PICTH		0x02F0	
#define DP_SET_GUI_ENGINE2	0x02F8	
#define DP_SET_GUI_ENGINE	0x02FC	


#define CLR_CMP_CLR		0x0300	
#define CLR_CMP_MASK		0x0304	
#define CLR_CMP_CNTL		0x0308	


#define FIFO_STAT		0x0310	

#define CONTEXT_MASK		0x0320	
#define CONTEXT_LOAD_CNTL	0x032C	


#define GUI_TRAJ_CNTL		0x0330	


#define GUI_STAT		0x0338	

#define TEX_PALETTE_INDEX	0x0340	
#define STW_EXP			0x0344	
#define LOG_MAX_INC		0x0348	
#define S_X_INC			0x034C	
#define S_Y_INC__ALIAS__	0x0350	

#define SCALE_PITCH__ALIAS__	0x0350	

#define S_START			0x0354	
#define W_X_INC			0x0358	
#define W_Y_INC			0x035C	
#define W_START			0x0360	
#define T_X_INC			0x0364	
#define T_Y_INC			0x0368	

#define SECONDARY_SCALE_PITCH	0x0368	

#define T_START			0x036C	
#define TEX_SIZE_PITCH		0x0370	
#define TEX_CNTL		0x0374	
#define SECONDARY_TEX_OFFSET	0x0378	
#define TEX_PALETTE		0x037C	

#define SCALE_PITCH_BOTH	0x0380	
#define SECONDARY_SCALE_OFF_ACC	0x0384	
#define SCALE_OFF_ACC		0x0388	
#define SCALE_DST_Y_X		0x038C	


#define COMPOSITE_SHADOW_ID	0x0398	

#define SECONDARY_SCALE_X_INC	0x039C	

#define SPECULAR_RED_X_INC	0x039C	
#define SPECULAR_RED_Y_INC	0x03A0	
#define SPECULAR_RED_START	0x03A4	

#define SECONDARY_SCALE_HACC	0x03A4	

#define SPECULAR_GREEN_X_INC	0x03A8	
#define SPECULAR_GREEN_Y_INC	0x03AC	
#define SPECULAR_GREEN_START	0x03B0	
#define SPECULAR_BLUE_X_INC	0x03B4	
#define SPECULAR_BLUE_Y_INC	0x03B8	
#define SPECULAR_BLUE_START	0x03BC	

#define SCALE_X_INC__ALIAS__	0x03C0	

#define RED_X_INC__ALIAS__	0x03C0	
#define RED_Y_INC		0x03C4	
#define RED_START		0x03C8	

#define SCALE_HACC		0x03C8	
#define SCALE_Y_INC__ALIAS__	0x03CC	

#define GREEN_X_INC__ALIAS__	0x03CC	
#define GREEN_Y_INC		0x03D0	

#define SECONDARY_SCALE_Y_INC	0x03D0	
#define SECONDARY_SCALE_VACC	0x03D4	

#define GREEN_START		0x03D4	
#define BLUE_X_INC		0x03D8	
#define BLUE_Y_INC		0x03DC	
#define BLUE_START		0x03E0	
#define Z_X_INC			0x03E4	
#define Z_Y_INC			0x03E8	
#define Z_START			0x03EC	
#define ALPHA_X_INC		0x03F0	
#define FOG_X_INC		0x03F0	
#define ALPHA_Y_INC		0x03F4	
#define FOG_Y_INC		0x03F4	
#define ALPHA_START		0x03F8	
#define FOG_START		0x03F8	

#define OVERLAY_Y_X_START		0x0400	
#define OVERLAY_Y_X_END			0x0404	
#define OVERLAY_VIDEO_KEY_CLR		0x0408	
#define OVERLAY_VIDEO_KEY_MSK		0x040C	
#define OVERLAY_GRAPHICS_KEY_CLR	0x0410	
#define OVERLAY_GRAPHICS_KEY_MSK	0x0414	
#define OVERLAY_KEY_CNTL		0x0418	

#define OVERLAY_SCALE_INC	0x0420	
#define OVERLAY_SCALE_CNTL	0x0424	
#define SCALER_HEIGHT_WIDTH	0x0428	
#define SCALER_TEST		0x042C	
#define SCALER_BUF0_OFFSET	0x0434	
#define SCALER_BUF1_OFFSET	0x0438	
#define SCALE_BUF_PITCH		0x043C	

#define CAPTURE_START_END	0x0440	
#define CAPTURE_X_WIDTH		0x0444	
#define VIDEO_FORMAT		0x0448	
#define VBI_START_END		0x044C	
#define CAPTURE_CONFIG		0x0450	
#define TRIG_CNTL		0x0454	

#define OVERLAY_EXCLUSIVE_HORZ	0x0458	
#define OVERLAY_EXCLUSIVE_VERT	0x045C	

#define VAL_WIDTH		0x0460	
#define CAPTURE_DEBUG		0x0464	
#define VIDEO_SYNC_TEST		0x0468	


#define SNAPSHOT_VH_COUNTS	0x0470	
#define SNAPSHOT_F_COUNT	0x0474	
#define N_VIF_COUNT		0x0478	
#define SNAPSHOT_VIF_COUNT	0x047C	

#define CAPTURE_BUF0_OFFSET	0x0480	
#define CAPTURE_BUF1_OFFSET	0x0484	
#define CAPTURE_BUF_PITCH	0x0488	


#define SNAPSHOT2_VH_COUNTS	0x04B0	
#define SNAPSHOT2_F_COUNT	0x04B4	
#define N_VIF2_COUNT		0x04B8	
#define SNAPSHOT2_VIF_COUNT	0x04BC	

#define MPP_CONFIG		0x04C0	
#define MPP_STROBE_SEQ		0x04C4	
#define MPP_ADDR		0x04C8	
#define MPP_DATA		0x04CC	
#define TVO_CNTL		0x0500	


#define CRT_HORZ_VERT_LOAD	0x0544	


#define AGP_BASE		0x0548	
#define AGP_CNTL		0x054C	

#define SCALER_COLOUR_CNTL	0x0550	
#define SCALER_H_COEFF0		0x0554	
#define SCALER_H_COEFF1		0x0558	
#define SCALER_H_COEFF2		0x055C	
#define SCALER_H_COEFF3		0x0560	
#define SCALER_H_COEFF4		0x0564	


#define GUI_CMDFIFO_DEBUG	0x0570	
#define GUI_CMDFIFO_DATA	0x0574	
#define GUI_CNTL		0x0578	


#define BM_FRAME_BUF_OFFSET	0x0580	
#define BM_SYSTEM_MEM_ADDR	0x0584	
#define BM_COMMAND		0x0588	
#define BM_STATUS		0x058C	
#define BM_GUI_TABLE		0x05B8	
#define BM_SYSTEM_TABLE		0x05BC	

#define SCALER_BUF0_OFFSET_U	0x05D4	
#define SCALER_BUF0_OFFSET_V	0x05D8	
#define SCALER_BUF1_OFFSET_U	0x05DC	
#define SCALER_BUF1_OFFSET_V	0x05E0	


#define VERTEX_1_S		0x0640	
#define VERTEX_1_T		0x0644	
#define VERTEX_1_W		0x0648	
#define VERTEX_1_SPEC_ARGB	0x064C	
#define VERTEX_1_Z		0x0650	
#define VERTEX_1_ARGB		0x0654	
#define VERTEX_1_X_Y		0x0658	
#define ONE_OVER_AREA		0x065C	
#define VERTEX_2_S		0x0660	
#define VERTEX_2_T		0x0664	
#define VERTEX_2_W		0x0668	
#define VERTEX_2_SPEC_ARGB	0x066C	
#define VERTEX_2_Z		0x0670	
#define VERTEX_2_ARGB		0x0674	
#define VERTEX_2_X_Y		0x0678	
#define ONE_OVER_AREA		0x065C	
#define VERTEX_3_S		0x0680	
#define VERTEX_3_T		0x0684	
#define VERTEX_3_W		0x0688	
#define VERTEX_3_SPEC_ARGB	0x068C	
#define VERTEX_3_Z		0x0690	
#define VERTEX_3_ARGB		0x0694	
#define VERTEX_3_X_Y		0x0698	
#define ONE_OVER_AREA		0x065C	
#define VERTEX_1_S		0x0640	
#define VERTEX_1_T		0x0644	
#define VERTEX_1_W		0x0648	
#define VERTEX_2_S		0x0660	
#define VERTEX_2_T		0x0664	
#define VERTEX_2_W		0x0668	
#define VERTEX_3_SECONDARY_S	0x06C0	
#define VERTEX_3_S		0x0680	
#define VERTEX_3_SECONDARY_T	0x06C4	
#define VERTEX_3_T		0x0684	
#define VERTEX_3_SECONDARY_W	0x06C8	
#define VERTEX_3_W		0x0688	
#define VERTEX_1_SPEC_ARGB	0x064C	
#define VERTEX_2_SPEC_ARGB	0x066C	
#define VERTEX_3_SPEC_ARGB	0x068C	
#define VERTEX_1_Z		0x0650	
#define VERTEX_2_Z		0x0670	
#define VERTEX_3_Z		0x0690	
#define VERTEX_1_ARGB		0x0654	
#define VERTEX_2_ARGB		0x0674	
#define VERTEX_3_ARGB		0x0694	
#define VERTEX_1_X_Y		0x0658	
#define VERTEX_2_X_Y		0x0678	
#define VERTEX_3_X_Y		0x0698	
#define ONE_OVER_AREA_UC	0x0700	
#define SETUP_CNTL		0x0704	
#define VERTEX_1_SECONDARY_S	0x0728	
#define VERTEX_1_SECONDARY_T	0x072C	
#define VERTEX_1_SECONDARY_W	0x0730	
#define VERTEX_2_SECONDARY_S	0x0734	
#define VERTEX_2_SECONDARY_T	0x0738	
#define VERTEX_2_SECONDARY_W	0x073C	


#define GTC_3D_RESET_DELAY	3	



#define CRTC_H_SYNC_NEG		0x00200000
#define CRTC_V_SYNC_NEG		0x00200000

#define CRTC_DBL_SCAN_EN	0x00000001
#define CRTC_INTERLACE_EN	0x00000002
#define CRTC_HSYNC_DIS		0x00000004
#define CRTC_VSYNC_DIS		0x00000008
#define CRTC_CSYNC_EN		0x00000010
#define CRTC_PIX_BY_2_EN	0x00000020	
#define CRTC_DISPLAY_DIS	0x00000040
#define CRTC_VGA_XOVERSCAN	0x00000080

#define CRTC_PIX_WIDTH_MASK	0x00000700
#define CRTC_PIX_WIDTH_4BPP	0x00000100
#define CRTC_PIX_WIDTH_8BPP	0x00000200
#define CRTC_PIX_WIDTH_15BPP	0x00000300
#define CRTC_PIX_WIDTH_16BPP	0x00000400
#define CRTC_PIX_WIDTH_24BPP	0x00000500
#define CRTC_PIX_WIDTH_32BPP	0x00000600

#define CRTC_BYTE_PIX_ORDER	0x00000800
#define CRTC_PIX_ORDER_MSN_LSN	0x00000000
#define CRTC_PIX_ORDER_LSN_MSN	0x00000800

#define CRTC_VSYNC_INT_EN	0x00001000ul	
#define CRTC_VSYNC_INT		0x00002000ul	
#define CRTC_FIFO_OVERFILL	0x0000c000ul	
#define CRTC2_VSYNC_INT_EN	0x00004000ul	
#define CRTC2_VSYNC_INT		0x00008000ul	

#define CRTC_FIFO_LWM		0x000f0000
#define CRTC_HVSYNC_IO_DRIVE	0x00010000	
#define CRTC2_PIX_WIDTH		0x000e0000	

#define CRTC_VGA_128KAP_PAGING	0x00100000
#define CRTC_VFC_SYNC_TRISTATE	0x00200000	
#define CRTC2_EN		0x00200000	
#define CRTC_LOCK_REGS		0x00400000
#define CRTC_SYNC_TRISTATE	0x00800000

#define CRTC_EXT_DISP_EN	0x01000000
#define CRTC_EN			0x02000000
#define CRTC_DISP_REQ_EN	0x04000000
#define CRTC_VGA_LINEAR		0x08000000
#define CRTC_VSYNC_FALL_EDGE	0x10000000
#define CRTC_VGA_TEXT_132	0x20000000
#define CRTC_CNT_EN		0x40000000
#define CRTC_CUR_B_TEST		0x80000000

#define CRTC_CRNT_VLINE		0x07f00000

#define CRTC_PRESERVED_MASK	0x0001f000

#define CRTC_VBLANK		0x00000001
#define CRTC_VBLANK_INT_EN	0x00000002
#define CRTC_VBLANK_INT		0x00000004
#define CRTC_VBLANK_INT_AK	CRTC_VBLANK_INT
#define CRTC_VLINE_INT_EN	0x00000008
#define CRTC_VLINE_INT		0x00000010
#define CRTC_VLINE_INT_AK	CRTC_VLINE_INT
#define CRTC_VLINE_SYNC		0x00000020
#define CRTC_FRAME		0x00000040
#define SNAPSHOT_INT_EN		0x00000080
#define SNAPSHOT_INT		0x00000100
#define SNAPSHOT_INT_AK		SNAPSHOT_INT
#define I2C_INT_EN		0x00000200
#define I2C_INT			0x00000400
#define I2C_INT_AK		I2C_INT
#define CRTC2_VBLANK		0x00000800
#define CRTC2_VBLANK_INT_EN	0x00001000
#define CRTC2_VBLANK_INT	0x00002000
#define CRTC2_VBLANK_INT_AK	CRTC2_VBLANK_INT
#define CRTC2_VLINE_INT_EN	0x00004000
#define CRTC2_VLINE_INT		0x00008000
#define CRTC2_VLINE_INT_AK	CRTC2_VLINE_INT
#define CAPBUF0_INT_EN		0x00010000
#define CAPBUF0_INT		0x00020000
#define CAPBUF0_INT_AK		CAPBUF0_INT
#define CAPBUF1_INT_EN		0x00040000
#define CAPBUF1_INT		0x00080000
#define CAPBUF1_INT_AK		CAPBUF1_INT
#define OVERLAY_EOF_INT_EN	0x00100000
#define OVERLAY_EOF_INT		0x00200000
#define OVERLAY_EOF_INT_AK	OVERLAY_EOF_INT
#define ONESHOT_CAP_INT_EN	0x00400000
#define ONESHOT_CAP_INT		0x00800000
#define ONESHOT_CAP_INT_AK	ONESHOT_CAP_INT
#define BUSMASTER_EOL_INT_EN	0x01000000
#define BUSMASTER_EOL_INT	0x02000000
#define BUSMASTER_EOL_INT_AK	BUSMASTER_EOL_INT
#define GP_INT_EN		0x04000000
#define GP_INT			0x08000000
#define GP_INT_AK		GP_INT
#define CRTC2_VLINE_SYNC	0x10000000
#define SNAPSHOT2_INT_EN	0x20000000
#define SNAPSHOT2_INT		0x40000000
#define SNAPSHOT2_INT_AK	SNAPSHOT2_INT
#define VBLANK_BIT2_INT		0x80000000
#define VBLANK_BIT2_INT_AK	VBLANK_BIT2_INT

#define CRTC_INT_EN_MASK	(CRTC_VBLANK_INT_EN |	\
				 CRTC_VLINE_INT_EN |	\
				 SNAPSHOT_INT_EN |	\
				 I2C_INT_EN |		\
				 CRTC2_VBLANK_INT_EN |	\
				 CRTC2_VLINE_INT_EN |	\
				 CAPBUF0_INT_EN |	\
				 CAPBUF1_INT_EN |	\
				 OVERLAY_EOF_INT_EN |	\
				 ONESHOT_CAP_INT_EN |	\
				 BUSMASTER_EOL_INT_EN |	\
				 GP_INT_EN |		\
				 SNAPSHOT2_INT_EN)



#define DAC_EXT_SEL_RS2		0x01
#define DAC_EXT_SEL_RS3		0x02
#define DAC_8BIT_EN		0x00000100
#define DAC_PIX_DLY_MASK	0x00000600
#define DAC_PIX_DLY_0NS		0x00000000
#define DAC_PIX_DLY_2NS		0x00000200
#define DAC_PIX_DLY_4NS		0x00000400
#define DAC_BLANK_ADJ_MASK	0x00001800
#define DAC_BLANK_ADJ_0		0x00000000
#define DAC_BLANK_ADJ_1		0x00000800
#define DAC_BLANK_ADJ_2		0x00001000


#define DAC_OUTPUT_MASK         0x00000001  
#define DAC_MISTERY_BIT         0x00000002  
#define DAC_BLANKING            0x00000004
#define DAC_CMP_DISABLE         0x00000008
#define DAC1_CLK_SEL            0x00000010
#define PALETTE_ACCESS_CNTL     0x00000020
#define PALETTE2_SNOOP_EN       0x00000040
#define DAC_CMP_OUTPUT          0x00000080 

#define CRT_SENSE               0x00000800 
#define CRT_DETECTION_ON        0x00001000
#define DAC_VGA_ADR_EN          0x00002000
#define DAC_FEA_CON_EN          0x00004000
#define DAC_PDWN                0x00008000
#define DAC_TYPE_MASK           0x00070000 





#define MIX_NOT_DST		0x0000
#define MIX_0			0x0001
#define MIX_1			0x0002
#define MIX_DST			0x0003
#define MIX_NOT_SRC		0x0004
#define MIX_XOR			0x0005
#define MIX_XNOR		0x0006
#define MIX_SRC			0x0007
#define MIX_NAND		0x0008
#define MIX_NOT_SRC_OR_DST	0x0009
#define MIX_SRC_OR_NOT_DST	0x000a
#define MIX_OR			0x000b
#define MIX_AND			0x000c
#define MIX_SRC_AND_NOT_DST	0x000d
#define MIX_NOT_SRC_AND_DST	0x000e
#define MIX_NOR			0x000f


#define ENGINE_MIN_X		0
#define ENGINE_MIN_Y		0
#define ENGINE_MAX_X		4095
#define ENGINE_MAX_Y		16383




#define BUS_APER_REG_DIS	0x00000010
#define BUS_FIFO_ERR_ACK	0x00200000
#define BUS_HOST_ERR_ACK	0x00800000


#define GEN_OVR_OUTPUT_EN	0x20
#define HWCURSOR_ENABLE		0x80
#define GUI_ENGINE_ENABLE	0x100
#define BLOCK_WRITE_ENABLE	0x200


#define DSP_XCLKS_PER_QW	0x00003fff
#define DSP_LOOP_LATENCY	0x000f0000
#define DSP_PRECISION		0x00700000


#define DSP_OFF			0x000007ff
#define DSP_ON			0x07ff0000
#define VGA_DSP_OFF		DSP_OFF
#define VGA_DSP_ON		DSP_ON
#define VGA_DSP_XCLKS_PER_QW	DSP_XCLKS_PER_QW


#define MPLL_CNTL		0x00
#define PLL_PC_GAIN		0x07
#define PLL_VC_GAIN		0x18
#define PLL_DUTY_CYC		0xE0
#define VPLL_CNTL		0x01
#define PLL_REF_DIV		0x02
#define PLL_GEN_CNTL		0x03
#define PLL_OVERRIDE		0x01	
#define PLL_MCLK_RST		0x02	
#define OSC_EN			0x04
#define EXT_CLK_EN		0x08
#define FORCE_DCLK_TRI_STATE	0x08    
#define MCLK_SRC_SEL		0x70
#define EXT_CLK_CNTL		0x80
#define DLL_PWDN		0x80    
#define MCLK_FB_DIV		0x04
#define PLL_VCLK_CNTL		0x05
#define PLL_VCLK_SRC_SEL	0x03
#define PLL_VCLK_RST		0x04
#define PLL_VCLK_INVERT		0x08
#define VCLK_POST_DIV		0x06
#define VCLK0_POST		0x03
#define VCLK1_POST		0x0C
#define VCLK2_POST		0x30
#define VCLK3_POST		0xC0
#define VCLK0_FB_DIV		0x07
#define VCLK1_FB_DIV		0x08
#define VCLK2_FB_DIV		0x09
#define VCLK3_FB_DIV		0x0A
#define PLL_EXT_CNTL		0x0B
#define PLL_XCLK_MCLK_RATIO	0x03
#define PLL_XCLK_SRC_SEL	0x07
#define PLL_MFB_TIMES_4_2B	0x08
#define PLL_VCLK0_XDIV		0x10
#define PLL_VCLK1_XDIV		0x20
#define PLL_VCLK2_XDIV		0x40
#define PLL_VCLK3_XDIV		0x80
#define DLL_CNTL		0x0C
#define DLL1_CNTL		0x0C
#define VFC_CNTL		0x0D
#define PLL_TEST_CNTL		0x0E
#define PLL_TEST_COUNT		0x0F
#define LVDS_CNTL0		0x10
#define LVDS_CNTL1		0x11
#define AGP1_CNTL		0x12
#define AGP2_CNTL		0x13
#define DLL2_CNTL		0x14
#define SCLK_FB_DIV		0x15
#define SPLL_CNTL1		0x16
#define SPLL_CNTL2		0x17
#define APLL_STRAPS		0x18
#define EXT_VPLL_CNTL		0x19
#define EXT_VPLL_EN		0x04
#define EXT_VPLL_VGA_EN		0x08
#define EXT_VPLL_INSYNC		0x10
#define EXT_VPLL_REF_DIV	0x1A
#define EXT_VPLL_FB_DIV		0x1B
#define EXT_VPLL_MSB		0x1C
#define HTOTAL_CNTL		0x1D
#define BYTE_CLK_CNTL		0x1E
#define TV_PLL_CNTL1		0x1F
#define TV_PLL_CNTL2		0x20
#define TV_PLL_CNTL		0x21
#define EXT_TV_PLL		0x22
#define V2PLL_CNTL		0x23
#define PLL_V2CLK_CNTL		0x24
#define EXT_V2PLL_REF_DIV	0x25
#define EXT_V2PLL_FB_DIV	0x26
#define EXT_V2PLL_MSB		0x27
#define HTOTAL2_CNTL		0x28
#define PLL_YCLK_CNTL		0x29
#define PM_DYN_CLK_CNTL		0x2A


#define APERTURE_4M_ENABLE	1
#define APERTURE_8M_ENABLE	2
#define VGA_APERTURE_ENABLE	4


#define CFG_BUS_TYPE		0x00000007
#define CFG_MEM_TYPE		0x00000038
#define CFG_INIT_DAC_TYPE	0x00000e00


#define CFG_MEM_TYPE_xT		0x00000007

#define ISA			0
#define EISA			1
#define LOCAL_BUS		6
#define PCI			7


#define DRAMx4			0
#define VRAMx16			1
#define VRAMx16ssr		2
#define DRAMx16			3
#define GraphicsDRAMx16		4
#define EnhancedVRAMx16		5
#define EnhancedVRAMx16ssr	6


#define DRAM			1
#define EDO			2
#define PSEUDO_EDO		3
#define SDRAM			4
#define SGRAM			5
#define WRAM			6
#define SDRAM32			6

#define DAC_INTERNAL		0x00
#define DAC_IBMRGB514		0x01
#define DAC_ATI68875		0x02
#define DAC_TVP3026_A		0x72
#define DAC_BT476		0x03
#define DAC_BT481		0x04
#define DAC_ATT20C491		0x14
#define DAC_SC15026		0x24
#define DAC_MU9C1880		0x34
#define DAC_IMSG174		0x44
#define DAC_ATI68860_B		0x05
#define DAC_ATI68860_C		0x15
#define DAC_TVP3026_B		0x75
#define DAC_STG1700		0x06
#define DAC_ATT498		0x16
#define DAC_STG1702		0x07
#define DAC_SC15021		0x17
#define DAC_ATT21C498		0x27
#define DAC_STG1703		0x37
#define DAC_CH8398		0x47
#define DAC_ATT20C408		0x57

#define CLK_ATI18818_0		0
#define CLK_ATI18818_1		1
#define CLK_STG1703		2
#define CLK_CH8398		3
#define CLK_INTERNAL		4
#define CLK_ATT20C408		5
#define CLK_IBMRGB514		6


#define MEM_SIZE_ALIAS		0x00000007
#define MEM_SIZE_512K		0x00000000
#define MEM_SIZE_1M		0x00000001
#define MEM_SIZE_2M		0x00000002
#define MEM_SIZE_4M		0x00000003
#define MEM_SIZE_6M		0x00000004
#define MEM_SIZE_8M		0x00000005
#define MEM_SIZE_ALIAS_GTB	0x0000000F
#define MEM_SIZE_2M_GTB		0x00000003
#define MEM_SIZE_4M_GTB		0x00000007
#define MEM_SIZE_6M_GTB		0x00000009
#define MEM_SIZE_8M_GTB		0x0000000B
#define MEM_BNDRY		0x00030000
#define MEM_BNDRY_0K		0x00000000
#define MEM_BNDRY_256K		0x00010000
#define MEM_BNDRY_512K		0x00020000
#define MEM_BNDRY_1M		0x00030000
#define MEM_BNDRY_EN		0x00040000

#define ONE_MB			0x100000

#define PCI_ATI_VENDOR_ID	0x1002



#define CFG_CHIP_TYPE		0x0000FFFF
#define CFG_CHIP_CLASS		0x00FF0000
#define CFG_CHIP_REV		0xFF000000
#define CFG_CHIP_MAJOR		0x07000000
#define CFG_CHIP_FND_ID		0x38000000
#define CFG_CHIP_MINOR		0xC0000000





#define GX_CHIP_ID	0xD7	
#define CX_CHIP_ID	0x57	

#define GX_PCI_ID	0x4758	
#define CX_PCI_ID	0x4358	


#define CT_CHIP_ID	0x4354	
#define ET_CHIP_ID	0x4554	


#define VT_CHIP_ID	0x5654	
#define VU_CHIP_ID	0x5655	
#define VV_CHIP_ID	0x5656	


#define LB_CHIP_ID	0x4c42	
#define LD_CHIP_ID	0x4c44	
#define LG_CHIP_ID	0x4c47	
#define LI_CHIP_ID	0x4c49	
#define LP_CHIP_ID	0x4c50	
#define LT_CHIP_ID	0x4c54	


#define GR_CHIP_ID	0x4752	
#define GS_CHIP_ID	0x4753	
#define GM_CHIP_ID	0x474d	
#define GN_CHIP_ID	0x474e	
#define GO_CHIP_ID	0x474f	
#define GL_CHIP_ID	0x474c	

#define IS_XL(id) ((id)==GR_CHIP_ID || (id)==GS_CHIP_ID || \
		   (id)==GM_CHIP_ID || (id)==GN_CHIP_ID || \
		   (id)==GO_CHIP_ID || (id)==GL_CHIP_ID)

#define GT_CHIP_ID	0x4754	
#define GU_CHIP_ID	0x4755	
#define GV_CHIP_ID	0x4756	
#define GW_CHIP_ID	0x4757	
#define GZ_CHIP_ID	0x475a	
#define GB_CHIP_ID	0x4742	
#define GD_CHIP_ID	0x4744	
#define GI_CHIP_ID	0x4749	
#define GP_CHIP_ID	0x4750	
#define GQ_CHIP_ID	0x4751	

#define LM_CHIP_ID	0x4c4d	
#define LN_CHIP_ID	0x4c4e	
#define LR_CHIP_ID	0x4c52	
#define LS_CHIP_ID	0x4c53	

#define IS_MOBILITY(id) ((id)==LM_CHIP_ID || (id)==LN_CHIP_ID || \
			(id)==LR_CHIP_ID || (id)==LS_CHIP_ID)

#define MACH64_ASIC_NEC_VT_A3		0x08
#define MACH64_ASIC_NEC_VT_A4		0x48
#define MACH64_ASIC_SGS_VT_A4		0x40
#define MACH64_ASIC_SGS_VT_B1S1		0x01
#define MACH64_ASIC_SGS_GT_B1S1		0x01
#define MACH64_ASIC_SGS_GT_B1S2		0x41
#define MACH64_ASIC_UMC_GT_B2U1		0x1a
#define MACH64_ASIC_UMC_GT_B2U2		0x5a
#define MACH64_ASIC_UMC_VT_B2U3		0x9a
#define MACH64_ASIC_UMC_GT_B2U3		0x9a
#define MACH64_ASIC_UMC_R3B_D_P_A1	0x1b
#define MACH64_ASIC_UMC_R3B_D_P_A2	0x5b
#define MACH64_ASIC_UMC_R3B_D_P_A3	0x1c
#define MACH64_ASIC_UMC_R3B_D_P_A4	0x5c


#define MACH64_FND_SGS		0
#define MACH64_FND_NEC		1
#define MACH64_FND_UMC		3


#define MACH64_UNKNOWN		0
#define MACH64_GX		1
#define MACH64_CX		2
#define MACH64_CT		3Restore
#define MACH64_ET		4
#define MACH64_VT		5
#define MACH64_GT		6


#define DST_X_RIGHT_TO_LEFT	0
#define DST_X_LEFT_TO_RIGHT	1
#define DST_Y_BOTTOM_TO_TOP	0
#define DST_Y_TOP_TO_BOTTOM	2
#define DST_X_MAJOR		0
#define DST_Y_MAJOR		4
#define DST_X_TILE		8
#define DST_Y_TILE		0x10
#define DST_LAST_PEL		0x20
#define DST_POLYGON_ENABLE	0x40
#define DST_24_ROTATION_ENABLE	0x80


#define SRC_PATTERN_ENABLE		1
#define SRC_ROTATION_ENABLE		2
#define SRC_LINEAR_ENABLE		4
#define SRC_BYTE_ALIGN			8
#define SRC_LINE_X_RIGHT_TO_LEFT	0
#define SRC_LINE_X_LEFT_TO_RIGHT	0x10


#define HOST_BYTE_ALIGN		1


#define PAT_MONO_8x8_ENABLE	0x01000000
#define PAT_CLR_4x2_ENABLE	0x02000000
#define PAT_CLR_8x1_ENABLE	0x04000000


#define DP_CHAIN_4BPP		0x8888
#define DP_CHAIN_7BPP		0xD2D2
#define DP_CHAIN_8BPP		0x8080
#define DP_CHAIN_8BPP_RGB	0x9292
#define DP_CHAIN_15BPP		0x4210
#define DP_CHAIN_16BPP		0x8410
#define DP_CHAIN_24BPP		0x8080
#define DP_CHAIN_32BPP		0x8080


#define DST_1BPP		0x0
#define DST_4BPP		0x1
#define DST_8BPP		0x2
#define DST_15BPP		0x3
#define DST_16BPP		0x4
#define DST_24BPP		0x5
#define DST_32BPP		0x6
#define DST_MASK		0xF
#define SRC_1BPP		0x000
#define SRC_4BPP		0x100
#define SRC_8BPP		0x200
#define SRC_15BPP		0x300
#define SRC_16BPP		0x400
#define SRC_24BPP		0x500
#define SRC_32BPP		0x600
#define SRC_MASK		0xF00
#define DP_HOST_TRIPLE_EN	0x2000
#define HOST_1BPP		0x00000
#define HOST_4BPP		0x10000
#define HOST_8BPP		0x20000
#define HOST_15BPP		0x30000
#define HOST_16BPP		0x40000
#define HOST_24BPP		0x50000
#define HOST_32BPP		0x60000
#define HOST_MASK		0xF0000
#define BYTE_ORDER_MSB_TO_LSB	0
#define BYTE_ORDER_LSB_TO_MSB	0x1000000
#define BYTE_ORDER_MASK		0x1000000


#define BKGD_MIX_NOT_D			0
#define BKGD_MIX_ZERO			1
#define BKGD_MIX_ONE			2
#define BKGD_MIX_D			3
#define BKGD_MIX_NOT_S			4
#define BKGD_MIX_D_XOR_S		5
#define BKGD_MIX_NOT_D_XOR_S		6
#define BKGD_MIX_S			7
#define BKGD_MIX_NOT_D_OR_NOT_S		8
#define BKGD_MIX_D_OR_NOT_S		9
#define BKGD_MIX_NOT_D_OR_S		10
#define BKGD_MIX_D_OR_S			11
#define BKGD_MIX_D_AND_S		12
#define BKGD_MIX_NOT_D_AND_S		13
#define BKGD_MIX_D_AND_NOT_S		14
#define BKGD_MIX_NOT_D_AND_NOT_S	15
#define BKGD_MIX_D_PLUS_S_DIV2		0x17
#define FRGD_MIX_NOT_D			0
#define FRGD_MIX_ZERO			0x10000
#define FRGD_MIX_ONE			0x20000
#define FRGD_MIX_D			0x30000
#define FRGD_MIX_NOT_S			0x40000
#define FRGD_MIX_D_XOR_S		0x50000
#define FRGD_MIX_NOT_D_XOR_S		0x60000
#define FRGD_MIX_S			0x70000
#define FRGD_MIX_NOT_D_OR_NOT_S		0x80000
#define FRGD_MIX_D_OR_NOT_S		0x90000
#define FRGD_MIX_NOT_D_OR_S		0xa0000
#define FRGD_MIX_D_OR_S			0xb0000
#define FRGD_MIX_D_AND_S		0xc0000
#define FRGD_MIX_NOT_D_AND_S		0xd0000
#define FRGD_MIX_D_AND_NOT_S		0xe0000
#define FRGD_MIX_NOT_D_AND_NOT_S	0xf0000
#define FRGD_MIX_D_PLUS_S_DIV2		0x170000


#define BKGD_SRC_BKGD_CLR	0
#define BKGD_SRC_FRGD_CLR	1
#define BKGD_SRC_HOST		2
#define BKGD_SRC_BLIT		3
#define BKGD_SRC_PATTERN	4
#define FRGD_SRC_BKGD_CLR	0
#define FRGD_SRC_FRGD_CLR	0x100
#define FRGD_SRC_HOST		0x200
#define FRGD_SRC_BLIT		0x300
#define FRGD_SRC_PATTERN	0x400
#define MONO_SRC_ONE		0
#define MONO_SRC_PATTERN	0x10000
#define MONO_SRC_HOST		0x20000
#define MONO_SRC_BLIT		0x30000


#define COMPARE_FALSE		0
#define COMPARE_TRUE		1
#define COMPARE_NOT_EQUAL	4
#define COMPARE_EQUAL		5
#define COMPARE_DESTINATION	0
#define COMPARE_SOURCE		0x1000000


#define FIFO_ERR		0x80000000


#define CONTEXT_NO_LOAD			0
#define CONTEXT_LOAD			0x10000
#define CONTEXT_LOAD_AND_DO_FILL	0x20000
#define CONTEXT_LOAD_AND_DO_LINE	0x30000
#define CONTEXT_EXECUTE			0
#define CONTEXT_CMD_DISABLE		0x80000000


#define ENGINE_IDLE			0
#define ENGINE_BUSY			1
#define SCISSOR_LEFT_FLAG		0x10
#define SCISSOR_RIGHT_FLAG		0x20
#define SCISSOR_TOP_FLAG		0x40
#define SCISSOR_BOTTOM_FLAG		0x80


#define sioATIEXT		0x1ce
#define bioATIEXT		0x3ce

#define ATI2E			0xae
#define ATI32			0xb2
#define ATI36			0xb6


#define R_GENMO			0x3cc
#define VGAGRA			0x3ce
#define GRA06			0x06


#define VGASEQ			0x3c4
#define SEQ02			0x02
#define SEQ04			0x04

#define MACH64_MAX_X		ENGINE_MAX_X
#define MACH64_MAX_Y		ENGINE_MAX_Y

#define INC_X			0x0020
#define INC_Y			0x0080

#define RGB16_555		0x0000
#define RGB16_565		0x0040
#define RGB16_655		0x0080
#define RGB16_664		0x00c0

#define POLY_TEXT_TYPE		0x0001
#define IMAGE_TEXT_TYPE		0x0002
#define TEXT_TYPE_8_BIT		0x0004
#define TEXT_TYPE_16_BIT	0x0008
#define POLY_TEXT_TYPE_8	(POLY_TEXT_TYPE | TEXT_TYPE_8_BIT)
#define IMAGE_TEXT_TYPE_8	(IMAGE_TEXT_TYPE | TEXT_TYPE_8_BIT)
#define POLY_TEXT_TYPE_16	(POLY_TEXT_TYPE | TEXT_TYPE_16_BIT)
#define IMAGE_TEXT_TYPE_16	(IMAGE_TEXT_TYPE | TEXT_TYPE_16_BIT)

#define MACH64_NUM_CLOCKS	16
#define MACH64_NUM_FREQS	50


#define PWR_MGT_ON		0x00000001
#define PWR_MGT_MODE_MASK	0x00000006
#define AUTO_PWR_UP		0x00000008
#define USE_F32KHZ		0x00000400
#define TRISTATE_MEM_EN		0x00000800
#define SELF_REFRESH		0x00000080
#define PWR_BLON		0x02000000
#define STANDBY_NOW		0x10000000
#define SUSPEND_NOW		0x20000000
#define PWR_MGT_STATUS_MASK	0xC0000000
#define PWR_MGT_STATUS_SUSPEND	0x80000000


#define PWR_MGT_MODE_PIN	0x00000000
#define PWR_MGT_MODE_REG	0x00000002
#define PWR_MGT_MODE_TIMER	0x00000004
#define PWR_MGT_MODE_PCI	0x00000006




#define LCD_INDEX_MASK		0x0000003F
#define LCD_DISPLAY_DIS		0x00000100
#define LCD_SRC_SEL		0x00000200
#define CRTC2_DISPLAY_DIS	0x00000400


#define CNFG_PANEL		0x00
#define LCD_GEN_CNTL		0x01
#define DSTN_CONTROL		0x02
#define HFB_PITCH_ADDR		0x03
#define HORZ_STRETCHING		0x04
#define VERT_STRETCHING		0x05
#define EXT_VERT_STRETCH	0x06
#define LT_GIO			0x07
#define POWER_MANAGEMENT	0x08
#define ZVGPIO			0x09
#define ICON_CLR0		0x0A
#define ICON_CLR1		0x0B
#define ICON_OFFSET		0x0C
#define ICON_HORZ_VERT_POSN	0x0D
#define ICON_HORZ_VERT_OFF	0x0E
#define ICON2_CLR0		0x0F
#define ICON2_CLR1		0x10
#define ICON2_OFFSET		0x11
#define ICON2_HORZ_VERT_POSN	0x12
#define ICON2_HORZ_VERT_OFF	0x13
#define LCD_MISC_CNTL		0x14
#define APC_CNTL		0x1C
#define POWER_MANAGEMENT_2	0x1D
#define ALPHA_BLENDING		0x25
#define PORTRAIT_GEN_CNTL	0x26
#define APC_CTRL_IO		0x27
#define TEST_IO			0x28
#define TEST_OUTPUTS		0x29
#define DP1_MEM_ACCESS		0x2A
#define DP0_MEM_ACCESS		0x2B
#define DP0_DEBUG_A		0x2C
#define DP0_DEBUG_B		0x2D
#define DP1_DEBUG_A		0x2E
#define DP1_DEBUG_B		0x2F
#define DPCTRL_DEBUG_A		0x30
#define DPCTRL_DEBUG_B		0x31
#define MEMBLK_DEBUG		0x32
#define APC_LUT_AB		0x33
#define APC_LUT_CD		0x34
#define APC_LUT_EF		0x35
#define APC_LUT_GH		0x36
#define APC_LUT_IJ		0x37
#define APC_LUT_KL		0x38
#define APC_LUT_MN		0x39
#define APC_LUT_OP		0x3A


#define CRT_ON                          0x00000001ul
#define LCD_ON                          0x00000002ul
#define HORZ_DIVBY2_EN                  0x00000004ul
#define DONT_DS_ICON                    0x00000008ul
#define LOCK_8DOT                       0x00000010ul
#define ICON_ENABLE                     0x00000020ul
#define DONT_SHADOW_VPAR                0x00000040ul
#define V2CLK_PM_EN                     0x00000080ul
#define RST_FM                          0x00000100ul
#define DISABLE_PCLK_RESET              0x00000200ul	
#define DIS_HOR_CRT_DIVBY2              0x00000400ul
#define SCLK_SEL                        0x00000800ul
#define SCLK_DELAY                      0x0000f000ul
#define TVCLK_PM_EN                     0x00010000ul
#define VCLK_DAC_PM_EN                  0x00020000ul
#define VCLK_LCD_OFF                    0x00040000ul
#define SELECT_WAIT_4MS                 0x00080000ul
#define XTALIN_PM_EN                    0x00080000ul	
#define V2CLK_DAC_PM_EN                 0x00100000ul
#define LVDS_EN                         0x00200000ul
#define LVDS_PLL_EN                     0x00400000ul
#define LVDS_PLL_RESET                  0x00800000ul
#define LVDS_RESERVED_BITS              0x07000000ul
#define CRTC_RW_SELECT                  0x08000000ul	
#define USE_SHADOWED_VEND               0x10000000ul
#define USE_SHADOWED_ROWCUR             0x20000000ul
#define SHADOW_EN                       0x40000000ul
#define SHADOW_RW_EN                  	0x80000000ul

#define LCD_SET_PRIMARY_MASK            0x07FFFBFBul


#define HORZ_STRETCH_BLEND		0x00000ffful
#define HORZ_STRETCH_RATIO		0x0000fffful
#define HORZ_STRETCH_LOOP		0x00070000ul
#define HORZ_STRETCH_LOOP09		0x00000000ul
#define HORZ_STRETCH_LOOP11		0x00010000ul
#define HORZ_STRETCH_LOOP12		0x00020000ul
#define HORZ_STRETCH_LOOP14		0x00030000ul
#define HORZ_STRETCH_LOOP15		0x00040000ul




#define HORZ_PANEL_SIZE			0x0ff00000ul	

#define AUTO_HORZ_RATIO			0x20000000ul	
#define HORZ_STRETCH_MODE		0x40000000ul
#define HORZ_STRETCH_EN			0x80000000ul


#define VERT_STRETCH_RATIO0		0x000003fful
#define VERT_STRETCH_RATIO1		0x000ffc00ul
#define VERT_STRETCH_RATIO2		0x3ff00000ul
#define VERT_STRETCH_USE0		0x40000000ul
#define VERT_STRETCH_EN			0x80000000ul


#define VERT_STRETCH_RATIO3		0x000003fful
#define FORCE_DAC_DATA			0x000000fful
#define FORCE_DAC_DATA_SEL		0x00000300ul
#define VERT_STRETCH_MODE		0x00000400ul
#define VERT_PANEL_SIZE			0x003ff800ul
#define AUTO_VERT_RATIO			0x00400000ul
#define USE_AUTO_FP_POS			0x00800000ul
#define USE_AUTO_LCD_VSYNC		0x01000000ul



#define BIAS_MOD_LEVEL_MASK		0x0000ff00
#define BIAS_MOD_LEVEL_SHIFT		8
#define BLMOD_EN			0x00010000
#define BIASMOD_EN			0x00020000

#endif				
