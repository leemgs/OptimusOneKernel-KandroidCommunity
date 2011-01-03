

#ifndef BNX2X_INIT_H
#define BNX2X_INIT_H


#define STORM_INTMEM_SIZE_E1		0x5800
#define STORM_INTMEM_SIZE_E1H		0x10000
#define STORM_INTMEM_SIZE(bp) ((CHIP_IS_E1(bp) ? STORM_INTMEM_SIZE_E1 : \
						    STORM_INTMEM_SIZE_E1H) / 4)




#define OP_RD			0x1 
#define OP_WR			0x2 
#define OP_IW			0x3 
#define OP_SW			0x4 
#define OP_SI			0x5 
#define OP_ZR			0x6 
#define OP_ZP			0x7 
#define OP_WR_64		0x8 
#define OP_WB			0x9 


#define OP_WR_EMUL		0xa 
#define OP_WR_FPGA		0xb 
#define OP_WR_ASIC		0xc 



#define COMMON_STAGE		0
#define PORT0_STAGE		1
#define PORT1_STAGE		2
#define FUNC0_STAGE		3
#define FUNC1_STAGE		4
#define FUNC2_STAGE		5
#define FUNC3_STAGE		6
#define FUNC4_STAGE		7
#define FUNC5_STAGE		8
#define FUNC6_STAGE		9
#define FUNC7_STAGE		10
#define STAGE_IDX_MAX		11

#define STAGE_START		0
#define STAGE_END		1



#define PRS_BLOCK		0
#define SRCH_BLOCK		1
#define TSDM_BLOCK		2
#define TCM_BLOCK		3
#define BRB1_BLOCK		4
#define TSEM_BLOCK		5
#define PXPCS_BLOCK		6
#define EMAC0_BLOCK		7
#define EMAC1_BLOCK		8
#define DBU_BLOCK		9
#define MISC_BLOCK		10
#define DBG_BLOCK		11
#define NIG_BLOCK		12
#define MCP_BLOCK		13
#define UPB_BLOCK		14
#define CSDM_BLOCK		15
#define USDM_BLOCK		16
#define CCM_BLOCK		17
#define UCM_BLOCK		18
#define USEM_BLOCK		19
#define CSEM_BLOCK		20
#define XPB_BLOCK		21
#define DQ_BLOCK		22
#define TIMERS_BLOCK		23
#define XSDM_BLOCK		24
#define QM_BLOCK		25
#define PBF_BLOCK		26
#define XCM_BLOCK		27
#define XSEM_BLOCK		28
#define CDU_BLOCK		29
#define DMAE_BLOCK		30
#define PXP_BLOCK		31
#define CFC_BLOCK		32
#define HC_BLOCK		33
#define PXP2_BLOCK		34
#define MISC_AEU_BLOCK		35
#define PGLUE_B_BLOCK		36
#define IGU_BLOCK		37



#define BLOCK_OPS_IDX(block, stage, end) \
			(2*(((block)*STAGE_IDX_MAX) + (stage)) + (end))


struct raw_op {
	u32 op:8;
	u32 offset:24;
	u32 raw_data;
};

struct op_read {
	u32 op:8;
	u32 offset:24;
	u32 pad;
};

struct op_write {
	u32 op:8;
	u32 offset:24;
	u32 val;
};

struct op_string_write {
	u32 op:8;
	u32 offset:24;
#ifdef __LITTLE_ENDIAN
	u16 data_off;
	u16 data_len;
#else 
	u16 data_len;
	u16 data_off;
#endif
};

struct op_zero {
	u32 op:8;
	u32 offset:24;
	u32 len;
};

union init_op {
	struct op_read		read;
	struct op_write		write;
	struct op_string_write	str_wr;
	struct op_zero		zero;
	struct raw_op		raw;
};

#endif 

