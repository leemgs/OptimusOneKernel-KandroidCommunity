

#ifndef __FSL_DIU_FB_H__
#define __FSL_DIU_FB_H__


#define MEM_ALLOC_THRESHOLD (1024*768*4+32)

#define MIN_PIX_CLK 5629
#define MAX_PIX_CLK 96096

#include <linux/types.h>

struct mfb_alpha {
	int enable;
	int alpha;
};

struct mfb_chroma_key {
	int enable;
	__u8  red_max;
	__u8  green_max;
	__u8  blue_max;
	__u8  red_min;
	__u8  green_min;
	__u8  blue_min;
};

struct aoi_display_offset {
	int x_aoi_d;
	int y_aoi_d;
};

#define MFB_SET_CHROMA_KEY	_IOW('M', 1, struct mfb_chroma_key)
#define MFB_WAIT_FOR_VSYNC	_IOW('F', 0x20, u_int32_t)
#define MFB_SET_BRIGHTNESS	_IOW('M', 3, __u8)

#define MFB_SET_ALPHA		0x80014d00
#define MFB_GET_ALPHA		0x40014d00
#define MFB_SET_AOID		0x80084d04
#define MFB_GET_AOID		0x40084d04
#define MFB_SET_PIXFMT		0x80014d08
#define MFB_GET_PIXFMT		0x40014d08

#define FBIOGET_GWINFO		0x46E0
#define FBIOPUT_GWINFO		0x46E1

#ifdef __KERNEL__
#include <linux/spinlock.h>


struct diu_ad {
	










	__be32 pix_fmt; 

	
	__le32 addr;

	





	__le32 src_size_g_alpha;

	





	__le32 aoi_size;

	
	
	__le32 offset_xyi;

	
	
	__le32 offset_xyd;


	
	__u8 ckmax_r;
	__u8 ckmax_g;
	__u8 ckmax_b;
	__u8 res9;

	
	__u8 ckmin_r;
	__u8 ckmin_g;
	__u8 ckmin_b;
	__u8 res10;


	
	__le32 next_ad;

	
	__u32 paddr;
} __attribute__ ((packed));


struct diu {
	__be32 desc[3];
	__be32 gamma;
	__be32 pallete;
	__be32 cursor;
	__be32 curs_pos;
	__be32 diu_mode;
	__be32 bgnd;
	__be32 bgnd_wb;
	__be32 disp_size;
	__be32 wb_size;
	__be32 wb_mem_addr;
	__be32 hsyn_para;
	__be32 vsyn_para;
	__be32 syn_pol;
	__be32 thresholds;
	__be32 int_status;
	__be32 int_mask;
	__be32 colorbar[8];
	__be32 filling;
	__be32 plut;
} __attribute__ ((packed));

struct diu_hw {
	struct diu *diu_reg;
	spinlock_t reg_lock;

	__u32 mode;		
};

struct diu_addr {
	__u8 __iomem *vaddr;	
	dma_addr_t paddr;	
	__u32 	   offset;
};

struct diu_pool {
	struct diu_addr ad;
	struct diu_addr gamma;
	struct diu_addr pallete;
	struct diu_addr cursor;
};

#define FSL_DIU_BASE_OFFSET	0x2C000	
#define INT_LCDC		64	

#define FSL_AOI_NUM	6	
				


#define MIN_XRES	64
#define MIN_YRES	64


#define MAX_CURS		32


#define MFB_MODE0	0	
#define MFB_MODE1	1	
#define MFB_MODE2	2	
#define MFB_MODE3	3	
#define MFB_MODE4	4	


#define INT_VSYNC	0x01	
#define INT_VSYNC_WB	0x02	
#define INT_UNDRUN	0x04	
#define INT_PARERR	0x08	
#define INT_LS_BF_VS	0x10	


#define MFB_TYPE_OUTPUT	0	
#define MFB_TYPE_OFF	1	
#define MFB_TYPE_WB	2	
#define MFB_TYPE_TEST	3	

#endif 
#endif 
