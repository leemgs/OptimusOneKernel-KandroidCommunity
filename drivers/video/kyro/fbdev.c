

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/ioctl.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#include <video/kyro.h>

#include "STG4000Reg.h"
#include "STG4000Interface.h"


#define PCI_VENDOR_ID_ST	0x104a
#define PCI_DEVICE_ID_STG4000	0x0010

#define KHZ2PICOS(a) (1000000000UL/(a))


static struct fb_fix_screeninfo kyro_fix __devinitdata = {
	.id		= "ST Kyro",
	.type		= FB_TYPE_PACKED_PIXELS,
	.visual		= FB_VISUAL_TRUECOLOR,
	.accel		= FB_ACCEL_NONE,
};

static struct fb_var_screeninfo kyro_var __devinitdata = {
	
	.xres		= 640,
	.yres		= 480,
	.xres_virtual	= 640,
	.yres_virtual	= 480,
	.bits_per_pixel	= 16,
	.red		= { 11, 5, 0 },
	.green		= {  5, 6, 0 },
	.blue		= {  0, 5, 0 },
	.activate	= FB_ACTIVATE_NOW,
	.height		= -1,
	.width		= -1,
	.pixclock	= KHZ2PICOS(25175),
	.left_margin	= 48,
	.right_margin	= 16,
	.upper_margin	= 33,
	.lower_margin	= 10,
	.hsync_len	= 96,
	.vsync_len	= 2,
	.vmode		= FB_VMODE_NONINTERLACED,
};

typedef struct {
	STG4000REG __iomem *pSTGReg;	
	u32 ulNextFreeVidMem;	
	u32 ulOverlayOffset;	
	u32 ulOverlayStride;	
	u32 ulOverlayUVStride;	
} device_info_t;


static device_info_t deviceInfo;

static char *mode_option __devinitdata = NULL;
static int nopan __devinitdata = 0;
static int nowrap __devinitdata = 1;
#ifdef CONFIG_MTRR
static int nomtrr __devinitdata = 0;
#endif


static int kyrofb_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void kyrofb_remove(struct pci_dev *pdev);

static struct fb_videomode kyro_modedb[] __devinitdata = {
	{
		
		NULL, 85, 640, 350, KHZ2PICOS(31500),
		96, 32, 60, 32, 64, 3,
		FB_SYNC_HOR_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 640, 400, KHZ2PICOS(31500),
		96, 32, 41, 1, 64, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 720, 400, KHZ2PICOS(35500),
		108, 36, 42, 1, 72, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 640, 480, KHZ2PICOS(25175),
		48, 16, 33, 10, 96, 2,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 72, 640, 480, KHZ2PICOS(31500),
		128, 24, 28, 9, 40, 3,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 640, 480, KHZ2PICOS(31500),
		120, 16, 16, 1, 64, 3,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 640, 480, KHZ2PICOS(36000),
		80, 56, 25, 1, 56, 3,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 56, 800, 600, KHZ2PICOS(36000),
		128, 24, 22, 1, 72, 2,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 800, 600, KHZ2PICOS(40000),
		88, 40, 23, 1, 128, 4,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 72, 800, 600, KHZ2PICOS(50000),
		64, 56, 23, 37, 120, 6,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 800, 600, KHZ2PICOS(49500),
		160, 16, 21, 1, 80, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 800, 600, KHZ2PICOS(56250),
		152, 32, 27, 1, 64, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1024, 768, KHZ2PICOS(65000),
		160, 24, 29, 3, 136, 6,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 70, 1024, 768, KHZ2PICOS(75000),
		144, 24, 29, 3, 136, 6,
		0, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1024, 768, KHZ2PICOS(78750),
		176, 16, 28, 1, 96, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 1024, 768, KHZ2PICOS(94500),
		208, 48, 36, 1, 96, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1152, 864, KHZ2PICOS(108000),
		256, 64, 32, 1, 128, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1280, 960, KHZ2PICOS(108000),
		312, 96, 36, 1, 112, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 1280, 960, KHZ2PICOS(148500),
		224, 64, 47, 1, 160, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1280, 1024, KHZ2PICOS(108000),
		248, 48, 38, 1, 112, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1280, 1024, KHZ2PICOS(135000),
		248, 16, 38, 1, 144, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 1280, 1024, KHZ2PICOS(157500),
		224, 64, 44, 1, 160, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1600, 1200, KHZ2PICOS(162000),
		304, 64, 46, 1, 192, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 65, 1600, 1200, KHZ2PICOS(175500),
		304, 64, 46, 1, 192, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 70, 1600, 1200, KHZ2PICOS(189000),
		304, 64, 46, 1, 192, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1600, 1200, KHZ2PICOS(202500),
		304, 64, 46, 1, 192, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 85, 1600, 1200, KHZ2PICOS(229500),
		304, 64, 46, 1, 192, 3,
		FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1792, 1344, KHZ2PICOS(204750),
		328, 128, 46, 1, 200, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1792, 1344, KHZ2PICOS(261000),
		352, 96, 69, 1, 216, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1856, 1392, KHZ2PICOS(218250),
		352, 96, 43, 1, 224, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1856, 1392, KHZ2PICOS(288000),
		352, 128, 104, 1, 224, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 60, 1920, 1440, KHZ2PICOS(234000),
		344, 128, 56, 1, 208, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	}, {
		
		NULL, 75, 1920, 1440, KHZ2PICOS(297000),
		352, 144, 56, 1, 224, 3,
		FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
	},
};
#define NUM_TOTAL_MODES	ARRAY_SIZE(kyro_modedb)


enum {
	VMODE_640_350_85,
	VMODE_640_400_85,
	VMODE_720_400_85,
	VMODE_640_480_60,
	VMODE_640_480_72,
	VMODE_640_480_75,
	VMODE_640_480_85,
	VMODE_800_600_56,
	VMODE_800_600_60,
	VMODE_800_600_72,
	VMODE_800_600_75,
	VMODE_800_600_85,
	VMODE_1024_768_60,
	VMODE_1024_768_70,
	VMODE_1024_768_75,
	VMODE_1024_768_85,
	VMODE_1152_864_75,
	VMODE_1280_960_60,
	VMODE_1280_960_85,
	VMODE_1280_1024_60,
	VMODE_1280_1024_75,
	VMODE_1280_1024_85,
	VMODE_1600_1200_60,
	VMODE_1600_1200_65,
	VMODE_1600_1200_70,
	VMODE_1600_1200_75,
	VMODE_1600_1200_85,
	VMODE_1792_1344_60,
	VMODE_1792_1344_75,
	VMODE_1856_1392_60,
	VMODE_1856_1392_75,
	VMODE_1920_1440_60,
	VMODE_1920_1440_75,
};


static int kyro_dev_video_mode_set(struct fb_info *info)
{
	struct kyrofb_info *par = info->par;

	
	StopVTG(deviceInfo.pSTGReg);
	DisableRamdacOutput(deviceInfo.pSTGReg);

	
	DisableVGA(deviceInfo.pSTGReg);

	if (InitialiseRamdac(deviceInfo.pSTGReg,
			     info->var.bits_per_pixel,
			     info->var.xres, info->var.yres,
			     par->HSP, par->VSP, &par->PIXCLK) < 0)
		return -EINVAL;

	SetupVTG(deviceInfo.pSTGReg, par);

	ResetOverlayRegisters(deviceInfo.pSTGReg);

	
	EnableRamdacOutput(deviceInfo.pSTGReg);
	StartVTG(deviceInfo.pSTGReg);

	deviceInfo.ulNextFreeVidMem = info->var.xres * info->var.yres *
				      info->var.bits_per_pixel;
	deviceInfo.ulOverlayOffset = 0;

	return 0;
}

static int kyro_dev_overlay_create(u32 ulWidth,
				   u32 ulHeight, int bLinear)
{
	u32 offset;
	u32 stride, uvStride;

	if (deviceInfo.ulOverlayOffset != 0)
		
		return -EINVAL;

	ResetOverlayRegisters(deviceInfo.pSTGReg);

	
	offset = deviceInfo.ulNextFreeVidMem;
	if ((offset & 0x1f) != 0) {
		offset = (offset + 32L) & 0xffffffE0L;
	}

	if (CreateOverlaySurface(deviceInfo.pSTGReg, ulWidth, ulHeight,
				 bLinear, offset, &stride, &uvStride) < 0)
		return -EINVAL;

	deviceInfo.ulOverlayOffset = offset;
	deviceInfo.ulOverlayStride = stride;
	deviceInfo.ulOverlayUVStride = uvStride;
	deviceInfo.ulNextFreeVidMem = offset + (ulHeight * stride) + (ulHeight * 2 * uvStride);

	SetOverlayBlendMode(deviceInfo.pSTGReg, GLOBAL_ALPHA, 0xf, 0x0);

	return 0;
}

static int kyro_dev_overlay_viewport_set(u32 x, u32 y, u32 ulWidth, u32 ulHeight)
{
	if (deviceInfo.ulOverlayOffset == 0)
		
		return -EINVAL;

	
	DisableRamdacOutput(deviceInfo.pSTGReg);

	SetOverlayViewPort(deviceInfo.pSTGReg,
			   x, y, x + ulWidth - 1, y + ulHeight - 1);

	EnableOverlayPlane(deviceInfo.pSTGReg);
	
	EnableRamdacOutput(deviceInfo.pSTGReg);

	return 0;
}

static inline unsigned long get_line_length(int x, int bpp)
{
	return (unsigned long)((((x*bpp)+31)&~31) >> 3);
}

static int kyrofb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct kyrofb_info *par = info->par;

	if (var->bits_per_pixel != 16 && var->bits_per_pixel != 32) {
		printk(KERN_WARNING "kyrofb: depth not supported: %u\n", var->bits_per_pixel);
		return -EINVAL;
	}

	switch (var->bits_per_pixel) {
	case 16:
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.length = 5;
		break;
	case 32:
		var->transp.offset = 24;
		var->red.offset = 16;
		var->green.offset = 8;
		var->blue.offset = 0;

		var->red.length = 8;
		var->green.length = 8;
		var->blue.length = 8;
		var->transp.length = 8;
		break;
	}

	
	var->height = var->width = -1;

	

	
	


	

	
	if ((var->activate & FB_ACTIVATE_MASK) == FB_ACTIVATE_TEST)
		return 0;

	var->left_margin = par->HBP;
	var->hsync_len = par->HST;
	var->right_margin = par->HFP;

	var->upper_margin = par->VBP;
	var->vsync_len = par->VST;
	var->lower_margin = par->VFP;

	if (par->HSP == 1)
		var->sync |= FB_SYNC_HOR_HIGH_ACT;
	if (par->VSP == 1)
		var->sync |= FB_SYNC_VERT_HIGH_ACT;

	return 0;
}

static int kyrofb_set_par(struct fb_info *info)
{
	struct kyrofb_info *par = info->par;
	unsigned long lineclock;
	unsigned long frameclock;

	
	par->XRES = info->var.xres;
	par->YRES = info->var.yres;

	
	par->PIXDEPTH = info->var.bits_per_pixel;

	
	
	lineclock = (info->var.pixclock * (info->var.xres +
				    info->var.right_margin +
				    info->var.hsync_len +
				    info->var.left_margin)) / 1000;


	
	frameclock = lineclock * (info->var.yres +
				  info->var.lower_margin +
				  info->var.vsync_len +
				  info->var.upper_margin);

	
	par->VFREQ = (1000000000 + (frameclock / 2)) / frameclock;
	par->HCLK = (1000000000 + (lineclock / 2)) / lineclock;
	par->PIXCLK = ((1000000000 + (info->var.pixclock / 2))
					/ info->var.pixclock) * 10;

	
	par->HFP = info->var.right_margin;
	par->HST = info->var.hsync_len;
	par->HBP = info->var.left_margin;
	par->HTot = par->XRES + par->HBP + par->HST + par->HFP;

	
	par->VFP = info->var.lower_margin;
	par->VST = info->var.vsync_len;
	par->VBP = info->var.upper_margin;
	par->VTot = par->YRES + par->VBP + par->VST + par->VFP;

	par->HSP = (info->var.sync & FB_SYNC_HOR_HIGH_ACT) ? 1 : 0;
	par->VSP = (info->var.sync & FB_SYNC_VERT_HIGH_ACT) ? 1 : 0;

	kyro_dev_video_mode_set(info);

	
	info->fix.line_length = get_line_length(par->XRES, par->PIXDEPTH);
	info->fix.visual = FB_VISUAL_TRUECOLOR;

	return 0;
}

static int kyrofb_setcolreg(u_int regno, u_int red, u_int green,
			    u_int blue, u_int transp, struct fb_info *info)
{
	struct kyrofb_info *par = info->par;

	if (regno > 255)
		return 1;	

	if (regno < 16) {
		switch (info->var.bits_per_pixel) {
		case 16:
			par->palette[regno] =
			     (red   & 0xf800) |
			    ((green & 0xfc00) >> 5) |
			    ((blue  & 0xf800) >> 11);
			break;
		case 32:
			red >>= 8; green >>= 8; blue >>= 8; transp >>= 8;
			par->palette[regno] =
			    (transp << 24) | (red << 16) | (green << 8) | blue;
			break;
		}
	}

	return 0;
}

#ifndef MODULE
static int __init kyrofb_setup(char *options)
{
	char *this_opt;

	if (!options || !*options)
		return 0;

	while ((this_opt = strsep(&options, ","))) {
		if (!*this_opt)
			continue;
		if (strcmp(this_opt, "nopan") == 0) {
			nopan = 1;
		} else if (strcmp(this_opt, "nowrap") == 0) {
			nowrap = 1;
#ifdef CONFIG_MTRR
		} else if (strcmp(this_opt, "nomtrr") == 0) {
			nomtrr = 1;
#endif
		} else {
			mode_option = this_opt;
		}
	}

	return 0;
}
#endif

static int kyrofb_ioctl(struct fb_info *info,
			unsigned int cmd, unsigned long arg)
{
	overlay_create ol_create;
	overlay_viewport_set ol_viewport_set;
	void __user *argp = (void __user *)arg;

	switch (cmd) {
	case KYRO_IOCTL_OVERLAY_CREATE:
		if (copy_from_user(&ol_create, argp, sizeof(overlay_create)))
			return -EFAULT;

		if (kyro_dev_overlay_create(ol_create.ulWidth,
					    ol_create.ulHeight, 0) < 0) {
			printk(KERN_ERR "Kyro FB: failed to create overlay surface.\n");

			return -EINVAL;
		}
		break;
	case KYRO_IOCTL_OVERLAY_VIEWPORT_SET:
		if (copy_from_user(&ol_viewport_set, argp,
			       sizeof(overlay_viewport_set)))
			return -EFAULT;

		if (kyro_dev_overlay_viewport_set(ol_viewport_set.xOrgin,
						  ol_viewport_set.yOrgin,
						  ol_viewport_set.xSize,
						  ol_viewport_set.ySize) != 0)
		{
			printk(KERN_ERR "Kyro FB: failed to create overlay viewport.\n");
			return -EINVAL;
		}
		break;
	case KYRO_IOCTL_SET_VIDEO_MODE:
		{
			printk(KERN_ERR "Kyro FB: KYRO_IOCTL_SET_VIDEO_MODE is"
				"obsolete, use the appropriate fb_ioctl()"
				"command instead.\n");
			return -EINVAL;
		}
		break;
	case KYRO_IOCTL_UVSTRIDE:
		if (copy_to_user(argp, &deviceInfo.ulOverlayUVStride, sizeof(unsigned long)))
			return -EFAULT;
		break;
	case KYRO_IOCTL_STRIDE:
		if (copy_to_user(argp, &deviceInfo.ulOverlayStride, sizeof(unsigned long)))
			return -EFAULT;
		break;
	case KYRO_IOCTL_OVERLAY_OFFSET:
		if (copy_to_user(argp, &deviceInfo.ulOverlayOffset, sizeof(unsigned long)))
			return -EFAULT;
		break;
	}

	return 0;
}

static struct pci_device_id kyrofb_pci_tbl[] = {
	{ PCI_VENDOR_ID_ST, PCI_DEVICE_ID_STG4000,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, kyrofb_pci_tbl);

static struct pci_driver kyrofb_pci_driver = {
	.name		= "kyrofb",
	.id_table	= kyrofb_pci_tbl,
	.probe		= kyrofb_probe,
	.remove		= __devexit_p(kyrofb_remove),
};

static struct fb_ops kyrofb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= kyrofb_check_var,
	.fb_set_par	= kyrofb_set_par,
	.fb_setcolreg	= kyrofb_setcolreg,
	.fb_ioctl	= kyrofb_ioctl,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static int __devinit kyrofb_probe(struct pci_dev *pdev,
				  const struct pci_device_id *ent)
{
	struct fb_info *info;
	struct kyrofb_info *currentpar;
	unsigned long size;
	int err;

	if ((err = pci_enable_device(pdev))) {
		printk(KERN_WARNING "kyrofb: Can't enable pdev: %d\n", err);
		return err;
	}

	info = framebuffer_alloc(sizeof(struct kyrofb_info), &pdev->dev);
	if (!info)
		return -ENOMEM;

	currentpar = info->par;

	kyro_fix.smem_start = pci_resource_start(pdev, 0);
	kyro_fix.smem_len   = pci_resource_len(pdev, 0);
	kyro_fix.mmio_start = pci_resource_start(pdev, 1);
	kyro_fix.mmio_len   = pci_resource_len(pdev, 1);

	currentpar->regbase = deviceInfo.pSTGReg =
		ioremap_nocache(kyro_fix.mmio_start, kyro_fix.mmio_len);

	info->screen_base = ioremap_nocache(kyro_fix.smem_start,
					    kyro_fix.smem_len);

#ifdef CONFIG_MTRR
	if (!nomtrr)
		currentpar->mtrr_handle =
			mtrr_add(kyro_fix.smem_start,
				 kyro_fix.smem_len,
				 MTRR_TYPE_WRCOMB, 1);
#endif

	kyro_fix.ypanstep	= nopan ? 0 : 1;
	kyro_fix.ywrapstep	= nowrap ? 0 : 1;

	info->fbops		= &kyrofb_ops;
	info->fix		= kyro_fix;
	info->pseudo_palette	= currentpar->palette;
	info->flags		= FBINFO_DEFAULT;

	SetCoreClockPLL(deviceInfo.pSTGReg, pdev);

	deviceInfo.ulNextFreeVidMem = 0;
	deviceInfo.ulOverlayOffset = 0;

	
	if (!fb_find_mode(&info->var, info, mode_option, kyro_modedb,
			  NUM_TOTAL_MODES, &kyro_modedb[VMODE_1024_768_75], 32))
		info->var = kyro_var;

	fb_alloc_cmap(&info->cmap, 256, 0);

	kyrofb_set_par(info);
	kyrofb_check_var(&info->var, info);

	size = get_line_length(info->var.xres_virtual,
			       info->var.bits_per_pixel);
	size *= info->var.yres_virtual;

	fb_memset(info->screen_base, 0, size);

	if (register_framebuffer(info) < 0)
		goto out_unmap;

	printk("fb%d: %s frame buffer device, at %dx%d@%d using %ldk/%ldk of VRAM\n",
	       info->node, info->fix.id, info->var.xres,
	       info->var.yres, info->var.bits_per_pixel, size >> 10,
	       (unsigned long)info->fix.smem_len >> 10);

	pci_set_drvdata(pdev, info);

	return 0;

out_unmap:
	iounmap(currentpar->regbase);
	iounmap(info->screen_base);
	framebuffer_release(info);

	return -EINVAL;
}

static void __devexit kyrofb_remove(struct pci_dev *pdev)
{
	struct fb_info *info = pci_get_drvdata(pdev);
	struct kyrofb_info *par = info->par;

	
	StopVTG(deviceInfo.pSTGReg);
	DisableRamdacOutput(deviceInfo.pSTGReg);

	
	SetCoreClockPLL(deviceInfo.pSTGReg, pdev);

	deviceInfo.ulNextFreeVidMem = 0;
	deviceInfo.ulOverlayOffset = 0;

	iounmap(info->screen_base);
	iounmap(par->regbase);

#ifdef CONFIG_MTRR
	if (par->mtrr_handle)
		mtrr_del(par->mtrr_handle,
			 info->fix.smem_start,
			 info->fix.smem_len);
#endif

	unregister_framebuffer(info);
	pci_set_drvdata(pdev, NULL);
	framebuffer_release(info);
}

static int __init kyrofb_init(void)
{
#ifndef MODULE
	char *option = NULL;

	if (fb_get_options("kyrofb", &option))
		return -ENODEV;
	kyrofb_setup(option);
#endif
	return pci_register_driver(&kyrofb_pci_driver);
}

static void __exit kyrofb_exit(void)
{
	pci_unregister_driver(&kyrofb_pci_driver);
}

module_init(kyrofb_init);

#ifdef MODULE
module_exit(kyrofb_exit);
#endif

MODULE_AUTHOR("STMicroelectronics; Paul Mundt <lethal@linux-sh.org>");
MODULE_LICENSE("GPL");
