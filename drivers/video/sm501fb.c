

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/console.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#ifdef CONFIG_PM
#include <linux/pm.h>
#endif

#include <linux/sm501.h>
#include <linux/sm501-regs.h>

#define NR_PALETTE	256

enum sm501_controller {
	HEAD_CRT	= 0,
	HEAD_PANEL	= 1,
};


struct sm501_mem {
	unsigned long	 size;
	unsigned long	 sm_addr;	
	void __iomem	*k_addr;
};


struct sm501fb_info {
	struct device		*dev;
	struct fb_info		*fb[2];		
	struct resource		*fbmem_res;	
	struct resource		*regs_res;	
	struct sm501_platdata_fb *pdata;	

	unsigned long		 pm_crt_ctrl;	

	int			 irq;
	int			 swap_endian;	
	void __iomem		*regs;		
	void __iomem		*fbmem;		
	size_t			 fbmem_len;	
};


struct sm501fb_par {
	u32			 pseudo_palette[16];

	enum sm501_controller	 head;
	struct sm501_mem	 cursor;
	struct sm501_mem	 screen;
	struct fb_ops		 ops;

	void			*store_fb;
	void			*store_cursor;
	void __iomem		*cursor_regs;
	struct sm501fb_info	*info;
};



static inline int h_total(struct fb_var_screeninfo *var)
{
	return var->xres + var->left_margin +
		var->right_margin + var->hsync_len;
}

static inline int v_total(struct fb_var_screeninfo *var)
{
	return var->yres + var->upper_margin +
		var->lower_margin + var->vsync_len;
}



static inline void sm501fb_sync_regs(struct sm501fb_info *info)
{
	readl(info->regs);
}



#define SM501_MEMF_CURSOR		(1)
#define SM501_MEMF_PANEL		(2)
#define SM501_MEMF_CRT			(4)
#define SM501_MEMF_ACCEL		(8)

static int sm501_alloc_mem(struct sm501fb_info *inf, struct sm501_mem *mem,
			   unsigned int why, size_t size, u32 smem_len)
{
	struct sm501fb_par *par;
	struct fb_info *fbi;
	unsigned int ptr;
	unsigned int end;

	switch (why) {
	case SM501_MEMF_CURSOR:
		ptr = inf->fbmem_len - size;
		inf->fbmem_len = ptr;	
		break;

	case SM501_MEMF_PANEL:
		if (size > inf->fbmem_len)
			return -ENOMEM;

		ptr = inf->fbmem_len - size;
		fbi = inf->fb[HEAD_CRT];

		

		if (ptr > 0)
			ptr &= ~(PAGE_SIZE - 1);

		if (fbi && ptr < smem_len)
			return -ENOMEM;

		break;

	case SM501_MEMF_CRT:
		ptr = 0;

		

		fbi = inf->fb[HEAD_PANEL];
		if (fbi) {
			par = fbi->par;
			end = par->screen.k_addr ? par->screen.sm_addr : inf->fbmem_len;
		} else
			end = inf->fbmem_len;

		if ((ptr + size) > end)
			return -ENOMEM;

		break;

	case SM501_MEMF_ACCEL:
		fbi = inf->fb[HEAD_CRT];
		ptr = fbi ? smem_len : 0;

		fbi = inf->fb[HEAD_PANEL];
		if (fbi) {
			par = fbi->par;
			end = par->screen.sm_addr;
		} else
			end = inf->fbmem_len;

		if ((ptr + size) > end)
			return -ENOMEM;

		break;

	default:
		return -EINVAL;
	}

	mem->size    = size;
	mem->sm_addr = ptr;
	mem->k_addr  = inf->fbmem + ptr;

	dev_dbg(inf->dev, "%s: result %08lx, %p - %u, %zd\n",
		__func__, mem->sm_addr, mem->k_addr, why, size);

	return 0;
}



static unsigned long sm501fb_ps_to_hz(unsigned long psvalue)
{
	unsigned long long numerator=1000000000000ULL;

	
	do_div(numerator, psvalue);
	return (unsigned long)numerator;
}



#define sm501fb_hz_to_ps(x) sm501fb_ps_to_hz(x)



static void sm501fb_setup_gamma(struct sm501fb_info *fbi,
				unsigned long palette)
{
	unsigned long value = 0;
	int offset;

	
	for (offset = 0; offset < 256 * 4; offset += 4) {
		writel(value, fbi->regs + palette + offset);
		value += 0x010101; 	
	}
}



static int sm501fb_check_var(struct fb_var_screeninfo *var,
			     struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *sm  = par->info;
	unsigned long tmp;

	

	if (var->hsync_len > 255 || var->vsync_len > 63)
		return -EINVAL;

	
	if ((var->xres + var->right_margin) > 4096)
		return -EINVAL;

	
	if ((var->yres + var->lower_margin) > 2048)
		return -EINVAL;

	

	if (h_total(var) > 4096 || v_total(var) > 2048)
		return -EINVAL;

	

	tmp = (var->xres * var->bits_per_pixel) / 8;
	if ((tmp & 15) != 0)
		return -EINVAL;

	

	if (var->xres_virtual > 4096 || var->yres_virtual > 2048)
		return -EINVAL;

	

	if (var->bits_per_pixel <= 8)
		var->bits_per_pixel = 8;
	else if (var->bits_per_pixel <= 16)
		var->bits_per_pixel = 16;
	else if (var->bits_per_pixel == 24)
		var->bits_per_pixel = 32;

	
	switch(var->bits_per_pixel) {
	case 8:
		var->red.length		= var->bits_per_pixel;
		var->red.offset		= 0;
		var->green.length	= var->bits_per_pixel;
		var->green.offset	= 0;
		var->blue.length	= var->bits_per_pixel;
		var->blue.offset	= 0;
		var->transp.length	= 0;
		var->transp.offset	= 0;

		break;

	case 16:
		if (sm->pdata->flags & SM501_FBPD_SWAP_FB_ENDIAN) {
			var->blue.offset	= 11;
			var->green.offset	= 5;
			var->red.offset		= 0;
		} else {
			var->red.offset		= 11;
			var->green.offset	= 5;
			var->blue.offset	= 0;
		}
		var->transp.offset	= 0;

		var->red.length		= 5;
		var->green.length	= 6;
		var->blue.length	= 5;
		var->transp.length	= 0;
		break;

	case 32:
		if (sm->pdata->flags & SM501_FBPD_SWAP_FB_ENDIAN) {
			var->transp.offset	= 0;
			var->red.offset		= 8;
			var->green.offset	= 16;
			var->blue.offset	= 24;
		} else {
			var->transp.offset	= 24;
			var->red.offset		= 16;
			var->green.offset	= 8;
			var->blue.offset	= 0;
		}

		var->red.length		= 8;
		var->green.length	= 8;
		var->blue.length	= 8;
		var->transp.length	= 0;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}



static int sm501fb_check_var_crt(struct fb_var_screeninfo *var,
				 struct fb_info *info)
{
	return sm501fb_check_var(var, info);
}



static int sm501fb_check_var_pnl(struct fb_var_screeninfo *var,
				 struct fb_info *info)
{
	return sm501fb_check_var(var, info);
}



static int sm501fb_set_par_common(struct fb_info *info,
				  struct fb_var_screeninfo *var)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	unsigned long pixclock;      
	unsigned long sm501pixclock; 
	unsigned int mem_type;
	unsigned int clock_type;
	unsigned int head_addr;
	unsigned int smem_len;

	dev_dbg(fbi->dev, "%s: %dx%d, bpp = %d, virtual %dx%d\n",
		__func__, var->xres, var->yres, var->bits_per_pixel,
		var->xres_virtual, var->yres_virtual);

	switch (par->head) {
	case HEAD_CRT:
		mem_type = SM501_MEMF_CRT;
		clock_type = SM501_CLOCK_V2XCLK;
		head_addr = SM501_DC_CRT_FB_ADDR;
		break;

	case HEAD_PANEL:
		mem_type = SM501_MEMF_PANEL;
		clock_type = SM501_CLOCK_P2XCLK;
		head_addr = SM501_DC_PANEL_FB_ADDR;
		break;

	default:
		mem_type = 0;		
		head_addr = 0;
		clock_type = 0;
	}

	switch (var->bits_per_pixel) {
	case 8:
		info->fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;

	case 16:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;

	case 32:
		info->fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	}

	
	info->fix.line_length = (var->xres_virtual * var->bits_per_pixel)/8;
	smem_len = info->fix.line_length * var->yres_virtual;

	dev_dbg(fbi->dev, "%s: line length = %u\n", __func__,
		info->fix.line_length);

	if (sm501_alloc_mem(fbi, &par->screen, mem_type, smem_len, smem_len)) {
		dev_err(fbi->dev, "no memory available\n");
		return -ENOMEM;
	}

	mutex_lock(&info->mm_lock);
	info->fix.smem_start = fbi->fbmem_res->start + par->screen.sm_addr;
	info->fix.smem_len   = smem_len;
	mutex_unlock(&info->mm_lock);

	info->screen_base = fbi->fbmem + par->screen.sm_addr;
	info->screen_size = info->fix.smem_len;

	

	writel(par->screen.sm_addr | SM501_ADDR_FLIP, fbi->regs + head_addr);

	

	pixclock = sm501fb_ps_to_hz(var->pixclock);

	sm501pixclock = sm501_set_clock(fbi->dev->parent, clock_type,
					pixclock);

	
	var->pixclock = sm501fb_hz_to_ps(sm501pixclock);

	dev_dbg(fbi->dev, "%s: pixclock(ps) = %u, pixclock(Hz)  = %lu, "
	       "sm501pixclock = %lu,  error = %ld%%\n",
	       __func__, var->pixclock, pixclock, sm501pixclock,
	       ((pixclock - sm501pixclock)*100)/pixclock);

	return 0;
}



static void sm501fb_set_par_geometry(struct fb_info *info,
				     struct fb_var_screeninfo *var)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	void __iomem *base = fbi->regs;
	unsigned long reg;

	if (par->head == HEAD_CRT)
		base += SM501_DC_CRT_H_TOT;
	else
		base += SM501_DC_PANEL_H_TOT;

	

	reg = info->fix.line_length;
	reg |= ((var->xres * var->bits_per_pixel)/8) << 16;

	writel(reg, fbi->regs + (par->head == HEAD_CRT ?
		    SM501_DC_CRT_FB_OFFSET :  SM501_DC_PANEL_FB_OFFSET));

	

	reg  = (h_total(var) - 1) << 16;
	reg |= (var->xres - 1);

	writel(reg, base + SM501_OFF_DC_H_TOT);

	

	reg  = var->hsync_len << 16;
	reg |= var->xres + var->right_margin - 1;

	writel(reg, base + SM501_OFF_DC_H_SYNC);

	

	reg  = (v_total(var) - 1) << 16;
	reg |= (var->yres - 1);

	writel(reg, base + SM501_OFF_DC_V_TOT);

	
	reg  = var->vsync_len << 16;
	reg |= var->yres + var->lower_margin - 1;

	writel(reg, base + SM501_OFF_DC_V_SYNC);
}



static int sm501fb_pan_crt(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	unsigned int bytes_pixel = var->bits_per_pixel / 8;
	unsigned long reg;
	unsigned long xoffs;

	xoffs = var->xoffset * bytes_pixel;

	reg = readl(fbi->regs + SM501_DC_CRT_CONTROL);

	reg &= ~SM501_DC_CRT_CONTROL_PIXEL_MASK;
	reg |= ((xoffs & 15) / bytes_pixel) << 4;
	writel(reg, fbi->regs + SM501_DC_CRT_CONTROL);

	reg = (par->screen.sm_addr + xoffs +
	       var->yoffset * info->fix.line_length);
	writel(reg | SM501_ADDR_FLIP, fbi->regs + SM501_DC_CRT_FB_ADDR);

	sm501fb_sync_regs(fbi);
	return 0;
}



static int sm501fb_pan_pnl(struct fb_var_screeninfo *var,
			   struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	unsigned long reg;

	reg = var->xoffset | (var->xres_virtual << 16);
	writel(reg, fbi->regs + SM501_DC_PANEL_FB_WIDTH);

	reg = var->yoffset | (var->yres_virtual << 16);
	writel(reg, fbi->regs + SM501_DC_PANEL_FB_HEIGHT);

	sm501fb_sync_regs(fbi);
	return 0;
}



static int sm501fb_set_par_crt(struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	struct fb_var_screeninfo *var = &info->var;
	unsigned long control;       
	int ret;

	

	dev_dbg(fbi->dev, "%s(%p)\n", __func__, info);

	
	sm501_misc_control(fbi->dev->parent, 0, SM501_MISC_DAC_POWER);

	control = readl(fbi->regs + SM501_DC_CRT_CONTROL);

	control &= (SM501_DC_CRT_CONTROL_PIXEL_MASK |
		    SM501_DC_CRT_CONTROL_GAMMA |
		    SM501_DC_CRT_CONTROL_BLANK |
		    SM501_DC_CRT_CONTROL_SEL |
		    SM501_DC_CRT_CONTROL_CP |
		    SM501_DC_CRT_CONTROL_TVP);

	

	if ((var->sync & FB_SYNC_HOR_HIGH_ACT) == 0)
		control |= SM501_DC_CRT_CONTROL_HSP;

	if ((var->sync & FB_SYNC_VERT_HIGH_ACT) == 0)
		control |= SM501_DC_CRT_CONTROL_VSP;

	if ((control & SM501_DC_CRT_CONTROL_SEL) == 0) {
		

		sm501_alloc_mem(fbi, &par->screen, SM501_MEMF_CRT, 0,
				info->fix.smem_len);
		goto out_update;
	}

	ret = sm501fb_set_par_common(info, var);
	if (ret) {
		dev_err(fbi->dev, "failed to set common parameters\n");
		return ret;
	}

	sm501fb_pan_crt(var, info);
	sm501fb_set_par_geometry(info, var);

	control |= SM501_FIFO_3;	

	switch(var->bits_per_pixel) {
	case 8:
		control |= SM501_DC_CRT_CONTROL_8BPP;
		break;

	case 16:
		control |= SM501_DC_CRT_CONTROL_16BPP;
		sm501fb_setup_gamma(fbi, SM501_DC_CRT_PALETTE);
		break;

	case 32:
		control |= SM501_DC_CRT_CONTROL_32BPP;
		sm501fb_setup_gamma(fbi, SM501_DC_CRT_PALETTE);
		break;

	default:
		BUG();
	}

	control |= SM501_DC_CRT_CONTROL_SEL;	
	control |= SM501_DC_CRT_CONTROL_TE;	
	control |= SM501_DC_CRT_CONTROL_ENABLE;	

 out_update:
	dev_dbg(fbi->dev, "new control is %08lx\n", control);

	writel(control, fbi->regs + SM501_DC_CRT_CONTROL);
	sm501fb_sync_regs(fbi);

	return 0;
}

static void sm501fb_panel_power(struct sm501fb_info *fbi, int to)
{
	unsigned long control;
	void __iomem *ctrl_reg = fbi->regs + SM501_DC_PANEL_CONTROL;
	struct sm501_platdata_fbsub *pd = fbi->pdata->fb_pnl;

	control = readl(ctrl_reg);

	if (to && (control & SM501_DC_PANEL_CONTROL_VDD) == 0) {
		

		control |= SM501_DC_PANEL_CONTROL_VDD;	
		writel(control, ctrl_reg);
		sm501fb_sync_regs(fbi);
		mdelay(10);

		control |= SM501_DC_PANEL_CONTROL_DATA;	
		writel(control, ctrl_reg);
		sm501fb_sync_regs(fbi);
		mdelay(10);

		

		if (!(pd->flags & SM501FB_FLAG_PANEL_NO_VBIASEN)) {
			if (pd->flags & SM501FB_FLAG_PANEL_INV_VBIASEN)
				control &= ~SM501_DC_PANEL_CONTROL_BIAS;
			else
				control |= SM501_DC_PANEL_CONTROL_BIAS;

			writel(control, ctrl_reg);
			sm501fb_sync_regs(fbi);
			mdelay(10);
		}

		if (!(pd->flags & SM501FB_FLAG_PANEL_NO_FPEN)) {
			if (pd->flags & SM501FB_FLAG_PANEL_INV_FPEN)
				control &= ~SM501_DC_PANEL_CONTROL_FPEN;
			else
				control |= SM501_DC_PANEL_CONTROL_FPEN;

			writel(control, ctrl_reg);
			sm501fb_sync_regs(fbi);
			mdelay(10);
		}
	} else if (!to && (control & SM501_DC_PANEL_CONTROL_VDD) != 0) {
		
		if (!(pd->flags & SM501FB_FLAG_PANEL_NO_FPEN)) {
			if (pd->flags & SM501FB_FLAG_PANEL_INV_FPEN)
				control |= SM501_DC_PANEL_CONTROL_FPEN;
			else
				control &= ~SM501_DC_PANEL_CONTROL_FPEN;

			writel(control, ctrl_reg);
			sm501fb_sync_regs(fbi);
			mdelay(10);
		}

		if (!(pd->flags & SM501FB_FLAG_PANEL_NO_VBIASEN)) {
			if (pd->flags & SM501FB_FLAG_PANEL_INV_VBIASEN)
				control |= SM501_DC_PANEL_CONTROL_BIAS;
			else
				control &= ~SM501_DC_PANEL_CONTROL_BIAS;

			writel(control, ctrl_reg);
			sm501fb_sync_regs(fbi);
			mdelay(10);
		}

		control &= ~SM501_DC_PANEL_CONTROL_DATA;
		writel(control, ctrl_reg);
		sm501fb_sync_regs(fbi);
		mdelay(10);

		control &= ~SM501_DC_PANEL_CONTROL_VDD;
		writel(control, ctrl_reg);
		sm501fb_sync_regs(fbi);
		mdelay(10);
	}

	sm501fb_sync_regs(fbi);
}



static int sm501fb_set_par_pnl(struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	struct fb_var_screeninfo *var = &info->var;
	unsigned long control;
	unsigned long reg;
	int ret;

	dev_dbg(fbi->dev, "%s(%p)\n", __func__, info);

	

	ret = sm501fb_set_par_common(info, var);
	if (ret)
		return ret;

	sm501fb_pan_pnl(var, info);
	sm501fb_set_par_geometry(info, var);

	

	control = readl(fbi->regs + SM501_DC_PANEL_CONTROL);
	control &= (SM501_DC_PANEL_CONTROL_GAMMA |
		    SM501_DC_PANEL_CONTROL_VDD  |
		    SM501_DC_PANEL_CONTROL_DATA |
		    SM501_DC_PANEL_CONTROL_BIAS |
		    SM501_DC_PANEL_CONTROL_FPEN |
		    SM501_DC_PANEL_CONTROL_CP |
		    SM501_DC_PANEL_CONTROL_CK |
		    SM501_DC_PANEL_CONTROL_HP |
		    SM501_DC_PANEL_CONTROL_VP |
		    SM501_DC_PANEL_CONTROL_HPD |
		    SM501_DC_PANEL_CONTROL_VPD);

	control |= SM501_FIFO_3;	

	switch(var->bits_per_pixel) {
	case 8:
		control |= SM501_DC_PANEL_CONTROL_8BPP;
		break;

	case 16:
		control |= SM501_DC_PANEL_CONTROL_16BPP;
		sm501fb_setup_gamma(fbi, SM501_DC_PANEL_PALETTE);
		break;

	case 32:
		control |= SM501_DC_PANEL_CONTROL_32BPP;
		sm501fb_setup_gamma(fbi, SM501_DC_PANEL_PALETTE);
		break;

	default:
		BUG();
	}

	writel(0x0, fbi->regs + SM501_DC_PANEL_PANNING_CONTROL);

	

	writel(0x00, fbi->regs + SM501_DC_PANEL_TL_LOC);

	reg  = var->xres - 1;
	reg |= (var->yres - 1) << 16;

	writel(reg, fbi->regs + SM501_DC_PANEL_BR_LOC);

	

	control |= SM501_DC_PANEL_CONTROL_TE;	
	control |= SM501_DC_PANEL_CONTROL_EN;	

	if ((var->sync & FB_SYNC_HOR_HIGH_ACT) == 0)
		control |= SM501_DC_PANEL_CONTROL_HSP;

	if ((var->sync & FB_SYNC_VERT_HIGH_ACT) == 0)
		control |= SM501_DC_PANEL_CONTROL_VSP;

	writel(control, fbi->regs + SM501_DC_PANEL_CONTROL);
	sm501fb_sync_regs(fbi);

	

	sm501_modify_reg(fbi->dev->parent, SM501_SYSTEM_CONTROL,
			 0, SM501_SYSCTRL_PANEL_TRISTATE);

	
	sm501fb_panel_power(fbi, 1);
	return 0;
}




static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}



static int sm501fb_setcolreg(unsigned regno,
			     unsigned red, unsigned green, unsigned blue,
			     unsigned transp, struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	void __iomem *base = fbi->regs;
	unsigned int val;

	if (par->head == HEAD_CRT)
		base += SM501_DC_CRT_PALETTE;
	else
		base += SM501_DC_PANEL_PALETTE;

	switch (info->fix.visual) {
	case FB_VISUAL_TRUECOLOR:
		

		if (regno < 16) {
			u32 *pal = par->pseudo_palette;

			val  = chan_to_field(red,   &info->var.red);
			val |= chan_to_field(green, &info->var.green);
			val |= chan_to_field(blue,  &info->var.blue);

			pal[regno] = val;
		}
		break;

	case FB_VISUAL_PSEUDOCOLOR:
		if (regno < 256) {
			val = (red >> 8) << 16;
			val |= (green >> 8) << 8;
			val |= blue >> 8;

			writel(val, base + (regno * 4));
		}

		break;

	default:
		return 1;   
	}

	return 0;
}



static int sm501fb_blank_pnl(int blank_mode, struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;

	dev_dbg(fbi->dev, "%s(mode=%d, %p)\n", __func__, blank_mode, info);

	switch (blank_mode) {
	case FB_BLANK_POWERDOWN:
		sm501fb_panel_power(fbi, 0);
		break;

	case FB_BLANK_UNBLANK:
		sm501fb_panel_power(fbi, 1);
		break;

	case FB_BLANK_NORMAL:
	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	default:
		return 1;
	}

	return 0;
}



static int sm501fb_blank_crt(int blank_mode, struct fb_info *info)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	unsigned long ctrl;

	dev_dbg(fbi->dev, "%s(mode=%d, %p)\n", __func__, blank_mode, info);

	ctrl = readl(fbi->regs + SM501_DC_CRT_CONTROL);

	switch (blank_mode) {
	case FB_BLANK_POWERDOWN:
		ctrl &= ~SM501_DC_CRT_CONTROL_ENABLE;
		sm501_misc_control(fbi->dev->parent, SM501_MISC_DAC_POWER, 0);

	case FB_BLANK_NORMAL:
		ctrl |= SM501_DC_CRT_CONTROL_BLANK;
		break;

	case FB_BLANK_UNBLANK:
		ctrl &= ~SM501_DC_CRT_CONTROL_BLANK;
		ctrl |=  SM501_DC_CRT_CONTROL_ENABLE;
		sm501_misc_control(fbi->dev->parent, 0, SM501_MISC_DAC_POWER);
		break;

	case FB_BLANK_VSYNC_SUSPEND:
	case FB_BLANK_HSYNC_SUSPEND:
	default:
		return 1;

	}

	writel(ctrl, fbi->regs + SM501_DC_CRT_CONTROL);
	sm501fb_sync_regs(fbi);

	return 0;
}



static int sm501fb_cursor(struct fb_info *info, struct fb_cursor *cursor)
{
	struct sm501fb_par  *par = info->par;
	struct sm501fb_info *fbi = par->info;
	void __iomem *base = fbi->regs;
	unsigned long hwc_addr;
	unsigned long fg, bg;

	dev_dbg(fbi->dev, "%s(%p,%p)\n", __func__, info, cursor);

	if (par->head == HEAD_CRT)
		base += SM501_DC_CRT_HWC_BASE;
	else
		base += SM501_DC_PANEL_HWC_BASE;

	

	if (cursor->image.width > 64)
		return -EINVAL;

	if (cursor->image.height > 64)
		return -EINVAL;

	if (cursor->image.depth > 1)
		return -EINVAL;

	hwc_addr = readl(base + SM501_OFF_HWC_ADDR);

	if (cursor->enable)
		writel(hwc_addr | SM501_HWC_EN, base + SM501_OFF_HWC_ADDR);
	else
		writel(hwc_addr & ~SM501_HWC_EN, base + SM501_OFF_HWC_ADDR);

	
	if (cursor->set & FB_CUR_SETPOS) {
		unsigned int x = cursor->image.dx;
		unsigned int y = cursor->image.dy;

		if (x >= 2048 || y >= 2048 )
			return -EINVAL;

		dev_dbg(fbi->dev, "set position %d,%d\n", x, y);

		

		writel(x | (y << 16), base + SM501_OFF_HWC_LOC);
	}

	if (cursor->set & FB_CUR_SETCMAP) {
		unsigned int bg_col = cursor->image.bg_color;
		unsigned int fg_col = cursor->image.fg_color;

		dev_dbg(fbi->dev, "%s: update cmap (%08x,%08x)\n",
			__func__, bg_col, fg_col);

		bg = ((info->cmap.red[bg_col] & 0xF8) << 8) |
			((info->cmap.green[bg_col] & 0xFC) << 3) |
			((info->cmap.blue[bg_col] & 0xF8) >> 3);

		fg = ((info->cmap.red[fg_col] & 0xF8) << 8) |
			((info->cmap.green[fg_col] & 0xFC) << 3) |
			((info->cmap.blue[fg_col] & 0xF8) >> 3);

		dev_dbg(fbi->dev, "fgcol %08lx, bgcol %08lx\n", fg, bg);

		writel(bg, base + SM501_OFF_HWC_COLOR_1_2);
		writel(fg, base + SM501_OFF_HWC_COLOR_3);
	}

	if (cursor->set & FB_CUR_SETSIZE ||
	    cursor->set & (FB_CUR_SETIMAGE | FB_CUR_SETSHAPE)) {
		
		int x, y;
		const unsigned char *pcol = cursor->image.data;
		const unsigned char *pmsk = cursor->mask;
		void __iomem   *dst = par->cursor.k_addr;
		unsigned char  dcol = 0;
		unsigned char  dmsk = 0;
		unsigned int   op;

		dev_dbg(fbi->dev, "%s: setting shape (%d,%d)\n",
			__func__, cursor->image.width, cursor->image.height);

		for (op = 0; op < (64*64*2)/8; op+=4)
			writel(0x0, dst + op);

		for (y = 0; y < cursor->image.height; y++) {
			for (x = 0; x < cursor->image.width; x++) {
				if ((x % 8) == 0) {
					dcol = *pcol++;
					dmsk = *pmsk++;
				} else {
					dcol >>= 1;
					dmsk >>= 1;
				}

				if (dmsk & 1) {
					op = (dcol & 1) ? 1 : 3;
					op <<= ((x % 4) * 2);

					op |= readb(dst + (x / 4));
					writeb(op, dst + (x / 4));
				}
			}
			dst += (64*2)/8;
		}
	}

	sm501fb_sync_regs(fbi);	
	return 0;
}



static ssize_t sm501fb_crtsrc_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct sm501fb_info *info = dev_get_drvdata(dev);
	unsigned long ctrl;

	ctrl = readl(info->regs + SM501_DC_CRT_CONTROL);
	ctrl &= SM501_DC_CRT_CONTROL_SEL;

	return snprintf(buf, PAGE_SIZE, "%s\n", ctrl ? "crt" : "panel");
}



static ssize_t sm501fb_crtsrc_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t len)
{
	struct sm501fb_info *info = dev_get_drvdata(dev);
	enum sm501_controller head;
	unsigned long ctrl;

	if (len < 1)
		return -EINVAL;

	if (strnicmp(buf, "crt", 3) == 0)
		head = HEAD_CRT;
	else if (strnicmp(buf, "panel", 5) == 0)
		head = HEAD_PANEL;
	else
		return -EINVAL;

	dev_info(dev, "setting crt source to head %d\n", head);

	ctrl = readl(info->regs + SM501_DC_CRT_CONTROL);

	if (head == HEAD_CRT) {
		ctrl |= SM501_DC_CRT_CONTROL_SEL;
		ctrl |= SM501_DC_CRT_CONTROL_ENABLE;
		ctrl |= SM501_DC_CRT_CONTROL_TE;
	} else {
		ctrl &= ~SM501_DC_CRT_CONTROL_SEL;
		ctrl &= ~SM501_DC_CRT_CONTROL_ENABLE;
		ctrl &= ~SM501_DC_CRT_CONTROL_TE;
	}

	writel(ctrl, info->regs + SM501_DC_CRT_CONTROL);
	sm501fb_sync_regs(info);

	return len;
}


static DEVICE_ATTR(crt_src, 0666, sm501fb_crtsrc_show, sm501fb_crtsrc_store);


static int sm501fb_show_regs(struct sm501fb_info *info, char *ptr,
			     unsigned int start, unsigned int len)
{
	void __iomem *mem = info->regs;
	char *buf = ptr;
	unsigned int reg;

	for (reg = start; reg < (len + start); reg += 4)
		ptr += sprintf(ptr, "%08x = %08x\n", reg, readl(mem + reg));

	return ptr - buf;
}



static ssize_t sm501fb_debug_show_crt(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct sm501fb_info *info = dev_get_drvdata(dev);
	char *ptr = buf;

	ptr += sm501fb_show_regs(info, ptr, SM501_DC_CRT_CONTROL, 0x40);
	ptr += sm501fb_show_regs(info, ptr, SM501_DC_CRT_HWC_BASE, 0x10);

	return ptr - buf;
}

static DEVICE_ATTR(fbregs_crt, 0444, sm501fb_debug_show_crt, NULL);



static ssize_t sm501fb_debug_show_pnl(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct sm501fb_info *info = dev_get_drvdata(dev);
	char *ptr = buf;

	ptr += sm501fb_show_regs(info, ptr, 0x0, 0x40);
	ptr += sm501fb_show_regs(info, ptr, SM501_DC_PANEL_HWC_BASE, 0x10);

	return ptr - buf;
}

static DEVICE_ATTR(fbregs_pnl, 0444, sm501fb_debug_show_pnl, NULL);



static struct fb_ops sm501fb_ops_crt = {
	.owner		= THIS_MODULE,
	.fb_check_var	= sm501fb_check_var_crt,
	.fb_set_par	= sm501fb_set_par_crt,
	.fb_blank	= sm501fb_blank_crt,
	.fb_setcolreg	= sm501fb_setcolreg,
	.fb_pan_display	= sm501fb_pan_crt,
	.fb_cursor	= sm501fb_cursor,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static struct fb_ops sm501fb_ops_pnl = {
	.owner		= THIS_MODULE,
	.fb_check_var	= sm501fb_check_var_pnl,
	.fb_set_par	= sm501fb_set_par_pnl,
	.fb_pan_display	= sm501fb_pan_pnl,
	.fb_blank	= sm501fb_blank_pnl,
	.fb_setcolreg	= sm501fb_setcolreg,
	.fb_cursor	= sm501fb_cursor,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};



static int sm501_init_cursor(struct fb_info *fbi, unsigned int reg_base)
{
	struct sm501fb_par *par;
	struct sm501fb_info *info;
	int ret;

	if (fbi == NULL)
		return 0;

	par = fbi->par;
	info = par->info;

	par->cursor_regs = info->regs + reg_base;

	ret = sm501_alloc_mem(info, &par->cursor, SM501_MEMF_CURSOR, 1024,
			      fbi->fix.smem_len);
	if (ret < 0)
		return ret;

	

	writel(par->cursor.sm_addr, par->cursor_regs + SM501_OFF_HWC_ADDR);

	writel(0x00, par->cursor_regs + SM501_OFF_HWC_LOC);
	writel(0x00, par->cursor_regs + SM501_OFF_HWC_COLOR_1_2);
	writel(0x00, par->cursor_regs + SM501_OFF_HWC_COLOR_3);
	sm501fb_sync_regs(info);

	return 0;
}



static int sm501fb_start(struct sm501fb_info *info,
			 struct platform_device *pdev)
{
	struct resource	*res;
	struct device *dev = &pdev->dev;
	int k;
	int ret;

	info->irq = ret = platform_get_irq(pdev, 0);
	if (ret < 0) {
		
		dev_warn(dev, "no irq for device\n");
	}

	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(dev, "no resource definition for registers\n");
		ret = -ENOENT;
		goto err_release;
	}

	info->regs_res = request_mem_region(res->start,
					    res->end - res->start,
					    pdev->name);

	if (info->regs_res == NULL) {
		dev_err(dev, "cannot claim registers\n");
		ret = -ENXIO;
		goto err_release;
	}

	info->regs = ioremap(res->start, (res->end - res->start)+1);
	if (info->regs == NULL) {
		dev_err(dev, "cannot remap registers\n");
		ret = -ENXIO;
		goto err_regs_res;
	}

	
	res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	if (res == NULL) {
		dev_err(dev, "no memory resource defined\n");
		ret = -ENXIO;
		goto err_regs_map;
	}

	info->fbmem_res = request_mem_region(res->start,
					     (res->end - res->start)+1,
					     pdev->name);
	if (info->fbmem_res == NULL) {
		dev_err(dev, "cannot claim framebuffer\n");
		ret = -ENXIO;
		goto err_regs_map;
	}

	info->fbmem = ioremap(res->start, (res->end - res->start)+1);
	if (info->fbmem == NULL) {
		dev_err(dev, "cannot remap framebuffer\n");
		goto err_mem_res;
	}

	info->fbmem_len = (res->end - res->start)+1;

	
	memset(info->fbmem, 0, info->fbmem_len);

	
	for (k = 0; k < (256 * 3); k++)
		writel(0, info->regs + SM501_DC_PANEL_PALETTE + (k * 4));

	
	sm501_unit_power(dev->parent, SM501_GATE_DISPLAY, 1);

	

	sm501_init_cursor(info->fb[HEAD_CRT], SM501_DC_CRT_HWC_ADDR);
	sm501_init_cursor(info->fb[HEAD_PANEL], SM501_DC_PANEL_HWC_ADDR);

	return 0; 

 err_mem_res:
	release_resource(info->fbmem_res);
	kfree(info->fbmem_res);

 err_regs_map:
	iounmap(info->regs);

 err_regs_res:
	release_resource(info->regs_res);
	kfree(info->regs_res);

 err_release:
	return ret;
}

static void sm501fb_stop(struct sm501fb_info *info)
{
	
	sm501_unit_power(info->dev->parent, SM501_GATE_DISPLAY, 0);

	iounmap(info->fbmem);
	release_resource(info->fbmem_res);
	kfree(info->fbmem_res);

	iounmap(info->regs);
	release_resource(info->regs_res);
	kfree(info->regs_res);
}

static int sm501fb_init_fb(struct fb_info *fb,
			   enum sm501_controller head,
			   const char *fbname)
{
	struct sm501_platdata_fbsub *pd;
	struct sm501fb_par *par = fb->par;
	struct sm501fb_info *info = par->info;
	unsigned long ctrl;
	unsigned int enable;
	int ret;

	switch (head) {
	case HEAD_CRT:
		pd = info->pdata->fb_crt;
		ctrl = readl(info->regs + SM501_DC_CRT_CONTROL);
		enable = (ctrl & SM501_DC_CRT_CONTROL_ENABLE) ? 1 : 0;

		
		if (info->pdata->fb_route != SM501_FB_CRT_PANEL) {
			ctrl |= SM501_DC_CRT_CONTROL_SEL;
			writel(ctrl, info->regs + SM501_DC_CRT_CONTROL);
		}

		break;

	case HEAD_PANEL:
		pd = info->pdata->fb_pnl;
		ctrl = readl(info->regs + SM501_DC_PANEL_CONTROL);
		enable = (ctrl & SM501_DC_PANEL_CONTROL_EN) ? 1 : 0;
		break;

	default:
		pd = NULL;		
		ctrl = 0;
		enable = 0;
		BUG();
	}

	dev_info(info->dev, "fb %s %sabled at start\n",
		 fbname, enable ? "en" : "dis");

	

	if (head == HEAD_CRT && info->pdata->fb_route == SM501_FB_CRT_PANEL) {
		ctrl &= ~SM501_DC_CRT_CONTROL_SEL;
		writel(ctrl, info->regs + SM501_DC_CRT_CONTROL);
		enable = 0;
	}

	strlcpy(fb->fix.id, fbname, sizeof(fb->fix.id));

	memcpy(&par->ops,
	       (head == HEAD_CRT) ? &sm501fb_ops_crt : &sm501fb_ops_pnl,
	       sizeof(struct fb_ops));

	

	if ((pd->flags & SM501FB_FLAG_USE_HWCURSOR) == 0)
		par->ops.fb_cursor = NULL;

	fb->fbops = &par->ops;
	fb->flags = FBINFO_FLAG_DEFAULT |
		FBINFO_HWACCEL_XPAN | FBINFO_HWACCEL_YPAN;

	

	fb->fix.type		= FB_TYPE_PACKED_PIXELS;
	fb->fix.type_aux	= 0;
	fb->fix.xpanstep	= 1;
	fb->fix.ypanstep	= 1;
	fb->fix.ywrapstep	= 0;
	fb->fix.accel		= FB_ACCEL_NONE;

	

	fb->var.nonstd		= 0;
	fb->var.activate	= FB_ACTIVATE_NOW;
	fb->var.accel_flags	= 0;
	fb->var.vmode		= FB_VMODE_NONINTERLACED;
	fb->var.bits_per_pixel  = 16;

	if (enable && (pd->flags & SM501FB_FLAG_USE_INIT_MODE) && 0) {
		

	} else {
		if (pd->def_mode) {
			dev_info(info->dev, "using supplied mode\n");
			fb_videomode_to_var(&fb->var, pd->def_mode);

			fb->var.bits_per_pixel = pd->def_bpp ? pd->def_bpp : 8;
			fb->var.xres_virtual = fb->var.xres;
			fb->var.yres_virtual = fb->var.yres;
		} else {
			ret = fb_find_mode(&fb->var, fb,
					   NULL, NULL, 0, NULL, 8);

			if (ret == 0 || ret == 4) {
				dev_err(info->dev,
					"failed to get initial mode\n");
				return -EINVAL;
			}
		}
	}

	
	if (fb_alloc_cmap(&fb->cmap, NR_PALETTE, 0)) {
		dev_err(info->dev, "failed to allocate cmap memory\n");
		return -ENOMEM;
	}
	fb_set_cmap(&fb->cmap, fb);

	ret = (fb->fbops->fb_check_var)(&fb->var, fb);
	if (ret)
		dev_err(info->dev, "check_var() failed on initial setup?\n");

	return 0;
}



static struct sm501_platdata_fbsub sm501fb_pdata_crt = {
	.flags		= (SM501FB_FLAG_USE_INIT_MODE |
			   SM501FB_FLAG_USE_HWCURSOR |
			   SM501FB_FLAG_USE_HWACCEL |
			   SM501FB_FLAG_DISABLE_AT_EXIT),

};

static struct sm501_platdata_fbsub sm501fb_pdata_pnl = {
	.flags		= (SM501FB_FLAG_USE_INIT_MODE |
			   SM501FB_FLAG_USE_HWCURSOR |
			   SM501FB_FLAG_USE_HWACCEL |
			   SM501FB_FLAG_DISABLE_AT_EXIT),
};

static struct sm501_platdata_fb sm501fb_def_pdata = {
	.fb_route		= SM501_FB_OWN,
	.fb_crt			= &sm501fb_pdata_crt,
	.fb_pnl			= &sm501fb_pdata_pnl,
};

static char driver_name_crt[] = "sm501fb-crt";
static char driver_name_pnl[] = "sm501fb-panel";

static int __devinit sm501fb_probe_one(struct sm501fb_info *info,
				       enum sm501_controller head)
{
	unsigned char *name = (head == HEAD_CRT) ? "crt" : "panel";
	struct sm501_platdata_fbsub *pd;
	struct sm501fb_par *par;
	struct fb_info *fbi;

	pd = (head == HEAD_CRT) ? info->pdata->fb_crt : info->pdata->fb_pnl;

	
	if (pd == NULL) {
		dev_info(info->dev, "no data for fb %s (disabled)\n", name);
		return 0;
	}

	fbi = framebuffer_alloc(sizeof(struct sm501fb_par), info->dev);
	if (fbi == NULL) {
		dev_err(info->dev, "cannot allocate %s framebuffer\n", name);
		return -ENOMEM;
	}

	par = fbi->par;
	par->info = info;
	par->head = head;
	fbi->pseudo_palette = &par->pseudo_palette;

	info->fb[head] = fbi;

	return 0;
}



static void sm501_free_init_fb(struct sm501fb_info *info,
				enum sm501_controller head)
{
	struct fb_info *fbi = info->fb[head];

	fb_dealloc_cmap(&fbi->cmap);
}

static int __devinit sm501fb_start_one(struct sm501fb_info *info,
				       enum sm501_controller head,
				       const char *drvname)
{
	struct fb_info *fbi = info->fb[head];
	int ret;

	if (!fbi)
		return 0;

	mutex_init(&info->fb[head]->mm_lock);

	ret = sm501fb_init_fb(info->fb[head], head, drvname);
	if (ret) {
		dev_err(info->dev, "cannot initialise fb %s\n", drvname);
		return ret;
	}

	ret = register_framebuffer(info->fb[head]);
	if (ret) {
		dev_err(info->dev, "failed to register fb %s\n", drvname);
		sm501_free_init_fb(info, head);
		return ret;
	}

	dev_info(info->dev, "fb%d: %s frame buffer\n", fbi->node, fbi->fix.id);

	return 0;
}

static int __devinit sm501fb_probe(struct platform_device *pdev)
{
	struct sm501fb_info *info;
	struct device *dev = &pdev->dev;
	int ret;

	

	info = kzalloc(sizeof(struct sm501fb_info), GFP_KERNEL);
	if (!info) {
		dev_err(dev, "failed to allocate state\n");
		return -ENOMEM;
	}

	info->dev = dev = &pdev->dev;
	platform_set_drvdata(pdev, info);

	if (dev->parent->platform_data) {
		struct sm501_platdata *pd = dev->parent->platform_data;
		info->pdata = pd->fb;
	}

	if (info->pdata == NULL) {
		dev_info(dev, "using default configuration data\n");
		info->pdata = &sm501fb_def_pdata;
	}

	

	ret = sm501fb_probe_one(info, HEAD_CRT);
	if (ret < 0) {
		dev_err(dev, "failed to probe CRT\n");
		goto err_alloc;
	}

	ret = sm501fb_probe_one(info, HEAD_PANEL);
	if (ret < 0) {
		dev_err(dev, "failed to probe PANEL\n");
		goto err_probed_crt;
	}

	if (info->fb[HEAD_PANEL] == NULL &&
	    info->fb[HEAD_CRT] == NULL) {
		dev_err(dev, "no framebuffers found\n");
		goto err_alloc;
	}

	

	ret = sm501fb_start(info, pdev);
	if (ret) {
		dev_err(dev, "cannot initialise SM501\n");
		goto err_probed_panel;
	}

	ret = sm501fb_start_one(info, HEAD_CRT, driver_name_crt);
	if (ret) {
		dev_err(dev, "failed to start CRT\n");
		goto err_started;
	}

	ret = sm501fb_start_one(info, HEAD_PANEL, driver_name_pnl);
	if (ret) {
		dev_err(dev, "failed to start Panel\n");
		goto err_started_crt;
	}

	

	ret = device_create_file(dev, &dev_attr_crt_src);
	if (ret)
		goto err_started_panel;

	ret = device_create_file(dev, &dev_attr_fbregs_pnl);
	if (ret)
		goto err_attached_crtsrc_file;

	ret = device_create_file(dev, &dev_attr_fbregs_crt);
	if (ret)
		goto err_attached_pnlregs_file;

	
	return 0;

err_attached_pnlregs_file:
	device_remove_file(dev, &dev_attr_fbregs_pnl);

err_attached_crtsrc_file:
	device_remove_file(dev, &dev_attr_crt_src);

err_started_panel:
	unregister_framebuffer(info->fb[HEAD_PANEL]);
	sm501_free_init_fb(info, HEAD_PANEL);

err_started_crt:
	unregister_framebuffer(info->fb[HEAD_CRT]);
	sm501_free_init_fb(info, HEAD_CRT);

err_started:
	sm501fb_stop(info);

err_probed_panel:
	framebuffer_release(info->fb[HEAD_PANEL]);

err_probed_crt:
	framebuffer_release(info->fb[HEAD_CRT]);

err_alloc:
	kfree(info);

	return ret;
}



static int sm501fb_remove(struct platform_device *pdev)
{
	struct sm501fb_info *info = platform_get_drvdata(pdev);
	struct fb_info	   *fbinfo_crt = info->fb[0];
	struct fb_info	   *fbinfo_pnl = info->fb[1];

	device_remove_file(&pdev->dev, &dev_attr_fbregs_crt);
	device_remove_file(&pdev->dev, &dev_attr_fbregs_pnl);
	device_remove_file(&pdev->dev, &dev_attr_crt_src);

	sm501_free_init_fb(info, HEAD_CRT);
	sm501_free_init_fb(info, HEAD_PANEL);

	unregister_framebuffer(fbinfo_crt);
	unregister_framebuffer(fbinfo_pnl);

	sm501fb_stop(info);
	kfree(info);

	framebuffer_release(fbinfo_pnl);
	framebuffer_release(fbinfo_crt);

	return 0;
}

#ifdef CONFIG_PM

static int sm501fb_suspend_fb(struct sm501fb_info *info,
			      enum sm501_controller head)
{
	struct fb_info *fbi = info->fb[head];
	struct sm501fb_par *par = fbi->par;

	if (par->screen.size == 0)
		return 0;

	
	(par->ops.fb_blank)(FB_BLANK_POWERDOWN, fbi);

	

	acquire_console_sem();
	fb_set_suspend(fbi, 1);
	release_console_sem();

	

	par->store_fb = vmalloc(par->screen.size);
	if (par->store_fb == NULL) {
		dev_err(info->dev, "no memory to store screen\n");
		return -ENOMEM;
	}

	par->store_cursor = vmalloc(par->cursor.size);
	if (par->store_cursor == NULL) {
		dev_err(info->dev, "no memory to store cursor\n");
		goto err_nocursor;
	}

	dev_dbg(info->dev, "suspending screen to %p\n", par->store_fb);
	dev_dbg(info->dev, "suspending cursor to %p\n", par->store_cursor);

	memcpy_fromio(par->store_fb, par->screen.k_addr, par->screen.size);
	memcpy_fromio(par->store_cursor, par->cursor.k_addr, par->cursor.size);

	return 0;

 err_nocursor:
	vfree(par->store_fb);
	par->store_fb = NULL;

	return -ENOMEM;
}

static void sm501fb_resume_fb(struct sm501fb_info *info,
			      enum sm501_controller head)
{
	struct fb_info *fbi = info->fb[head];
	struct sm501fb_par *par = fbi->par;

	if (par->screen.size == 0)
		return;

	

	(par->ops.fb_set_par)(fbi);

	

	dev_dbg(info->dev, "restoring screen from %p\n", par->store_fb);
	dev_dbg(info->dev, "restoring cursor from %p\n", par->store_cursor);

	if (par->store_fb)
		memcpy_toio(par->screen.k_addr, par->store_fb,
			    par->screen.size);

	if (par->store_cursor)
		memcpy_toio(par->cursor.k_addr, par->store_cursor,
			    par->cursor.size);

	acquire_console_sem();
	fb_set_suspend(fbi, 0);
	release_console_sem();

	vfree(par->store_fb);
	vfree(par->store_cursor);
}




static int sm501fb_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct sm501fb_info *info = platform_get_drvdata(pdev);

	
	info->pm_crt_ctrl = readl(info->regs + SM501_DC_CRT_CONTROL);

	sm501fb_suspend_fb(info, HEAD_CRT);
	sm501fb_suspend_fb(info, HEAD_PANEL);

	
	sm501_unit_power(info->dev->parent, SM501_GATE_DISPLAY, 0);

	return 0;
}

#define SM501_CRT_CTRL_SAVE (SM501_DC_CRT_CONTROL_TVP |        \
			     SM501_DC_CRT_CONTROL_SEL)


static int sm501fb_resume(struct platform_device *pdev)
{
	struct sm501fb_info *info = platform_get_drvdata(pdev);
	unsigned long crt_ctrl;

	sm501_unit_power(info->dev->parent, SM501_GATE_DISPLAY, 1);

	

	crt_ctrl = readl(info->regs + SM501_DC_CRT_CONTROL);
	crt_ctrl &= ~SM501_CRT_CTRL_SAVE;
	crt_ctrl |= info->pm_crt_ctrl & SM501_CRT_CTRL_SAVE;
	writel(crt_ctrl, info->regs + SM501_DC_CRT_CONTROL);

	sm501fb_resume_fb(info, HEAD_CRT);
	sm501fb_resume_fb(info, HEAD_PANEL);

	return 0;
}

#else
#define sm501fb_suspend NULL
#define sm501fb_resume  NULL
#endif

static struct platform_driver sm501fb_driver = {
	.probe		= sm501fb_probe,
	.remove		= sm501fb_remove,
	.suspend	= sm501fb_suspend,
	.resume		= sm501fb_resume,
	.driver		= {
		.name	= "sm501-fb",
		.owner	= THIS_MODULE,
	},
};

static int __devinit sm501fb_init(void)
{
	return platform_driver_register(&sm501fb_driver);
}

static void __exit sm501fb_cleanup(void)
{
	platform_driver_unregister(&sm501fb_driver);
}

module_init(sm501fb_init);
module_exit(sm501fb_cleanup);

MODULE_AUTHOR("Ben Dooks, Vincent Sanders");
MODULE_DESCRIPTION("SM501 Framebuffer driver");
MODULE_LICENSE("GPL v2");
