
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/videodev2.h>
#include <media/davinci/dm644x_ccdc.h>
#include <media/davinci/vpss.h>
#include "dm644x_ccdc_regs.h"
#include "ccdc_hw_device.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CCDC Driver for DM6446");
MODULE_AUTHOR("Texas Instruments");

static struct device *dev;


static struct ccdc_params_raw ccdc_hw_params_raw = {
	.pix_fmt = CCDC_PIXFMT_RAW,
	.frm_fmt = CCDC_FRMFMT_PROGRESSIVE,
	.win = CCDC_WIN_VGA,
	.fid_pol = VPFE_PINPOL_POSITIVE,
	.vd_pol = VPFE_PINPOL_POSITIVE,
	.hd_pol = VPFE_PINPOL_POSITIVE,
	.config_params = {
		.data_sz = CCDC_DATA_10BITS,
	},
};


static struct ccdc_params_ycbcr ccdc_hw_params_ycbcr = {
	.pix_fmt = CCDC_PIXFMT_YCBCR_8BIT,
	.frm_fmt = CCDC_FRMFMT_INTERLACED,
	.win = CCDC_WIN_PAL,
	.fid_pol = VPFE_PINPOL_POSITIVE,
	.vd_pol = VPFE_PINPOL_POSITIVE,
	.hd_pol = VPFE_PINPOL_POSITIVE,
	.bt656_enable = 1,
	.pix_order = CCDC_PIXORDER_CBYCRY,
	.buf_type = CCDC_BUFTYPE_FLD_INTERLEAVED
};

#define CCDC_MAX_RAW_YUV_FORMATS	2


static u32 ccdc_raw_bayer_pix_formats[] =
	{V4L2_PIX_FMT_SBGGR8, V4L2_PIX_FMT_SBGGR16};


static u32 ccdc_raw_yuv_pix_formats[] =
	{V4L2_PIX_FMT_UYVY, V4L2_PIX_FMT_YUYV};

static void *__iomem ccdc_base_addr;
static int ccdc_addr_size;
static enum vpfe_hw_if_type ccdc_if_type;


static inline u32 regr(u32 offset)
{
	return __raw_readl(ccdc_base_addr + offset);
}

static inline void regw(u32 val, u32 offset)
{
	__raw_writel(val, ccdc_base_addr + offset);
}

static void ccdc_set_ccdc_base(void *addr, int size)
{
	ccdc_base_addr = addr;
	ccdc_addr_size = size;
}

static void ccdc_enable(int flag)
{
	regw(flag, CCDC_PCR);
}

static void ccdc_enable_vport(int flag)
{
	if (flag)
		
		regw(CCDC_ENABLE_VIDEO_PORT, CCDC_FMTCFG);
	else
		regw(CCDC_DISABLE_VIDEO_PORT, CCDC_FMTCFG);
}


void ccdc_setwin(struct v4l2_rect *image_win,
		enum ccdc_frmfmt frm_fmt,
		int ppc)
{
	int horz_start, horz_nr_pixels;
	int vert_start, vert_nr_lines;
	int val = 0, mid_img = 0;

	dev_dbg(dev, "\nStarting ccdc_setwin...");
	
	horz_start = image_win->left << (ppc - 1);
	horz_nr_pixels = (image_win->width << (ppc - 1)) - 1;
	regw((horz_start << CCDC_HORZ_INFO_SPH_SHIFT) | horz_nr_pixels,
	     CCDC_HORZ_INFO);

	vert_start = image_win->top;

	if (frm_fmt == CCDC_FRMFMT_INTERLACED) {
		vert_nr_lines = (image_win->height >> 1) - 1;
		vert_start >>= 1;
		
		vert_start += 1;
		
		val = (vert_start << CCDC_VDINT_VDINT0_SHIFT);
		regw(val, CCDC_VDINT);

	} else {
		
		vert_start += 1;
		vert_nr_lines = image_win->height - 1;
		
		mid_img = vert_start + (image_win->height / 2);
		val = (vert_start << CCDC_VDINT_VDINT0_SHIFT) |
		    (mid_img & CCDC_VDINT_VDINT1_MASK);
		regw(val, CCDC_VDINT);

	}
	regw((vert_start << CCDC_VERT_START_SLV0_SHIFT) | vert_start,
	     CCDC_VERT_START);
	regw(vert_nr_lines, CCDC_VERT_LINES);
	dev_dbg(dev, "\nEnd of ccdc_setwin...");
}

static void ccdc_readregs(void)
{
	unsigned int val = 0;

	val = regr(CCDC_ALAW);
	dev_notice(dev, "\nReading 0x%x to ALAW...\n", val);
	val = regr(CCDC_CLAMP);
	dev_notice(dev, "\nReading 0x%x to CLAMP...\n", val);
	val = regr(CCDC_DCSUB);
	dev_notice(dev, "\nReading 0x%x to DCSUB...\n", val);
	val = regr(CCDC_BLKCMP);
	dev_notice(dev, "\nReading 0x%x to BLKCMP...\n", val);
	val = regr(CCDC_FPC_ADDR);
	dev_notice(dev, "\nReading 0x%x to FPC_ADDR...\n", val);
	val = regr(CCDC_FPC);
	dev_notice(dev, "\nReading 0x%x to FPC...\n", val);
	val = regr(CCDC_FMTCFG);
	dev_notice(dev, "\nReading 0x%x to FMTCFG...\n", val);
	val = regr(CCDC_COLPTN);
	dev_notice(dev, "\nReading 0x%x to COLPTN...\n", val);
	val = regr(CCDC_FMT_HORZ);
	dev_notice(dev, "\nReading 0x%x to FMT_HORZ...\n", val);
	val = regr(CCDC_FMT_VERT);
	dev_notice(dev, "\nReading 0x%x to FMT_VERT...\n", val);
	val = regr(CCDC_HSIZE_OFF);
	dev_notice(dev, "\nReading 0x%x to HSIZE_OFF...\n", val);
	val = regr(CCDC_SDOFST);
	dev_notice(dev, "\nReading 0x%x to SDOFST...\n", val);
	val = regr(CCDC_VP_OUT);
	dev_notice(dev, "\nReading 0x%x to VP_OUT...\n", val);
	val = regr(CCDC_SYN_MODE);
	dev_notice(dev, "\nReading 0x%x to SYN_MODE...\n", val);
	val = regr(CCDC_HORZ_INFO);
	dev_notice(dev, "\nReading 0x%x to HORZ_INFO...\n", val);
	val = regr(CCDC_VERT_START);
	dev_notice(dev, "\nReading 0x%x to VERT_START...\n", val);
	val = regr(CCDC_VERT_LINES);
	dev_notice(dev, "\nReading 0x%x to VERT_LINES...\n", val);
}

static int validate_ccdc_param(struct ccdc_config_params_raw *ccdcparam)
{
	if (ccdcparam->alaw.enable) {
		if ((ccdcparam->alaw.gama_wd > CCDC_GAMMA_BITS_09_0) ||
		    (ccdcparam->alaw.gama_wd < CCDC_GAMMA_BITS_15_6) ||
		    (ccdcparam->alaw.gama_wd < ccdcparam->data_sz)) {
			dev_dbg(dev, "\nInvalid data line select");
			return -1;
		}
	}
	return 0;
}

static int ccdc_update_raw_params(struct ccdc_config_params_raw *raw_params)
{
	struct ccdc_config_params_raw *config_params =
		&ccdc_hw_params_raw.config_params;
	unsigned int *fpc_virtaddr = NULL;
	unsigned int *fpc_physaddr = NULL;

	memcpy(config_params, raw_params, sizeof(*raw_params));
	
	if (!config_params->fault_pxl.enable)
		return 0;

	fpc_physaddr = (unsigned int *)config_params->fault_pxl.fpc_table_addr;
	fpc_virtaddr = (unsigned int *)phys_to_virt(
				(unsigned long)fpc_physaddr);
	
	if (raw_params->fault_pxl.fp_num != config_params->fault_pxl.fp_num) {
		if (fpc_physaddr != NULL) {
			free_pages((unsigned long)fpc_physaddr,
				   get_order
				   (config_params->fault_pxl.fp_num *
				   FP_NUM_BYTES));
		}

		
		fpc_virtaddr =
			(unsigned int *)__get_free_pages(GFP_KERNEL | GFP_DMA,
							 get_order(raw_params->
							 fault_pxl.fp_num *
							 FP_NUM_BYTES));

		if (fpc_virtaddr == NULL) {
			dev_dbg(dev,
				"\nUnable to allocate memory for FPC");
			return -EFAULT;
		}
		fpc_physaddr =
		    (unsigned int *)virt_to_phys((void *)fpc_virtaddr);
	}

	
	config_params->fault_pxl.fp_num = raw_params->fault_pxl.fp_num;
	if (copy_from_user(fpc_virtaddr,
			(void __user *)raw_params->fault_pxl.fpc_table_addr,
			config_params->fault_pxl.fp_num * FP_NUM_BYTES)) {
		dev_dbg(dev, "\n copy_from_user failed");
		return -EFAULT;
	}
	config_params->fault_pxl.fpc_table_addr = (unsigned int)fpc_physaddr;
	return 0;
}

static int ccdc_close(struct device *dev)
{
	struct ccdc_config_params_raw *config_params =
		&ccdc_hw_params_raw.config_params;
	unsigned int *fpc_physaddr = NULL, *fpc_virtaddr = NULL;

	fpc_physaddr = (unsigned int *)config_params->fault_pxl.fpc_table_addr;

	if (fpc_physaddr != NULL) {
		fpc_virtaddr = (unsigned int *)
		    phys_to_virt((unsigned long)fpc_physaddr);
		free_pages((unsigned long)fpc_virtaddr,
			   get_order(config_params->fault_pxl.fp_num *
			   FP_NUM_BYTES));
	}
	return 0;
}


static void ccdc_restore_defaults(void)
{
	int i;

	
	ccdc_enable(0);
	
	for (i = 4; i <= 0x94; i += 4)
		regw(0,  i);
	regw(CCDC_NO_CULLING, CCDC_CULLING);
	regw(CCDC_GAMMA_BITS_11_2, CCDC_ALAW);
}

static int ccdc_open(struct device *device)
{
	dev = device;
	ccdc_restore_defaults();
	if (ccdc_if_type == VPFE_RAW_BAYER)
		ccdc_enable_vport(1);
	return 0;
}

static void ccdc_sbl_reset(void)
{
	vpss_clear_wbl_overflow(VPSS_PCR_CCDC_WBL_O);
}


static int ccdc_set_params(void __user *params)
{
	struct ccdc_config_params_raw ccdc_raw_params;
	int x;

	if (ccdc_if_type != VPFE_RAW_BAYER)
		return -EINVAL;

	x = copy_from_user(&ccdc_raw_params, params, sizeof(ccdc_raw_params));
	if (x) {
		dev_dbg(dev, "ccdc_set_params: error in copying"
			   "ccdc params, %d\n", x);
		return -EFAULT;
	}

	if (!validate_ccdc_param(&ccdc_raw_params)) {
		if (!ccdc_update_raw_params(&ccdc_raw_params))
			return 0;
	}
	return -EINVAL;
}


void ccdc_config_ycbcr(void)
{
	struct ccdc_params_ycbcr *params = &ccdc_hw_params_ycbcr;
	u32 syn_mode;

	dev_dbg(dev, "\nStarting ccdc_config_ycbcr...");
	
	ccdc_restore_defaults();

	
	syn_mode = (((params->pix_fmt & CCDC_SYN_MODE_INPMOD_MASK) <<
		    CCDC_SYN_MODE_INPMOD_SHIFT) |
		    ((params->frm_fmt & CCDC_SYN_FLDMODE_MASK) <<
		    CCDC_SYN_FLDMODE_SHIFT) | CCDC_VDHDEN_ENABLE |
		    CCDC_WEN_ENABLE | CCDC_DATA_PACK_ENABLE);

	
	if (params->bt656_enable) {
		regw(CCDC_REC656IF_BT656_EN, CCDC_REC656IF);

		
		syn_mode |= CCDC_SYN_MODE_VD_POL_NEGATIVE | CCDC_SYN_MODE_8BITS;
	} else {
		
		syn_mode |= (((params->fid_pol & CCDC_FID_POL_MASK) <<
			     CCDC_FID_POL_SHIFT) |
			     ((params->hd_pol & CCDC_HD_POL_MASK) <<
			     CCDC_HD_POL_SHIFT) |
			     ((params->vd_pol & CCDC_VD_POL_MASK) <<
			     CCDC_VD_POL_SHIFT));
	}
	regw(syn_mode, CCDC_SYN_MODE);

	
	ccdc_setwin(&params->win, params->frm_fmt, 2);

	
	regw((params->pix_order << CCDC_CCDCFG_Y8POS_SHIFT) |
		 CCDC_LATCH_ON_VSYNC_DISABLE, CCDC_CCDCFG);

	
	regw(((params->win.width * 2  + 31) & ~0x1f), CCDC_HSIZE_OFF);

	
	if (params->buf_type == CCDC_BUFTYPE_FLD_INTERLEAVED)
		
		regw(CCDC_SDOFST_FIELD_INTERLEAVED, CCDC_SDOFST);

	ccdc_sbl_reset();
	dev_dbg(dev, "\nEnd of ccdc_config_ycbcr...\n");
	ccdc_readregs();
}

static void ccdc_config_black_clamp(struct ccdc_black_clamp *bclamp)
{
	u32 val;

	if (!bclamp->enable) {
		
		val = (bclamp->dc_sub) & CCDC_BLK_DC_SUB_MASK;
		regw(val, CCDC_DCSUB);
		dev_dbg(dev, "\nWriting 0x%x to DCSUB...\n", val);
		regw(CCDC_CLAMP_DEFAULT_VAL, CCDC_CLAMP);
		dev_dbg(dev, "\nWriting 0x0000 to CLAMP...\n");
		return;
	}
	
	val = ((bclamp->sgain & CCDC_BLK_SGAIN_MASK) |
	       ((bclamp->start_pixel & CCDC_BLK_ST_PXL_MASK) <<
		CCDC_BLK_ST_PXL_SHIFT) |
	       ((bclamp->sample_ln & CCDC_BLK_SAMPLE_LINE_MASK) <<
		CCDC_BLK_SAMPLE_LINE_SHIFT) |
	       ((bclamp->sample_pixel & CCDC_BLK_SAMPLE_LN_MASK) <<
		CCDC_BLK_SAMPLE_LN_SHIFT) | CCDC_BLK_CLAMP_ENABLE);
	regw(val, CCDC_CLAMP);
	dev_dbg(dev, "\nWriting 0x%x to CLAMP...\n", val);
	
	regw(CCDC_DCSUB_DEFAULT_VAL, CCDC_DCSUB);
	dev_dbg(dev, "\nWriting 0x00000000 to DCSUB...\n");
}

static void ccdc_config_black_compense(struct ccdc_black_compensation *bcomp)
{
	u32 val;

	val = ((bcomp->b & CCDC_BLK_COMP_MASK) |
	      ((bcomp->gb & CCDC_BLK_COMP_MASK) <<
	       CCDC_BLK_COMP_GB_COMP_SHIFT) |
	      ((bcomp->gr & CCDC_BLK_COMP_MASK) <<
	       CCDC_BLK_COMP_GR_COMP_SHIFT) |
	      ((bcomp->r & CCDC_BLK_COMP_MASK) <<
	       CCDC_BLK_COMP_R_COMP_SHIFT));
	regw(val, CCDC_BLKCMP);
}

static void ccdc_config_fpc(struct ccdc_fault_pixel *fpc)
{
	u32 val;

	
	val = CCDC_FPC_DISABLE;
	regw(val, CCDC_FPC);

	if (!fpc->enable)
		return;

	
	regw(fpc->fpc_table_addr, CCDC_FPC_ADDR);
	dev_dbg(dev, "\nWriting 0x%x to FPC_ADDR...\n",
		       (fpc->fpc_table_addr));
	
	val = fpc->fp_num & CCDC_FPC_FPC_NUM_MASK;
	regw(val, CCDC_FPC);

	dev_dbg(dev, "\nWriting 0x%x to FPC...\n", val);
	
	val = regr(CCDC_FPC) | CCDC_FPC_ENABLE;
	regw(val, CCDC_FPC);
	dev_dbg(dev, "\nWriting 0x%x to FPC...\n", val);
}


void ccdc_config_raw(void)
{
	struct ccdc_params_raw *params = &ccdc_hw_params_raw;
	struct ccdc_config_params_raw *config_params =
		&ccdc_hw_params_raw.config_params;
	unsigned int syn_mode = 0;
	unsigned int val;

	dev_dbg(dev, "\nStarting ccdc_config_raw...");

	
	ccdc_restore_defaults();

	
	regw(CCDC_LATCH_ON_VSYNC_DISABLE, CCDC_CCDCFG);

	
	syn_mode =
		(((params->vd_pol & CCDC_VD_POL_MASK) << CCDC_VD_POL_SHIFT) |
		((params->hd_pol & CCDC_HD_POL_MASK) << CCDC_HD_POL_SHIFT) |
		((params->fid_pol & CCDC_FID_POL_MASK) << CCDC_FID_POL_SHIFT) |
		((params->frm_fmt & CCDC_FRM_FMT_MASK) << CCDC_FRM_FMT_SHIFT) |
		((config_params->data_sz & CCDC_DATA_SZ_MASK) <<
		CCDC_DATA_SZ_SHIFT) |
		((params->pix_fmt & CCDC_PIX_FMT_MASK) << CCDC_PIX_FMT_SHIFT) |
		CCDC_WEN_ENABLE | CCDC_VDHDEN_ENABLE);

	
	if (config_params->alaw.enable) {
		val = ((config_params->alaw.gama_wd &
		      CCDC_ALAW_GAMA_WD_MASK) | CCDC_ALAW_ENABLE);
		regw(val, CCDC_ALAW);
		dev_dbg(dev, "\nWriting 0x%x to ALAW...\n", val);
	}

	
	ccdc_setwin(&params->win, params->frm_fmt, CCDC_PPC_RAW);

	
	ccdc_config_black_clamp(&config_params->blk_clamp);

	
	ccdc_config_black_compense(&config_params->blk_comp);

	
	ccdc_config_fpc(&config_params->fault_pxl);

	
	if ((config_params->data_sz == CCDC_DATA_8BITS) ||
	     config_params->alaw.enable)
		syn_mode |= CCDC_DATA_PACK_ENABLE;

#ifdef CONFIG_DM644X_VIDEO_PORT_ENABLE
	
	val = CCDC_ENABLE_VIDEO_PORT;
#else
	
	val = CCDC_DISABLE_VIDEO_PORT;
#endif

	if (config_params->data_sz == CCDC_DATA_8BITS)
		val |= (CCDC_DATA_10BITS & CCDC_FMTCFG_VPIN_MASK)
		    << CCDC_FMTCFG_VPIN_SHIFT;
	else
		val |= (config_params->data_sz & CCDC_FMTCFG_VPIN_MASK)
		    << CCDC_FMTCFG_VPIN_SHIFT;
	
	regw(val, CCDC_FMTCFG);

	dev_dbg(dev, "\nWriting 0x%x to FMTCFG...\n", val);
	
	regw(CCDC_COLPTN_VAL, CCDC_COLPTN);

	dev_dbg(dev, "\nWriting 0xBB11BB11 to COLPTN...\n");
	
	val = ((params->win.left & CCDC_FMT_HORZ_FMTSPH_MASK) <<
	      CCDC_FMT_HORZ_FMTSPH_SHIFT) |
	      (params->win.width & CCDC_FMT_HORZ_FMTLNH_MASK);
	regw(val, CCDC_FMT_HORZ);

	dev_dbg(dev, "\nWriting 0x%x to FMT_HORZ...\n", val);
	val = (params->win.top & CCDC_FMT_VERT_FMTSLV_MASK)
	    << CCDC_FMT_VERT_FMTSLV_SHIFT;
	if (params->frm_fmt == CCDC_FRMFMT_PROGRESSIVE)
		val |= (params->win.height) & CCDC_FMT_VERT_FMTLNV_MASK;
	else
		val |= (params->win.height >> 1) & CCDC_FMT_VERT_FMTLNV_MASK;

	dev_dbg(dev, "\nparams->win.height  0x%x ...\n",
	       params->win.height);
	regw(val, CCDC_FMT_VERT);

	dev_dbg(dev, "\nWriting 0x%x to FMT_VERT...\n", val);

	dev_dbg(dev, "\nbelow regw(val, FMT_VERT)...");

	
	if ((config_params->data_sz == CCDC_DATA_8BITS) ||
	    config_params->alaw.enable)
		regw((params->win.width + CCDC_32BYTE_ALIGN_VAL) &
		    CCDC_HSIZE_OFF_MASK, CCDC_HSIZE_OFF);
	else
		
		regw(((params->win.width * CCDC_TWO_BYTES_PER_PIXEL) +
		    CCDC_32BYTE_ALIGN_VAL) & CCDC_HSIZE_OFF_MASK,
		    CCDC_HSIZE_OFF);

	
	if (params->frm_fmt == CCDC_FRMFMT_INTERLACED) {
		if (params->image_invert_enable) {
			
			regw(CCDC_INTERLACED_IMAGE_INVERT, CCDC_SDOFST);
			dev_dbg(dev, "\nWriting 0x4B6D to SDOFST...\n");
		}

		else {
			
			regw(CCDC_INTERLACED_NO_IMAGE_INVERT, CCDC_SDOFST);
			dev_dbg(dev, "\nWriting 0x0249 to SDOFST...\n");
		}
	} else if (params->frm_fmt == CCDC_FRMFMT_PROGRESSIVE) {
		regw(CCDC_PROGRESSIVE_NO_IMAGE_INVERT, CCDC_SDOFST);
		dev_dbg(dev, "\nWriting 0x0000 to SDOFST...\n");
	}

	
	if (params->frm_fmt == CCDC_FRMFMT_PROGRESSIVE)
		val = (((params->win.height - 1) & CCDC_VP_OUT_VERT_NUM_MASK))
		    << CCDC_VP_OUT_VERT_NUM_SHIFT;
	else
		val =
		    ((((params->win.height >> CCDC_INTERLACED_HEIGHT_SHIFT) -
		     1) & CCDC_VP_OUT_VERT_NUM_MASK)) <<
		    CCDC_VP_OUT_VERT_NUM_SHIFT;

	val |= ((((params->win.width))) & CCDC_VP_OUT_HORZ_NUM_MASK)
	    << CCDC_VP_OUT_HORZ_NUM_SHIFT;
	val |= (params->win.left) & CCDC_VP_OUT_HORZ_ST_MASK;
	regw(val, CCDC_VP_OUT);

	dev_dbg(dev, "\nWriting 0x%x to VP_OUT...\n", val);
	regw(syn_mode, CCDC_SYN_MODE);
	dev_dbg(dev, "\nWriting 0x%x to SYN_MODE...\n", syn_mode);

	ccdc_sbl_reset();
	dev_dbg(dev, "\nend of ccdc_config_raw...");
	ccdc_readregs();
}

static int ccdc_configure(void)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		ccdc_config_raw();
	else
		ccdc_config_ycbcr();
	return 0;
}

static int ccdc_set_buftype(enum ccdc_buftype buf_type)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		ccdc_hw_params_raw.buf_type = buf_type;
	else
		ccdc_hw_params_ycbcr.buf_type = buf_type;
	return 0;
}

static enum ccdc_buftype ccdc_get_buftype(void)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		return ccdc_hw_params_raw.buf_type;
	return ccdc_hw_params_ycbcr.buf_type;
}

static int ccdc_enum_pix(u32 *pix, int i)
{
	int ret = -EINVAL;
	if (ccdc_if_type == VPFE_RAW_BAYER) {
		if (i < ARRAY_SIZE(ccdc_raw_bayer_pix_formats)) {
			*pix = ccdc_raw_bayer_pix_formats[i];
			ret = 0;
		}
	} else {
		if (i < ARRAY_SIZE(ccdc_raw_yuv_pix_formats)) {
			*pix = ccdc_raw_yuv_pix_formats[i];
			ret = 0;
		}
	}
	return ret;
}

static int ccdc_set_pixel_format(u32 pixfmt)
{
	if (ccdc_if_type == VPFE_RAW_BAYER) {
		ccdc_hw_params_raw.pix_fmt = CCDC_PIXFMT_RAW;
		if (pixfmt == V4L2_PIX_FMT_SBGGR8)
			ccdc_hw_params_raw.config_params.alaw.enable = 1;
		else if (pixfmt != V4L2_PIX_FMT_SBGGR16)
			return -EINVAL;
	} else {
		if (pixfmt == V4L2_PIX_FMT_YUYV)
			ccdc_hw_params_ycbcr.pix_order = CCDC_PIXORDER_YCBYCR;
		else if (pixfmt == V4L2_PIX_FMT_UYVY)
			ccdc_hw_params_ycbcr.pix_order = CCDC_PIXORDER_CBYCRY;
		else
			return -EINVAL;
	}
	return 0;
}

static u32 ccdc_get_pixel_format(void)
{
	struct ccdc_a_law *alaw =
		&ccdc_hw_params_raw.config_params.alaw;
	u32 pixfmt;

	if (ccdc_if_type == VPFE_RAW_BAYER)
		if (alaw->enable)
			pixfmt = V4L2_PIX_FMT_SBGGR8;
		else
			pixfmt = V4L2_PIX_FMT_SBGGR16;
	else {
		if (ccdc_hw_params_ycbcr.pix_order == CCDC_PIXORDER_YCBYCR)
			pixfmt = V4L2_PIX_FMT_YUYV;
		else
			pixfmt = V4L2_PIX_FMT_UYVY;
	}
	return pixfmt;
}

static int ccdc_set_image_window(struct v4l2_rect *win)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		ccdc_hw_params_raw.win = *win;
	else
		ccdc_hw_params_ycbcr.win = *win;
	return 0;
}

static void ccdc_get_image_window(struct v4l2_rect *win)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		*win = ccdc_hw_params_raw.win;
	else
		*win = ccdc_hw_params_ycbcr.win;
}

static unsigned int ccdc_get_line_length(void)
{
	struct ccdc_config_params_raw *config_params =
		&ccdc_hw_params_raw.config_params;
	unsigned int len;

	if (ccdc_if_type == VPFE_RAW_BAYER) {
		if ((config_params->alaw.enable) ||
		    (config_params->data_sz == CCDC_DATA_8BITS))
			len = ccdc_hw_params_raw.win.width;
		else
			len = ccdc_hw_params_raw.win.width * 2;
	} else
		len = ccdc_hw_params_ycbcr.win.width * 2;
	return ALIGN(len, 32);
}

static int ccdc_set_frame_format(enum ccdc_frmfmt frm_fmt)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		ccdc_hw_params_raw.frm_fmt = frm_fmt;
	else
		ccdc_hw_params_ycbcr.frm_fmt = frm_fmt;
	return 0;
}

static enum ccdc_frmfmt ccdc_get_frame_format(void)
{
	if (ccdc_if_type == VPFE_RAW_BAYER)
		return ccdc_hw_params_raw.frm_fmt;
	else
		return ccdc_hw_params_ycbcr.frm_fmt;
}

static int ccdc_getfid(void)
{
	return (regr(CCDC_SYN_MODE) >> 15) & 1;
}


static inline void ccdc_setfbaddr(unsigned long addr)
{
	regw(addr & 0xffffffe0, CCDC_SDR_ADDR);
}

static int ccdc_set_hw_if_params(struct vpfe_hw_if_param *params)
{
	ccdc_if_type = params->if_type;

	switch (params->if_type) {
	case VPFE_BT656:
	case VPFE_YCBCR_SYNC_16:
	case VPFE_YCBCR_SYNC_8:
		ccdc_hw_params_ycbcr.vd_pol = params->vdpol;
		ccdc_hw_params_ycbcr.hd_pol = params->hdpol;
		break;
	default:
		
		return -EINVAL;
	}
	return 0;
}

static struct ccdc_hw_device ccdc_hw_dev = {
	.name = "DM6446 CCDC",
	.owner = THIS_MODULE,
	.hw_ops = {
		.open = ccdc_open,
		.close = ccdc_close,
		.set_ccdc_base = ccdc_set_ccdc_base,
		.reset = ccdc_sbl_reset,
		.enable = ccdc_enable,
		.set_hw_if_params = ccdc_set_hw_if_params,
		.set_params = ccdc_set_params,
		.configure = ccdc_configure,
		.set_buftype = ccdc_set_buftype,
		.get_buftype = ccdc_get_buftype,
		.enum_pix = ccdc_enum_pix,
		.set_pixel_format = ccdc_set_pixel_format,
		.get_pixel_format = ccdc_get_pixel_format,
		.set_frame_format = ccdc_set_frame_format,
		.get_frame_format = ccdc_get_frame_format,
		.set_image_window = ccdc_set_image_window,
		.get_image_window = ccdc_get_image_window,
		.get_line_length = ccdc_get_line_length,
		.setfbaddr = ccdc_setfbaddr,
		.getfid = ccdc_getfid,
	},
};

static int dm644x_ccdc_init(void)
{
	printk(KERN_NOTICE "dm644x_ccdc_init\n");
	if (vpfe_register_ccdc_device(&ccdc_hw_dev) < 0)
		return -1;
	printk(KERN_NOTICE "%s is registered with vpfe.\n",
		ccdc_hw_dev.name);
	return 0;
}

static void dm644x_ccdc_exit(void)
{
	vpfe_unregister_ccdc_device(&ccdc_hw_dev);
}

module_init(dm644x_ccdc_init);
module_exit(dm644x_ccdc_exit);
