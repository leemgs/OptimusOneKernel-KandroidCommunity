

#ifndef __LINUX_USB_GADGET_MSM72K_OTG_H__
#define __LINUX_USB_GADGET_MSM72K_OTG_H__

#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h>
#include <asm/mach-types.h>

#define OTGSC_BSVIE            (1 << 27)
#define OTGSC_IDIE             (1 << 24)
#define OTGSC_BSVIS            (1 << 19)
#define OTGSC_ID               (1 << 8)
#define OTGSC_IDIS             (1 << 16)
#define OTGSC_BSV              (1 << 11)

#define ULPI_STP_CTRL   (1 << 30)
#define ASYNC_INTR_CTRL (1 << 29)

#define PORTSC_PHCD     (1 << 23)
#define disable_phy_clk() (writel(readl(USB_PORTSC) | PORTSC_PHCD, USB_PORTSC))
#define enable_phy_clk() (writel(readl(USB_PORTSC) & ~PORTSC_PHCD, USB_PORTSC))
#define is_phy_clk_disabled() (readl(USB_PORTSC) & PORTSC_PHCD)
#define is_usb_active()       (!(readl(USB_PORTSC) & PORTSC_SUSP))

struct msm_otg {
	struct otg_transceiver otg;

	
	struct clk		*hs_clk;
	struct clk		*hs_pclk;
	struct clk		*hs_cclk;
	
	struct clk		*phy_reset_clk;

	int			irq;
	int			vbus_on_irq;
	void __iomem		*regs;
	atomic_t		in_lpm;
	unsigned int 		core_clk;

	atomic_t		chg_type;

	void (*start_host)	(struct usb_bus *bus, int suspend);
	
	int (*set_clk)		(struct otg_transceiver *otg, int on);
	
	void (*reset)		(struct otg_transceiver *otg);
	
	u8 pmic_notif_supp;
	struct msm_otg_platform_data *pdata;
};


static inline int depends_on_axi_freq(struct otg_transceiver *xceiv)
{
	struct msm_otg *dev;

	if (!xceiv)
		return 0;

	
	if (machine_is_msm8x60_surf() || machine_is_msm8x60_ffa())
		return 0;

	dev = container_of(xceiv, struct msm_otg, otg);

	return !dev->core_clk;
}

#endif
