



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/nubus.h>
#include <linux/init.h>
#include <linux/fb.h>

#include <asm/setup.h>
#include <asm/bootinfo.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/irq.h>
#include <asm/macintosh.h>
#include <asm/io.h>


#define DAC_BASE 0x50f24000


#define DAFB_BASE 0xf9800200


#define CIVIC_BASE 0x50f30800	


#define GSC_BASE 0x50F20000


#define CSC_BASE 0x50F20000

static int (*macfb_setpalette) (unsigned int regno, unsigned int red,
				unsigned int green, unsigned int blue,
				struct fb_info *info) = NULL;
static int valkyrie_setpalette (unsigned int regno, unsigned int red,
				unsigned int green, unsigned int blue,
				struct fb_info *info);
static int dafb_setpalette (unsigned int regno, unsigned int red,
			    unsigned int green, unsigned int blue,
			    struct fb_info *fb_info);
static int rbv_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *fb_info);
static int mdc_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *fb_info);
static int toby_setpalette (unsigned int regno, unsigned int red,
			    unsigned int green, unsigned int blue,
			    struct fb_info *fb_info);
static int civic_setpalette (unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     struct fb_info *fb_info);
static int csc_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *fb_info);

static struct {
	unsigned char addr;
	
	char pad[3];
	unsigned char lut;
} __iomem *valkyrie_cmap_regs;

static struct {
	unsigned char addr;
	unsigned char lut;
} __iomem *v8_brazil_cmap_regs;

static struct {
	unsigned char addr;
	char pad1[3]; 
	unsigned char lut;
	char pad2[3]; 
	unsigned char cntl; 
} __iomem *rbv_cmap_regs;

static struct {
	unsigned long reset;
	unsigned long pad1[3];
	unsigned char pad2[3];
	unsigned char lut;
} __iomem *dafb_cmap_regs;

static struct {
	unsigned char addr;	
	unsigned char pad1[15];
	unsigned char lut;	
	unsigned char pad2[15];
	unsigned char status;	
	unsigned char pad3[7];
	unsigned long vbl_addr;	
	unsigned int  status2;	
} __iomem *civic_cmap_regs;

static struct {
	char    pad1[0x40];
        unsigned char	clut_waddr;	
        char    pad2;
        unsigned char	clut_data;	
        char	pad3[0x3];
        unsigned char	clut_raddr;	
} __iomem *csc_cmap_regs;


struct mdc_cmap_regs {
	char pad1[0x200200];
	unsigned char addr;
	char pad2[6];
	unsigned char lut;
};

struct toby_cmap_regs {
	char pad1[0x90018];
	unsigned char lut; 
	char pad2[3];
	unsigned char addr; 
};

struct jet_cmap_regs {
	char pad1[0xe0e000];
	unsigned char addr;
	unsigned char lut;
};

#define PIXEL_TO_MM(a)	(((a)*10)/28)		


static int  video_slot = 0;

static struct fb_var_screeninfo macfb_defined = {
	.bits_per_pixel	= 8,	
	.activate	= FB_ACTIVATE_NOW,
	.width		= -1,
	.height		= -1,
	.right_margin	= 32,
	.upper_margin	= 16,
	.lower_margin	= 4,
	.vsync_len	= 4,
	.vmode		= FB_VMODE_NONINTERLACED,
};

static struct fb_fix_screeninfo macfb_fix = {
	.type	= FB_TYPE_PACKED_PIXELS,
	.accel	= FB_ACCEL_NONE,
};

static struct fb_info fb_info;
static u32 pseudo_palette[16];
static int inverse   = 0;
static int vidtest   = 0;

static int valkyrie_setpalette (unsigned int regno, unsigned int red,
				unsigned int green, unsigned int blue,
				struct fb_info *info)
{
	unsigned long flags;
	
	red >>= 8;
	green >>= 8;
	blue >>= 8;

	local_irq_save(flags);
	
	
	nubus_writeb(regno, &valkyrie_cmap_regs->addr);
	nop();

	
	nubus_writeb(red, &valkyrie_cmap_regs->lut);
	nop();
	nubus_writeb(green, &valkyrie_cmap_regs->lut);
	nop();
	nubus_writeb(blue, &valkyrie_cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}


static int dafb_setpalette (unsigned int regno, unsigned int red,
			    unsigned int green, unsigned int blue,
			    struct fb_info *info)
{
	
	static int lastreg = -1;
	unsigned long flags;
	
	red >>= 8;
	green >>= 8;
	blue >>= 8;

	local_irq_save(flags);
	
	
	if (regno != lastreg+1) {
		int i;
		
		
		nubus_writel(0, &dafb_cmap_regs->reset);
		nop();
		
		
		for (i = 0; i < regno; i++) {
			nubus_writeb(info->cmap.red[i] >> 8, &dafb_cmap_regs->lut);
			nop();
			nubus_writeb(info->cmap.green[i] >> 8, &dafb_cmap_regs->lut);
			nop();
			nubus_writeb(info->cmap.blue[i] >> 8, &dafb_cmap_regs->lut);
			nop();
		}
	}
		
	nubus_writeb(red, &dafb_cmap_regs->lut);
	nop();
	nubus_writeb(green, &dafb_cmap_regs->lut);
	nop();
	nubus_writeb(blue, &dafb_cmap_regs->lut);
	
	local_irq_restore(flags);
	lastreg = regno;
	return 0;
}


static int v8_brazil_setpalette (unsigned int regno, unsigned int red,
				 unsigned int green, unsigned int blue,
				 struct fb_info *info)	
{
	unsigned int bpp = info->var.bits_per_pixel;
	unsigned char _red  =red>>8;
	unsigned char _green=green>>8;
	unsigned char _blue =blue>>8;
	unsigned char _regno;
	unsigned long flags;

	if (bpp > 8) return 1; 

	local_irq_save(flags);

	
  	_regno = (regno << (8 - bpp)) | (0xFF >> bpp);
	nubus_writeb(_regno, &v8_brazil_cmap_regs->addr); nop();

	
	nubus_writeb(_red, &v8_brazil_cmap_regs->lut); nop();
	nubus_writeb(_green, &v8_brazil_cmap_regs->lut); nop();
	nubus_writeb(_blue, &v8_brazil_cmap_regs->lut);

	local_irq_restore(flags);	
	return 0;
}

static int rbv_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *info)
{
	
	unsigned char _red  =red>>8;
	unsigned char _green=green>>8;
	unsigned char _blue =blue>>8;
	unsigned char _regno;
	unsigned long flags;

	if (info->var.bits_per_pixel > 8) return 1; 

	local_irq_save(flags);
	
	
	_regno = regno + (256-(1 << info->var.bits_per_pixel));

	
	nubus_writeb(0xFF, &rbv_cmap_regs->cntl); nop();
	
	
	nubus_writeb(_regno, &rbv_cmap_regs->addr); nop();
	
	
	nubus_writeb(_red,   &rbv_cmap_regs->lut); nop();
	nubus_writeb(_green, &rbv_cmap_regs->lut); nop();
	nubus_writeb(_blue,  &rbv_cmap_regs->lut);
	
	local_irq_restore(flags); 
	return 0;
}


static int mdc_setpalette(unsigned int regno, unsigned int red,
			  unsigned int green, unsigned int blue,
			  struct fb_info *info)
{
	volatile struct mdc_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	
	unsigned char _red  =red>>8;
	unsigned char _green=green>>8;
	unsigned char _blue =blue>>8;
	unsigned char _regno=regno;
	unsigned long flags;

	local_irq_save(flags);
	
	
	nubus_writeb(_regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut);    nop();
	nubus_writeb(_green, &cmap_regs->lut);  nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}


static int toby_setpalette(unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *info) 
{
	volatile struct toby_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	unsigned int bpp = info->var.bits_per_pixel;
	
	unsigned char _red  =~(red>>8);
	unsigned char _green=~(green>>8);
	unsigned char _blue =~(blue>>8);
	unsigned char _regno = (regno << (8 - bpp)) | (0xFF >> bpp);
	unsigned long flags;

	local_irq_save(flags);
		
	nubus_writeb(_regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut);    nop();
	nubus_writeb(_green, &cmap_regs->lut);  nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}


static int jet_setpalette(unsigned int regno, unsigned int red,
			  unsigned int green, unsigned int blue,
			  struct fb_info *info)
{
	volatile struct jet_cmap_regs *cmap_regs =
		nubus_slot_addr(video_slot);
	
	unsigned char _red   = (red>>8);
	unsigned char _green = (green>>8);
	unsigned char _blue  = (blue>>8);
	unsigned long flags;

	local_irq_save(flags);
	
	nubus_writeb(regno, &cmap_regs->addr); nop();
	nubus_writeb(_red, &cmap_regs->lut); nop();
	nubus_writeb(_green, &cmap_regs->lut); nop();
	nubus_writeb(_blue, &cmap_regs->lut);

	local_irq_restore(flags);
	return 0;
}


static int civic_setpalette (unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     struct fb_info *info)
{
	static int lastreg = -1;
	unsigned long flags;
	int clut_status;
	
	if (info->var.bits_per_pixel > 8) return 1; 

	red   >>= 8;
	green >>= 8;
	blue  >>= 8;

	local_irq_save(flags);
	
	
	nubus_writeb(regno, &civic_cmap_regs->addr); nop();

	
#if 0
	{
#define CIVIC_VBL_OFFSET	0x120
		volatile unsigned long *vbl = nubus_readl(civic_cmap_regs->vbl_addr + CIVIC_VBL_OFFSET);
		
		*vbl = 0L; nop();	
		*vbl = 1L; nop();	
		while (*vbl != 0L)	
		{
			usleep(10);	
		}
		
	}
#endif

	
	clut_status =  nubus_readb(&civic_cmap_regs->status2);

	if ((clut_status & 0x0008) == 0)
	{
#if 0
		if ((clut_status & 0x000D) != 0)
		{
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
		}
#endif

		nubus_writeb(  red, &civic_cmap_regs->lut); nop();
		nubus_writeb(green, &civic_cmap_regs->lut); nop();
		nubus_writeb( blue, &civic_cmap_regs->lut); nop();
		nubus_writeb( 0x00, &civic_cmap_regs->lut); nop();
	}
	else
	{
		unsigned char junk;

		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();
		junk = nubus_readb(&civic_cmap_regs->lut); nop();

		if ((clut_status & 0x000D) != 0)
		{
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
			nubus_writeb(0x00, &civic_cmap_regs->lut); nop();
		}

		nubus_writeb(  red, &civic_cmap_regs->lut); nop();
		nubus_writeb(green, &civic_cmap_regs->lut); nop();
		nubus_writeb( blue, &civic_cmap_regs->lut); nop();
		nubus_writeb( junk, &civic_cmap_regs->lut); nop();
	}

	local_irq_restore(flags);
	lastreg = regno;
	return 0;
}



static int csc_setpalette (unsigned int regno, unsigned int red,
			   unsigned int green, unsigned int blue,
			   struct fb_info *info)
{
	mdelay(1);
	nubus_writeb(regno, &csc_cmap_regs->clut_waddr);
	nubus_writeb(red,   &csc_cmap_regs->clut_data);
	nubus_writeb(green, &csc_cmap_regs->clut_data);
	nubus_writeb(blue,  &csc_cmap_regs->clut_data);
	return 0;
}

static int macfb_setcolreg(unsigned regno, unsigned red, unsigned green,
			   unsigned blue, unsigned transp,
			   struct fb_info *fb_info)
{
	
	
	if (regno >= fb_info->cmap.len)
		return 1;

	if (fb_info->var.bits_per_pixel <= 8) {
		switch (fb_info->var.bits_per_pixel) {
		case 1:
			
			break;
		case 2:
		case 4:
		case 8:
			if (macfb_setpalette)
				macfb_setpalette(regno, red, green, blue,
						 fb_info);
			else
				return 1;
			break;
		}
	} else if (regno < 16) {
		switch (fb_info->var.bits_per_pixel) {
		case 16:
			if (fb_info->var.red.offset == 10) {
				
				((u32*) (fb_info->pseudo_palette))[regno] =
					((red   & 0xf800) >>  1) |
					((green & 0xf800) >>  6) |
					((blue  & 0xf800) >> 11) |
					((transp != 0) << 15);
			} else {
				
				((u32*) (fb_info->pseudo_palette))[regno] =
					((red   & 0xf800)      ) |
					((green & 0xfc00) >>  5) |
					((blue  & 0xf800) >> 11);
			}
			break;
			
		case 24:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((u32 *)(fb_info->pseudo_palette))[regno] =
				(red   << fb_info->var.red.offset)   |
				(green << fb_info->var.green.offset) |
				(blue  << fb_info->var.blue.offset);
			break;
		case 32:
			red   >>= 8;
			green >>= 8;
			blue  >>= 8;
			((u32 *)(fb_info->pseudo_palette))[regno] =
				(red   << fb_info->var.red.offset)   |
				(green << fb_info->var.green.offset) |
				(blue  << fb_info->var.blue.offset);
			break;
		}
	}

	return 0;
}

static struct fb_ops macfb_ops = {
	.owner		= THIS_MODULE,
	.fb_setcolreg	= macfb_setcolreg,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};

static void __init macfb_setup(char *options)
{
	char *this_opt;
	
	if (!options || !*options)
		return;
	
	while ((this_opt = strsep(&options, ",")) != NULL) {
		if (!*this_opt) continue;
		
		if (! strcmp(this_opt, "inverse"))
			inverse=1;
		
		else if (!strcmp(this_opt, "vidtest"))
			vidtest=1;
	}
}

static void __init iounmap_macfb(void)
{
	if (valkyrie_cmap_regs)
		iounmap(valkyrie_cmap_regs);
	if (dafb_cmap_regs)
		iounmap(dafb_cmap_regs);
	if (v8_brazil_cmap_regs)
		iounmap(v8_brazil_cmap_regs);
	if (rbv_cmap_regs)
		iounmap(rbv_cmap_regs);
	if (civic_cmap_regs)
		iounmap(civic_cmap_regs);
	if (csc_cmap_regs)
		iounmap(csc_cmap_regs);
}

static int __init macfb_init(void)
{
	int video_cmap_len, video_is_nubus = 0;
	struct nubus_dev* ndev = NULL;
	char *option = NULL;
	int err;

	if (fb_get_options("macfb", &option))
		return -ENODEV;
	macfb_setup(option);

	if (!MACH_IS_MAC) 
		return -ENODEV;

	
	macfb_defined.xres = mac_bi_data.dimensions & 0xFFFF;
	macfb_defined.yres = mac_bi_data.dimensions >> 16;
	macfb_defined.bits_per_pixel = mac_bi_data.videodepth;
	macfb_fix.line_length = mac_bi_data.videorow;
	macfb_fix.smem_len = macfb_fix.line_length * macfb_defined.yres;
	
	macfb_fix.smem_start = mac_bi_data.videoaddr;
	
	fb_info.screen_base = ioremap(mac_bi_data.videoaddr, macfb_fix.smem_len);
	
	printk("macfb: framebuffer at 0x%08lx, mapped to 0x%p, size %dk\n",
	       macfb_fix.smem_start, fb_info.screen_base, macfb_fix.smem_len/1024);
	printk("macfb: mode is %dx%dx%d, linelength=%d\n",
	       macfb_defined.xres, macfb_defined.yres, macfb_defined.bits_per_pixel, macfb_fix.line_length);
	
	
	 
	macfb_defined.xres_virtual   = macfb_defined.xres;
	macfb_defined.yres_virtual   = macfb_defined.yres;
	macfb_defined.height = PIXEL_TO_MM(macfb_defined.yres);
	macfb_defined.width  = PIXEL_TO_MM(macfb_defined.xres);	 

	printk("macfb: scrolling: redraw\n");
	macfb_defined.yres_virtual = macfb_defined.yres;

	
	macfb_defined.pixclock     = 10000000 / macfb_defined.xres * 1000 / macfb_defined.yres;
	macfb_defined.left_margin  = (macfb_defined.xres / 8) & 0xf8;
	macfb_defined.hsync_len    = (macfb_defined.xres / 8) & 0xf8;

	switch (macfb_defined.bits_per_pixel) {
	case 1:
		
		macfb_defined.red.length = macfb_defined.bits_per_pixel;
		macfb_defined.green.length = macfb_defined.bits_per_pixel;
		macfb_defined.blue.length = macfb_defined.bits_per_pixel;
		video_cmap_len = 0;
		macfb_fix.visual = FB_VISUAL_MONO01;
		break;
	case 2:
	case 4:
	case 8:
		macfb_defined.red.length = macfb_defined.bits_per_pixel;
		macfb_defined.green.length = macfb_defined.bits_per_pixel;
		macfb_defined.blue.length = macfb_defined.bits_per_pixel;
		video_cmap_len = 1 << macfb_defined.bits_per_pixel;
		macfb_fix.visual = FB_VISUAL_PSEUDOCOLOR;
		break;
	case 16:
		macfb_defined.transp.offset = 15;
		macfb_defined.transp.length = 1;
		macfb_defined.red.offset = 10;
		macfb_defined.red.length = 5;
		macfb_defined.green.offset = 5;
		macfb_defined.green.length = 5;
		macfb_defined.blue.offset = 0;
		macfb_defined.blue.length = 5;
		printk("macfb: directcolor: "
		       "size=1:5:5:5, shift=15:10:5:0\n");
		video_cmap_len = 16;
		
		macfb_fix.visual = FB_VISUAL_TRUECOLOR;
		break;
	case 24:
	case 32:
		
		macfb_defined.red.offset = 16;
		macfb_defined.red.length = 8;
		macfb_defined.green.offset = 8;
		macfb_defined.green.length = 8;
		macfb_defined.blue.offset = 0;
		macfb_defined.blue.length = 8;
		printk("macfb: truecolor: "
		       "size=0:8:8:8, shift=0:16:8:0\n");
		video_cmap_len = 16;
		macfb_fix.visual = FB_VISUAL_TRUECOLOR;
	default:
		video_cmap_len = 0;
		macfb_fix.visual = FB_VISUAL_MONO01;
		printk("macfb: unknown or unsupported bit depth: %d\n", macfb_defined.bits_per_pixel);
		break;
	}
	
	
	
	

	while ((ndev = nubus_find_type(NUBUS_CAT_DISPLAY, NUBUS_TYPE_VIDEO, ndev))
		!= NULL)
	{
		if (!(mac_bi_data.videoaddr >= ndev->board->slot_addr
		      && (mac_bi_data.videoaddr <
			  (unsigned long)nubus_slot_addr(ndev->board->slot+1))))
			continue;
		video_is_nubus = 1;
		
		video_slot = ndev->board->slot;

		switch(ndev->dr_hw) {
		case NUBUS_DRHW_APPLE_MDC:
			strcpy(macfb_fix.id, "Mac Disp. Card");
			macfb_setpalette = mdc_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			break;
		case NUBUS_DRHW_APPLE_TFB:
			strcpy(macfb_fix.id, "Toby");
			macfb_setpalette = toby_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			break;
		case NUBUS_DRHW_APPLE_JET:
			strcpy(macfb_fix.id, "Jet");
			macfb_setpalette = jet_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			break;			
		default:
			strcpy(macfb_fix.id, "Generic NuBus");
			break;
		}
	}

	
	
	if (!video_is_nubus)
		switch( mac_bi_data.id )
		{
			
		case MAC_MODEL_Q630:
			
		case MAC_MODEL_P588:
			strcpy(macfb_fix.id, "Valkyrie");
			macfb_setpalette = valkyrie_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			valkyrie_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;

			
			
		case MAC_MODEL_P475:
		case MAC_MODEL_P475F:
		case MAC_MODEL_P575:
		case MAC_MODEL_Q605:
	
		case MAC_MODEL_Q800:
		case MAC_MODEL_Q650:
		case MAC_MODEL_Q610:
		case MAC_MODEL_C650:
		case MAC_MODEL_C610:
		case MAC_MODEL_Q700:
		case MAC_MODEL_Q900:
		case MAC_MODEL_Q950:
			strcpy(macfb_fix.id, "DAFB");
			macfb_setpalette = dafb_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			dafb_cmap_regs = ioremap(DAFB_BASE, 0x1000);
			break;

			
		case MAC_MODEL_LCII:
			strcpy(macfb_fix.id, "V8");
			macfb_setpalette = v8_brazil_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			v8_brazil_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;
		
			
		case MAC_MODEL_IIVI:
		case MAC_MODEL_IIVX:
		case MAC_MODEL_P600:
			strcpy(macfb_fix.id, "Brazil");
			macfb_setpalette = v8_brazil_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			v8_brazil_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;
		
			
			
			
		case MAC_MODEL_LCIII:
		case MAC_MODEL_P520:
		case MAC_MODEL_P550:
		case MAC_MODEL_P460:
			macfb_setpalette = v8_brazil_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "Sonora");
			v8_brazil_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;

			
		case MAC_MODEL_IICI:
		case MAC_MODEL_IISI:
			macfb_setpalette = rbv_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "RBV");
			rbv_cmap_regs = ioremap(DAC_BASE, 0x1000);
			break;

			
		case MAC_MODEL_Q840:
		case MAC_MODEL_C660:
			macfb_setpalette = civic_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "Civic");
			civic_cmap_regs = ioremap(CIVIC_BASE, 0x1000);
			break;

		
			

			
			
		case MAC_MODEL_LC:
			if (vidtest) {
				macfb_setpalette = v8_brazil_setpalette;
				macfb_defined.activate = FB_ACTIVATE_NOW;
				v8_brazil_cmap_regs =
					ioremap(DAC_BASE, 0x1000);
			}
			strcpy(macfb_fix.id, "LC");
			break;
			
		case MAC_MODEL_CCL:
			if (vidtest) {
				macfb_setpalette = v8_brazil_setpalette;
				macfb_defined.activate = FB_ACTIVATE_NOW;
				v8_brazil_cmap_regs =
					ioremap(DAC_BASE, 0x1000);
			}
			strcpy(macfb_fix.id, "Color Classic");
			break;

			
		case MAC_MODEL_TV:
			strcpy(macfb_fix.id, "Mac TV");
			break;

			
		case MAC_MODEL_SE30:
		case MAC_MODEL_CLII:
			strcpy(macfb_fix.id, "Monochrome");
			break;

			

		case MAC_MODEL_PB140:
		case MAC_MODEL_PB145:
		case MAC_MODEL_PB170:
			strcpy(macfb_fix.id, "DDC");
			break;

			
		case MAC_MODEL_PB150:	
		case MAC_MODEL_PB160:
		case MAC_MODEL_PB165:
		case MAC_MODEL_PB180:
		case MAC_MODEL_PB210:
		case MAC_MODEL_PB230:
			strcpy(macfb_fix.id, "GSC");
			break;

			
		case MAC_MODEL_PB165C:
		case MAC_MODEL_PB180C:
			strcpy(macfb_fix.id, "TIM");
			break;

			
		case MAC_MODEL_PB190:	
		case MAC_MODEL_PB520:
		case MAC_MODEL_PB250:
		case MAC_MODEL_PB270C:
		case MAC_MODEL_PB280:
		case MAC_MODEL_PB280C:
			macfb_setpalette = csc_setpalette;
			macfb_defined.activate = FB_ACTIVATE_NOW;
			strcpy(macfb_fix.id, "CSC");
			csc_cmap_regs = ioremap(CSC_BASE, 0x1000);
			break;
		
		default:
			strcpy(macfb_fix.id, "Unknown");
			break;
		}

	fb_info.fbops		= &macfb_ops;
	fb_info.var		= macfb_defined;
	fb_info.fix		= macfb_fix;
	fb_info.pseudo_palette	= pseudo_palette;
	fb_info.flags		= FBINFO_DEFAULT;

	err = fb_alloc_cmap(&fb_info.cmap, video_cmap_len, 0);
	if (err)
		goto fail_unmap;
	
	err = register_framebuffer(&fb_info);
	if (err)
		goto fail_dealloc;

	printk("fb%d: %s frame buffer device\n",
	       fb_info.node, fb_info.fix.id);
	return 0;

fail_dealloc:
	fb_dealloc_cmap(&fb_info.cmap);
fail_unmap:
	iounmap(fb_info.screen_base);
	iounmap_macfb();
	return err;
}

module_init(macfb_init);
MODULE_LICENSE("GPL");
