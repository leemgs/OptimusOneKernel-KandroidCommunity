

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>

#include <linux/usb/musb.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <mach/mux.h>
#include <mach/usb.h>

#ifdef CONFIG_USB_MUSB_SOC

static struct resource musb_resources[] = {
	[0] = { 
		.flags	= IORESOURCE_MEM,
	},
	[1] = {	
		.start	= INT_243X_HS_USB_MC,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {	
		.start	= INT_243X_HS_USB_DMA,
		.flags	= IORESOURCE_IRQ,
	},
};

static int clk_on;

static int musb_set_clock(struct clk *clk, int state)
{
	if (state) {
		if (clk_on > 0)
			return -ENODEV;

		clk_enable(clk);
		clk_on = 1;
	} else {
		if (clk_on == 0)
			return -ENODEV;

		clk_disable(clk);
		clk_on = 0;
	}

	return 0;
}

static struct musb_hdrc_eps_bits musb_eps[] = {
	{	"ep1_tx", 10,	},
	{	"ep1_rx", 10,	},
	{	"ep2_tx", 9,	},
	{	"ep2_rx", 9,	},
	{	"ep3_tx", 3,	},
	{	"ep3_rx", 3,	},
	{	"ep4_tx", 3,	},
	{	"ep4_rx", 3,	},
	{	"ep5_tx", 3,	},
	{	"ep5_rx", 3,	},
	{	"ep6_tx", 3,	},
	{	"ep6_rx", 3,	},
	{	"ep7_tx", 3,	},
	{	"ep7_rx", 3,	},
	{	"ep8_tx", 2,	},
	{	"ep8_rx", 2,	},
	{	"ep9_tx", 2,	},
	{	"ep9_rx", 2,	},
	{	"ep10_tx", 2,	},
	{	"ep10_rx", 2,	},
	{	"ep11_tx", 2,	},
	{	"ep11_rx", 2,	},
	{	"ep12_tx", 2,	},
	{	"ep12_rx", 2,	},
	{	"ep13_tx", 2,	},
	{	"ep13_rx", 2,	},
	{	"ep14_tx", 2,	},
	{	"ep14_rx", 2,	},
	{	"ep15_tx", 2,	},
	{	"ep15_rx", 2,	},
};

static struct musb_hdrc_config musb_config = {
	.multipoint	= 1,
	.dyn_fifo	= 1,
	.soft_con	= 1,
	.dma		= 1,
	.num_eps	= 16,
	.dma_channels	= 7,
	.dma_req_chan	= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3),
	.ram_bits	= 12,
	.eps_bits	= musb_eps,
};

static struct musb_hdrc_platform_data musb_plat = {
#ifdef CONFIG_USB_MUSB_OTG
	.mode		= MUSB_OTG,
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
	.mode		= MUSB_HOST,
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
	.mode		= MUSB_PERIPHERAL,
#endif
	
	.set_clock	= musb_set_clock,
	.config		= &musb_config,

	
	.power		= 50,			
};

static u64 musb_dmamask = DMA_BIT_MASK(32);

static struct platform_device musb_device = {
	.name		= "musb_hdrc",
	.id		= -1,
	.dev = {
		.dma_mask		= &musb_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &musb_plat,
	},
	.num_resources	= ARRAY_SIZE(musb_resources),
	.resource	= musb_resources,
};

void __init usb_musb_init(void)
{
	if (cpu_is_omap243x())
		musb_resources[0].start = OMAP243X_HS_BASE;
	else
		musb_resources[0].start = OMAP34XX_HSUSB_OTG_BASE;
	musb_resources[0].end = musb_resources[0].start + SZ_8K - 1;

	
	musb_plat.clock = "ick";

	if (platform_device_register(&musb_device) < 0) {
		printk(KERN_ERR "Unable to register HS-USB (MUSB) device\n");
		return;
	}
}

#else
void __init usb_musb_init(void)
{
}
#endif 
