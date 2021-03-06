

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/hrtimer.h>
#include <linux/clk.h>
#include <mach/hardware.h>
#include <linux/io.h>
#include <linux/debugfs.h>
#include <linux/fb.h>
#include <linux/msm_mdp.h>
#include <linux/file.h>
#include <linux/android_pmem.h>
#include <linux/major.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "mdp.h"
#include "msm_fb.h"
#include "mdp4.h"


struct mdp4_overlay_ctrl {
	struct mdp4_pipe_desc ov_pipe[OVERLAY_PIPE_MAX];
	struct mdp4_overlay_pipe plist[MDP4_MAX_PIPE];	
	struct mdp4_overlay_pipe *stage[MDP4_MAX_MIXER][MDP4_MAX_STAGE];
} mdp4_overlay_db = {
	.plist = {
		{
			.pipe_type = OVERLAY_TYPE_RGB,
			.pipe_num = OVERLAY_PIPE_RGB1,
			.pipe_ndx = 1,
		},
		{
			.pipe_type = OVERLAY_TYPE_RGB,
			.pipe_num = OVERLAY_PIPE_RGB2,
			.pipe_ndx = 2,
		},
		{
			.pipe_type = OVERLAY_TYPE_RGB, 
			.pipe_num = OVERLAY_PIPE_VG1,
			.pipe_ndx = 3,
			.pipe_used = 1,	
		},
		{
			.pipe_type = OVERLAY_TYPE_RGB, 
			.pipe_num = OVERLAY_PIPE_VG2,
			.pipe_ndx = 4,
		},
		{
			.pipe_type = OVERLAY_TYPE_VIDEO, 
			.pipe_num = OVERLAY_PIPE_VG1,
			.pipe_ndx = 5,
		},
		{
			.pipe_type = OVERLAY_TYPE_VIDEO, 
			.pipe_num = OVERLAY_PIPE_VG2,
			.pipe_ndx = 6,
		}
	}
};

static struct mdp4_overlay_ctrl *ctrl = &mdp4_overlay_db;

void mdp4_overlay_dmae_cfg(struct msm_fb_data_type *mfd, int lcdc)
{
	uint32	dmae_cfg_reg;

#ifdef DMAE_DEFLAGER
	dmae_cfg_reg = DMA_DEFLKR_EN;
#else
	dmae_cfg_reg = 0;
#endif
	if (mfd->fb_imgType == MDP_BGR_565)
		dmae_cfg_reg |= DMA_PACK_PATTERN_BGR;
	else
		dmae_cfg_reg |= DMA_PACK_PATTERN_RGB;


	if (mfd->panel_info.bpp == 18) {
		dmae_cfg_reg |= DMA_DSTC0G_6BITS |	
		    DMA_DSTC1B_6BITS | DMA_DSTC2R_6BITS;
	} else if (mfd->panel_info.bpp == 16) {
		dmae_cfg_reg |= DMA_DSTC0G_6BITS |	
		    DMA_DSTC1B_5BITS | DMA_DSTC2R_5BITS;
	} else {
		dmae_cfg_reg |= DMA_DSTC0G_8BITS |	
		    DMA_DSTC1B_8BITS | DMA_DSTC2R_8BITS;
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	
	MDP_OUTP(MDP_BASE + 0xb0000, dmae_cfg_reg);
	MDP_OUTP(MDP_BASE + 0xb0070, 0xff0000);
	MDP_OUTP(MDP_BASE + 0xb0074, 0xff0000);
	MDP_OUTP(MDP_BASE + 0xb0078, 0xff0000);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

void mdp4_overlay_dmae_xy(struct mdp4_overlay_pipe *pipe)
{

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	
	MDP_OUTP(MDP_BASE + 0xb0004,
			(pipe->src_height << 16 | pipe->src_width));
	MDP_OUTP(MDP_BASE + 0xb0008, pipe->srcp0_addr);
	MDP_OUTP(MDP_BASE + 0xb000c, pipe->srcp0_ystride);

	
	MDP_OUTP(MDP_BASE + 0xb0010, (pipe->dst_y << 16 | pipe->dst_x));

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

void mdp4_overlay_dmap_cfg(struct msm_fb_data_type *mfd, int lcdc)
{
	uint32	dma2_cfg_reg;

	dma2_cfg_reg = DMA_DITHER_EN;

	if (mfd->fb_imgType == MDP_BGR_565)
		dma2_cfg_reg |= DMA_PACK_PATTERN_BGR;
	else
		dma2_cfg_reg |= DMA_PACK_PATTERN_RGB;


	if (mfd->panel_info.bpp == 18) {
		dma2_cfg_reg |= DMA_DSTC0G_6BITS |	
		    DMA_DSTC1B_6BITS | DMA_DSTC2R_6BITS;
	} else if (mfd->panel_info.bpp == 16) {
		dma2_cfg_reg |= DMA_DSTC0G_6BITS |	
		    DMA_DSTC1B_5BITS | DMA_DSTC2R_5BITS;
	} else {
		dma2_cfg_reg |= DMA_DSTC0G_8BITS |	
		    DMA_DSTC1B_8BITS | DMA_DSTC2R_8BITS;
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	if (lcdc)
		dma2_cfg_reg |= DMA_PACK_ALIGN_MSB;

	
	MDP_OUTP(MDP_BASE + 0x90000, dma2_cfg_reg);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

void mdp4_overlay_dmap_xy(struct mdp4_overlay_pipe *pipe)
{
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	
	MDP_OUTP(MDP_BASE + 0x90004,
			(pipe->src_height << 16 | pipe->src_width));
	MDP_OUTP(MDP_BASE + 0x90008, pipe->srcp0_addr);
	MDP_OUTP(MDP_BASE + 0x9000c, pipe->srcp0_ystride);

	
	MDP_OUTP(MDP_BASE + 0x90010, (pipe->dst_y << 16 | pipe->dst_x));

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

#define MDP4_VG_PHASE_STEP_DEFAULT	0x20000000
#define MDP4_VG_PHASE_STEP_SHIFT	29

static int mdp4_leading_0(uint32 num)
{
	uint32 bit = 0x80000000;
	int i;

	for (i = 0; i < 32; i++) {
		if (bit & num)
			return i;
		bit >>= 1;
	}

	return i;
}

static uint32 mdp4_scale_phase_step(int f_num, uint32 src, uint32 dst)
{
	uint32 val;
	int	n;

	n = mdp4_leading_0(src);
	if (n > f_num)
		n = f_num;
	val = src << n;	
	val /= dst;
	if (n < f_num) {
		n = f_num - n;
		val <<= n;
	}

	return val;
}

static void mdp4_scale_setup(struct mdp4_overlay_pipe *pipe)
{
	int ptype;

	pipe->phasex_step = MDP4_VG_PHASE_STEP_DEFAULT;
	pipe->phasey_step = MDP4_VG_PHASE_STEP_DEFAULT;
	ptype = mdp4_overlay_format2type(pipe->src_format);

	if (pipe->dst_h && pipe->src_h != pipe->dst_h) {
		if (pipe->dst_h >= pipe->src_h * 8)	
			return;
		pipe->op_mode |= MDP4_OP_SCALEY_EN;

		if (pipe->pipe_num >= OVERLAY_PIPE_VG1) {
			if (pipe->dst_h <= (pipe->src_h / 4))
				pipe->op_mode |= MDP4_OP_SCALEY_MN_PHASE;
			else
				pipe->op_mode |= MDP4_OP_SCALEY_FIR;
		}

		pipe->phasey_step = mdp4_scale_phase_step(29,
					pipe->src_h, pipe->dst_h);
	}

	if (pipe->dst_w && pipe->src_w != pipe->dst_w) {
		if (pipe->dst_w >= pipe->src_w * 8)	
			return;
		pipe->op_mode |= MDP4_OP_SCALEX_EN;

		if (pipe->pipe_num >= OVERLAY_PIPE_VG1) {
			if (pipe->dst_w <= (pipe->src_w / 4))
				pipe->op_mode |= MDP4_OP_SCALEX_MN_PHASE;
			else
				pipe->op_mode |= MDP4_OP_SCALEX_FIR;
		}

		pipe->phasex_step = mdp4_scale_phase_step(29,
					pipe->src_w, pipe->dst_w);
	}
}

void mdp4_overlay_rgb_setup(struct mdp4_overlay_pipe *pipe)
{
	char *rgb_base;
	uint32 src_size, src_xy, dst_size, dst_xy;
	uint32 format, pattern;

	rgb_base = MDP_BASE + MDP4_RGB_BASE;
	rgb_base += (MDP4_RGB_OFF * pipe->pipe_num);

	src_size = ((pipe->src_h << 16) | pipe->src_w);
	src_xy = ((pipe->src_y << 16) | pipe->src_x);
	dst_size = ((pipe->dst_h << 16) | pipe->dst_w);
	dst_xy = ((pipe->dst_y << 16) | pipe->dst_x);

	format = mdp4_overlay_format(pipe);
	pattern = mdp4_overlay_unpack_pattern(pipe);

#ifdef MDP4_IGC_LUT_ENABLE
	pipe->op_mode = MDP4_OP_IGC_LUT_EN;
#else
	pipe->op_mode = 0;
#endif

	mdp4_scale_setup(pipe);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	outpdw(rgb_base + 0x0000, src_size);	
	outpdw(rgb_base + 0x0004, src_xy);	
	outpdw(rgb_base + 0x0008, dst_size);	
	outpdw(rgb_base + 0x000c, dst_xy);	

	outpdw(rgb_base + 0x0010, pipe->srcp0_addr);
	outpdw(rgb_base + 0x0040, pipe->srcp0_ystride);

	outpdw(rgb_base + 0x0050, format);
	outpdw(rgb_base + 0x0054, pattern);
	outpdw(rgb_base + 0x0058, pipe->op_mode);
	outpdw(rgb_base + 0x005c, pipe->phasex_step);
	outpdw(rgb_base + 0x0060, pipe->phasey_step);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	mdp4_stat.pipe[pipe->pipe_num]++;
}

void mdp4_overlay_vg_setup(struct mdp4_overlay_pipe *pipe)
{
	char *vg_base;
	uint32 frame_size, src_size, src_xy, dst_size, dst_xy;
	uint32 format, pattern;
	int pnum;

	pnum = pipe->pipe_num - OVERLAY_PIPE_VG1; 
	vg_base = MDP_BASE + MDP4_VIDEO_BASE;
	vg_base += (MDP4_VIDEO_OFF * pnum);

	frame_size = ((pipe->src_height << 16) | pipe->src_width);
	src_size = ((pipe->src_h << 16) | pipe->src_w);
	src_xy = ((pipe->src_y << 16) | pipe->src_x);
	dst_size = ((pipe->dst_h << 16) | pipe->dst_w);
	dst_xy = ((pipe->dst_y << 16) | pipe->dst_x);

	format = mdp4_overlay_format(pipe);
	pattern = mdp4_overlay_unpack_pattern(pipe);

	if (pipe->pipe_type == OVERLAY_TYPE_RGB)
		pipe->op_mode = 0;	
	else
#ifdef MDP4_IGC_LUT_ENABLE
		pipe->op_mode = (MDP4_OP_CSC_EN | MDP4_OP_SRC_DATA_YCBCR |
				MDP4_OP_IGC_LUT_EN);
#else
		pipe->op_mode = (MDP4_OP_CSC_EN | MDP4_OP_SRC_DATA_YCBCR);
#endif

	mdp4_scale_setup(pipe);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	outpdw(vg_base + 0x0000, src_size);	
	outpdw(vg_base + 0x0004, src_xy);	
	outpdw(vg_base + 0x0008, dst_size);	
	outpdw(vg_base + 0x000c, dst_xy);	
	outpdw(vg_base + 0x0048, frame_size);	

	
	outpdw(vg_base + 0x0010, pipe->srcp0_addr);

	
	outpdw(vg_base + 0x0014, pipe->srcp1_addr);

	outpdw(vg_base + 0x0040,
			pipe->srcp1_ystride << 16 | pipe->srcp0_ystride);

	outpdw(vg_base + 0x0050, format);	
	outpdw(vg_base + 0x0054, pattern);	
	outpdw(vg_base + 0x0058, pipe->op_mode);
	outpdw(vg_base + 0x005c, pipe->phasex_step);
	outpdw(vg_base + 0x0060, pipe->phasey_step);

	if (pipe->op_mode & MDP4_OP_DITHER_EN) {
		outpdw(vg_base + 0x0068,
			pipe->r_bit << 4 | pipe->b_bit << 2 | pipe->g_bit);
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	mdp4_stat.pipe[pipe->pipe_num]++;
}

int mdp4_overlay_format2type(uint32 format)
{
	switch (format) {
	case MDP_RGB_565:
	case MDP_RGB_888:
	case MDP_BGR_565:
	case MDP_XRGB_8888:
	case MDP_ARGB_8888:
	case MDP_RGBA_8888:
	case MDP_BGRA_8888:
	case MDP_RGBX_8888:
		return OVERLAY_TYPE_RGB;
	case MDP_YCRYCB_H2V1:
	case MDP_Y_CRCB_H2V1:
	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CBCR_H2V2_TILE:
	case MDP_Y_CRCB_H2V2_TILE:
		return OVERLAY_TYPE_VIDEO;
	default:
		mdp4_stat.err_format++;
		return -ERANGE;
	}

}

#define C3_ALPHA	3	
#define C2_R_Cr		2	
#define C1_B_Cb		1	
#define C0_G_Y		0	

int mdp4_overlay_format2pipe(struct mdp4_overlay_pipe *pipe)
{
	switch (pipe->src_format) {
	case MDP_RGB_565:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 0;
		pipe->r_bit = 1;	
		pipe->b_bit = 1;	
		pipe->g_bit = 2;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 2;
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 2;	
		break;
	case MDP_RGB_888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 0;
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 2;
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 3;	
		break;
	case MDP_BGR_565:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 0;
		pipe->r_bit = 1;	
		pipe->b_bit = 1;	
		pipe->g_bit = 2;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 2;
		pipe->element2 = C1_B_Cb;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C2_R_Cr;	
		pipe->bpp = 2;	
		break;
	case MDP_XRGB_8888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 3;	
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C3_ALPHA;	
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 4;		
		break;
	case MDP_ARGB_8888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 3;	
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 1;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C3_ALPHA;	
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 4;		
		break;
	case MDP_RGBA_8888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 3;	
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 1;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C3_ALPHA;	
		pipe->element2 = C1_B_Cb;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C2_R_Cr;	
		pipe->bpp = 4;		
		break;
	case MDP_RGBX_8888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 3;
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C3_ALPHA;	
		pipe->element2 = C1_B_Cb;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C2_R_Cr;	
		pipe->bpp = 4;		
		break;
	case MDP_BGRA_8888:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 3;	
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 1;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C3_ALPHA;	
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 4;		
		break;
	case MDP_YCRYCB_H2V1:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_INTERLEAVED;
		pipe->a_bit = 0;	
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 3;
		pipe->element3 = C0_G_Y;	
		pipe->element2 = C2_R_Cr;	
		pipe->element1 = C0_G_Y;	
		pipe->element0 = C1_B_Cb;	
		pipe->bpp = 2;		
		pipe->chroma_sample = MDP4_CHROMA_H2V1;
		break;
	case MDP_Y_CRCB_H2V1:
	case MDP_Y_CBCR_H2V1:
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CBCR_H2V2:
		pipe->frame_format = MDP4_FRAME_FORMAT_LINEAR;
		pipe->fetch_plane = OVERLAY_PLANE_PSEUDO_PLANAR;
		pipe->a_bit = 0;
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 1;		
		pipe->element3 = C0_G_Y;	
		pipe->element2 = C0_G_Y;	
		if (pipe->src_format == MDP_Y_CRCB_H2V1) {
			pipe->element1 = C2_R_Cr;	
			pipe->element0 = C1_B_Cb;	
			pipe->chroma_sample = MDP4_CHROMA_H2V1;
		} else if (pipe->src_format == MDP_Y_CBCR_H2V1) {
			pipe->element1 = C1_B_Cb;	
			pipe->element0 = C2_R_Cr;	
			pipe->chroma_sample = MDP4_CHROMA_H2V1;
		} else if (pipe->src_format == MDP_Y_CRCB_H2V2) {
			pipe->element1 = C2_R_Cr;	
			pipe->element0 = C1_B_Cb;	
			pipe->chroma_sample = MDP4_CHROMA_420;
		} else if (pipe->src_format == MDP_Y_CBCR_H2V2) {
			pipe->element1 = C1_B_Cb;	
			pipe->element0 = C2_R_Cr;	
			pipe->chroma_sample = MDP4_CHROMA_420;
		}
		pipe->bpp = 2;	
		break;
	case MDP_Y_CBCR_H2V2_TILE:
	case MDP_Y_CRCB_H2V2_TILE:
		pipe->frame_format = MDP4_FRAME_FORMAT_VIDEO_SUPERTILE;
		pipe->fetch_plane = OVERLAY_PLANE_PSEUDO_PLANAR;
		pipe->a_bit = 0;
		pipe->r_bit = 3;	
		pipe->b_bit = 3;	
		pipe->g_bit = 3;	
		pipe->alpha_enable = 0;
		pipe->unpack_tight = 1;
		pipe->unpack_align_msb = 0;
		pipe->unpack_count = 1;		
		pipe->element3 = C0_G_Y;	
		pipe->element2 = C0_G_Y;	
		if (pipe->src_format == MDP_Y_CRCB_H2V2_TILE) {
			pipe->element1 = C2_R_Cr;	
			pipe->element0 = C1_B_Cb;	
			pipe->chroma_sample = MDP4_CHROMA_420;
		} else if (pipe->src_format == MDP_Y_CBCR_H2V2_TILE) {
			pipe->element1 = C1_B_Cb;	
			pipe->element0 = C2_R_Cr;	
			pipe->chroma_sample = MDP4_CHROMA_420;
		}
		pipe->bpp = 2;	
		break;
	default:
		
		mdp4_stat.err_format++;
		return -ERANGE;
	}

	return 0;
}


static uint32 color_key_convert(int start, int num, uint32 color)
{
	uint32 data;

	data = (color >> start) & ((1 << num) - 1);

	
	if (num == 5)
		data = ((data << 3) | (data >> 2));
	else if (num == 6)
		data = ((data << 2) | (data >> 4));

	
	data = (data << 4) | (data >> 4);

	return data;
}

void transp_color_key(int format, uint32 transp,
			uint32 *c0, uint32 *c1, uint32 *c2)
{
	int b_start, g_start, r_start;
	int b_num, g_num, r_num;

	switch (format) {
	case MDP_RGB_565:
		b_start = 0;
		g_start = 5;
		r_start = 11;
		r_num = 5;
		g_num = 6;
		b_num = 5;
		break;
	case MDP_RGB_888:
	case MDP_XRGB_8888:
	case MDP_ARGB_8888:
	case MDP_BGRA_8888:
		b_start = 0;
		g_start = 8;
		r_start = 16;
		r_num = 8;
		g_num = 8;
		b_num = 8;
		break;
	case MDP_RGBA_8888:
	case MDP_RGBX_8888:
		b_start = 16;
		g_start = 8;
		r_start = 0;
		r_num = 8;
		g_num = 8;
		b_num = 8;
		break;
	case MDP_BGR_565:
		b_start = 11;
		g_start = 5;
		r_start = 0;
		r_num = 5;
		g_num = 6;
		b_num = 5;
		break;
	case MDP_Y_CBCR_H2V2:
	case MDP_Y_CBCR_H2V1:
		b_start = 8;
		g_start = 16;
		r_start = 0;
		r_num = 8;
		g_num = 8;
		b_num = 8;
		break;
	case MDP_Y_CRCB_H2V2:
	case MDP_Y_CRCB_H2V1:
		b_start = 0;
		g_start = 16;
		r_start = 8;
		r_num = 8;
		g_num = 8;
		b_num = 8;
		break;
	default:
		b_start = 0;
		g_start = 8;
		r_start = 16;
		r_num = 8;
		g_num = 8;
		b_num = 8;
		break;
	}

	*c0 = color_key_convert(g_start, g_num, transp);
	*c1 = color_key_convert(b_start, b_num, transp);
	*c2 = color_key_convert(r_start, r_num, transp);
}

uint32 mdp4_overlay_format(struct mdp4_overlay_pipe *pipe)
{
	uint32	format;

	format = 0;

	if (pipe->solid_fill)
		format |= MDP4_FORMAT_SOLID_FILL;

	if (pipe->unpack_align_msb)
		format |= MDP4_FORMAT_UNPACK_ALIGN_MSB;

	if (pipe->unpack_tight)
		format |= MDP4_FORMAT_UNPACK_TIGHT;

	if (pipe->alpha_enable)
		format |= MDP4_FORMAT_ALPHA_ENABLE;

	format |= (pipe->unpack_count << 13);
	format |= ((pipe->bpp - 1) << 9);
	format |= (pipe->a_bit << 6);
	format |= (pipe->r_bit << 4);
	format |= (pipe->b_bit << 2);
	format |= pipe->g_bit;

	format |= (pipe->frame_format << 29);

	if (pipe->fetch_plane == OVERLAY_PLANE_PSEUDO_PLANAR) {
		
		format |= (pipe->fetch_plane << 19);
		format |= (pipe->chroma_site << 28);
		format |= (pipe->chroma_sample << 26);
	}

	return format;
}

uint32 mdp4_overlay_unpack_pattern(struct mdp4_overlay_pipe *pipe)
{
	return (pipe->element3 << 24) | (pipe->element2 << 16) |
			(pipe->element1 << 8) | pipe->element0;
}

void mdp4_overlayproc_cfg(struct mdp4_overlay_pipe *pipe)
{
	uint32 data;
	char *overlay_base;

	if (pipe->mixer_num == MDP4_MIXER1)
		overlay_base = MDP_BASE + MDP4_OVERLAYPROC1_BASE;
	else
		overlay_base = MDP_BASE + MDP4_OVERLAYPROC0_BASE;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	
	outpdw(overlay_base + 0x0004, 0x01); 
	data = pipe->src_height;
	data <<= 16;
	data |= pipe->src_width;
	outpdw(overlay_base + 0x0008, data); 
	outpdw(overlay_base + 0x000c, pipe->srcp0_addr);
	outpdw(overlay_base + 0x0010, pipe->srcp0_ystride);

#ifdef MDP4_IGC_LUT_ENABLE
	outpdw(overlay_base + 0x0014, 0x4);	
#endif
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

int mdp4_overlay_pipe_staged(int mixer)
{
	uint32 data, mask, i;
	int p1, p2;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	data = inpdw(MDP_BASE + 0x10100);
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
	p1 = 0;
	p2 = 0;
	for (i = 0; i < 8; i++) {
		mask = data & 0x0f;
		if (mask) {
			if (mask <= 4)
				p1++;
			else
				p2++;
		}
		data >>= 4;
	}

	if (mixer)
		return p2;
	else
		return p1;
}

void mdp4_mixer_stage_up(struct mdp4_overlay_pipe *pipe)
{
	uint32 data, mask, snum, stage, mixer, pnum;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	stage = pipe->mixer_stage;
	mixer = pipe->mixer_num;
	pnum = pipe->pipe_num;

	
	data = inpdw(MDP_BASE + 0x10100);

	if (mixer == MDP4_MIXER1)
		stage += 8;

	if (pipe->pipe_num >= OVERLAY_PIPE_VG1) {
		pnum -= OVERLAY_PIPE_VG1; 
		snum = 0;
		snum += (4 * pnum);
	} else {
		snum = 8;
		snum += (4 * pnum);	
	}

	mask = 0x0f;
	mask <<= snum;
	stage <<= snum;
	data &= ~mask;	

	data |= stage;

	outpdw(MDP_BASE + 0x10100, data); 

	data = inpdw(MDP_BASE + 0x10100);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	ctrl->stage[pipe->mixer_num][pipe->mixer_stage] = pipe;	
}

void mdp4_mixer_stage_down(struct mdp4_overlay_pipe *pipe)
{
	uint32 data, mask, snum, stage, mixer, pnum;

	stage = pipe->mixer_stage;
	mixer = pipe->mixer_num;
	pnum = pipe->pipe_num;

	if (pipe != ctrl->stage[mixer][stage])	
		return;

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	
	data = inpdw(MDP_BASE + 0x10100);

	if (mixer == MDP4_MIXER1)
		stage += 8;

	if (pipe->pipe_num >= OVERLAY_PIPE_VG1) {
		pnum -= OVERLAY_PIPE_VG1; 
		snum = 0;
		snum += (4 * pnum);
	} else {
		snum = 8;
		snum += (4 * pnum);	
	}

	mask = 0x0f;
	mask <<= snum;
	data &= ~mask;	

	outpdw(MDP_BASE + 0x10100, data); 

	data = inpdw(MDP_BASE + 0x10100);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);

	ctrl->stage[pipe->mixer_num][pipe->mixer_stage] = NULL;	
}

void mdp4_mixer_blend_setup(struct mdp4_overlay_pipe *pipe)
{
	struct mdp4_overlay_pipe *bg_pipe;
	unsigned char *overlay_base;
	uint32 c0, c1, c2, blend_op;
	int off;

	if (pipe->mixer_num) 	
		overlay_base = MDP_BASE + MDP4_OVERLAYPROC1_BASE;
	else
		overlay_base = MDP_BASE + MDP4_OVERLAYPROC0_BASE;

	
	off = 0x20 * (pipe->mixer_stage - MDP4_MIXER_STAGE0);

	bg_pipe = mdp4_overlay_stage_pipe(pipe->mixer_num,
					MDP4_MIXER_STAGE_BASE);
	if (bg_pipe == NULL) {
		printk(KERN_INFO "%s: Error: no bg_pipe\n", __func__);
		return;
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);

	blend_op = 0;

	if (bg_pipe->alpha_enable && pipe->alpha_enable) {
		
		blend_op |= (MDP4_BLEND_FG_ALPHA_BG_PIXEL |
				MDP4_BLEND_FG_INV_ALPHA |
				MDP4_BLEND_BG_ALPHA_BG_PIXEL);
	} else if (bg_pipe->alpha_enable && pipe->alpha_enable == 0) {
		blend_op = (MDP4_BLEND_BG_ALPHA_BG_PIXEL |
				MDP4_BLEND_FG_ALPHA_BG_PIXEL |
				MDP4_BLEND_FG_INV_ALPHA);
	} else {
		
		blend_op |= (MDP4_BLEND_FG_ALPHA_FG_CONST |
					MDP4_BLEND_BG_ALPHA_BG_CONST);
		if (pipe->is_fg) {
			outpdw(overlay_base + off + 0x108, pipe->alpha);
			outpdw(overlay_base + off + 0x10c, 0xff - pipe->alpha);
		} else {
			outpdw(overlay_base + off + 0x108, 0xff - pipe->alpha);
			outpdw(overlay_base + off + 0x10c, pipe->alpha);
		}

		if (pipe->transp != MDP_TRANSP_NOP) {
			if (pipe->is_fg) {
				transp_color_key(pipe->src_format, pipe->transp,
						&c0, &c1, &c2);
				
				blend_op |= MDP4_BLEND_FG_TRANSP_EN;
				
				outpdw(overlay_base + off + 0x110,
						(c1 << 16 | c0));
				outpdw(overlay_base + off + 0x114, c2);
				
				outpdw(overlay_base + off + 0x118,
						(c1 << 16 | c0));
				outpdw(overlay_base + off + 0x11c, c2);
			} else {
				transp_color_key(bg_pipe->src_format,
					pipe->transp, &c0, &c1, &c2);
				
				blend_op |= MDP4_BLEND_BG_TRANSP_EN;
				
				outpdw(overlay_base + 0x180,
						(c1 << 16 | c0));
				outpdw(overlay_base + 0x184, c2);
				
				outpdw(overlay_base + 0x188,
						(c1 << 16 | c0));
				outpdw(overlay_base + 0x18c, c2);
			}
		}
	}
	outpdw(overlay_base + off + 0x104, blend_op);

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

void mdp4_overlay_reg_flush(struct mdp4_overlay_pipe *pipe, int all)
{
	uint32 bits = 0;

	if (pipe->mixer_num == MDP4_MIXER1)
		bits |= 0x02;
	else
		bits |= 0x01;

	if (all) {
		if (pipe->pipe_num <= OVERLAY_PIPE_RGB2) {
			if (pipe->pipe_num == OVERLAY_PIPE_RGB2)
				bits |= 0x20;
			else
				bits |= 0x10;
		} else {
			if (pipe->pipe_num == OVERLAY_PIPE_VG2)
				bits |= 0x08;
			else
				bits |= 0x04;
		}
	}

	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_ON, FALSE);
	outpdw(MDP_BASE + 0x18000, bits);	
	mdp_pipe_ctrl(MDP_CMD_BLOCK, MDP_BLOCK_POWER_OFF, FALSE);
}

struct mdp4_overlay_pipe *mdp4_overlay_stage_pipe(int mixer, int stage)
{
	return ctrl->stage[mixer][stage];
}

struct mdp4_overlay_pipe *mdp4_overlay_ndx2pipe(int ndx)
{
	struct mdp4_overlay_pipe *pipe;

	if (ndx <= 0 || ndx > MDP4_MAX_PIPE)
		return NULL;

	pipe = &ctrl->plist[ndx - 1];	

	if (pipe->pipe_used == 0)
		return NULL;

	return pipe;
}

struct mdp4_overlay_pipe *mdp4_overlay_pipe_alloc(int ptype)
{
	int i;
	struct mdp4_overlay_pipe *pipe;

	pipe = &ctrl->plist[0];
	for (i = 0; i < MDP4_MAX_PIPE; i++) {
		if (pipe->pipe_type == ptype && pipe->pipe_used == 0) {
			init_completion(&pipe->comp);
#ifdef MDP4_MDDI_DMA_SWITCH
			init_completion(&pipe->dmas_comp);
#endif
	printk(KERN_INFO "mdp4_overlay_pipe_alloc: pipe=%x ndx=%d\n",
					(int)pipe, pipe->pipe_ndx);
			return pipe;
		}
		pipe++;
	}

	printk(KERN_INFO "mdp4_overlay_pipe_alloc: ptype=%d FAILED\n",
							ptype);

	return NULL;
}


void mdp4_overlay_pipe_free(struct mdp4_overlay_pipe *pipe)
{
	uint32 ptype, num, ndx;
	struct mdp4_pipe_desc  *pd;

	printk(KERN_INFO "mdp4_overlay_pipe_free: pipe=%x ndx=%d\n",
					(int)pipe, pipe->pipe_ndx);
	pd = &ctrl->ov_pipe[pipe->pipe_num];
	if (pd->ref_cnt)
		pd->ref_cnt--;

	pd->player = NULL;

	ptype = pipe->pipe_type;
	num = pipe->pipe_num;
	ndx = pipe->pipe_ndx;

	memset(pipe, 0, sizeof(*pipe));

	pipe->pipe_type = ptype;
	pipe->pipe_num = num;
	pipe->pipe_ndx = ndx;
}

int mdp4_overlay_req_check(uint32 id, uint32 z_order, uint32 mixer)
{
	struct mdp4_overlay_pipe *pipe;

	pipe = ctrl->stage[mixer][z_order];

	if (pipe == NULL)
		return 0;

	if (pipe->pipe_ndx == id)	
		return 0;

	if (id == MSMFB_NEW_REQUEST) {  
		if (pipe->pipe_num >= OVERLAY_PIPE_VG1) 
			return 0;
	}

	return -EPERM;
}

static int mdp4_overlay_req2pipe(struct mdp_overlay *req, int mixer,
			struct mdp4_overlay_pipe **ppipe)
{
	struct mdp4_overlay_pipe *pipe;
	struct mdp4_pipe_desc  *pd;
	int ret, ptype;

	if (mixer >= MDP4_MAX_MIXER) {
		printk(KERN_ERR "mpd_overlay_req2pipe: mixer out of range!\n");
		mdp4_stat.err_mixer++;
		return -ERANGE;
	}

	if (req->z_order < 0 || req->z_order > 2) {
		printk(KERN_ERR "mpd_overlay_req2pipe: z_order=%d out of range!\n",
				req->z_order);
		mdp4_stat.err_zorder++;
		return -ERANGE;
	}

	if (req->src_rect.h == 0 || req->src_rect.w == 0) {
		printk(KERN_ERR "mpd_overlay_req2pipe: src img of zero size!\n");
		mdp4_stat.err_size++;
		return -EINVAL;
	}


	if (req->dst_rect.h > (req->src_rect.h * 8)) {	
		mdp4_stat.err_scale++;
		return -ERANGE;
	}

	if (req->src_rect.h > (req->dst_rect.h * 8)) {	
		mdp4_stat.err_scale++;
		return -ERANGE;
	}

	if (req->dst_rect.w > (req->src_rect.w * 8)) {	
		mdp4_stat.err_scale++;
		return -ERANGE;
	}

	if (req->src_rect.w > (req->dst_rect.w * 8)) {	
		mdp4_stat.err_scale++;
		return -ERANGE;
	}

	
	if (req->src_rect.h > (req->dst_rect.h * 4)) {
		if (req->src_rect.h % req->dst_rect.h) { 
			mdp4_stat.err_scale++;
			return -ERANGE;
		}
	}

	if (req->src_rect.w > (req->dst_rect.w * 4)) {
		if (req->src_rect.w % req->dst_rect.w) { 
			mdp4_stat.err_scale++;
			return -ERANGE;
		}
	}

	ptype = mdp4_overlay_format2type(req->src.format);
	if (ptype < 0)
		return ptype;

	ret = mdp4_overlay_req_check(req->id, req->z_order, mixer);
	if (ret < 0)
		return ret;

	if (req->id == MSMFB_NEW_REQUEST)  
		pipe = mdp4_overlay_pipe_alloc(ptype);
	else
		pipe = mdp4_overlay_ndx2pipe(req->id);

	if (pipe == NULL)
		return -ENOMEM;

	
	if (pipe->pipe_num <= OVERLAY_PIPE_RGB2) {
		if ((req->src_rect.h > req->dst_rect.h) ||
			(req->src_rect.w > req->dst_rect.w))
				return -ERANGE;
	}

	pipe->src_format = req->src.format;
	ret = mdp4_overlay_format2pipe(pipe);
	if (ret < 0)
		return ret;

	
	if (req->id == MSMFB_NEW_REQUEST) {  
		pd = &ctrl->ov_pipe[pipe->pipe_num];
		pd->ref_cnt++;
		pipe->pipe_used++;
		pipe->mixer_num = mixer;
		pipe->mixer_stage = req->z_order + MDP4_MIXER_STAGE0;
		printk(KERN_INFO "mpd4_overlay_req2pipe: zorder=%d pipe_num=%d\n",
				req->z_order, pipe->pipe_num);

	}

	pipe->src_width = req->src.width & 0x07ff;	
	pipe->src_height = req->src.height & 0x07ff;	
	pipe->src_h = req->src_rect.h & 0x07ff;
	pipe->src_w = req->src_rect.w & 0x07ff;
	pipe->src_y = req->src_rect.y & 0x07ff;
	pipe->src_x = req->src_rect.x & 0x07ff;
	pipe->dst_h = req->dst_rect.h & 0x07ff;
	pipe->dst_w = req->dst_rect.w & 0x07ff;
	pipe->dst_y = req->dst_rect.y & 0x07ff;
	pipe->dst_x = req->dst_rect.x & 0x07ff;

	if (req->flags & MDP_FLIP_LR)
		pipe->op_mode |= MDP4_OP_FLIP_LR;

	if (req->flags & MDP_FLIP_UD)
		pipe->op_mode |= MDP4_OP_FLIP_UD;

	if (req->flags & MDP_DITHER)
		pipe->op_mode |= MDP4_OP_DITHER_EN;

	if (req->flags & MDP_DEINTERLACE)
		pipe->op_mode |= MDP4_OP_DEINT_ODD_REF;

	pipe->is_fg = req->is_fg;

	pipe->alpha = req->alpha & 0x0ff;

	pipe->transp = req->transp_mask;

	*ppipe = pipe;

	return 0;
}

static int get_img(struct msmfb_data *img, struct fb_info *info,
	unsigned long *start, unsigned long *len, struct file **pp_file)
{
	int put_needed, ret = 0, fb_num;
	struct file *file;
#ifdef CONFIG_ANDROID_PMEM
	unsigned long vstart;
#endif

#ifdef CONFIG_ANDROID_PMEM
	if (!get_pmem_file(img->memory_id, start, &vstart, len, pp_file))
		return 0;
#endif
	file = fget_light(img->memory_id, &put_needed);
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

int mdp4_overlay_get(struct fb_info *info, struct mdp_overlay *req)
{
	struct mdp4_overlay_pipe *pipe;

	pipe = mdp4_overlay_ndx2pipe(req->id);
	if (pipe == NULL)
		return -ENODEV;

	*req = pipe->req_data;

	return 0;
}

static int mdp4_pull_mode(int mixer)
{
	uint32 lcdc;
	int off;

	if (mixer == MDP4_MIXER1) 
		off = 0xd0000;
	else			
		off = 0xc0000;

	lcdc = inpdw(MDP_BASE + off);
	lcdc &= 0x01;

	return lcdc;
}

int mdp4_overlay_set(struct fb_info *info, struct mdp_overlay *req)
{
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)info->par;
	int ret, mixer;
	struct mdp4_overlay_pipe *pipe;

	if (mfd == NULL)
		return -ENODEV;

	if (req->src.format == MDP_FB_FORMAT)
		req->src.format = mfd->fb_imgType;

	if (mutex_lock_interruptible(&mfd->dma->ov_mutex))
		return -EINTR;

	mixer = mfd->panel_info.pdest;	

	ret = mdp4_overlay_req2pipe(req, mixer, &pipe);
	if (ret < 0) {
		mutex_unlock(&mfd->dma->ov_mutex);
		return ret;
	}

	
	req->id = pipe->pipe_ndx;	
	pipe->req_data = *req;		

	mdp4_stat.overlay_set[pipe->mixer_num]++;

	mutex_unlock(&mfd->dma->ov_mutex);

	return 0;
}

int mdp4_overlay_unset(struct fb_info *info, int ndx)
{
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)info->par;
	struct mdp4_overlay_pipe *pipe;
	int pull;

	if (mfd == NULL)
		return -ENODEV;

	if (mutex_lock_interruptible(&mfd->dma->ov_mutex))
		return -EINTR;

	pipe = mdp4_overlay_ndx2pipe(ndx);

	if (pipe == NULL) {
		mutex_unlock(&mfd->dma->ov_mutex);
		return -ENODEV;
	}

	pull = mdp4_pull_mode(pipe->mixer_num);

	mdp4_mixer_stage_down(pipe);

	if (pull) 
		mdp4_overlay_reg_flush(pipe, 0);
	else  	
		mdp4_mddi_overlay_restore();

	mdp4_stat.overlay_unset[pipe->mixer_num]++;

	mdp4_overlay_pipe_free(pipe);

	mutex_unlock(&mfd->dma->ov_mutex);

	return 0;
}

struct tile_desc {
	uint32 width;  
	uint32 height; 
	uint32 row_tile_w; 
	uint32 row_tile_h; 
};

void tile_samsung(struct tile_desc *tp)
{
	
	tp->width = 64;		
	tp->row_tile_w = 2;	
	tp->height = 32;	
	tp->row_tile_h = 1;	
}

uint32 tile_mem_size(struct mdp4_overlay_pipe *pipe, struct tile_desc *tp)
{
	uint32 tile_w, tile_h;
	uint32 row_num_w, row_num_h;


	tile_w = tp->width * tp->row_tile_w;
	tile_h = tp->height * tp->row_tile_h;

	row_num_w = (pipe->src_width + tile_w - 1) / tile_w;
	row_num_h = (pipe->src_height + tile_h - 1) / tile_h;
	return ((row_num_w * row_num_h * tile_w * tile_h) + 8191) & ~8191;
}

int mdp4_overlay_play(struct fb_info *info, struct msmfb_overlay_data *req,
		struct file **pp_src_file)
{
	struct msm_fb_data_type *mfd = (struct msm_fb_data_type *)info->par;
	struct msmfb_data *img;
	struct mdp4_overlay_pipe *pipe;
	struct mdp4_pipe_desc *pd;
	ulong start, addr;
	ulong len = 0;
	struct file *p_src_file = 0;
	int pull;

	if (mfd == NULL)
		return -ENODEV;

	pipe = mdp4_overlay_ndx2pipe(req->id);
	if (pipe == NULL)
		return -ENODEV;

	if (mutex_lock_interruptible(&mfd->dma->ov_mutex))
		return -EINTR;

	pd = &ctrl->ov_pipe[pipe->pipe_num];
	if (pd->player && pipe != pd->player) {
		if (pipe->pipe_type == OVERLAY_TYPE_RGB) {
			mutex_unlock(&mfd->dma->ov_mutex);
			return 0; 
		}
	}

	pd->player = pipe;	

	img = &req->data;
	get_img(img, info, &start, &len, &p_src_file);
	if (len == 0) {
		mutex_unlock(&mfd->dma->ov_mutex);
		printk(KERN_ERR "mdp_overlay_play: could not retrieve"
				       " image from memory\n");
		return -1;
	}
	*pp_src_file = p_src_file;

	addr = start + img->offset;
	pipe->srcp0_addr = addr;
	pipe->srcp0_ystride = pipe->src_width * pipe->bpp;

	if (pipe->fetch_plane == OVERLAY_PLANE_PSEUDO_PLANAR) {
		if (pipe->frame_format == MDP4_FRAME_FORMAT_VIDEO_SUPERTILE) {
			struct tile_desc tile;

			tile_samsung(&tile);
			pipe->srcp1_addr = addr + tile_mem_size(pipe, &tile);
		} else
			pipe->srcp1_addr = addr +
					pipe->src_width * pipe->src_height;

		pipe->srcp0_ystride = pipe->src_width;
		pipe->srcp1_ystride = pipe->src_width;
	}

	if (pipe->pipe_num >= OVERLAY_PIPE_VG1)
		mdp4_overlay_vg_setup(pipe);	
	else
		mdp4_overlay_rgb_setup(pipe);	

	mdp4_mixer_blend_setup(pipe);
	mdp4_mixer_stage_up(pipe);

	pull = mdp4_pull_mode(pipe->mixer_num);


	if (pull)	
		mdp4_overlay_reg_flush(pipe, 1);
	else { 	

#ifdef MDP4_NONBLOCKING
		if (mfd->panel_power_on)
#else
		if (!mfd->dma->busy && mfd->panel_power_on)
#endif
			mdp4_mddi_overlay_kickoff(mfd, pipe);

	}

	mdp4_stat.overlay_play[pipe->mixer_num]++;

	mutex_unlock(&mfd->dma->ov_mutex);

	return 0;
}
