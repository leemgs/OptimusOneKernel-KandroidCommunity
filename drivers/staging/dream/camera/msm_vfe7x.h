
#ifndef __MSM_VFE7X_H__
#define __MSM_VFE7X_H__
#include <media/msm_camera.h>
#include <mach/camera.h>

struct vfe_frame_extra {
	uint32_t  bl_evencol;
	uint32_t  bl_oddcol;
	uint16_t  g_def_p_cnt;
	uint16_t  r_b_def_p_cnt;
};

struct vfe_endframe {
	uint32_t      y_address;
	uint32_t      cbcr_address;

	unsigned int  blacklevelevencolumn:23;
	uint16_t      reserved1:9;
	unsigned int  blackleveloddcolumn:23;
	uint16_t      reserved2:9;

	uint16_t      greendefectpixelcount:8;
	uint16_t      reserved3:8;
	uint16_t      redbluedefectpixelcount:8;
	uint16_t      reserved4:8;
} __attribute__((packed, aligned(4)));

struct vfe_outputack {
	uint32_t  header;
	void      *output2newybufferaddress;
	void      *output2newcbcrbufferaddress;
} __attribute__((packed, aligned(4)));

struct vfe_stats_ack {
	uint32_t header;
	
	void     *bufaddr;
} __attribute__((packed, aligned(4)));


struct axiout {
	uint32_t            cmdheader:32;
	int 		    outputmode:3;
	uint8_t             format:2;
	uint32_t             : 27;

	
	uint32_t            out1yimageheight:12;
	uint32_t             : 4;
	uint32_t            out1yimagewidthin64bitwords:10;
	uint32_t             : 6;

	
	uint8_t             out1yburstlen:2;
	uint32_t            out1ynumrows:12;
	uint32_t            out1yrowincin64bitincs:12;
	uint32_t             : 6;

	
	uint32_t            out1cbcrimageheight:12;
	uint32_t             : 4;
	uint32_t            out1cbcrimagewidthin64bitwords:10;
	uint32_t             : 6;

	
	uint8_t             out1cbcrburstlen:2;
	uint32_t            out1cbcrnumrows:12;
	uint32_t            out1cbcrrowincin64bitincs:12;
	uint32_t             : 6;

	
	uint32_t            out2yimageheight:12;
	uint32_t             : 4;
	uint32_t            out2yimagewidthin64bitwords:10;
	uint32_t             : 6;

	
	uint8_t             out2yburstlen:2;
	uint32_t            out2ynumrows:12;
	uint32_t            out2yrowincin64bitincs:12;
	uint32_t             : 6;

	
	uint32_t            out2cbcrimageheight:12;
	uint32_t             : 4;
	uint32_t            out2cbcrimagewidtein64bitwords:10;
	uint32_t             : 6;

	
	uint8_t             out2cbcrburstlen:2;
	uint32_t            out2cbcrnumrows:12;
	uint32_t            out2cbcrrowincin64bitincs:12;
	uint32_t             : 6;

	
	unsigned long   output1buffer1_y_phy;
	unsigned long   output1buffer1_cbcr_phy;
	unsigned long   output1buffer2_y_phy;
	unsigned long   output1buffer2_cbcr_phy;
	unsigned long   output1buffer3_y_phy;
	unsigned long   output1buffer3_cbcr_phy;
	unsigned long   output1buffer4_y_phy;
	unsigned long   output1buffer4_cbcr_phy;
	unsigned long   output1buffer5_y_phy;
	unsigned long   output1buffer5_cbcr_phy;
	unsigned long   output1buffer6_y_phy;
	unsigned long   output1buffer6_cbcr_phy;
	unsigned long   output1buffer7_y_phy;
	unsigned long   output1buffer7_cbcr_phy;
	unsigned long   output1buffer8_y_phy;
	unsigned long   output1buffer8_cbcr_phy;

	
	unsigned long   output2buffer1_y_phy;
	unsigned long   output2buffer1_cbcr_phy;
	unsigned long   output2buffer2_y_phy;
	unsigned long   output2buffer2_cbcr_phy;
	unsigned long   output2buffer3_y_phy;
	unsigned long   output2buffer3_cbcr_phy;
	unsigned long   output2buffer4_y_phy;
	unsigned long   output2buffer4_cbcr_phy;
	unsigned long   output2buffer5_y_phy;
	unsigned long   output2buffer5_cbcr_phy;
	unsigned long   output2buffer6_y_phy;
	unsigned long   output2buffer6_cbcr_phy;
	unsigned long   output2buffer7_y_phy;
	unsigned long   output2buffer7_cbcr_phy;
	unsigned long   output2buffer8_y_phy;
	unsigned long   output2buffer8_cbcr_phy;
} __attribute__((packed, aligned(4)));

struct vfe_stats_we_cfg {
	uint32_t       header;

	
	uint8_t        wb_expstatsenable:1;
	uint8_t        wb_expstatbuspriorityselection:1;
	unsigned int   wb_expstatbuspriorityvalue:4;
	unsigned int    : 26;

	
	uint8_t        exposurestatregions:1;
	uint8_t        exposurestatsubregions:1;
	unsigned int    : 14;

	unsigned int   whitebalanceminimumy:8;
	unsigned int   whitebalancemaximumy:8;

	
	uint8_t wb_expstatslopeofneutralregionline[
		NUM_WB_EXP_NEUTRAL_REGION_LINES];

	
	unsigned int   wb_expstatcrinterceptofneutralregionline2:12;
	unsigned int    : 4;
	unsigned int   wb_expstatcbinterceptofneutralreginnline1:12;
	unsigned int     : 4;

	
	unsigned int   wb_expstatcrinterceptofneutralregionline4:12;
	unsigned int    : 4;
	unsigned int   wb_expstatcbinterceptofneutralregionline3:12;
	unsigned int    : 4;

	
	unsigned int   wb_expmetricheaderpattern:8;
	unsigned int    : 24;

	
	void  *wb_expstatoutputbuffer[NUM_WB_EXP_STAT_OUTPUT_BUFFERS];
} __attribute__((packed, aligned(4)));

struct vfe_stats_af_cfg {
	uint32_t header;

	
	uint8_t       af_enable:1;
	uint8_t       af_busprioritysel:1;
	unsigned int  af_buspriorityval:4;
	unsigned int   : 26;

	
	unsigned int  af_singlewinvoffset:12;
	unsigned int   : 4;
	unsigned int  af_singlewinhoffset:12;
	unsigned int   : 3;
	uint8_t       af_winmode:1;

	
	unsigned int  af_singglewinvh:11;
	unsigned int   : 5;
	unsigned int  af_singlewinhw:11;
	unsigned int   : 5;

	
	uint8_t       af_multiwingrid[NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS];

	
	signed int    af_metrichpfcoefa00:5;
	signed int    af_metrichpfcoefa04:5;
	unsigned int  af_metricmaxval:11;
	uint8_t       af_metricsel:1;
	unsigned int   : 10;

	
	signed int    af_metrichpfcoefa20:5;
	signed int    af_metrichpfcoefa21:5;
	signed int    af_metrichpfcoefa22:5;
	signed int    af_metrichpfcoefa23:5;
	signed int    af_metrichpfcoefa24:5;
	unsigned int   : 7;

	
	unsigned int  af_metrichp:8;
	unsigned int   : 24;

	
	void *af_outbuf[NUM_AF_STAT_OUTPUT_BUFFERS];
} __attribute__((packed, aligned(4))); 

struct msm_camera_frame_msg {
	unsigned long   output_y_address;
	unsigned long   output_cbcr_address;

	unsigned int    blacklevelevenColumn:23;
	uint16_t        reserved1:9;
	unsigned int    blackleveloddColumn:23;
	uint16_t        reserved2:9;

	uint16_t        greendefectpixelcount:8;
	uint16_t        reserved3:8;
	uint16_t        redbluedefectpixelcount:8;
	uint16_t        reserved4:8;
} __attribute__((packed, aligned(4)));


struct msm_vfe_command_7k {
	uint16_t queue;
	uint16_t length;
	void     *value;
};

struct stop_event {
  wait_queue_head_t wait;
	int state;
  int timeout;
};


#endif 
