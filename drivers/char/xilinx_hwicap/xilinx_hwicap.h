

#ifndef XILINX_HWICAP_H_	
#define XILINX_HWICAP_H_	

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <asm/io.h>

struct hwicap_drvdata {
	u32 write_buffer_in_use;  
	u8 write_buffer[4];
	u32 read_buffer_in_use;	  
	u8 read_buffer[4];
	resource_size_t mem_start;
	resource_size_t mem_end;  
	resource_size_t mem_size;
	void __iomem *base_address;

	struct device *dev;
	struct cdev cdev;	
	dev_t devt;

	const struct hwicap_driver_config *config;
	const struct config_registers *config_regs;
	void *private_data;
	bool is_open;
	struct mutex sem;
};

struct hwicap_driver_config {
	
	int (*get_configuration)(struct hwicap_drvdata *drvdata, u32 *data,
			u32 size);
	
	int (*set_configuration)(struct hwicap_drvdata *drvdata, u32 *data,
			u32 size);
	
	u32 (*get_status)(struct hwicap_drvdata *drvdata);
	
	void (*reset)(struct hwicap_drvdata *drvdata);
};


#define XHI_MAX_RETRIES     10



#define XHI_PAD_FRAMES              0x1


#define XHI_WORD_COUNT_MASK_TYPE_1  0x7FFUL
#define XHI_WORD_COUNT_MASK_TYPE_2  0x1FFFFFUL
#define XHI_TYPE_MASK               0x7
#define XHI_REGISTER_MASK           0xF
#define XHI_OP_MASK                 0x3

#define XHI_TYPE_SHIFT              29
#define XHI_REGISTER_SHIFT          13
#define XHI_OP_SHIFT                27

#define XHI_TYPE_1                  1
#define XHI_TYPE_2                  2
#define XHI_OP_WRITE                2
#define XHI_OP_READ                 1


#define XHI_FAR_CLB_BLOCK           0
#define XHI_FAR_BRAM_BLOCK          1
#define XHI_FAR_BRAM_INT_BLOCK      2

struct config_registers {
	u32 CRC;
	u32 FAR;
	u32 FDRI;
	u32 FDRO;
	u32 CMD;
	u32 CTL;
	u32 MASK;
	u32 STAT;
	u32 LOUT;
	u32 COR;
	u32 MFWR;
	u32 FLR;
	u32 KEY;
	u32 CBC;
	u32 IDCODE;
	u32 AXSS;
	u32 C0R_1;
	u32 CSOB;
	u32 WBSTAR;
	u32 TIMER;
	u32 BOOTSTS;
	u32 CTL_1;
};


#define XHI_CMD_NULL                0
#define XHI_CMD_WCFG                1
#define XHI_CMD_MFW                 2
#define XHI_CMD_DGHIGH              3
#define XHI_CMD_RCFG                4
#define XHI_CMD_START               5
#define XHI_CMD_RCAP                6
#define XHI_CMD_RCRC                7
#define XHI_CMD_AGHIGH              8
#define XHI_CMD_SWITCH              9
#define XHI_CMD_GRESTORE            10
#define XHI_CMD_SHUTDOWN            11
#define XHI_CMD_GCAPTURE            12
#define XHI_CMD_DESYNCH             13
#define XHI_CMD_IPROG               15 
#define XHI_CMD_CRCC                16 
#define XHI_CMD_LTIMER              17 


#define XHI_SYNC_PACKET             0xAA995566UL
#define XHI_DUMMY_PACKET            0xFFFFFFFFUL
#define XHI_NOOP_PACKET             (XHI_TYPE_1 << XHI_TYPE_SHIFT)
#define XHI_TYPE_2_READ ((XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
			(XHI_OP_READ << XHI_OP_SHIFT))

#define XHI_TYPE_2_WRITE ((XHI_TYPE_2 << XHI_TYPE_SHIFT) | \
			(XHI_OP_WRITE << XHI_OP_SHIFT))

#define XHI_TYPE2_CNT_MASK          0x07FFFFFF

#define XHI_TYPE_1_PACKET_MAX_WORDS 2047UL
#define XHI_TYPE_1_HEADER_BYTES     4
#define XHI_TYPE_2_HEADER_BYTES     8


#define XHI_DISABLED_AUTO_CRC       0x0000DEFCUL


#define XHI_SR_CFGERR_N_MASK 0x00000100 
#define XHI_SR_DALIGN_MASK 0x00000080 
#define XHI_SR_RIP_MASK 0x00000040 
#define XHI_SR_IN_ABORT_N_MASK 0x00000020 
#define XHI_SR_DONE_MASK 0x00000001 


static inline u32 hwicap_type_1_read(u32 reg)
{
	return (XHI_TYPE_1 << XHI_TYPE_SHIFT) |
		(reg << XHI_REGISTER_SHIFT) |
		(XHI_OP_READ << XHI_OP_SHIFT);
}


static inline u32 hwicap_type_1_write(u32 reg)
{
	return (XHI_TYPE_1 << XHI_TYPE_SHIFT) |
		(reg << XHI_REGISTER_SHIFT) |
		(XHI_OP_WRITE << XHI_OP_SHIFT);
}

#endif
