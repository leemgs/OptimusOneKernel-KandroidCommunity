
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/clk.h>
#include <linux/android_pmem.h>
#include <linux/msm_rotator.h>
#include <linux/io.h>
#include <mach/msm_rotator_imem.h>
#include <linux/ktime.h>
#include <linux/workqueue.h>
#include <linux/file.h>
#include <linux/major.h>

#define DRIVER_NAME "msm_rotator"

#define MSM_ROTATOR_BASE (msm_rotator_dev->io_base)
#define MSM_ROTATOR_INTR_ENABLE			(MSM_ROTATOR_BASE+0x0020)
#define MSM_ROTATOR_INTR_STATUS			(MSM_ROTATOR_BASE+0x0024)
#define MSM_ROTATOR_INTR_CLEAR			(MSM_ROTATOR_BASE+0x0028)
#define MSM_ROTATOR_START			(MSM_ROTATOR_BASE+0x0030)
#define MSM_ROTATOR_MAX_BURST_SIZE		(MSM_ROTATOR_BASE+0x0050)
#define MSM_ROTATOR_HW_VERSION			(MSM_ROTATOR_BASE+0x0070)
#define MSM_ROTATOR_SRC_SIZE			(MSM_ROTATOR_BASE+0x1108)
#define MSM_ROTATOR_SRCP0_ADDR			(MSM_ROTATOR_BASE+0x110c)
#define MSM_ROTATOR_SRCP1_ADDR			(MSM_ROTATOR_BASE+0x1110)
#define MSM_ROTATOR_SRC_YSTRIDE1		(MSM_ROTATOR_BASE+0x111c)
#define MSM_ROTATOR_SRC_YSTRIDE2		(MSM_ROTATOR_BASE+0x1120)
#define MSM_ROTATOR_SRC_FORMAT			(MSM_ROTATOR_BASE+0x1124)
#define MSM_ROTATOR_SRC_UNPACK_PATTERN1		(MSM_ROTATOR_BASE+0x1128)
#define MSM_ROTATOR_SUB_BLOCK_CFG		(MSM_ROTATOR_BASE+0x1138)
#define MSM_ROTATOR_OUT_PACK_PATTERN1		(MSM_ROTATOR_BASE+0x1154)
#define MSM_ROTATOR_OUTP0_ADDR			(MSM_ROTATOR_BASE+0x1168)
#define MSM_ROTATOR_OUTP1_ADDR			(MSM_ROTATOR_BASE+0x116c)
#define MSM_ROTATOR_OUT_YSTRIDE1		(MSM_ROTATOR_BASE+0x1178)
#define MSM_ROTATOR_OUT_YSTRIDE2		(MSM_ROTATOR_BASE+0x117c)
#define MSM_ROTATOR_SRC_XY			(MSM_ROTATOR_BASE+0x1200)
#define MSM_ROTATOR_SRC_IMAGE_SIZE		(MSM_ROTATOR_BASE+0x1208)

#define MSM_ROTATOR_HW_VERSION_VALUE 0x1000303

#define MSM_ROTATOR_MAX_ROT	0x07
#define MSM_ROTATOR_MAX_H	0x1fff
#define MSM_ROTATOR_MAX_W	0x1fff


#define GET_PACK_PATTERN(a, x, y, z, bit) \
			(((a)<<((bit)*3))|((x)<<((bit)*2))|((y)<<(bit))|(z))
#define CLR_G 0x0
#define CLR_B 0x1
#define CLR_R 0x2
#define CLR_ALPHA 0x3

#define CLR_Y  CLR_G
#define CLR_CB CLR_B
#define CLR_CR CLR_R

#define ROTATIONS_TO_BITMASK(r) ((((r) & MDP_ROT_90) ? 1 : 0)  | \
				 (((r) & MDP_FLIP_LR) ? 2 : 0) | \
				 (((r) & MDP_FLIP_UD) ? 4 : 0))

#define IMEM_NO_OWNER -1;

#define MAX_SESSIONS 16
#define INVALID_SESSION -1

struct tile_parm {
	unsigned int width;  
	unsigned int height; 
	unsigned int row_tile_w; 
	unsigned int row_tile_h; 
};

struct msm_rotator_dev {
	void __iomem *io_base;
	int irq;
	struct msm_rotator_img_info *img_info[MAX_SESSIONS];
	struct clk *pclk;
	struct clk *axi_clk;
	int rot_clk_state;
	struct delayed_work rot_clk_work;
	struct clk *imem_clk;
	int imem_clk_state;
	struct delayed_work imem_clk_work;
	struct platform_device *pdev;
	struct cdev cdev;
	struct device *device;
	struct class *class;
	dev_t dev_num;
	int processing;
	int last_session_idx;
	struct mutex rotator_lock;
	struct mutex imem_lock;
	int imem_owner;
	wait_queue_head_t wq;
};

#define chroma_addr(start, w, h, bpp) ((start) + ((h) * (w) * (bpp)))

#define COMPONENT_5BITS 1
#define COMPONENT_6BITS 2
#define COMPONENT_8BITS 3

static struct msm_rotator_dev *msm_rotator_dev;

enum {
	CLK_EN,
	CLK_DIS
};

int msm_rotator_imem_allocate(int requestor)
{
	int rc = 1;

#ifdef CONFIG_MSM_ROTATOR_USE_IMEM
	switch (requestor) {
	case ROTATOR_REQUEST:
		if (mutex_trylock(&msm_rotator_dev->imem_lock)) {
			msm_rotator_dev->imem_owner = ROTATOR_REQUEST;
			rc = 1;
		} else
			rc = 0;
		break;
	case JPEG_REQUEST:
		mutex_lock(&msm_rotator_dev->imem_lock);
		msm_rotator_dev->imem_owner = JPEG_REQUEST;
		rc = 1;
		break;
	default:
		rc = 0;
	}
#endif

	if (rc == 1) {
		cancel_delayed_work(&msm_rotator_dev->imem_clk_work);
		if (msm_rotator_dev->imem_clk_state == CLK_DIS) {
			clk_enable(msm_rotator_dev->imem_clk);
			msm_rotator_dev->imem_clk_state = CLK_EN;
		}
	}

	return rc;
}
EXPORT_SYMBOL(msm_rotator_imem_allocate);

void msm_rotator_imem_free(int requestor)
{
#ifdef CONFIG_MSM_ROTATOR_USE_IMEM
	if (msm_rotator_dev->imem_owner == requestor) {
		schedule_delayed_work(&msm_rotator_dev->imem_clk_work, HZ);
		mutex_unlock(&msm_rotator_dev->imem_lock);
	}
#else
	schedule_delayed_work(&msm_rotator_dev->imem_clk_work, HZ);
#endif
}
EXPORT_SYMBOL(msm_rotator_imem_free);

#ifdef CONFIG_MSM_ROTATOR_USE_IMEM
static void msm_rotator_imem_clk_work_f(struct work_struct *work)
{
	if (mutex_trylock(&msm_rotator_dev->imem_lock)) {
		if (msm_rotator_dev->imem_clk_state == CLK_EN) {
			clk_disable(msm_rotator_dev->imem_clk);
			msm_rotator_dev->imem_clk_state = CLK_DIS;
		}
		mutex_unlock(&msm_rotator_dev->imem_lock);
	}
}
#endif


static void enable_rot_clks(void)
{
	clk_enable(msm_rotator_dev->pclk);
	clk_enable(msm_rotator_dev->axi_clk);
}


static void disable_rot_clks(void)
{
	clk_disable(msm_rotator_dev->pclk);
	clk_disable(msm_rotator_dev->axi_clk);
}

static void msm_rotator_rot_clk_work_f(struct work_struct *work)
{
	if (mutex_trylock(&msm_rotator_dev->rotator_lock)) {
		if (msm_rotator_dev->rot_clk_state == CLK_EN) {
			disable_rot_clks();
			msm_rotator_dev->rot_clk_state = CLK_DIS;
		}
		mutex_unlock(&msm_rotator_dev->rotator_lock);
	}
}

static irqreturn_t msm_rotator_isr(int irq, void *dev_id)
{
	if (msm_rotator_dev->processing) {
		msm_rotator_dev->processing = 0;
		wake_up(&msm_rotator_dev->wq);
	} else
		printk(KERN_WARNING "%s: unexpected interrupt\n", DRIVER_NAME);

	return IRQ_HANDLED;
}

static int get_bpp(int format)
{
	switch (format) {
	case MDP_RGB_565:
	case MDP_BGR_565:
		return 2;

	case MDP_XRGB_8888:
	case MDP_ARGB_8888:
	case MDP_RGBA_8888:
	case MDP_BGRA_8888:
	case MDP_RGBX_8888:
		return 4;

	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CRCB_H2V2_TILE:
	case MDP_Y_CBCR_H2V2_TILE:
		return 1;

	case MDP_RGB_888:
		return 3;

	case MDP_YCRYCB_H2V1:
		return 2;

	case MDP_Y_CRCB_H2V1:
	case MDP_Y_CBCR_H2V1:
		return 1;

	default:
		return -1;
	}

}

static int msm_rotator_ycxcx_h2v1(struct msm_rotator_img_info *info,
				  unsigned int in_paddr,
				  unsigned int out_paddr,
				  unsigned int use_imem,
				  int new_session)
{
	int bpp;

	if (info->src.format != info->dst.format)
		return -EINVAL;

	bpp = get_bpp(info->src.format);
	if (bpp < 0)
		return -ENOTTY;

	iowrite32(in_paddr, MSM_ROTATOR_SRCP0_ADDR);
	iowrite32(chroma_addr(in_paddr, info->src.width, info->src.height, bpp),
		  MSM_ROTATOR_SRCP1_ADDR);
	iowrite32(out_paddr +
			((info->dst_y * info->dst.width) + info->dst_x),
		  MSM_ROTATOR_OUTP0_ADDR);
	iowrite32(chroma_addr(out_paddr, info->dst.width, info->dst.height,
			      bpp) +
			((info->dst_y * info->dst.width) + info->dst_x),
		  MSM_ROTATOR_OUTP1_ADDR);

	if (new_session) {
		iowrite32(info->src.width |
			  info->src.width << 16,
			  MSM_ROTATOR_SRC_YSTRIDE1);
		if (info->rotations & MDP_ROT_90)
			iowrite32(info->dst.width |
				  info->dst.width*2 << 16,
				  MSM_ROTATOR_OUT_YSTRIDE1);
		else
			iowrite32(info->dst.width |
				  info->dst.width << 16,
				  MSM_ROTATOR_OUT_YSTRIDE1);
		if (info->src.format == MDP_Y_CBCR_H2V1) {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		} else {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		}
		iowrite32((1  << 18) | 		
			  (ROTATIONS_TO_BITMASK(info->rotations) << 9) |
			  1 << 8,      		
			  MSM_ROTATOR_SUB_BLOCK_CFG);
		iowrite32(0 << 29 | 		
			  (use_imem ? 0 : 1) << 22 | 
			  2 << 19 | 		
			  0 << 18 | 		
			  1 << 17 | 		
			  1 << 13 | 		
			  (bpp-1) << 9 |	
			  0 << 8  | 		
			  0 << 6  | 		
			  3 << 4  | 		
			  3 << 2  | 		
			  3 << 0,   		
			  MSM_ROTATOR_SRC_FORMAT);
	}

	return 0;
}

static int msm_rotator_ycxcx_h2v2(struct msm_rotator_img_info *info,
				  unsigned int in_paddr,
				  unsigned int out_paddr,
				  unsigned int use_imem,
				  int new_session)
{
	int bpp;

	if (info->src.format != info->dst.format)
		return -EINVAL;

	bpp = get_bpp(info->src.format);
	if (bpp < 0)
		return -ENOTTY;

	iowrite32(in_paddr, MSM_ROTATOR_SRCP0_ADDR);
	iowrite32(chroma_addr(in_paddr, info->src.width, info->src.height, bpp),
		  MSM_ROTATOR_SRCP1_ADDR);
	iowrite32(out_paddr +
			((info->dst_y * info->dst.width) + info->dst_x),
		  MSM_ROTATOR_OUTP0_ADDR);
	iowrite32(chroma_addr(out_paddr, info->dst.width, info->dst.height,
			      bpp) +
			((info->dst_y * info->dst.width)/2 + info->dst_x),
		  MSM_ROTATOR_OUTP1_ADDR);

	if (new_session) {
		iowrite32(info->src.width |
			  info->src.width << 16,
			  MSM_ROTATOR_SRC_YSTRIDE1);
		iowrite32(info->dst.width |
			  info->dst.width << 16,
			  MSM_ROTATOR_OUT_YSTRIDE1);
		if (info->src.format == MDP_Y_CBCR_H2V2) {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		} else {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		}
		iowrite32((3  << 18) | 		
			  (ROTATIONS_TO_BITMASK(info->rotations) << 9) |
			  1 << 8,      		
			  MSM_ROTATOR_SUB_BLOCK_CFG);
		iowrite32(0 << 29 | 		
			  (use_imem ? 0 : 1) << 22 | 
			  2 << 19 | 		
			  0 << 18 | 		
			  1 << 17 | 		
			  1 << 13 | 		
			  (bpp-1) << 9  |	
			  0 << 8  | 		
			  0 << 6  | 		
			  3 << 4  | 		
			  3 << 2  | 		
			  3 << 0,   		
			  MSM_ROTATOR_SRC_FORMAT);
	}
	return 0;
}

static unsigned int tile_size(unsigned int src_width,
		unsigned int src_height,
		const struct tile_parm *tp)
{
	unsigned int tile_w, tile_h;
	unsigned int row_num_w, row_num_h;
	tile_w = tp->width * tp->row_tile_w;
	tile_h = tp->height * tp->row_tile_h;
	row_num_w = (src_width + tile_w - 1) / tile_w;
	row_num_h = (src_height + tile_h - 1) / tile_h;
	return ((row_num_w * row_num_h * tile_w * tile_h) + 8191) & ~8191;
}

static int msm_rotator_ycxcx_h2v2_tile(struct msm_rotator_img_info *info,
				  unsigned int in_paddr,
				  unsigned int out_paddr,
				  unsigned int use_imem,
				  int new_session)
{
	int bpp;
	unsigned int offset = 0;
	
	const struct tile_parm tile = {64, 32, 2, 1};
	if ((info->src.format == MDP_Y_CRCB_H2V2_TILE &&
		info->dst.format != MDP_Y_CRCB_H2V2) ||
		(info->src.format == MDP_Y_CBCR_H2V2_TILE &&
		info->dst.format != MDP_Y_CBCR_H2V2))
		return -EINVAL;

	bpp = get_bpp(info->src.format);
	if (bpp < 0)
		return -ENOTTY;

	offset = tile_size(info->src.width, info->src.height, &tile);

	iowrite32(in_paddr, MSM_ROTATOR_SRCP0_ADDR);
	iowrite32(in_paddr + offset, MSM_ROTATOR_SRCP1_ADDR);
	iowrite32(out_paddr +
			((info->dst_y * info->dst.width) + info->dst_x),
		  MSM_ROTATOR_OUTP0_ADDR);
	iowrite32(chroma_addr(out_paddr, info->dst.width, info->dst.height,
			      bpp) +
			((info->dst_y * info->dst.width)/2 + info->dst_x),
		  MSM_ROTATOR_OUTP1_ADDR);

	if (new_session) {
		iowrite32(info->src.width |
			  info->src.width << 16,
			  MSM_ROTATOR_SRC_YSTRIDE1);

		iowrite32(info->dst.width |
			  info->dst.width << 16,
			  MSM_ROTATOR_OUT_YSTRIDE1);
		if (info->src.format == MDP_Y_CBCR_H2V2_TILE) {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CB, CLR_CR, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		} else {
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, 0, CLR_CR, CLR_CB, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
		}
		iowrite32((3  << 18) | 		
			  (ROTATIONS_TO_BITMASK(info->rotations) << 9) |
			  1 << 8,      		
			  MSM_ROTATOR_SUB_BLOCK_CFG);
		iowrite32(2 << 29 | 		
			  (use_imem ? 0 : 1) << 22 | 
			  2 << 19 | 		
			  0 << 18 | 		
			  1 << 17 | 		
			  1 << 13 | 		
			  (bpp-1) << 9  |	
			  0 << 8  | 		
			  0 << 6  | 		
			  3 << 4  | 		
			  3 << 2  | 		
			  3 << 0,   		
			  MSM_ROTATOR_SRC_FORMAT);
	}
	return 0;
}

static int msm_rotator_ycrycb(struct msm_rotator_img_info *info,
			      unsigned int in_paddr,
			      unsigned int out_paddr,
			      unsigned int use_imem,
			      int new_session)
{
	int bpp;

	if (info->src.format != info->dst.format)
		return -EINVAL;

	bpp = get_bpp(info->src.format);
	if (bpp < 0)
		return -ENOTTY;

	iowrite32(in_paddr, MSM_ROTATOR_SRCP0_ADDR);
	iowrite32(out_paddr +
			((info->dst_y * info->dst.width) + info->dst_x),
		  MSM_ROTATOR_OUTP0_ADDR);

	if (new_session) {
		iowrite32(info->src.width,
			  MSM_ROTATOR_SRC_YSTRIDE1);
		iowrite32(info->dst.width,
			  MSM_ROTATOR_OUT_YSTRIDE1);
		iowrite32(GET_PACK_PATTERN(CLR_Y, CLR_CR, CLR_Y, CLR_CB, 8),
			  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
		iowrite32(GET_PACK_PATTERN(CLR_Y, CLR_CR, CLR_Y, CLR_CB, 8),
			  MSM_ROTATOR_OUT_PACK_PATTERN1);
		iowrite32((1  << 18) | 		
			  (ROTATIONS_TO_BITMASK(info->rotations) << 9) |
			  1 << 8,      		
			  MSM_ROTATOR_SUB_BLOCK_CFG);
		iowrite32(0 << 29 | 		
			  (use_imem ? 0 : 1) << 22 | 
			  0 << 19 | 		
			  0 << 18 | 		
			  1 << 17 | 		
			  3 << 13 | 		
			  (bpp-1) << 9 |	
			  0 << 8  | 		
			  0 << 6  | 		
			  3 << 4  | 		
			  3 << 2  | 		
			  3 << 0,   		
			  MSM_ROTATOR_SRC_FORMAT);
	}

	return 0;
}

static int msm_rotator_rgb_types(struct msm_rotator_img_info *info,
				 unsigned int in_paddr,
				 unsigned int out_paddr,
				 unsigned int use_imem,
				 int new_session)
{
	int bpp, abits, rbits, gbits, bbits;

	if (info->src.format != info->dst.format)
		return -EINVAL;

	bpp = get_bpp(info->src.format);
	if (bpp < 0)
		return -ENOTTY;

	iowrite32(in_paddr, MSM_ROTATOR_SRCP0_ADDR);
	iowrite32(out_paddr +
			((info->dst_y * info->dst.width) + info->dst_x) * bpp,
		  MSM_ROTATOR_OUTP0_ADDR);

	if (new_session) {
		iowrite32(info->src.width * bpp, MSM_ROTATOR_SRC_YSTRIDE1);
		iowrite32(info->dst.width * bpp, MSM_ROTATOR_OUT_YSTRIDE1);
		iowrite32((0  << 18) | 		
			  (ROTATIONS_TO_BITMASK(info->rotations) << 9) |
			  1 << 8,      		
			  MSM_ROTATOR_SUB_BLOCK_CFG);
		switch (info->src.format) {
		case MDP_RGB_565:
			iowrite32(GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
			abits = 0;
			rbits = COMPONENT_5BITS;
			gbits = COMPONENT_6BITS;
			bbits = COMPONENT_5BITS;
			break;

		case MDP_BGR_565:
			iowrite32(GET_PACK_PATTERN(0, CLR_B, CLR_G, CLR_R, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, CLR_B, CLR_G, CLR_R, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
			abits = 0;
			rbits = COMPONENT_5BITS;
			gbits = COMPONENT_6BITS;
			bbits = COMPONENT_5BITS;
			break;

		case MDP_RGB_888:
			iowrite32(GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(0, CLR_R, CLR_G, CLR_B, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
			abits = 0;
			rbits = COMPONENT_8BITS;
			gbits = COMPONENT_8BITS;
			bbits = COMPONENT_8BITS;
			break;

		case MDP_ARGB_8888:
		case MDP_RGBA_8888:
		case MDP_XRGB_8888:
		case MDP_RGBX_8888:
			iowrite32(GET_PACK_PATTERN(CLR_ALPHA, CLR_R, CLR_G,
						   CLR_B, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(CLR_ALPHA, CLR_R, CLR_G,
						   CLR_B, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
			abits = COMPONENT_8BITS;
			rbits = COMPONENT_8BITS;
			gbits = COMPONENT_8BITS;
			bbits = COMPONENT_8BITS;
			break;

		case MDP_BGRA_8888:
			iowrite32(GET_PACK_PATTERN(CLR_ALPHA, CLR_B, CLR_G,
						   CLR_R, 8),
				  MSM_ROTATOR_SRC_UNPACK_PATTERN1);
			iowrite32(GET_PACK_PATTERN(CLR_ALPHA, CLR_B, CLR_G,
						   CLR_R, 8),
				  MSM_ROTATOR_OUT_PACK_PATTERN1);
			abits = COMPONENT_8BITS;
			rbits = COMPONENT_8BITS;
			gbits = COMPONENT_8BITS;
			bbits = COMPONENT_8BITS;
			break;

		default:
			return -EINVAL;
		}
		iowrite32(0 << 29 | 		
			  (use_imem ? 0 : 1) << 22 | 
			  0 << 19 | 		
			  0 << 18 | 		
			  1 << 17 | 		
			  (abits ? 3 : 2) << 13 | 
			  (bpp-1) << 9 | 	
			  (abits ? 1 : 0) << 8  | 
			  abits << 6  | 	
			  rbits << 4  | 	
			  bbits << 2  | 	
			  gbits << 0,   	
			  MSM_ROTATOR_SRC_FORMAT);
	}

	return 0;
}

static int get_img(int memory_id, unsigned long *start, unsigned long *len,
		struct file **pp_file)
{
	int put_needed, ret = 0, fb_num;
	struct file *file;
#ifdef CONFIG_ANDROID_PMEM
	unsigned long vstart;
#endif

#ifdef CONFIG_ANDROID_PMEM
	if (!get_pmem_file(memory_id, start, &vstart, len, pp_file))
		return 0;
#endif
	file = fget_light(memory_id, &put_needed);
	if (file == NULL)
		return -1;

	if (MAJOR(file->f_dentry->d_inode->i_rdev) == FB_MAJOR) {
		fb_num = MINOR(file->f_dentry->d_inode->i_rdev);
		if (get_fb_phys_info(start, len, fb_num))
			ret = -1;
		else
			*pp_file = file;
	} else
		ret = -1;
	if (ret)
		fput_light(file, put_needed);
	return ret;
}

static int msm_rotator_do_rotate(unsigned long arg)
{
	int rc = 0;
	unsigned int status;
	struct msm_rotator_data_info info;
	unsigned int in_paddr, out_paddr;
	unsigned long len;
	struct file *src_file = 0;
	struct file *dst_file = 0;
	int use_imem = 0;
	int s;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
		return -EFAULT;

	rc = get_img(info.src.memory_id, (unsigned long *)&in_paddr,
			(unsigned long *)&len, &src_file);
	if (rc) {
		printk(KERN_ERR "%s: in get_img() failed id=0x%08x\n",
		       DRIVER_NAME, info.src.memory_id);
		return rc;
	}
	in_paddr += info.src.offset;

	rc = get_img(info.dst.memory_id, (unsigned long *)&out_paddr,
			(unsigned long *)&len, &dst_file);
	if (rc) {
		printk(KERN_ERR "%s: out get_img() failed id=0x%08x\n",
		       DRIVER_NAME, info.dst.memory_id);
		return rc;
	}
	out_paddr += info.dst.offset;

	mutex_lock(&msm_rotator_dev->rotator_lock);
	for (s = 0; s < MAX_SESSIONS; s++)
		if ((msm_rotator_dev->img_info[s] != NULL) &&
			(info.session_id ==
			(unsigned int)msm_rotator_dev->img_info[s]
			))
			break;

	if (s == MAX_SESSIONS) {
		dev_dbg(msm_rotator_dev->device,
			"%s() : Attempt to use invalid session_id %d\n",
			__func__, s);
		rc = -EINVAL;
		goto do_rotate_unlock_mutex;
	}

	if (msm_rotator_dev->img_info[s]->enable == 0) {
		dev_dbg(msm_rotator_dev->device,
			"%s() : Session_id %d not enabled \n",
			__func__, s);
		rc = -EINVAL;
		goto do_rotate_unlock_mutex;
	}

	cancel_delayed_work(&msm_rotator_dev->rot_clk_work);
	if (msm_rotator_dev->rot_clk_state == CLK_DIS) {
		enable_rot_clks();
		msm_rotator_dev->rot_clk_state = CLK_EN;
	}
	enable_irq(msm_rotator_dev->irq);

#ifdef CONFIG_MSM_ROTATOR_USE_IMEM
	use_imem = msm_rotator_imem_allocate(ROTATOR_REQUEST);
#else
	use_imem = 0;
#endif

	iowrite32(((msm_rotator_dev->img_info[s]->src_rect.h & 0x1fff)
				<< 16) |
		  (msm_rotator_dev->img_info[s]->src_rect.w & 0x1fff),
		  MSM_ROTATOR_SRC_SIZE);
	iowrite32(((msm_rotator_dev->img_info[s]->src_rect.y & 0x1fff)
				<< 16) |
		  (msm_rotator_dev->img_info[s]->src_rect.x & 0x1fff),
		  MSM_ROTATOR_SRC_XY);
	iowrite32(((msm_rotator_dev->img_info[s]->src.height & 0x1fff)
				<< 16) |
		  (msm_rotator_dev->img_info[s]->src.width & 0x1fff),
		  MSM_ROTATOR_SRC_IMAGE_SIZE);

	switch (msm_rotator_dev->img_info[s]->src.format) {
	case MDP_RGB_565:
	case MDP_BGR_565:
	case MDP_RGB_888:
	case MDP_ARGB_8888:
	case MDP_RGBA_8888:
	case MDP_XRGB_8888:
	case MDP_BGRA_8888:
	case MDP_RGBX_8888:
		rc = msm_rotator_rgb_types(msm_rotator_dev->img_info[s],
					   in_paddr, out_paddr,
					   use_imem,
					   msm_rotator_dev->last_session_idx
								!= s);
		break;
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CRCB_H2V2:
		rc = msm_rotator_ycxcx_h2v2(msm_rotator_dev->img_info[s],
					    in_paddr, out_paddr, use_imem,
					    msm_rotator_dev->last_session_idx
								!= s);
		break;
	case MDP_Y_CRCB_H2V2_TILE:
	case MDP_Y_CBCR_H2V2_TILE:
		rc = msm_rotator_ycxcx_h2v2_tile(msm_rotator_dev->img_info[s],
				in_paddr, out_paddr, use_imem,
				msm_rotator_dev->last_session_idx
				!= s);
	break;

	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V1:
		rc = msm_rotator_ycxcx_h2v1(msm_rotator_dev->img_info[s],
					    in_paddr, out_paddr, use_imem,
					    msm_rotator_dev->last_session_idx
								!= s);
		break;
	case MDP_YCRYCB_H2V1:
		rc = msm_rotator_ycrycb(msm_rotator_dev->img_info[s],
				in_paddr, out_paddr, use_imem,
				msm_rotator_dev->last_session_idx != s);
		break;
	default:
		rc = -EINVAL;
		goto do_rotate_exit;
	}

	if (rc != 0) {
		msm_rotator_dev->last_session_idx = INVALID_SESSION;
		goto do_rotate_exit;
	}

	iowrite32(3, MSM_ROTATOR_INTR_ENABLE);

	msm_rotator_dev->processing = 1;
	iowrite32(0x1, MSM_ROTATOR_START);

	wait_event(msm_rotator_dev->wq,
		   (msm_rotator_dev->processing == 0));
	status = (unsigned char)ioread32(MSM_ROTATOR_INTR_STATUS);
	if ((status & 0x03) != 0x01)
		rc = -EFAULT;
	iowrite32(0, MSM_ROTATOR_INTR_ENABLE);
	iowrite32(3, MSM_ROTATOR_INTR_CLEAR);

do_rotate_exit:
	disable_irq(msm_rotator_dev->irq);
	msm_rotator_imem_free(ROTATOR_REQUEST);
	schedule_delayed_work(&msm_rotator_dev->rot_clk_work, HZ);
do_rotate_unlock_mutex:
	if (src_file)
		put_pmem_file(src_file);
	if (dst_file)
		put_pmem_file(dst_file);
	mutex_unlock(&msm_rotator_dev->rotator_lock);
	dev_dbg(msm_rotator_dev->device, "%s() returning rc = %d\n",
		__func__, rc);
	return rc;
}

static int msm_rotator_start(unsigned long arg)
{
	struct msm_rotator_img_info info;
	int rc = 0;
	int s;
	int first_free_index = INVALID_SESSION;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
		return -EFAULT;

	if ((info.rotations > MSM_ROTATOR_MAX_ROT) ||
	    (info.src.height > MSM_ROTATOR_MAX_H) ||
	    (info.src.width > MSM_ROTATOR_MAX_W) ||
	    (info.dst.height > MSM_ROTATOR_MAX_H) ||
	    (info.dst.width > MSM_ROTATOR_MAX_W) ||
	    ((info.src_rect.x + info.src_rect.w) > info.src.width) ||
	    ((info.src_rect.y + info.src_rect.h) > info.src.height) ||
	    ((info.rotations & MDP_ROT_90) &&
		((info.dst_x + info.src_rect.h) > info.dst.width)) ||
	    ((info.rotations & MDP_ROT_90) &&
		((info.dst_y + info.src_rect.w) > info.dst.height)) ||
	    (!(info.rotations & MDP_ROT_90) &&
		((info.dst_x + info.src_rect.w) > info.dst.width)) ||
	    (!(info.rotations & MDP_ROT_90) &&
		((info.dst_y + info.src_rect.h) > info.dst.height)))
		return -EINVAL;

	switch (info.src.format) {
	case MDP_RGB_565:
	case MDP_BGR_565:
	case MDP_RGB_888:
	case MDP_ARGB_8888:
	case MDP_RGBA_8888:
	case MDP_XRGB_8888:
	case MDP_RGBX_8888:
	case MDP_BGRA_8888:
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V1:
	case MDP_YCRYCB_H2V1:
	case MDP_Y_CRCB_H2V2_TILE:
	case MDP_Y_CBCR_H2V2_TILE:
		break;
	default:
		return -EINVAL;
	}

	switch (info.dst.format) {
	case MDP_RGB_565:
	case MDP_BGR_565:
	case MDP_RGB_888:
	case MDP_ARGB_8888:
	case MDP_RGBA_8888:
	case MDP_XRGB_8888:
	case MDP_BGRA_8888:
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V1:
	case MDP_YCRYCB_H2V1:
		break;
	default:
		return -EINVAL;
	}

	mutex_lock(&msm_rotator_dev->rotator_lock);
	for (s = 0; s < MAX_SESSIONS; s++) {
		if ((msm_rotator_dev->img_info[s] != NULL) &&
			(info.session_id ==
			(unsigned int)msm_rotator_dev->img_info[s]
			)) {
			*(msm_rotator_dev->img_info[s]) = info;

			if (msm_rotator_dev->last_session_idx == s)
				msm_rotator_dev->last_session_idx =
				INVALID_SESSION;
			break;
		}

		if ((msm_rotator_dev->img_info[s] == NULL) &&
			(first_free_index ==
			INVALID_SESSION))
			first_free_index = s;
	}

	if ((s == MAX_SESSIONS) && (first_free_index != INVALID_SESSION)) {
		
		msm_rotator_dev->img_info[first_free_index] =
			kzalloc(sizeof(struct msm_rotator_img_info),
					GFP_KERNEL);
		if (!msm_rotator_dev->img_info[first_free_index]) {
			printk(KERN_ERR "%s : unable to alloc mem\n",
					__func__);
			rc = -ENOMEM;
			goto rotator_start_exit;
		}
		info.session_id = (unsigned int)
			msm_rotator_dev->img_info[first_free_index];
		*(msm_rotator_dev->img_info[first_free_index]) = info;

		if (copy_to_user((void __user *)arg, &info, sizeof(info)))
			rc = -EFAULT;
	} else if (s == MAX_SESSIONS) {
		dev_dbg(msm_rotator_dev->device, "%s: all sessions in use\n",
			__func__);
		rc = -EBUSY;
	}

rotator_start_exit:
	mutex_unlock(&msm_rotator_dev->rotator_lock);

	return rc;
}

static int msm_rotator_finish(unsigned long arg)
{
	int rc = 0;
	int s;
	unsigned int session_id;

	if (copy_from_user(&session_id, (void __user *)arg, sizeof(s)))
		return -EFAULT;

	mutex_lock(&msm_rotator_dev->rotator_lock);
	for (s = 0; s < MAX_SESSIONS; s++) {
		if ((msm_rotator_dev->img_info[s] != NULL) &&
			(session_id ==
			(unsigned int)msm_rotator_dev->img_info[s])) {
			if (msm_rotator_dev->last_session_idx == s)
				msm_rotator_dev->last_session_idx =
					INVALID_SESSION;
			kfree(msm_rotator_dev->img_info[s]);
			msm_rotator_dev->img_info[s] = NULL;
			break;
		}
	}

	if (s == MAX_SESSIONS)
		rc = -EINVAL;
	mutex_unlock(&msm_rotator_dev->rotator_lock);
	return rc;
}

static int msm_rotator_ioctl(struct inode *inode, struct file *file,
			     unsigned cmd, unsigned long arg)
{
	if (_IOC_TYPE(cmd) != MSM_ROTATOR_IOCTL_MAGIC)
		return -ENOTTY;

	switch (cmd) {
	case MSM_ROTATOR_IOCTL_START:
		return msm_rotator_start(arg);
	case MSM_ROTATOR_IOCTL_ROTATE:
		return msm_rotator_do_rotate(arg);
	case MSM_ROTATOR_IOCTL_FINISH:
		return msm_rotator_finish(arg);

	default:
		dev_dbg(msm_rotator_dev->device,
			"unexpected IOCTL %d\n", cmd);
		return -ENOTTY;
	}
}

static const struct file_operations msm_rotator_fops = {
	.owner = THIS_MODULE,
	.ioctl = msm_rotator_ioctl,
};

static int __devinit msm_rotator_probe(struct platform_device *pdev)
{
	int rc = 0;
	struct resource *res;
	int i;
	uint32_t ver;

	msm_rotator_dev = kzalloc(sizeof(struct msm_rotator_dev), GFP_KERNEL);
	if (!msm_rotator_dev) {
		printk(KERN_ERR "%s Unable to allocate memory for struct\n",
		       __func__);
		return -ENOMEM;
	}
	for (i = 0; i < MAX_SESSIONS; i++)
		msm_rotator_dev->img_info[i] = NULL;
	msm_rotator_dev->last_session_idx = INVALID_SESSION;

	msm_rotator_dev->imem_owner = IMEM_NO_OWNER;
	mutex_init(&msm_rotator_dev->imem_lock);

	msm_rotator_dev->imem_clk_state = CLK_DIS;
	INIT_DELAYED_WORK(&msm_rotator_dev->imem_clk_work,
			  msm_rotator_imem_clk_work_f);
	msm_rotator_dev->imem_clk =
		clk_get(&msm_rotator_dev->pdev->dev, "rotator_imem_clk");
	if (IS_ERR(msm_rotator_dev->imem_clk)) {
		rc = PTR_ERR(msm_rotator_dev->imem_clk);
		msm_rotator_dev->imem_clk = NULL;
		printk(KERN_ERR "%s: cannot get imem_clk rc=%d\n",
		       DRIVER_NAME, rc);
		goto error_imem_clk;
	}

	msm_rotator_dev->pclk =
		clk_get(&msm_rotator_dev->pdev->dev, "rotator_pclk");
	if (IS_ERR(msm_rotator_dev->pclk)) {
		rc = PTR_ERR(msm_rotator_dev->pclk);
		msm_rotator_dev->pclk = NULL;
		printk(KERN_ERR "%s: cannot get pclk rc=%d\n",
		       DRIVER_NAME, rc);
		goto error_pclk;
	}
	msm_rotator_dev->axi_clk =
		clk_get(&msm_rotator_dev->pdev->dev, "rotator_clk");
	if (IS_ERR(msm_rotator_dev->axi_clk)) {
		rc = PTR_ERR(msm_rotator_dev->axi_clk);
		msm_rotator_dev->axi_clk = NULL;
		printk(KERN_ERR "%s: cannot get axi clk rc=%d\n",
		       DRIVER_NAME, rc);
		goto error_axi_clk;
	}
	msm_rotator_dev->rot_clk_state = CLK_DIS;
	INIT_DELAYED_WORK(&msm_rotator_dev->rot_clk_work,
			  msm_rotator_rot_clk_work_f);

	mutex_init(&msm_rotator_dev->rotator_lock);

	msm_rotator_dev->pdev = pdev;

	platform_set_drvdata(pdev, msm_rotator_dev);


	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk(KERN_ALERT
		       "%s: could not get IORESOURCE_MEM\n", DRIVER_NAME);
		rc = -ENODEV;
		goto error_get_resource;
	}
	msm_rotator_dev->io_base = ioremap(res->start,
					   resource_size(res));
	clk_enable(msm_rotator_dev->imem_clk);
	enable_rot_clks();
	ver = ioread32(MSM_ROTATOR_HW_VERSION);
	disable_rot_clks();
	clk_disable(msm_rotator_dev->imem_clk);
	if (ver != MSM_ROTATOR_HW_VERSION_VALUE) {
		printk(KERN_ALERT "%s: invalid HW version\n", DRIVER_NAME);
		rc = -ENODEV;
		goto error_get_resource;
	}
	msm_rotator_dev->irq = platform_get_irq(pdev, 0);
	if (msm_rotator_dev->irq < 0) {
		printk(KERN_ALERT "%s: could not get IORESOURCE_IRQ\n",
		       DRIVER_NAME);
		rc = -ENODEV;
		goto error_get_irq;
	}
	rc = request_irq(msm_rotator_dev->irq, msm_rotator_isr,
			 IRQF_TRIGGER_RISING, DRIVER_NAME, NULL);
	if (rc) {
		printk(KERN_ERR "%s: request_irq() failed\n", DRIVER_NAME);
		goto error_get_irq;
	}
	
	disable_irq(msm_rotator_dev->irq);

	rc = alloc_chrdev_region(&msm_rotator_dev->dev_num, 0, 1, DRIVER_NAME);
	if (rc < 0) {
		printk(KERN_ERR "%s: alloc_chrdev_region Failed rc = %d\n",
		       __func__, rc);
		goto error_get_irq;
	}

	msm_rotator_dev->class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(msm_rotator_dev->class)) {
		rc = PTR_ERR(msm_rotator_dev->class);
		printk(KERN_ERR "%s: couldn't create class rc = %d\n",
		       DRIVER_NAME, rc);
		goto error_class_create;
	}

	msm_rotator_dev->device = device_create(msm_rotator_dev->class, NULL,
						msm_rotator_dev->dev_num, NULL,
						DRIVER_NAME);
	if (IS_ERR(msm_rotator_dev->device)) {
		rc = PTR_ERR(msm_rotator_dev->device);
		printk(KERN_ERR "%s: device_create failed %d\n",
		       DRIVER_NAME, rc);
		goto error_class_device_create;
	}

	cdev_init(&msm_rotator_dev->cdev, &msm_rotator_fops);
	rc = cdev_add(&msm_rotator_dev->cdev,
		      MKDEV(MAJOR(msm_rotator_dev->dev_num), 0),
		      1);
	if (rc < 0) {
		printk(KERN_ERR "%s: cdev_add failed %d\n", __func__, rc);
		goto error_cdev_add;
	}

	init_waitqueue_head(&msm_rotator_dev->wq);

	iowrite32(0x42, MSM_ROTATOR_MAX_BURST_SIZE);

	dev_dbg(msm_rotator_dev->device, "probe successful\n");
	return rc;

error_cdev_add:
	device_destroy(msm_rotator_dev->class, msm_rotator_dev->dev_num);
error_class_device_create:
	class_destroy(msm_rotator_dev->class);
error_class_create:
	unregister_chrdev_region(msm_rotator_dev->dev_num, 1);
error_get_irq:
	iounmap(msm_rotator_dev->io_base);
error_get_resource:
	mutex_destroy(&msm_rotator_dev->rotator_lock);
	clk_put(msm_rotator_dev->axi_clk);
error_axi_clk:
	clk_put(msm_rotator_dev->pclk);
error_pclk:
	clk_put(msm_rotator_dev->imem_clk);
error_imem_clk:
	mutex_destroy(&msm_rotator_dev->imem_lock);
	kfree(msm_rotator_dev);

	return rc;
}

static int __devexit msm_rotator_remove(struct platform_device *plat_dev)
{
	int i;

	free_irq(msm_rotator_dev->irq, NULL);
	mutex_destroy(&msm_rotator_dev->rotator_lock);
	cdev_del(&msm_rotator_dev->cdev);
	device_destroy(msm_rotator_dev->class, msm_rotator_dev->dev_num);
	class_destroy(msm_rotator_dev->class);
	unregister_chrdev_region(msm_rotator_dev->dev_num, 1);
	iounmap(msm_rotator_dev->io_base);
	if (msm_rotator_dev->imem_clk_state == CLK_EN)
		clk_disable(msm_rotator_dev->imem_clk);
	clk_put(msm_rotator_dev->imem_clk);
	msm_rotator_dev->imem_clk = NULL;
	if (msm_rotator_dev->rot_clk_state == CLK_EN)
		disable_rot_clks();
	clk_put(msm_rotator_dev->pclk);
	clk_put(msm_rotator_dev->axi_clk);
	msm_rotator_dev->pclk = NULL;
	msm_rotator_dev->axi_clk = NULL;
	mutex_destroy(&msm_rotator_dev->imem_lock);
	for (i = 0; i < MAX_SESSIONS; i++)
		if (msm_rotator_dev->img_info[i] != NULL)
			kfree(msm_rotator_dev->img_info[i]);
	kfree(msm_rotator_dev);
	return 0;
}

#ifdef CONFIG_PM
static int msm_rotator_suspend(struct platform_device *dev, pm_message_t state)
{
	if (msm_rotator_dev->imem_clk_state == CLK_EN)
		clk_disable(msm_rotator_dev->imem_clk);
	if (msm_rotator_dev->rot_clk_state == CLK_EN)
		disable_rot_clks();
	return 0;
}

static int msm_rotator_resume(struct platform_device *dev)
{
	if (msm_rotator_dev->imem_clk_state == CLK_EN)
		clk_enable(msm_rotator_dev->imem_clk);
	if (msm_rotator_dev->rot_clk_state == CLK_EN)
		enable_rot_clks();
	return 0;
}
#endif

static struct platform_driver msm_rotator_platform_driver = {
	.probe = msm_rotator_probe,
	.remove = __devexit_p(msm_rotator_remove),
#ifdef CONFIG_PM
	.suspend = msm_rotator_suspend,
	.resume = msm_rotator_resume,
#endif
	.driver = {
		.owner = THIS_MODULE,
		.name = DRIVER_NAME
	}
};

static int __init msm_rotator_init(void)
{
	return platform_driver_register(&msm_rotator_platform_driver);
}

static void __exit msm_rotator_exit(void)
{
	return platform_driver_unregister(&msm_rotator_platform_driver);
}

module_init(msm_rotator_init);
module_exit(msm_rotator_exit);

MODULE_DESCRIPTION("MSM Offline Image Rotator driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL v2");
