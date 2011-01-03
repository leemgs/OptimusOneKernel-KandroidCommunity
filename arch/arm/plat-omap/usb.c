 

#undef	DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/usb/otg.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/system.h>
#include <mach/hardware.h>

#include <mach/control.h>
#include <mach/mux.h>
#include <mach/usb.h>
#include <mach/board.h>

#ifdef CONFIG_ARCH_OMAP1

#define INT_USB_IRQ_GEN		IH2_BASE + 20
#define INT_USB_IRQ_NISO	IH2_BASE + 30
#define INT_USB_IRQ_ISO		IH2_BASE + 29
#define INT_USB_IRQ_HGEN	INT_USB_HHC_1
#define INT_USB_IRQ_OTG		IH2_BASE + 8

#else

#define INT_USB_IRQ_GEN		INT_24XX_USB_IRQ_GEN
#define INT_USB_IRQ_NISO	INT_24XX_USB_IRQ_NISO
#define INT_USB_IRQ_ISO		INT_24XX_USB_IRQ_ISO
#define INT_USB_IRQ_HGEN	INT_24XX_USB_IRQ_HGEN
#define INT_USB_IRQ_OTG		INT_24XX_USB_IRQ_OTG

#endif








#if defined(CONFIG_ARCH_OMAP_OTG) || defined(CONFIG_ARCH_OMAP15XX)

static void omap2_usb_devconf_clear(u8 port, u32 mask)
{
	u32 r;

	r = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
	r &= ~USBTXWRMODEI(port, mask);
	omap_ctrl_writel(r, OMAP2_CONTROL_DEVCONF0);
}

static void omap2_usb_devconf_set(u8 port, u32 mask)
{
	u32 r;

	r = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
	r |= USBTXWRMODEI(port, mask);
	omap_ctrl_writel(r, OMAP2_CONTROL_DEVCONF0);
}

static void omap2_usb2_disable_5pinbitll(void)
{
	u32 r;

	r = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
	r &= ~(USBTXWRMODEI(2, USB_BIDIR_TLL) | USBT2TLL5PI);
	omap_ctrl_writel(r, OMAP2_CONTROL_DEVCONF0);
}

static void omap2_usb2_enable_5pinunitll(void)
{
	u32 r;

	r = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
	r |= USBTXWRMODEI(2, USB_UNIDIR_TLL) | USBT2TLL5PI;
	omap_ctrl_writel(r, OMAP2_CONTROL_DEVCONF0);
}

static u32 __init omap_usb0_init(unsigned nwires, unsigned is_device)
{
	u32	syscon1 = 0;

	if (cpu_is_omap24xx())
		omap2_usb_devconf_clear(0, USB_BIDIR_TLL);

	if (nwires == 0) {
		if (cpu_class_is_omap1() && !cpu_is_omap15xx()) {
			u32 l;

			
			l = omap_readl(USB_TRANSCEIVER_CTRL);
			l &= ~(3 << 1);
			omap_writel(l, USB_TRANSCEIVER_CTRL);
		}
		return 0;
	}

	if (is_device) {
		if (cpu_is_omap24xx())
			omap_cfg_reg(J20_24XX_USB0_PUEN);
		else
			omap_cfg_reg(W4_USB_PUEN);
	}

	
	if (!cpu_class_is_omap2() && nwires == 2) {
		u32 l;

		
		

		if (cpu_is_omap15xx()) {
			
			return 0;
		}

		

		l = omap_readl(USB_TRANSCEIVER_CTRL);
		l &= ~(7 << 4);
		if (!is_device)
			l |= (3 << 1);
		omap_writel(l, USB_TRANSCEIVER_CTRL);

		return 3 << 16;
	}

	
	if (cpu_is_omap15xx()) {
		printk(KERN_ERR "no usb0 alt pin config on 15xx\n");
		return 0;
	}

	if (cpu_is_omap24xx()) {
		omap_cfg_reg(K18_24XX_USB0_DAT);
		omap_cfg_reg(K19_24XX_USB0_TXEN);
		omap_cfg_reg(J14_24XX_USB0_SE0);
		if (nwires != 3)
			omap_cfg_reg(J18_24XX_USB0_RCV);
	} else {
		omap_cfg_reg(V6_USB0_TXD);
		omap_cfg_reg(W9_USB0_TXEN);
		omap_cfg_reg(W5_USB0_SE0);
		if (nwires != 3)
			omap_cfg_reg(Y5_USB0_RCV);
	}

	

	if (cpu_class_is_omap1() && nwires != 6) {
		u32 l;

		l = omap_readl(USB_TRANSCEIVER_CTRL);
		l &= ~CONF_USB2_UNI_R;
		omap_writel(l, USB_TRANSCEIVER_CTRL);
	}

	switch (nwires) {
	case 3:
		syscon1 = 2;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(0, USB_BIDIR);
		break;
	case 4:
		syscon1 = 1;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(0, USB_BIDIR);
		break;
	case 6:
		syscon1 = 3;
		if (cpu_is_omap24xx()) {
			omap_cfg_reg(J19_24XX_USB0_VP);
			omap_cfg_reg(K20_24XX_USB0_VM);
			omap2_usb_devconf_set(0, USB_UNIDIR);
		} else {
			u32 l;

			omap_cfg_reg(AA9_USB0_VP);
			omap_cfg_reg(R9_USB0_VM);
			l = omap_readl(USB_TRANSCEIVER_CTRL);
			l |= CONF_USB2_UNI_R;
			omap_writel(l, USB_TRANSCEIVER_CTRL);
		}
		break;
	default:
		printk(KERN_ERR "illegal usb%d %d-wire transceiver\n",
			0, nwires);
	}
	return syscon1 << 16;
}

static u32 __init omap_usb1_init(unsigned nwires)
{
	u32	syscon1 = 0;

	if (cpu_class_is_omap1() && !cpu_is_omap15xx() && nwires != 6) {
		u32 l;

		l = omap_readl(USB_TRANSCEIVER_CTRL);
		l &= ~CONF_USB1_UNI_R;
		omap_writel(l, USB_TRANSCEIVER_CTRL);
	}
	if (cpu_is_omap24xx())
		omap2_usb_devconf_clear(1, USB_BIDIR_TLL);

	if (nwires == 0)
		return 0;

	
	if (cpu_class_is_omap1()) {
		omap_cfg_reg(USB1_TXD);
		omap_cfg_reg(USB1_TXEN);
		if (nwires != 3)
			omap_cfg_reg(USB1_RCV);
	}

	if (cpu_is_omap15xx()) {
		omap_cfg_reg(USB1_SEO);
		omap_cfg_reg(USB1_SPEED);
		
	} else if (cpu_is_omap1610() || cpu_is_omap5912()) {
		omap_cfg_reg(W13_1610_USB1_SE0);
		omap_cfg_reg(R13_1610_USB1_SPEED);
		
	} else if (cpu_is_omap1710()) {
		omap_cfg_reg(R13_1710_USB1_SE0);
		
	} else if (cpu_is_omap24xx()) {
		
	} else {
		pr_debug("usb%d cpu unrecognized\n", 1);
		return 0;
	}

	switch (nwires) {
	case 2:
		if (!cpu_is_omap24xx())
			goto bad;
		
		syscon1 = 1;
		omap2_usb_devconf_set(1, USB_BIDIR_TLL);
		break;
	case 3:
		syscon1 = 2;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(1, USB_BIDIR);
		break;
	case 4:
		syscon1 = 1;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(1, USB_BIDIR);
		break;
	case 6:
		if (cpu_is_omap24xx())
			goto bad;
		syscon1 = 3;
		omap_cfg_reg(USB1_VP);
		omap_cfg_reg(USB1_VM);
		if (!cpu_is_omap15xx()) {
			u32 l;

			l = omap_readl(USB_TRANSCEIVER_CTRL);
			l |= CONF_USB1_UNI_R;
			omap_writel(l, USB_TRANSCEIVER_CTRL);
		}
		break;
	default:
bad:
		printk(KERN_ERR "illegal usb%d %d-wire transceiver\n",
			1, nwires);
	}
	return syscon1 << 20;
}

static u32 __init omap_usb2_init(unsigned nwires, unsigned alt_pingroup)
{
	u32	syscon1 = 0;

	if (cpu_is_omap24xx()) {
		omap2_usb2_disable_5pinbitll();
		alt_pingroup = 0;
	}

	
	if (alt_pingroup || nwires == 0)
		return 0;

	if (cpu_class_is_omap1() && !cpu_is_omap15xx() && nwires != 6) {
		u32 l;

		l = omap_readl(USB_TRANSCEIVER_CTRL);
		l &= ~CONF_USB2_UNI_R;
		omap_writel(l, USB_TRANSCEIVER_CTRL);
	}

	
	if (cpu_is_omap15xx()) {
		omap_cfg_reg(USB2_TXD);
		omap_cfg_reg(USB2_TXEN);
		omap_cfg_reg(USB2_SEO);
		if (nwires != 3)
			omap_cfg_reg(USB2_RCV);
		
	} else if (cpu_is_omap16xx()) {
		omap_cfg_reg(V6_USB2_TXD);
		omap_cfg_reg(W9_USB2_TXEN);
		omap_cfg_reg(W5_USB2_SE0);
		if (nwires != 3)
			omap_cfg_reg(Y5_USB2_RCV);
		
	} else if (cpu_is_omap24xx()) {
		omap_cfg_reg(Y11_24XX_USB2_DAT);
		omap_cfg_reg(AA10_24XX_USB2_SE0);
		if (nwires > 2)
			omap_cfg_reg(AA12_24XX_USB2_TXEN);
		if (nwires > 3)
			omap_cfg_reg(AA6_24XX_USB2_RCV);
	} else {
		pr_debug("usb%d cpu unrecognized\n", 1);
		return 0;
	}
	

	switch (nwires) {
	case 2:
		if (!cpu_is_omap24xx())
			goto bad;
		
		syscon1 = 1;
		omap2_usb_devconf_set(2, USB_BIDIR_TLL);
		break;
	case 3:
		syscon1 = 2;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(2, USB_BIDIR);
		break;
	case 4:
		syscon1 = 1;
		if (cpu_is_omap24xx())
			omap2_usb_devconf_set(2, USB_BIDIR);
		break;
	case 5:
		if (!cpu_is_omap24xx())
			goto bad;
		omap_cfg_reg(AA4_24XX_USB2_TLLSE0);
		
		syscon1 = 3;
		omap2_usb2_enable_5pinunitll();
		break;
	case 6:
		if (cpu_is_omap24xx())
			goto bad;
		syscon1 = 3;
		if (cpu_is_omap15xx()) {
			omap_cfg_reg(USB2_VP);
			omap_cfg_reg(USB2_VM);
		} else {
			u32 l;

			omap_cfg_reg(AA9_USB2_VP);
			omap_cfg_reg(R9_USB2_VM);
			l = omap_readl(USB_TRANSCEIVER_CTRL);
			l |= CONF_USB2_UNI_R;
			omap_writel(l, USB_TRANSCEIVER_CTRL);
		}
		break;
	default:
bad:
		printk(KERN_ERR "illegal usb%d %d-wire transceiver\n",
			2, nwires);
	}
	return syscon1 << 24;
}

#endif



#ifdef	CONFIG_USB_GADGET_OMAP

static struct resource udc_resources[] = {
	
	{		
		.start		= UDC_BASE,
		.end		= UDC_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	}, {		
		.start		= INT_USB_IRQ_GEN,
		.flags		= IORESOURCE_IRQ,
	}, {		
		.start		= INT_USB_IRQ_NISO,
		.flags		= IORESOURCE_IRQ,
	}, {		
		.start		= INT_USB_IRQ_ISO,
		.flags		= IORESOURCE_IRQ,
	},
};

static u64 udc_dmamask = ~(u32)0;

static struct platform_device udc_device = {
	.name		= "omap_udc",
	.id		= -1,
	.dev = {
		.dma_mask		= &udc_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(udc_resources),
	.resource	= udc_resources,
};

#endif

#if	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)


static u64 ohci_dmamask = ~(u32)0;

static struct resource ohci_resources[] = {
	{
		.start	= OMAP_OHCI_BASE,
		.end	= OMAP_OHCI_BASE + 0xff,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_USB_IRQ_HGEN,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device ohci_device = {
	.name			= "ohci",
	.id			= -1,
	.dev = {
		.dma_mask		= &ohci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.num_resources	= ARRAY_SIZE(ohci_resources),
	.resource		= ohci_resources,
};

#endif

#if	defined(CONFIG_USB_OTG) && defined(CONFIG_ARCH_OMAP_OTG)

static struct resource otg_resources[] = {
	
	{
		.start		= OTG_BASE,
		.end		= OTG_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	}, {
		.start		= INT_USB_IRQ_OTG,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct platform_device otg_device = {
	.name		= "omap_otg",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(otg_resources),
	.resource	= otg_resources,
};

#endif





#ifdef	CONFIG_USB_GADGET_OMAP
#define	is_usb0_device(config)	1
#else
#define	is_usb0_device(config)	0
#endif



#ifdef	CONFIG_ARCH_OMAP_OTG

void __init
omap_otg_init(struct omap_usb_config *config)
{
	u32		syscon;
	int		status;
	int		alt_pingroup = 0;

	

	syscon = omap_readl(OTG_SYSCON_1) & 0xffff;
	if (!(syscon & OTG_RESET_DONE))
		pr_debug("USB resets not complete?\n");

	

	
	if (config->pins[0] > 2)	
		alt_pingroup = 1;
	syscon |= omap_usb0_init(config->pins[0], is_usb0_device(config));
	syscon |= omap_usb1_init(config->pins[1]);
	syscon |= omap_usb2_init(config->pins[2], alt_pingroup);
	pr_debug("OTG_SYSCON_1 = %08x\n", omap_readl(OTG_SYSCON_1));
	omap_writel(syscon, OTG_SYSCON_1);

	syscon = config->hmc_mode;
	syscon |= USBX_SYNCHRO | (4 << 16) ;
#ifdef	CONFIG_USB_OTG
	if (config->otg)
		syscon |= OTG_EN;
#endif
	if (cpu_class_is_omap1())
		pr_debug("USB_TRANSCEIVER_CTRL = %03x\n",
			 omap_readl(USB_TRANSCEIVER_CTRL));
	pr_debug("OTG_SYSCON_2 = %08x\n", omap_readl(OTG_SYSCON_2));
	omap_writel(syscon, OTG_SYSCON_2);

	printk("USB: hmc %d", config->hmc_mode);
	if (!alt_pingroup)
		printk(", usb2 alt %d wires", config->pins[2]);
	else if (config->pins[0])
		printk(", usb0 %d wires%s", config->pins[0],
			is_usb0_device(config) ? " (dev)" : "");
	if (config->pins[1])
		printk(", usb1 %d wires", config->pins[1]);
	if (!alt_pingroup && config->pins[2])
		printk(", usb2 %d wires", config->pins[2]);
	if (config->otg)
		printk(", Mini-AB on usb%d", config->otg - 1);
	printk("\n");

	if (cpu_class_is_omap1()) {
		u16 w;

		
		w = omap_readw(ULPD_SOFT_REQ);
		w &= ~SOFT_USB_CLK_REQ;
		omap_writew(w, ULPD_SOFT_REQ);

		w = omap_readw(ULPD_CLOCK_CTRL);
		w &= ~USB_MCLK_EN;
		w |= DIS_USB_PVCI_CLK;
		omap_writew(w, ULPD_CLOCK_CTRL);
	}
	syscon = omap_readl(OTG_SYSCON_1);
	syscon |= HST_IDLE_EN|DEV_IDLE_EN|OTG_IDLE_EN;

#ifdef	CONFIG_USB_GADGET_OMAP
	if (config->otg || config->register_dev) {
		syscon &= ~DEV_IDLE_EN;
		udc_device.dev.platform_data = config;
		
		status = platform_device_register(&udc_device);
		if (status)
			pr_debug("can't register UDC device, %d\n", status);
	}
#endif

#if	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
	if (config->otg || config->register_host) {
		syscon &= ~HST_IDLE_EN;
		ohci_device.dev.platform_data = config;
		if (cpu_is_omap730())
			ohci_resources[1].start = INT_730_USB_HHC_1;
		status = platform_device_register(&ohci_device);
		if (status)
			pr_debug("can't register OHCI device, %d\n", status);
	}
#endif

#ifdef	CONFIG_USB_OTG
	if (config->otg) {
		syscon &= ~OTG_IDLE_EN;
		otg_device.dev.platform_data = config;
		if (cpu_is_omap730())
			otg_resources[1].start = INT_730_USB_OTG;
		status = platform_device_register(&otg_device);
		if (status)
			pr_debug("can't register OTG device, %d\n", status);
	}
#endif
	pr_debug("OTG_SYSCON_1 = %08x\n", omap_readl(OTG_SYSCON_1));
	omap_writel(syscon, OTG_SYSCON_1);

	status = 0;
}

#else
static inline void omap_otg_init(struct omap_usb_config *config) {}
#endif



#ifdef	CONFIG_ARCH_OMAP15XX


#define DPLL_IOB		(1 << 13)
#define DPLL_PLL_ENABLE		(1 << 4)
#define DPLL_LOCK		(1 << 0)


#define APLL_NDPLL_SWITCH	(1 << 0)


static void __init omap_1510_usb_init(struct omap_usb_config *config)
{
	unsigned int val;
	u16 w;

	omap_usb0_init(config->pins[0], is_usb0_device(config));
	omap_usb1_init(config->pins[1]);
	omap_usb2_init(config->pins[2], 0);

	val = omap_readl(MOD_CONF_CTRL_0) & ~(0x3f << 1);
	val |= (config->hmc_mode << 1);
	omap_writel(val, MOD_CONF_CTRL_0);

	printk("USB: hmc %d", config->hmc_mode);
	if (config->pins[0])
		printk(", usb0 %d wires%s", config->pins[0],
			is_usb0_device(config) ? " (dev)" : "");
	if (config->pins[1])
		printk(", usb1 %d wires", config->pins[1]);
	if (config->pins[2])
		printk(", usb2 %d wires", config->pins[2]);
	printk("\n");

	
	pr_debug("APLL %04x DPLL %04x REQ %04x\n", omap_readw(ULPD_APLL_CTRL),
			omap_readw(ULPD_DPLL_CTRL), omap_readw(ULPD_SOFT_REQ));

	w = omap_readw(ULPD_APLL_CTRL);
	w &= ~APLL_NDPLL_SWITCH;
	omap_writew(w, ULPD_APLL_CTRL);

	w = omap_readw(ULPD_DPLL_CTRL);
	w |= DPLL_IOB | DPLL_PLL_ENABLE;
	omap_writew(w, ULPD_DPLL_CTRL);

	w = omap_readw(ULPD_SOFT_REQ);
	w |= SOFT_UDC_REQ | SOFT_DPLL_REQ;
	omap_writew(w, ULPD_SOFT_REQ);

	while (!(omap_readw(ULPD_DPLL_CTRL) & DPLL_LOCK))
		cpu_relax();

#ifdef	CONFIG_USB_GADGET_OMAP
	if (config->register_dev) {
		int status;

		udc_device.dev.platform_data = config;
		status = platform_device_register(&udc_device);
		if (status)
			pr_debug("can't register UDC device, %d\n", status);
		
	}
#endif

#if	defined(CONFIG_USB_OHCI_HCD) || defined(CONFIG_USB_OHCI_HCD_MODULE)
	if (config->register_host) {
		int status;

		ohci_device.dev.platform_data = config;
		status = platform_device_register(&ohci_device);
		if (status)
			pr_debug("can't register OHCI device, %d\n", status);
		
	}
#endif
}

#else
static inline void omap_1510_usb_init(struct omap_usb_config *config) {}
#endif



void __init omap_usb_init(struct omap_usb_config *pdata)
{
	if (cpu_is_omap730() || cpu_is_omap16xx() || cpu_is_omap24xx())
		omap_otg_init(pdata);
	else if (cpu_is_omap15xx())
		omap_1510_usb_init(pdata);
	else
		printk(KERN_ERR "USB: No init for your chip yet\n");
}

