

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/irq.h>
#include <linux/platform_device.h>

#include "musb_core.h"

static void tusb_source_power(struct musb *musb, int is_on);

#define TUSB_REV_MAJOR(reg_val)		((reg_val >> 4) & 0xf)
#define TUSB_REV_MINOR(reg_val)		(reg_val & 0xf)


u8 tusb_get_revision(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		die_id;
	u8		rev;

	rev = musb_readl(tbase, TUSB_DMA_CTRL_REV) & 0xff;
	if (TUSB_REV_MAJOR(rev) == 3) {
		die_id = TUSB_DIDR1_HI_CHIP_REV(musb_readl(tbase,
				TUSB_DIDR1_HI));
		if (die_id >= TUSB_DIDR1_HI_REV_31)
			rev |= 1;
	}

	return rev;
}

static int __init tusb_print_revision(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;
	u8		rev;

	rev = tusb_get_revision(musb);

	pr_info("tusb: %s%i.%i %s%i.%i %s%i.%i %s%i.%i %s%i %s%i.%i\n",
		"prcm",
		TUSB_REV_MAJOR(musb_readl(tbase, TUSB_PRCM_REV)),
		TUSB_REV_MINOR(musb_readl(tbase, TUSB_PRCM_REV)),
		"int",
		TUSB_REV_MAJOR(musb_readl(tbase, TUSB_INT_CTRL_REV)),
		TUSB_REV_MINOR(musb_readl(tbase, TUSB_INT_CTRL_REV)),
		"gpio",
		TUSB_REV_MAJOR(musb_readl(tbase, TUSB_GPIO_REV)),
		TUSB_REV_MINOR(musb_readl(tbase, TUSB_GPIO_REV)),
		"dma",
		TUSB_REV_MAJOR(musb_readl(tbase, TUSB_DMA_CTRL_REV)),
		TUSB_REV_MINOR(musb_readl(tbase, TUSB_DMA_CTRL_REV)),
		"dieid",
		TUSB_DIDR1_HI_CHIP_REV(musb_readl(tbase, TUSB_DIDR1_HI)),
		"rev",
		TUSB_REV_MAJOR(rev), TUSB_REV_MINOR(rev));

	return tusb_get_revision(musb);
}

#define WBUS_QUIRK_MASK	(TUSB_PHY_OTG_CTRL_TESTM2 | TUSB_PHY_OTG_CTRL_TESTM1 \
				| TUSB_PHY_OTG_CTRL_TESTM0)


static void tusb_wbus_quirk(struct musb *musb, int enabled)
{
	void __iomem	*tbase = musb->ctrl_base;
	static u32	phy_otg_ctrl, phy_otg_ena;
	u32		tmp;

	if (enabled) {
		phy_otg_ctrl = musb_readl(tbase, TUSB_PHY_OTG_CTRL);
		phy_otg_ena = musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE);
		tmp = TUSB_PHY_OTG_CTRL_WRPROTECT
				| phy_otg_ena | WBUS_QUIRK_MASK;
		musb_writel(tbase, TUSB_PHY_OTG_CTRL, tmp);
		tmp = phy_otg_ena & ~WBUS_QUIRK_MASK;
		tmp |= TUSB_PHY_OTG_CTRL_WRPROTECT | TUSB_PHY_OTG_CTRL_TESTM2;
		musb_writel(tbase, TUSB_PHY_OTG_CTRL_ENABLE, tmp);
		DBG(2, "Enabled tusb wbus quirk ctrl %08x ena %08x\n",
			musb_readl(tbase, TUSB_PHY_OTG_CTRL),
			musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE));
	} else if (musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE)
					& TUSB_PHY_OTG_CTRL_TESTM2) {
		tmp = TUSB_PHY_OTG_CTRL_WRPROTECT | phy_otg_ctrl;
		musb_writel(tbase, TUSB_PHY_OTG_CTRL, tmp);
		tmp = TUSB_PHY_OTG_CTRL_WRPROTECT | phy_otg_ena;
		musb_writel(tbase, TUSB_PHY_OTG_CTRL_ENABLE, tmp);
		DBG(2, "Disabled tusb wbus quirk ctrl %08x ena %08x\n",
			musb_readl(tbase, TUSB_PHY_OTG_CTRL),
			musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE));
		phy_otg_ctrl = 0;
		phy_otg_ena = 0;
	}
}



static inline void
tusb_fifo_write_unaligned(void __iomem *fifo, const u8 *buf, u16 len)
{
	u32		val;
	int		i;

	if (len > 4) {
		for (i = 0; i < (len >> 2); i++) {
			memcpy(&val, buf, 4);
			musb_writel(fifo, 0, val);
			buf += 4;
		}
		len %= 4;
	}
	if (len > 0) {
		
		memcpy(&val, buf, len);
		musb_writel(fifo, 0, val);
	}
}

static inline void tusb_fifo_read_unaligned(void __iomem *fifo,
						void __iomem *buf, u16 len)
{
	u32		val;
	int		i;

	if (len > 4) {
		for (i = 0; i < (len >> 2); i++) {
			val = musb_readl(fifo, 0);
			memcpy(buf, &val, 4);
			buf += 4;
		}
		len %= 4;
	}
	if (len > 0) {
		
		val = musb_readl(fifo, 0);
		memcpy(buf, &val, len);
	}
}

void musb_write_fifo(struct musb_hw_ep *hw_ep, u16 len, const u8 *buf)
{
	void __iomem	*ep_conf = hw_ep->conf;
	void __iomem	*fifo = hw_ep->fifo;
	u8		epnum = hw_ep->epnum;

	prefetch(buf);

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n",
			'T', epnum, fifo, len, buf);

	if (epnum)
		musb_writel(ep_conf, TUSB_EP_TX_OFFSET,
			TUSB_EP_CONFIG_XFR_SIZE(len));
	else
		musb_writel(ep_conf, 0, TUSB_EP0_CONFIG_DIR_TX |
			TUSB_EP0_CONFIG_XFR_SIZE(len));

	if (likely((0x01 & (unsigned long) buf) == 0)) {

		
		if ((0x02 & (unsigned long) buf) == 0) {
			if (len >= 4) {
				writesl(fifo, buf, len >> 2);
				buf += (len & ~0x03);
				len &= 0x03;
			}
		} else {
			if (len >= 2) {
				u32 val;
				int i;

				
				for (i = 0; i < (len >> 2); i++) {
					val = (u32)(*(u16 *)buf);
					buf += 2;
					val |= (*(u16 *)buf) << 16;
					buf += 2;
					musb_writel(fifo, 0, val);
				}
				len &= 0x03;
			}
		}
	}

	if (len > 0)
		tusb_fifo_write_unaligned(fifo, buf, len);
}

void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *buf)
{
	void __iomem	*ep_conf = hw_ep->conf;
	void __iomem	*fifo = hw_ep->fifo;
	u8		epnum = hw_ep->epnum;

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n",
			'R', epnum, fifo, len, buf);

	if (epnum)
		musb_writel(ep_conf, TUSB_EP_RX_OFFSET,
			TUSB_EP_CONFIG_XFR_SIZE(len));
	else
		musb_writel(ep_conf, 0, TUSB_EP0_CONFIG_XFR_SIZE(len));

	if (likely((0x01 & (unsigned long) buf) == 0)) {

		
		if ((0x02 & (unsigned long) buf) == 0) {
			if (len >= 4) {
				readsl(fifo, buf, len >> 2);
				buf += (len & ~0x03);
				len &= 0x03;
			}
		} else {
			if (len >= 2) {
				u32 val;
				int i;

				
				for (i = 0; i < (len >> 2); i++) {
					val = musb_readl(fifo, 0);
					*(u16 *)buf = (u16)(val & 0xffff);
					buf += 2;
					*(u16 *)buf = (u16)(val >> 16);
					buf += 2;
				}
				len &= 0x03;
			}
		}
	}

	if (len > 0)
		tusb_fifo_read_unaligned(fifo, buf, len);
}

static struct musb *the_musb;

#ifdef CONFIG_USB_GADGET_MUSB_HDRC


static int tusb_draw_power(struct otg_transceiver *x, unsigned mA)
{
	struct musb	*musb = the_musb;
	void __iomem	*tbase = musb->ctrl_base;
	u32		reg;

	
	if (musb->set_clock) {
		if (mA)
			musb->set_clock(musb->clock, 1);
		else
			musb->set_clock(musb->clock, 0);
	}

	
	if (x->default_a || mA < (musb->min_power << 1))
		mA = 0;

	reg = musb_readl(tbase, TUSB_PRCM_MNGMT);
	if (mA) {
		musb->is_bus_powered = 1;
		reg |= TUSB_PRCM_MNGMT_15_SW_EN | TUSB_PRCM_MNGMT_33_SW_EN;
	} else {
		musb->is_bus_powered = 0;
		reg &= ~(TUSB_PRCM_MNGMT_15_SW_EN | TUSB_PRCM_MNGMT_33_SW_EN);
	}
	musb_writel(tbase, TUSB_PRCM_MNGMT, reg);

	DBG(2, "draw max %d mA VBUS\n", mA);
	return 0;
}

#else
#define tusb_draw_power	NULL
#endif


static void tusb_set_clock_source(struct musb *musb, unsigned mode)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		reg;

	reg = musb_readl(tbase, TUSB_PRCM_CONF);
	reg &= ~TUSB_PRCM_CONF_SYS_CLKSEL(0x3);

	
	if (mode > 0)
		reg |= TUSB_PRCM_CONF_SYS_CLKSEL(mode & 0x3);

	musb_writel(tbase, TUSB_PRCM_CONF, reg);

	
}


void tusb_allow_idle(struct musb *musb, u32 wakeup_enables)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		reg;

	if ((wakeup_enables & TUSB_PRCM_WBUS)
			&& (tusb_get_revision(musb) == TUSB_REV_30))
		tusb_wbus_quirk(musb, 1);

	tusb_set_clock_source(musb, 0);

	wakeup_enables |= TUSB_PRCM_WNORCS;
	musb_writel(tbase, TUSB_PRCM_WAKEUP_MASK, ~wakeup_enables);

	

	reg = musb_readl(tbase, TUSB_PRCM_MNGMT);
	
	if (is_host_active(musb)) {
		reg |= TUSB_PRCM_MNGMT_OTG_VBUS_DET_EN;
		reg &= ~TUSB_PRCM_MNGMT_OTG_SESS_END_EN;
	} else {
		reg |= TUSB_PRCM_MNGMT_OTG_SESS_END_EN;
		reg &= ~TUSB_PRCM_MNGMT_OTG_VBUS_DET_EN;
	}
	reg |= TUSB_PRCM_MNGMT_PM_IDLE | TUSB_PRCM_MNGMT_DEV_IDLE;
	musb_writel(tbase, TUSB_PRCM_MNGMT, reg);

	DBG(6, "idle, wake on %02x\n", wakeup_enables);
}


int musb_platform_get_vbus_status(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		otg_stat, prcm_mngmt;
	int		ret = 0;

	otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
	prcm_mngmt = musb_readl(tbase, TUSB_PRCM_MNGMT);

	
	if (!(prcm_mngmt & TUSB_PRCM_MNGMT_OTG_VBUS_DET_EN)) {
		u32 tmp = prcm_mngmt;
		tmp |= TUSB_PRCM_MNGMT_OTG_VBUS_DET_EN;
		musb_writel(tbase, TUSB_PRCM_MNGMT, tmp);
		otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
		musb_writel(tbase, TUSB_PRCM_MNGMT, prcm_mngmt);
	}

	if (otg_stat & TUSB_DEV_OTG_STAT_VBUS_VALID)
		ret = 1;

	return ret;
}

static struct timer_list musb_idle_timer;

static void musb_do_idle(unsigned long _musb)
{
	struct musb	*musb = (void *)_musb;
	unsigned long	flags;

	spin_lock_irqsave(&musb->lock, flags);

	switch (musb->xceiv->state) {
	case OTG_STATE_A_WAIT_BCON:
		if ((musb->a_wait_bcon != 0)
			&& (musb->idle_timeout == 0
				|| time_after(jiffies, musb->idle_timeout))) {
			DBG(4, "Nothing connected %s, turning off VBUS\n",
					otg_state_string(musb));
		}
		
	case OTG_STATE_A_IDLE:
		tusb_source_power(musb, 0);
	default:
		break;
	}

	if (!musb->is_active) {
		u32	wakeups;

		
		if (is_host_active(musb) && (musb->port1_status >> 16))
			goto done;

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
		if (is_peripheral_enabled(musb) && !musb->gadget_driver)
			wakeups = 0;
		else {
			wakeups = TUSB_PRCM_WHOSTDISCON
					| TUSB_PRCM_WBUS
					| TUSB_PRCM_WVBUS;
			if (is_otg_enabled(musb))
				wakeups |= TUSB_PRCM_WID;
		}
#else
		wakeups = TUSB_PRCM_WHOSTDISCON | TUSB_PRCM_WBUS;
#endif
		tusb_allow_idle(musb, wakeups);
	}
done:
	spin_unlock_irqrestore(&musb->lock, flags);
}


void musb_platform_try_idle(struct musb *musb, unsigned long timeout)
{
	unsigned long		default_timeout = jiffies + msecs_to_jiffies(3);
	static unsigned long	last_timer;

	if (timeout == 0)
		timeout = default_timeout;

	
	if (musb->is_active || ((musb->a_wait_bcon == 0)
			&& (musb->xceiv->state == OTG_STATE_A_WAIT_BCON))) {
		DBG(4, "%s active, deleting timer\n", otg_state_string(musb));
		del_timer(&musb_idle_timer);
		last_timer = jiffies;
		return;
	}

	if (time_after(last_timer, timeout)) {
		if (!timer_pending(&musb_idle_timer))
			last_timer = timeout;
		else {
			DBG(4, "Longer idle timer already pending, ignoring\n");
			return;
		}
	}
	last_timer = timeout;

	DBG(4, "%s inactive, for idle timer for %lu ms\n",
		otg_state_string(musb),
		(unsigned long)jiffies_to_msecs(timeout - jiffies));
	mod_timer(&musb_idle_timer, timeout);
}


#define DEVCLOCK		60000000
#define OTG_TIMER_MS(msecs)	((msecs) \
		? (TUSB_DEV_OTG_TIMER_VAL((DEVCLOCK/1000)*(msecs)) \
				| TUSB_DEV_OTG_TIMER_ENABLE) \
		: 0)

static void tusb_source_power(struct musb *musb, int is_on)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		conf, prcm, timer;
	u8		devctl;

	

	prcm = musb_readl(tbase, TUSB_PRCM_MNGMT);
	conf = musb_readl(tbase, TUSB_DEV_CONF);
	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);

	if (is_on) {
		if (musb->set_clock)
			musb->set_clock(musb->clock, 1);
		timer = OTG_TIMER_MS(OTG_TIME_A_WAIT_VRISE);
		musb->xceiv->default_a = 1;
		musb->xceiv->state = OTG_STATE_A_WAIT_VRISE;
		devctl |= MUSB_DEVCTL_SESSION;

		conf |= TUSB_DEV_CONF_USB_HOST_MODE;
		MUSB_HST_MODE(musb);
	} else {
		u32	otg_stat;

		timer = 0;

		
		otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
		if (!(otg_stat & TUSB_DEV_OTG_STAT_ID_STATUS)) {
			switch (musb->xceiv->state) {
			case OTG_STATE_A_WAIT_VRISE:
			case OTG_STATE_A_WAIT_BCON:
				musb->xceiv->state = OTG_STATE_A_WAIT_VFALL;
				break;
			case OTG_STATE_A_WAIT_VFALL:
				musb->xceiv->state = OTG_STATE_A_IDLE;
				break;
			default:
				musb->xceiv->state = OTG_STATE_A_IDLE;
			}
			musb->is_active = 0;
			musb->xceiv->default_a = 1;
			MUSB_HST_MODE(musb);
		} else {
			musb->is_active = 0;
			musb->xceiv->default_a = 0;
			musb->xceiv->state = OTG_STATE_B_IDLE;
			MUSB_DEV_MODE(musb);
		}

		devctl &= ~MUSB_DEVCTL_SESSION;
		conf &= ~TUSB_DEV_CONF_USB_HOST_MODE;
		if (musb->set_clock)
			musb->set_clock(musb->clock, 0);
	}
	prcm &= ~(TUSB_PRCM_MNGMT_15_SW_EN | TUSB_PRCM_MNGMT_33_SW_EN);

	musb_writel(tbase, TUSB_PRCM_MNGMT, prcm);
	musb_writel(tbase, TUSB_DEV_OTG_TIMER, timer);
	musb_writel(tbase, TUSB_DEV_CONF, conf);
	musb_writeb(musb->mregs, MUSB_DEVCTL, devctl);

	DBG(1, "VBUS %s, devctl %02x otg %3x conf %08x prcm %08x\n",
		otg_state_string(musb),
		musb_readb(musb->mregs, MUSB_DEVCTL),
		musb_readl(tbase, TUSB_DEV_OTG_STAT),
		conf, prcm);
}


int musb_platform_set_mode(struct musb *musb, u8 musb_mode)
{
	void __iomem	*tbase = musb->ctrl_base;
	u32		otg_stat, phy_otg_ctrl, phy_otg_ena, dev_conf;

	if (musb->board_mode != MUSB_OTG) {
		ERR("Changing mode currently only supported in OTG mode\n");
		return -EINVAL;
	}

	otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
	phy_otg_ctrl = musb_readl(tbase, TUSB_PHY_OTG_CTRL);
	phy_otg_ena = musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE);
	dev_conf = musb_readl(tbase, TUSB_DEV_CONF);

	switch (musb_mode) {

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	case MUSB_HOST:		
		phy_otg_ctrl &= ~TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		phy_otg_ena |= TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		dev_conf |= TUSB_DEV_CONF_ID_SEL;
		dev_conf &= ~TUSB_DEV_CONF_SOFT_ID;
		break;
#endif

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	case MUSB_PERIPHERAL:	
		phy_otg_ctrl |= TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		phy_otg_ena |= TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		dev_conf |= (TUSB_DEV_CONF_ID_SEL | TUSB_DEV_CONF_SOFT_ID);
		break;
#endif

#ifdef CONFIG_USB_MUSB_OTG
	case MUSB_OTG:		
		phy_otg_ctrl |= TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		phy_otg_ena |= TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
		dev_conf &= ~(TUSB_DEV_CONF_ID_SEL | TUSB_DEV_CONF_SOFT_ID);
		break;
#endif

	default:
		DBG(2, "Trying to set mode %i\n", musb_mode);
		return -EINVAL;
	}

	musb_writel(tbase, TUSB_PHY_OTG_CTRL,
			TUSB_PHY_OTG_CTRL_WRPROTECT | phy_otg_ctrl);
	musb_writel(tbase, TUSB_PHY_OTG_CTRL_ENABLE,
			TUSB_PHY_OTG_CTRL_WRPROTECT | phy_otg_ena);
	musb_writel(tbase, TUSB_DEV_CONF, dev_conf);

	otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
	if ((musb_mode == MUSB_PERIPHERAL) &&
		!(otg_stat & TUSB_DEV_OTG_STAT_ID_STATUS))
			INFO("Cannot be peripheral with mini-A cable "
			"otg_stat: %08x\n", otg_stat);

	return 0;
}

static inline unsigned long
tusb_otg_ints(struct musb *musb, u32 int_src, void __iomem *tbase)
{
	u32		otg_stat = musb_readl(tbase, TUSB_DEV_OTG_STAT);
	unsigned long	idle_timeout = 0;

	
	if ((int_src & TUSB_INT_SRC_ID_STATUS_CHNG)) {
		int	default_a;

		if (is_otg_enabled(musb))
			default_a = !(otg_stat & TUSB_DEV_OTG_STAT_ID_STATUS);
		else
			default_a = is_host_enabled(musb);
		DBG(2, "Default-%c\n", default_a ? 'A' : 'B');
		musb->xceiv->default_a = default_a;
		tusb_source_power(musb, default_a);

		
		if (default_a)
			idle_timeout = jiffies + (HZ * 3);
	}

	
	if (int_src & TUSB_INT_SRC_VBUS_SENSE_CHNG) {

		
		if ((is_otg_enabled(musb) && !musb->xceiv->default_a)
				|| !is_host_enabled(musb)) {
#ifdef CONFIG_USB_MUSB_HDRC_HCD
			
			musb->port1_status &=
				~(USB_PORT_STAT_CONNECTION
				| USB_PORT_STAT_ENABLE
				| USB_PORT_STAT_LOW_SPEED
				| USB_PORT_STAT_HIGH_SPEED
				| USB_PORT_STAT_TEST
				);
#endif

			if (otg_stat & TUSB_DEV_OTG_STAT_SESS_END) {
				DBG(1, "Forcing disconnect (no interrupt)\n");
				if (musb->xceiv->state != OTG_STATE_B_IDLE) {
					
					musb->xceiv->state = OTG_STATE_B_IDLE;
					musb->int_usb |= MUSB_INTR_DISCONNECT;
				}
				musb->is_active = 0;
			}
			DBG(2, "vbus change, %s, otg %03x\n",
				otg_state_string(musb), otg_stat);
			idle_timeout = jiffies + (1 * HZ);
			schedule_work(&musb->irq_work);

		} else  {
			DBG(2, "vbus change, %s, otg %03x\n",
				otg_state_string(musb), otg_stat);

			switch (musb->xceiv->state) {
			case OTG_STATE_A_IDLE:
				DBG(2, "Got SRP, turning on VBUS\n");
				musb_set_vbus(musb, 1);

				
				if (musb->a_wait_bcon != 0)
					musb->is_active = 0;
				else
					musb->is_active = 1;

				
				idle_timeout = jiffies + (2 * HZ);

				break;
			case OTG_STATE_A_WAIT_VRISE:
				
				break;
			case OTG_STATE_A_WAIT_VFALL:
				
				if (musb->vbuserr_retry) {
					musb->vbuserr_retry--;
					tusb_source_power(musb, 1);
				} else {
					musb->vbuserr_retry
						= VBUSERR_RETRY_COUNT;
					tusb_source_power(musb, 0);
				}
				break;
			default:
				break;
			}
		}
	}

	
	if (int_src & TUSB_INT_SRC_OTG_TIMEOUT) {
		u8	devctl;

		DBG(4, "%s timer, %03x\n", otg_state_string(musb), otg_stat);

		switch (musb->xceiv->state) {
		case OTG_STATE_A_WAIT_VRISE:
			
			devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
			if (otg_stat & TUSB_DEV_OTG_STAT_VBUS_VALID) {
				if ((devctl & MUSB_DEVCTL_VBUS)
						!= MUSB_DEVCTL_VBUS) {
					DBG(2, "devctl %02x\n", devctl);
					break;
				}
				musb->xceiv->state = OTG_STATE_A_WAIT_BCON;
				musb->is_active = 0;
				idle_timeout = jiffies
					+ msecs_to_jiffies(musb->a_wait_bcon);
			} else {
				
				ERR("vbus too slow, devctl %02x\n", devctl);
				tusb_source_power(musb, 0);
			}
			break;
		case OTG_STATE_A_WAIT_BCON:
			if (musb->a_wait_bcon != 0)
				idle_timeout = jiffies
					+ msecs_to_jiffies(musb->a_wait_bcon);
			break;
		case OTG_STATE_A_SUSPEND:
			break;
		case OTG_STATE_B_WAIT_ACON:
			break;
		default:
			break;
		}
	}
	schedule_work(&musb->irq_work);

	return idle_timeout;
}

static irqreturn_t tusb_interrupt(int irq, void *__hci)
{
	struct musb	*musb = __hci;
	void __iomem	*tbase = musb->ctrl_base;
	unsigned long	flags, idle_timeout = 0;
	u32		int_mask, int_src;

	spin_lock_irqsave(&musb->lock, flags);

	
	int_mask = musb_readl(tbase, TUSB_INT_MASK);
	musb_writel(tbase, TUSB_INT_MASK, ~TUSB_INT_MASK_RESERVED_BITS);

	int_src = musb_readl(tbase, TUSB_INT_SRC) & ~TUSB_INT_SRC_RESERVED_BITS;
	DBG(3, "TUSB IRQ %08x\n", int_src);

	musb->int_usb = (u8) int_src;

	
	if (int_src & TUSB_INT_SRC_DEV_WAKEUP) {
		u32	reg;
		u32	i;

		if (tusb_get_revision(musb) == TUSB_REV_30)
			tusb_wbus_quirk(musb, 0);

		

		
		for (i = 0xf7f7f7; i > 0xf7f7f7 - 1000; i--) {
			musb_writel(tbase, TUSB_SCRATCH_PAD, 0);
			musb_writel(tbase, TUSB_SCRATCH_PAD, i);
			reg = musb_readl(tbase, TUSB_SCRATCH_PAD);
			if (reg == i)
				break;
			DBG(6, "TUSB NOR not ready\n");
		}

		
		tusb_set_clock_source(musb, 1);

		reg = musb_readl(tbase, TUSB_PRCM_WAKEUP_SOURCE);
		musb_writel(tbase, TUSB_PRCM_WAKEUP_CLEAR, reg);
		if (reg & ~TUSB_PRCM_WNORCS) {
			musb->is_active = 1;
			schedule_work(&musb->irq_work);
		}
		DBG(3, "wake %sactive %02x\n",
				musb->is_active ? "" : "in", reg);

		
	}

	if (int_src & TUSB_INT_SRC_USB_IP_CONN)
		del_timer(&musb_idle_timer);

	
	if (int_src & (TUSB_INT_SRC_VBUS_SENSE_CHNG
				| TUSB_INT_SRC_OTG_TIMEOUT
				| TUSB_INT_SRC_ID_STATUS_CHNG))
		idle_timeout = tusb_otg_ints(musb, int_src, tbase);

	
	if ((int_src & TUSB_INT_SRC_TXRX_DMA_DONE)) {
		u32	dma_src = musb_readl(tbase, TUSB_DMA_INT_SRC);
		u32	real_dma_src = musb_readl(tbase, TUSB_DMA_INT_MASK);

		DBG(3, "DMA IRQ %08x\n", dma_src);
		real_dma_src = ~real_dma_src & dma_src;
		if (tusb_dma_omap() && real_dma_src) {
			int	tx_source = (real_dma_src & 0xffff);
			int	i;

			for (i = 1; i <= 15; i++) {
				if (tx_source & (1 << i)) {
					DBG(3, "completing ep%i %s\n", i, "tx");
					musb_dma_completion(musb, i, 1);
				}
			}
		}
		musb_writel(tbase, TUSB_DMA_INT_CLEAR, dma_src);
	}

	
	if (int_src & (TUSB_INT_SRC_USB_IP_TX | TUSB_INT_SRC_USB_IP_RX)) {
		u32	musb_src = musb_readl(tbase, TUSB_USBIP_INT_SRC);

		musb_writel(tbase, TUSB_USBIP_INT_CLEAR, musb_src);
		musb->int_rx = (((musb_src >> 16) & 0xffff) << 1);
		musb->int_tx = (musb_src & 0xffff);
	} else {
		musb->int_rx = 0;
		musb->int_tx = 0;
	}

	if (int_src & (TUSB_INT_SRC_USB_IP_TX | TUSB_INT_SRC_USB_IP_RX | 0xff))
		musb_interrupt(musb);

	
	musb_writel(tbase, TUSB_INT_SRC_CLEAR,
		int_src & ~TUSB_INT_MASK_RESERVED_BITS);

	musb_platform_try_idle(musb, idle_timeout);

	musb_writel(tbase, TUSB_INT_MASK, int_mask);
	spin_unlock_irqrestore(&musb->lock, flags);

	return IRQ_HANDLED;
}

static int dma_off;


void musb_platform_enable(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;

	
	musb_writel(tbase, TUSB_INT_MASK, TUSB_INT_SRC_USB_IP_SOF);

	
	musb_writel(tbase, TUSB_USBIP_INT_MASK, 0);
	musb_writel(tbase, TUSB_DMA_INT_MASK, 0x7fffffff);
	musb_writel(tbase, TUSB_GPIO_INT_MASK, 0x1ff);

	
	musb_writel(tbase, TUSB_USBIP_INT_CLEAR, 0x7fffffff);
	musb_writel(tbase, TUSB_DMA_INT_CLEAR, 0x7fffffff);
	musb_writel(tbase, TUSB_GPIO_INT_CLEAR, 0x1ff);

	
	musb_writel(tbase, TUSB_INT_SRC_CLEAR, ~TUSB_INT_MASK_RESERVED_BITS);

	
	musb_writel(tbase, TUSB_INT_CTRL_CONF,
			TUSB_INT_CTRL_CONF_INT_RELCYC(0));

	set_irq_type(musb->nIrq, IRQ_TYPE_LEVEL_LOW);

	
	if (!(musb_readl(tbase, TUSB_DEV_OTG_STAT)
			& TUSB_DEV_OTG_STAT_ID_STATUS))
		musb_writel(tbase, TUSB_INT_SRC_SET,
				TUSB_INT_SRC_ID_STATUS_CHNG);

	if (is_dma_capable() && dma_off)
		printk(KERN_WARNING "%s %s: dma not reactivated\n",
				__FILE__, __func__);
	else
		dma_off = 1;
}


void musb_platform_disable(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;

	

	
	musb_writel(tbase, TUSB_INT_MASK, ~TUSB_INT_MASK_RESERVED_BITS);
	musb_writel(tbase, TUSB_USBIP_INT_MASK, 0x7fffffff);
	musb_writel(tbase, TUSB_DMA_INT_MASK, 0x7fffffff);
	musb_writel(tbase, TUSB_GPIO_INT_MASK, 0x1ff);

	del_timer(&musb_idle_timer);

	if (is_dma_capable() && !dma_off) {
		printk(KERN_WARNING "%s %s: dma still active\n",
				__FILE__, __func__);
		dma_off = 1;
	}
}


static void __init tusb_setup_cpu_interface(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;

	
	musb_writel(tbase, TUSB_PULLUP_1_CTRL, 0x0000003F);

	
	musb_writel(tbase, TUSB_PULLUP_2_CTRL, 0x01FFFFFF);

	
	musb_writel(tbase, TUSB_GPIO_CONF, TUSB_GPIO_CONF_DMAREQ(0x3f));

	
	musb_writel(tbase, TUSB_DMA_REQ_CONF,
		TUSB_DMA_REQ_CONF_BURST_SIZE(2) |
		TUSB_DMA_REQ_CONF_DMA_REQ_EN(0x3f) |
		TUSB_DMA_REQ_CONF_DMA_REQ_ASSER(2));

	
	musb_writel(tbase, TUSB_WAIT_COUNT, 1);
}

static int __init tusb_start(struct musb *musb)
{
	void __iomem	*tbase = musb->ctrl_base;
	int		ret = 0;
	unsigned long	flags;
	u32		reg;

	if (musb->board_set_power)
		ret = musb->board_set_power(1);
	if (ret != 0) {
		printk(KERN_ERR "tusb: Cannot enable TUSB6010\n");
		return ret;
	}

	spin_lock_irqsave(&musb->lock, flags);

	if (musb_readl(tbase, TUSB_PROD_TEST_RESET) !=
		TUSB_PROD_TEST_RESET_VAL) {
		printk(KERN_ERR "tusb: Unable to detect TUSB6010\n");
		goto err;
	}

	ret = tusb_print_revision(musb);
	if (ret < 2) {
		printk(KERN_ERR "tusb: Unsupported TUSB6010 revision %i\n",
				ret);
		goto err;
	}

	
	musb_writel(tbase, TUSB_VLYNQ_CTRL, 8);

	
	tusb_set_clock_source(musb, 1);

	
	musb_writel(tbase, TUSB_PRCM_MNGMT,
		TUSB_PRCM_MNGMT_VBUS_VALID_TIMER(0xa) |
		TUSB_PRCM_MNGMT_VBUS_VALID_FLT_EN |
		TUSB_PRCM_MNGMT_OTG_SESS_END_EN |
		TUSB_PRCM_MNGMT_OTG_VBUS_DET_EN |
		TUSB_PRCM_MNGMT_OTG_ID_PULLUP);
	tusb_setup_cpu_interface(musb);

	
	reg = musb_readl(tbase, TUSB_PHY_OTG_CTRL_ENABLE);
	reg |= TUSB_PHY_OTG_CTRL_WRPROTECT | TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
	musb_writel(tbase, TUSB_PHY_OTG_CTRL_ENABLE, reg);

	reg = musb_readl(tbase, TUSB_PHY_OTG_CTRL);
	reg |= TUSB_PHY_OTG_CTRL_WRPROTECT | TUSB_PHY_OTG_CTRL_OTG_ID_PULLUP;
	musb_writel(tbase, TUSB_PHY_OTG_CTRL, reg);

	spin_unlock_irqrestore(&musb->lock, flags);

	return 0;

err:
	spin_unlock_irqrestore(&musb->lock, flags);

	if (musb->board_set_power)
		musb->board_set_power(0);

	return -ENODEV;
}

int __init musb_platform_init(struct musb *musb)
{
	struct platform_device	*pdev;
	struct resource		*mem;
	void __iomem		*sync = NULL;
	int			ret;

	usb_nop_xceiv_register();
	musb->xceiv = otg_get_transceiver();
	if (!musb->xceiv)
		return -ENODEV;

	pdev = to_platform_device(musb->controller);

	
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	musb->async = mem->start;

	
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!mem) {
		pr_debug("no sync dma resource?\n");
		ret = -ENODEV;
		goto done;
	}
	musb->sync = mem->start;

	sync = ioremap(mem->start, mem->end - mem->start + 1);
	if (!sync) {
		pr_debug("ioremap for sync failed\n");
		ret = -ENOMEM;
		goto done;
	}
	musb->sync_va = sync;

	
	musb->mregs += TUSB_BASE_OFFSET;

	ret = tusb_start(musb);
	if (ret) {
		printk(KERN_ERR "Could not start tusb6010 (%d)\n",
				ret);
		goto done;
	}
	musb->isr = tusb_interrupt;

	if (is_host_enabled(musb))
		musb->board_set_vbus = tusb_source_power;
	if (is_peripheral_enabled(musb)) {
		musb->xceiv->set_power = tusb_draw_power;
		the_musb = musb;
	}

	setup_timer(&musb_idle_timer, musb_do_idle, (unsigned long) musb);

done:
	if (ret < 0) {
		if (sync)
			iounmap(sync);
		usb_nop_xceiv_unregister();
	}
	return ret;
}

int musb_platform_exit(struct musb *musb)
{
	del_timer_sync(&musb_idle_timer);
	the_musb = NULL;

	if (musb->board_set_power)
		musb->board_set_power(0);

	iounmap(musb->sync_va);
	usb_nop_xceiv_unregister();
	return 0;
}
