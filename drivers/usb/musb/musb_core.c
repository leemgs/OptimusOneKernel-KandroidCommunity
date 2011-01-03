





#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/kobject.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#ifdef	CONFIG_ARM
#include <mach/hardware.h>
#include <mach/memory.h>
#include <asm/mach-types.h>
#endif

#include "musb_core.h"


#ifdef CONFIG_ARCH_DAVINCI
#include "davinci.h"
#endif

#define TA_WAIT_BCON(m) max_t(int, (m)->a_wait_bcon, OTG_TIME_A_WAIT_BCON)


unsigned musb_debug;
module_param_named(debug, musb_debug, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug message level. Default = 0");

#define DRIVER_AUTHOR "Mentor Graphics, Texas Instruments, Nokia"
#define DRIVER_DESC "Inventra Dual-Role USB Controller Driver"

#define MUSB_VERSION "6.0"

#define DRIVER_INFO DRIVER_DESC ", v" MUSB_VERSION

#define MUSB_DRIVER_NAME "musb_hdrc"
const char musb_driver_name[] = MUSB_DRIVER_NAME;

MODULE_DESCRIPTION(DRIVER_INFO);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" MUSB_DRIVER_NAME);




static inline struct musb *dev_to_musb(struct device *dev)
{
#ifdef CONFIG_USB_MUSB_HDRC_HCD
	
	return hcd_to_musb(dev_get_drvdata(dev));
#else
	return dev_get_drvdata(dev);
#endif
}



#if !defined(CONFIG_USB_TUSB6010) && !defined(CONFIG_BLACKFIN)


void musb_write_fifo(struct musb_hw_ep *hw_ep, u16 len, const u8 *src)
{
	void __iomem *fifo = hw_ep->fifo;

	prefetch((u8 *)src);

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n",
			'T', hw_ep->epnum, fifo, len, src);

	
	if (likely((0x01 & (unsigned long) src) == 0)) {
		u16	index = 0;

		
		if ((0x02 & (unsigned long) src) == 0) {
			if (len >= 4) {
				writesl(fifo, src + index, len >> 2);
				index += len & ~0x03;
			}
			if (len & 0x02) {
				musb_writew(fifo, 0, *(u16 *)&src[index]);
				index += 2;
			}
		} else {
			if (len >= 2) {
				writesw(fifo, src + index, len >> 1);
				index += len & ~0x01;
			}
		}
		if (len & 0x01)
			musb_writeb(fifo, 0, src[index]);
	} else  {
		
		writesb(fifo, src, len);
	}
}


void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst)
{
	void __iomem *fifo = hw_ep->fifo;

	DBG(4, "%cX ep%d fifo %p count %d buf %p\n",
			'R', hw_ep->epnum, fifo, len, dst);

	
	if (likely((0x01 & (unsigned long) dst) == 0)) {
		u16	index = 0;

		
		if ((0x02 & (unsigned long) dst) == 0) {
			if (len >= 4) {
				readsl(fifo, dst, len >> 2);
				index = len & ~0x03;
			}
			if (len & 0x02) {
				*(u16 *)&dst[index] = musb_readw(fifo, 0);
				index += 2;
			}
		} else {
			if (len >= 2) {
				readsw(fifo, dst, len >> 1);
				index = len & ~0x01;
			}
		}
		if (len & 0x01)
			dst[index] = musb_readb(fifo, 0);
	} else  {
		
		readsb(fifo, dst, len);
	}
}

#endif	





static const u8 musb_test_packet[53] = {
	

	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	
	0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
	
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	
	0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd,
	
	0xfc, 0x7e, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0x7e

	
};

void musb_load_testpacket(struct musb *musb)
{
	void __iomem	*regs = musb->endpoints[0].regs;

	musb_ep_select(musb->mregs, 0);
	musb_write_fifo(musb->control_ep,
			sizeof(musb_test_packet), musb_test_packet);
	musb_writew(regs, MUSB_CSR0, MUSB_CSR0_TXPKTRDY);
}



const char *otg_state_string(struct musb *musb)
{
	switch (musb->xceiv->state) {
	case OTG_STATE_A_IDLE:		return "a_idle";
	case OTG_STATE_A_WAIT_VRISE:	return "a_wait_vrise";
	case OTG_STATE_A_WAIT_BCON:	return "a_wait_bcon";
	case OTG_STATE_A_HOST:		return "a_host";
	case OTG_STATE_A_SUSPEND:	return "a_suspend";
	case OTG_STATE_A_PERIPHERAL:	return "a_peripheral";
	case OTG_STATE_A_WAIT_VFALL:	return "a_wait_vfall";
	case OTG_STATE_A_VBUS_ERR:	return "a_vbus_err";
	case OTG_STATE_B_IDLE:		return "b_idle";
	case OTG_STATE_B_SRP_INIT:	return "b_srp_init";
	case OTG_STATE_B_PERIPHERAL:	return "b_peripheral";
	case OTG_STATE_B_WAIT_ACON:	return "b_wait_acon";
	case OTG_STATE_B_HOST:		return "b_host";
	default:			return "UNDEFINED";
	}
}

#ifdef	CONFIG_USB_MUSB_OTG


void musb_otg_timer_func(unsigned long data)
{
	struct musb	*musb = (struct musb *)data;
	unsigned long	flags;

	spin_lock_irqsave(&musb->lock, flags);
	switch (musb->xceiv->state) {
	case OTG_STATE_B_WAIT_ACON:
		DBG(1, "HNP: b_wait_acon timeout; back to b_peripheral\n");
		musb_g_disconnect(musb);
		musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		musb->is_active = 0;
		break;
	case OTG_STATE_A_SUSPEND:
	case OTG_STATE_A_WAIT_BCON:
		DBG(1, "HNP: %s timeout\n", otg_state_string(musb));
		musb_set_vbus(musb, 0);
		musb->xceiv->state = OTG_STATE_A_WAIT_VFALL;
		break;
	default:
		DBG(1, "HNP: Unhandled mode %s\n", otg_state_string(musb));
	}
	musb->ignore_disconnect = 0;
	spin_unlock_irqrestore(&musb->lock, flags);
}


void musb_hnp_stop(struct musb *musb)
{
	struct usb_hcd	*hcd = musb_to_hcd(musb);
	void __iomem	*mbase = musb->mregs;
	u8	reg;

	DBG(1, "HNP: stop from %s\n", otg_state_string(musb));

	switch (musb->xceiv->state) {
	case OTG_STATE_A_PERIPHERAL:
		musb_g_disconnect(musb);
		DBG(1, "HNP: back to %s\n", otg_state_string(musb));
		break;
	case OTG_STATE_B_HOST:
		DBG(1, "HNP: Disabling HR\n");
		hcd->self.is_b_host = 0;
		musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
		MUSB_DEV_MODE(musb);
		reg = musb_readb(mbase, MUSB_POWER);
		reg |= MUSB_POWER_SUSPENDM;
		musb_writeb(mbase, MUSB_POWER, reg);
		
		break;
	default:
		DBG(1, "HNP: Stopping in unknown state %s\n",
			otg_state_string(musb));
	}

	
	musb->port1_status &=
		~(1 << USB_PORT_FEAT_C_CONNECTION);
}

#endif



#define STAGE0_MASK (MUSB_INTR_RESUME | MUSB_INTR_SESSREQ \
		| MUSB_INTR_VBUSERROR | MUSB_INTR_CONNECT \
		| MUSB_INTR_RESET)

static irqreturn_t musb_stage0_irq(struct musb *musb, u8 int_usb,
				u8 devctl, u8 power)
{
	irqreturn_t handled = IRQ_NONE;
	void __iomem *mbase = musb->mregs;

	DBG(3, "<== Power=%02x, DevCtl=%02x, int_usb=0x%x\n", power, devctl,
		int_usb);

	
	if (int_usb & MUSB_INTR_RESUME) {
		handled = IRQ_HANDLED;
		DBG(3, "RESUME (%s)\n", otg_state_string(musb));

		if (devctl & MUSB_DEVCTL_HM) {
#ifdef CONFIG_USB_MUSB_HDRC_HCD
			switch (musb->xceiv->state) {
			case OTG_STATE_A_SUSPEND:
				

				if (power & MUSB_POWER_SUSPENDM) {
					
					musb->int_usb &= ~MUSB_INTR_SUSPEND;
					DBG(2, "Spurious SUSPENDM\n");
					break;
				}

				power &= ~MUSB_POWER_SUSPENDM;
				musb_writeb(mbase, MUSB_POWER,
						power | MUSB_POWER_RESUME);

				musb->port1_status |=
						(USB_PORT_STAT_C_SUSPEND << 16)
						| MUSB_PORT_STAT_RESUME;
				musb->rh_timer = jiffies
						+ msecs_to_jiffies(20);

				musb->xceiv->state = OTG_STATE_A_HOST;
				musb->is_active = 1;
				usb_hcd_resume_root_hub(musb_to_hcd(musb));
				break;
			case OTG_STATE_B_WAIT_ACON:
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				musb->is_active = 1;
				MUSB_DEV_MODE(musb);
				break;
			default:
				WARNING("bogus %s RESUME (%s)\n",
					"host",
					otg_state_string(musb));
			}
#endif
		} else {
			switch (musb->xceiv->state) {
#ifdef CONFIG_USB_MUSB_HDRC_HCD
			case OTG_STATE_A_SUSPEND:
				
				musb->xceiv->state = OTG_STATE_A_HOST;
				usb_hcd_resume_root_hub(musb_to_hcd(musb));
				break;
#endif
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
			case OTG_STATE_B_WAIT_ACON:
			case OTG_STATE_B_PERIPHERAL:
				
				if ((devctl & MUSB_DEVCTL_VBUS)
						!= (3 << MUSB_DEVCTL_VBUS_SHIFT)
						) {
					musb->int_usb |= MUSB_INTR_DISCONNECT;
					musb->int_usb &= ~MUSB_INTR_SUSPEND;
					break;
				}
				musb_g_resume(musb);
				break;
			case OTG_STATE_B_IDLE:
				musb->int_usb &= ~MUSB_INTR_SUSPEND;
				break;
#endif
			default:
				WARNING("bogus %s RESUME (%s)\n",
					"peripheral",
					otg_state_string(musb));
			}
		}
	}

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	
	if (int_usb & MUSB_INTR_SESSREQ) {
		DBG(1, "SESSION_REQUEST (%s)\n", otg_state_string(musb));

		
		musb_writeb(mbase, MUSB_DEVCTL, MUSB_DEVCTL_SESSION);
		musb->ep0_stage = MUSB_EP0_START;
		musb->xceiv->state = OTG_STATE_A_IDLE;
		MUSB_HST_MODE(musb);
		musb_set_vbus(musb, 1);

		handled = IRQ_HANDLED;
	}

	if (int_usb & MUSB_INTR_VBUSERROR) {
		int	ignore = 0;

		
		switch (musb->xceiv->state) {
		case OTG_STATE_A_HOST:
			
		case OTG_STATE_A_WAIT_BCON:
		case OTG_STATE_A_WAIT_VRISE:
			if (musb->vbuserr_retry) {
				musb->vbuserr_retry--;
				ignore = 1;
				devctl |= MUSB_DEVCTL_SESSION;
				musb_writeb(mbase, MUSB_DEVCTL, devctl);
			} else {
				musb->port1_status |=
					  (1 << USB_PORT_FEAT_OVER_CURRENT)
					| (1 << USB_PORT_FEAT_C_OVER_CURRENT);
			}
			break;
		default:
			break;
		}

		DBG(1, "VBUS_ERROR in %s (%02x, %s), retry #%d, port1 %08x\n",
				otg_state_string(musb),
				devctl,
				({ char *s;
				switch (devctl & MUSB_DEVCTL_VBUS) {
				case 0 << MUSB_DEVCTL_VBUS_SHIFT:
					s = "<SessEnd"; break;
				case 1 << MUSB_DEVCTL_VBUS_SHIFT:
					s = "<AValid"; break;
				case 2 << MUSB_DEVCTL_VBUS_SHIFT:
					s = "<VBusValid"; break;
				
				default:
					s = "VALID"; break;
				}; s; }),
				VBUSERR_RETRY_COUNT - musb->vbuserr_retry,
				musb->port1_status);

		
		if (!ignore)
			musb_set_vbus(musb, 0);
		handled = IRQ_HANDLED;
	}

	if (int_usb & MUSB_INTR_CONNECT) {
		struct usb_hcd *hcd = musb_to_hcd(musb);

		handled = IRQ_HANDLED;
		musb->is_active = 1;
		set_bit(HCD_FLAG_SAW_IRQ, &hcd->flags);

		musb->ep0_stage = MUSB_EP0_START;

#ifdef CONFIG_USB_MUSB_OTG
		
		if (is_peripheral_active(musb)) {
			
		}
		musb_writew(mbase, MUSB_INTRTXE, musb->epmask);
		musb_writew(mbase, MUSB_INTRRXE, musb->epmask & 0xfffe);
		musb_writeb(mbase, MUSB_INTRUSBE, 0xf7);
#endif
		musb->port1_status &= ~(USB_PORT_STAT_LOW_SPEED
					|USB_PORT_STAT_HIGH_SPEED
					|USB_PORT_STAT_ENABLE
					);
		musb->port1_status |= USB_PORT_STAT_CONNECTION
					|(USB_PORT_STAT_C_CONNECTION << 16);

		
		if (devctl & MUSB_DEVCTL_LSDEV)
			musb->port1_status |= USB_PORT_STAT_LOW_SPEED;

		
		switch (musb->xceiv->state) {
		case OTG_STATE_B_PERIPHERAL:
			if (int_usb & MUSB_INTR_SUSPEND) {
				DBG(1, "HNP: SUSPEND+CONNECT, now b_host\n");
				int_usb &= ~MUSB_INTR_SUSPEND;
				goto b_host;
			} else
				DBG(1, "CONNECT as b_peripheral???\n");
			break;
		case OTG_STATE_B_WAIT_ACON:
			DBG(1, "HNP: CONNECT, now b_host\n");
b_host:
			musb->xceiv->state = OTG_STATE_B_HOST;
			hcd->self.is_b_host = 1;
			musb->ignore_disconnect = 0;
			del_timer(&musb->otg_timer);
			break;
		default:
			if ((devctl & MUSB_DEVCTL_VBUS)
					== (3 << MUSB_DEVCTL_VBUS_SHIFT)) {
				musb->xceiv->state = OTG_STATE_A_HOST;
				hcd->self.is_b_host = 0;
			}
			break;
		}

		
		MUSB_HST_MODE(musb);
		if (hcd->status_urb)
			usb_hcd_poll_rh_status(hcd);
		else
			usb_hcd_resume_root_hub(hcd);

		DBG(1, "CONNECT (%s) devctl %02x\n",
				otg_state_string(musb), devctl);
	}
#endif	

	
	if (int_usb & MUSB_INTR_RESET) {
		if (is_host_capable() && (devctl & MUSB_DEVCTL_HM) != 0) {
			
			if (devctl & (MUSB_DEVCTL_FSDEV | MUSB_DEVCTL_LSDEV))
				DBG(1, "BABBLE devctl: %02x\n", devctl);
			else {
				ERR("Stopping host session -- babble\n");
				musb_writeb(mbase, MUSB_DEVCTL, 0);
			}
		} else if (is_peripheral_capable()) {
			DBG(1, "BUS RESET as %s\n", otg_state_string(musb));
			switch (musb->xceiv->state) {
#ifdef CONFIG_USB_OTG
			case OTG_STATE_A_SUSPEND:
				
				musb->ignore_disconnect = 1;
				musb_g_reset(musb);
				
			case OTG_STATE_A_WAIT_BCON:	
				
				DBG(1, "HNP: in %s, %d msec timeout\n",
						otg_state_string(musb),
						TA_WAIT_BCON(musb));
				mod_timer(&musb->otg_timer, jiffies
					+ msecs_to_jiffies(TA_WAIT_BCON(musb)));
				break;
			case OTG_STATE_A_PERIPHERAL:
				musb->ignore_disconnect = 0;
				del_timer(&musb->otg_timer);
				musb_g_reset(musb);
				break;
			case OTG_STATE_B_WAIT_ACON:
				DBG(1, "HNP: RESET (%s), to b_peripheral\n",
					otg_state_string(musb));
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				musb_g_reset(musb);
				break;
#endif
			case OTG_STATE_B_IDLE:
				musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
				
			case OTG_STATE_B_PERIPHERAL:
				musb_g_reset(musb);
				break;
			default:
				DBG(1, "Unhandled BUS RESET as %s\n",
					otg_state_string(musb));
			}
		}

		handled = IRQ_HANDLED;
	}
	schedule_work(&musb->irq_work);

	return handled;
}


static irqreturn_t musb_stage2_irq(struct musb *musb, u8 int_usb,
				u8 devctl, u8 power)
{
	irqreturn_t handled = IRQ_NONE;

#if 0

	if (int_usb & MUSB_INTR_SOF) {
		void __iomem *mbase = musb->mregs;
		struct musb_hw_ep	*ep;
		u8 epnum;
		u16 frame;

		DBG(6, "START_OF_FRAME\n");
		handled = IRQ_HANDLED;

		
		frame = musb_readw(mbase, MUSB_FRAME);
		ep = musb->endpoints;
		for (epnum = 1; (epnum < musb->nr_endpoints)
					&& (musb->epmask >= (1 << epnum));
				epnum++, ep++) {
			
			if (ep->dwWaitFrame >= frame) {
				ep->dwWaitFrame = 0;
				pr_debug("SOF --> periodic TX%s on %d\n",
					ep->tx_channel ? " DMA" : "",
					epnum);
				if (!ep->tx_channel)
					musb_h_tx_start(musb, epnum);
				else
					cppi_hostdma_start(musb, epnum);
			}
		}		
	}
#endif

	if ((int_usb & MUSB_INTR_DISCONNECT) && !musb->ignore_disconnect) {
		DBG(1, "DISCONNECT (%s) as %s, devctl %02x\n",
				otg_state_string(musb),
				MUSB_MODE(musb), devctl);
		handled = IRQ_HANDLED;

		switch (musb->xceiv->state) {
#ifdef CONFIG_USB_MUSB_HDRC_HCD
		case OTG_STATE_A_HOST:
		case OTG_STATE_A_SUSPEND:
			usb_hcd_resume_root_hub(musb_to_hcd(musb));
			musb_root_disconnect(musb);
			if (musb->a_wait_bcon != 0 && is_otg_enabled(musb))
				musb_platform_try_idle(musb, jiffies
					+ msecs_to_jiffies(musb->a_wait_bcon));
			break;
#endif	
#ifdef CONFIG_USB_MUSB_OTG
		case OTG_STATE_B_HOST:
			
			musb_root_disconnect(musb);
			musb_to_hcd(musb)->self.is_b_host = 0;
			musb->xceiv->state = OTG_STATE_B_PERIPHERAL;
			MUSB_DEV_MODE(musb);
			musb_g_disconnect(musb);
			break;
		case OTG_STATE_A_PERIPHERAL:
			musb_hnp_stop(musb);
			musb_root_disconnect(musb);
			
		case OTG_STATE_B_WAIT_ACON:
			
#endif	
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
		case OTG_STATE_B_PERIPHERAL:
		case OTG_STATE_B_IDLE:
			musb_g_disconnect(musb);
			break;
#endif	
		default:
			WARNING("unhandled DISCONNECT transition (%s)\n",
				otg_state_string(musb));
			break;
		}

		schedule_work(&musb->irq_work);
	}

	if (int_usb & MUSB_INTR_SUSPEND) {
		DBG(1, "SUSPEND (%s) devctl %02x power %02x\n",
				otg_state_string(musb), devctl, power);
		handled = IRQ_HANDLED;

		switch (musb->xceiv->state) {
#ifdef	CONFIG_USB_MUSB_OTG
		case OTG_STATE_A_PERIPHERAL:
			
			musb_hnp_stop(musb);
			usb_hcd_resume_root_hub(musb_to_hcd(musb));
			musb_root_disconnect(musb);
			musb_platform_try_idle(musb, jiffies
					+ msecs_to_jiffies(musb->a_wait_bcon
						? : OTG_TIME_A_WAIT_BCON));
			break;
#endif
		case OTG_STATE_B_PERIPHERAL:
			musb_g_suspend(musb);
			musb->is_active = is_otg_enabled(musb)
					&& musb->xceiv->gadget->b_hnp_enable;
			if (musb->is_active) {
#ifdef	CONFIG_USB_MUSB_OTG
				musb->xceiv->state = OTG_STATE_B_WAIT_ACON;
				DBG(1, "HNP: Setting timer for b_ase0_brst\n");
				mod_timer(&musb->otg_timer, jiffies
					+ msecs_to_jiffies(
							OTG_TIME_B_ASE0_BRST));
#endif
			}
			break;
		case OTG_STATE_A_WAIT_BCON:
			if (musb->a_wait_bcon != 0)
				musb_platform_try_idle(musb, jiffies
					+ msecs_to_jiffies(musb->a_wait_bcon));
			break;
		case OTG_STATE_A_HOST:
			musb->xceiv->state = OTG_STATE_A_SUSPEND;
			musb->is_active = is_otg_enabled(musb)
					&& musb->xceiv->host->b_hnp_enable;
			break;
		case OTG_STATE_B_HOST:
			
			DBG(1, "REVISIT: SUSPEND as B_HOST\n");
			break;
		default:
			
			musb->is_active = 0;
			break;
		}
		schedule_work(&musb->irq_work);
	}


	return handled;
}




void musb_start(struct musb *musb)
{
	void __iomem	*regs = musb->mregs;
	u8		devctl = musb_readb(regs, MUSB_DEVCTL);

	DBG(2, "<== devctl %02x\n", devctl);

	
	musb_writew(regs, MUSB_INTRTXE, musb->epmask);
	musb_writew(regs, MUSB_INTRRXE, musb->epmask & 0xfffe);
	musb_writeb(regs, MUSB_INTRUSBE, 0xf7);

	musb_writeb(regs, MUSB_TESTMODE, 0);

	
	musb_writeb(regs, MUSB_POWER, MUSB_POWER_ISOUPDATE
						| MUSB_POWER_SOFTCONN
						| MUSB_POWER_HSENAB
						
						
						);

	musb->is_active = 0;
	devctl = musb_readb(regs, MUSB_DEVCTL);
	devctl &= ~MUSB_DEVCTL_SESSION;

	if (is_otg_enabled(musb)) {
		
		if ((devctl & MUSB_DEVCTL_VBUS) == MUSB_DEVCTL_VBUS)
			musb->is_active = 1;
		else
			devctl |= MUSB_DEVCTL_SESSION;

	} else if (is_host_enabled(musb)) {
		
		devctl |= MUSB_DEVCTL_SESSION;

	} else  {
		if ((devctl & MUSB_DEVCTL_VBUS) == MUSB_DEVCTL_VBUS)
			musb->is_active = 1;
	}
	musb_platform_enable(musb);
	musb_writeb(regs, MUSB_DEVCTL, devctl);
}


static void musb_generic_disable(struct musb *musb)
{
	void __iomem	*mbase = musb->mregs;
	u16	temp;

	
	musb_writeb(mbase, MUSB_INTRUSBE, 0);
	musb_writew(mbase, MUSB_INTRTXE, 0);
	musb_writew(mbase, MUSB_INTRRXE, 0);

	
	musb_writeb(mbase, MUSB_DEVCTL, 0);

	
	temp = musb_readb(mbase, MUSB_INTRUSB);
	temp = musb_readw(mbase, MUSB_INTRTX);
	temp = musb_readw(mbase, MUSB_INTRRX);

}


void musb_stop(struct musb *musb)
{
	
	musb_platform_disable(musb);
	musb_generic_disable(musb);
	DBG(3, "HDRC disabled\n");

	
	musb_platform_try_idle(musb, 0);
}

static void musb_shutdown(struct platform_device *pdev)
{
	struct musb	*musb = dev_to_musb(&pdev->dev);
	unsigned long	flags;

	spin_lock_irqsave(&musb->lock, flags);
	musb_platform_disable(musb);
	musb_generic_disable(musb);
	if (musb->clock) {
		clk_put(musb->clock);
		musb->clock = NULL;
	}
	spin_unlock_irqrestore(&musb->lock, flags);

	
}





#if defined(CONFIG_USB_TUSB6010) || \
	defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP34XX)
static ushort __initdata fifo_mode = 4;
#else
static ushort __initdata fifo_mode = 2;
#endif


module_param(fifo_mode, ushort, 0);
MODULE_PARM_DESC(fifo_mode, "initial endpoint configuration");


enum fifo_style { FIFO_RXTX, FIFO_TX, FIFO_RX } __attribute__ ((packed));
enum buf_mode { BUF_SINGLE, BUF_DOUBLE } __attribute__ ((packed));

struct fifo_cfg {
	u8		hw_ep_num;
	enum fifo_style	style;
	enum buf_mode	mode;
	u16		maxpacket;
};




static struct fifo_cfg __initdata mode_0_cfg[] = {
{ .hw_ep_num = 1, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num = 1, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num = 2, .style = FIFO_RXTX, .maxpacket = 512, },
{ .hw_ep_num = 3, .style = FIFO_RXTX, .maxpacket = 256, },
{ .hw_ep_num = 4, .style = FIFO_RXTX, .maxpacket = 256, },
};


static struct fifo_cfg __initdata mode_1_cfg[] = {
{ .hw_ep_num = 1, .style = FIFO_TX,   .maxpacket = 512, .mode = BUF_DOUBLE, },
{ .hw_ep_num = 1, .style = FIFO_RX,   .maxpacket = 512, .mode = BUF_DOUBLE, },
{ .hw_ep_num = 2, .style = FIFO_RXTX, .maxpacket = 512, .mode = BUF_DOUBLE, },
{ .hw_ep_num = 3, .style = FIFO_RXTX, .maxpacket = 256, },
{ .hw_ep_num = 4, .style = FIFO_RXTX, .maxpacket = 256, },
};


static struct fifo_cfg __initdata mode_2_cfg[] = {
{ .hw_ep_num = 1, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num = 1, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num = 2, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num = 2, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num = 3, .style = FIFO_RXTX, .maxpacket = 256, },
{ .hw_ep_num = 4, .style = FIFO_RXTX, .maxpacket = 256, },
};


static struct fifo_cfg __initdata mode_3_cfg[] = {
{ .hw_ep_num = 1, .style = FIFO_TX,   .maxpacket = 512, .mode = BUF_DOUBLE, },
{ .hw_ep_num = 1, .style = FIFO_RX,   .maxpacket = 512, .mode = BUF_DOUBLE, },
{ .hw_ep_num = 2, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num = 2, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num = 3, .style = FIFO_RXTX, .maxpacket = 256, },
{ .hw_ep_num = 4, .style = FIFO_RXTX, .maxpacket = 256, },
};


static struct fifo_cfg __initdata mode_4_cfg[] = {
{ .hw_ep_num =  1, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  1, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  2, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  2, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  3, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  3, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  4, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  4, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  5, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  5, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  6, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  6, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  7, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  7, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  8, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  8, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num =  9, .style = FIFO_TX,   .maxpacket = 512, },
{ .hw_ep_num =  9, .style = FIFO_RX,   .maxpacket = 512, },
{ .hw_ep_num = 10, .style = FIFO_TX,   .maxpacket = 256, },
{ .hw_ep_num = 10, .style = FIFO_RX,   .maxpacket = 64, },
{ .hw_ep_num = 11, .style = FIFO_TX,   .maxpacket = 256, },
{ .hw_ep_num = 11, .style = FIFO_RX,   .maxpacket = 64, },
{ .hw_ep_num = 12, .style = FIFO_TX,   .maxpacket = 256, },
{ .hw_ep_num = 12, .style = FIFO_RX,   .maxpacket = 64, },
{ .hw_ep_num = 13, .style = FIFO_RXTX, .maxpacket = 4096, },
{ .hw_ep_num = 14, .style = FIFO_RXTX, .maxpacket = 1024, },
{ .hw_ep_num = 15, .style = FIFO_RXTX, .maxpacket = 1024, },
};



static int __init
fifo_setup(struct musb *musb, struct musb_hw_ep  *hw_ep,
		const struct fifo_cfg *cfg, u16 offset)
{
	void __iomem	*mbase = musb->mregs;
	int	size = 0;
	u16	maxpacket = cfg->maxpacket;
	u16	c_off = offset >> 3;
	u8	c_size;

	

	size = ffs(max(maxpacket, (u16) 8)) - 1;
	maxpacket = 1 << size;

	c_size = size - 3;
	if (cfg->mode == BUF_DOUBLE) {
		if ((offset + (maxpacket << 1)) >
				(1 << (musb->config->ram_bits + 2)))
			return -EMSGSIZE;
		c_size |= MUSB_FIFOSZ_DPB;
	} else {
		if ((offset + maxpacket) > (1 << (musb->config->ram_bits + 2)))
			return -EMSGSIZE;
	}

	
	musb_writeb(mbase, MUSB_INDEX, hw_ep->epnum);

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	
	if (hw_ep->epnum == 1)
		musb->bulk_ep = hw_ep;
	
#endif
	switch (cfg->style) {
	case FIFO_TX:
		musb_write_txfifosz(mbase, c_size);
		musb_write_txfifoadd(mbase, c_off);
		hw_ep->tx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_tx = maxpacket;
		break;
	case FIFO_RX:
		musb_write_rxfifosz(mbase, c_size);
		musb_write_rxfifoadd(mbase, c_off);
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;
		break;
	case FIFO_RXTX:
		musb_write_txfifosz(mbase, c_size);
		musb_write_txfifoadd(mbase, c_off);
		hw_ep->rx_double_buffered = !!(c_size & MUSB_FIFOSZ_DPB);
		hw_ep->max_packet_sz_rx = maxpacket;

		musb_write_rxfifosz(mbase, c_size);
		musb_write_rxfifoadd(mbase, c_off);
		hw_ep->tx_double_buffered = hw_ep->rx_double_buffered;
		hw_ep->max_packet_sz_tx = maxpacket;

		hw_ep->is_shared_fifo = true;
		break;
	}

	
	musb->epmask |= (1 << hw_ep->epnum);

	return offset + (maxpacket << ((c_size & MUSB_FIFOSZ_DPB) ? 1 : 0));
}

static struct fifo_cfg __initdata ep0_cfg = {
	.style = FIFO_RXTX, .maxpacket = 64,
};

static int __init ep_config_from_table(struct musb *musb)
{
	const struct fifo_cfg	*cfg;
	unsigned		i, n;
	int			offset;
	struct musb_hw_ep	*hw_ep = musb->endpoints;

	switch (fifo_mode) {
	default:
		fifo_mode = 0;
		
	case 0:
		cfg = mode_0_cfg;
		n = ARRAY_SIZE(mode_0_cfg);
		break;
	case 1:
		cfg = mode_1_cfg;
		n = ARRAY_SIZE(mode_1_cfg);
		break;
	case 2:
		cfg = mode_2_cfg;
		n = ARRAY_SIZE(mode_2_cfg);
		break;
	case 3:
		cfg = mode_3_cfg;
		n = ARRAY_SIZE(mode_3_cfg);
		break;
	case 4:
		cfg = mode_4_cfg;
		n = ARRAY_SIZE(mode_4_cfg);
		break;
	}

	printk(KERN_DEBUG "%s: setup fifo_mode %d\n",
			musb_driver_name, fifo_mode);


	offset = fifo_setup(musb, hw_ep, &ep0_cfg, 0);
	

	

	for (i = 0; i < n; i++) {
		u8	epn = cfg->hw_ep_num;

		if (epn >= musb->config->num_eps) {
			pr_debug("%s: invalid ep %d\n",
					musb_driver_name, epn);
			return -EINVAL;
		}
		offset = fifo_setup(musb, hw_ep + epn, cfg++, offset);
		if (offset < 0) {
			pr_debug("%s: mem overrun, ep %d\n",
					musb_driver_name, epn);
			return -EINVAL;
		}
		epn++;
		musb->nr_endpoints = max(epn, musb->nr_endpoints);
	}

	printk(KERN_DEBUG "%s: %d/%d max ep, %d/%d memory\n",
			musb_driver_name,
			n + 1, musb->config->num_eps * 2 - 1,
			offset, (1 << (musb->config->ram_bits + 2)));

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	if (!musb->bulk_ep) {
		pr_debug("%s: missing bulk\n", musb_driver_name);
		return -EINVAL;
	}
#endif

	return 0;
}



static int __init ep_config_from_hw(struct musb *musb)
{
	u8 epnum = 0;
	struct musb_hw_ep *hw_ep;
	void *mbase = musb->mregs;
	int ret = 0;

	DBG(2, "<== static silicon ep config\n");

	

	for (epnum = 1; epnum < musb->config->num_eps; epnum++) {
		musb_ep_select(mbase, epnum);
		hw_ep = musb->endpoints + epnum;

		ret = musb_read_fifosize(musb, hw_ep, epnum);
		if (ret < 0)
			break;

		

#ifdef CONFIG_USB_MUSB_HDRC_HCD
		
		if (hw_ep->max_packet_sz_tx < 512
				|| hw_ep->max_packet_sz_rx < 512)
			continue;

		
		if (musb->bulk_ep)
			continue;
		musb->bulk_ep = hw_ep;
#endif
	}

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	if (!musb->bulk_ep) {
		pr_debug("%s: missing bulk\n", musb_driver_name);
		return -EINVAL;
	}
#endif

	return 0;
}

enum { MUSB_CONTROLLER_MHDRC, MUSB_CONTROLLER_HDRC, };


static int __init musb_core_init(u16 musb_type, struct musb *musb)
{
#ifdef MUSB_AHB_ID
	u32 data;
#endif
	u8 reg;
	char *type;
	u16 hwvers, rev_major, rev_minor;
	char aInfo[78], aRevision[32], aDate[12];
	void __iomem	*mbase = musb->mregs;
	int		status = 0;
	int		i;

	
	reg = musb_read_configdata(mbase);

	strcpy(aInfo, (reg & MUSB_CONFIGDATA_UTMIDW) ? "UTMI-16" : "UTMI-8");
	if (reg & MUSB_CONFIGDATA_DYNFIFO)
		strcat(aInfo, ", dyn FIFOs");
	if (reg & MUSB_CONFIGDATA_MPRXE) {
		strcat(aInfo, ", bulk combine");
#ifdef C_MP_RX
		musb->bulk_combine = true;
#else
		strcat(aInfo, " (X)");		
#endif
	}
	if (reg & MUSB_CONFIGDATA_MPTXE) {
		strcat(aInfo, ", bulk split");
#ifdef C_MP_TX
		musb->bulk_split = true;
#else
		strcat(aInfo, " (X)");		
#endif
	}
	if (reg & MUSB_CONFIGDATA_HBRXE) {
		strcat(aInfo, ", HB-ISO Rx");
		musb->hb_iso_rx = true;
	}
	if (reg & MUSB_CONFIGDATA_HBTXE) {
		strcat(aInfo, ", HB-ISO Tx");
		musb->hb_iso_tx = true;
	}
	if (reg & MUSB_CONFIGDATA_SOFTCONE)
		strcat(aInfo, ", SoftConn");

	printk(KERN_DEBUG "%s: ConfigData=0x%02x (%s)\n",
			musb_driver_name, reg, aInfo);

#ifdef MUSB_AHB_ID
	data = musb_readl(mbase, 0x404);
	sprintf(aDate, "%04d-%02x-%02x", (data & 0xffff),
		(data >> 16) & 0xff, (data >> 24) & 0xff);
	
	data = musb_readl(mbase, 0x408);
	printk(KERN_DEBUG "ID2=%lx\n", (long unsigned)data);
	data = musb_readl(mbase, 0x40c);
	printk(KERN_DEBUG "ID3=%lx\n", (long unsigned)data);
	reg = musb_readb(mbase, 0x400);
	musb_type = ('M' == reg) ? MUSB_CONTROLLER_MHDRC : MUSB_CONTROLLER_HDRC;
#else
	aDate[0] = 0;
#endif
	if (MUSB_CONTROLLER_MHDRC == musb_type) {
		musb->is_multipoint = 1;
		type = "M";
	} else {
		musb->is_multipoint = 0;
		type = "";
#ifdef CONFIG_USB_MUSB_HDRC_HCD
#ifndef	CONFIG_USB_OTG_BLACKLIST_HUB
		printk(KERN_ERR
			"%s: kernel must blacklist external hubs\n",
			musb_driver_name);
#endif
#endif
	}

	
	hwvers = musb_read_hwvers(mbase);
	rev_major = (hwvers >> 10) & 0x1f;
	rev_minor = hwvers & 0x3ff;
	snprintf(aRevision, 32, "%d.%d%s", rev_major,
		rev_minor, (hwvers & 0x8000) ? "RC" : "");
	printk(KERN_DEBUG "%s: %sHDRC RTL version %s %s\n",
			musb_driver_name, type, aRevision, aDate);

	
	musb_configure_ep0(musb);

	
	musb->nr_endpoints = 1;
	musb->epmask = 1;

	if (reg & MUSB_CONFIGDATA_DYNFIFO) {
		if (musb->config->dyn_fifo)
			status = ep_config_from_table(musb);
		else {
			ERR("reconfigure software for Dynamic FIFOs\n");
			status = -ENODEV;
		}
	} else {
		if (!musb->config->dyn_fifo)
			status = ep_config_from_hw(musb);
		else {
			ERR("reconfigure software for static FIFOs\n");
			return -ENODEV;
		}
	}

	if (status < 0)
		return status;

	
	for (i = 0; i < musb->nr_endpoints; i++) {
		struct musb_hw_ep	*hw_ep = musb->endpoints + i;

		hw_ep->fifo = MUSB_FIFO_OFFSET(i) + mbase;
#ifdef CONFIG_USB_TUSB6010
		hw_ep->fifo_async = musb->async + 0x400 + MUSB_FIFO_OFFSET(i);
		hw_ep->fifo_sync = musb->sync + 0x400 + MUSB_FIFO_OFFSET(i);
		hw_ep->fifo_sync_va =
			musb->sync_va + 0x400 + MUSB_FIFO_OFFSET(i);

		if (i == 0)
			hw_ep->conf = mbase - 0x400 + TUSB_EP0_CONF;
		else
			hw_ep->conf = mbase + 0x400 + (((i - 1) & 0xf) << 2);
#endif

		hw_ep->regs = MUSB_EP_OFFSET(i, 0) + mbase;
#ifdef CONFIG_USB_MUSB_HDRC_HCD
		hw_ep->target_regs = musb_read_target_reg_base(i, mbase);
		hw_ep->rx_reinit = 1;
		hw_ep->tx_reinit = 1;
#endif

		if (hw_ep->max_packet_sz_tx) {
			DBG(1,
				"%s: hw_ep %d%s, %smax %d\n",
				musb_driver_name, i,
				hw_ep->is_shared_fifo ? "shared" : "tx",
				hw_ep->tx_double_buffered
					? "doublebuffer, " : "",
				hw_ep->max_packet_sz_tx);
		}
		if (hw_ep->max_packet_sz_rx && !hw_ep->is_shared_fifo) {
			DBG(1,
				"%s: hw_ep %d%s, %smax %d\n",
				musb_driver_name, i,
				"rx",
				hw_ep->rx_double_buffered
					? "doublebuffer, " : "",
				hw_ep->max_packet_sz_rx);
		}
		if (!(hw_ep->max_packet_sz_tx || hw_ep->max_packet_sz_rx))
			DBG(1, "hw_ep %d not configured\n", i);
	}

	return 0;
}



#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP3430)

static irqreturn_t generic_interrupt(int irq, void *__hci)
{
	unsigned long	flags;
	irqreturn_t	retval = IRQ_NONE;
	struct musb	*musb = __hci;

	spin_lock_irqsave(&musb->lock, flags);

	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval = musb_interrupt(musb);

	spin_unlock_irqrestore(&musb->lock, flags);

	return retval;
}

#else
#define generic_interrupt	NULL
#endif


irqreturn_t musb_interrupt(struct musb *musb)
{
	irqreturn_t	retval = IRQ_NONE;
	u8		devctl, power;
	int		ep_num;
	u32		reg;

	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
	power = musb_readb(musb->mregs, MUSB_POWER);

	DBG(4, "** IRQ %s usb%04x tx%04x rx%04x\n",
		(devctl & MUSB_DEVCTL_HM) ? "host" : "peripheral",
		musb->int_usb, musb->int_tx, musb->int_rx);

	
	if (musb->int_usb & STAGE0_MASK)
		retval |= musb_stage0_irq(musb, musb->int_usb,
				devctl, power);

	

	
	if (musb->int_tx & 1) {
		if (devctl & MUSB_DEVCTL_HM)
			retval |= musb_h_ep0_irq(musb);
		else
			retval |= musb_g_ep0_irq(musb);
	}

	
	reg = musb->int_rx >> 1;
	ep_num = 1;
	while (reg) {
		if (reg & 1) {
			
			
			retval = IRQ_HANDLED;
			if (devctl & MUSB_DEVCTL_HM) {
				if (is_host_capable())
					musb_host_rx(musb, ep_num);
			} else {
				if (is_peripheral_capable())
					musb_g_rx(musb, ep_num);
			}
		}

		reg >>= 1;
		ep_num++;
	}

	
	reg = musb->int_tx >> 1;
	ep_num = 1;
	while (reg) {
		if (reg & 1) {
			
			
			retval = IRQ_HANDLED;
			if (devctl & MUSB_DEVCTL_HM) {
				if (is_host_capable())
					musb_host_tx(musb, ep_num);
			} else {
				if (is_peripheral_capable())
					musb_g_tx(musb, ep_num);
			}
		}
		reg >>= 1;
		ep_num++;
	}

	
	if (musb->int_usb)
		retval |= musb_stage2_irq(musb,
				musb->int_usb, devctl, power);

	return retval;
}


#ifndef CONFIG_MUSB_PIO_ONLY
static int __initdata use_dma = 1;


module_param(use_dma, bool, 0);
MODULE_PARM_DESC(use_dma, "enable/disable use of DMA");

void musb_dma_completion(struct musb *musb, u8 epnum, u8 transmit)
{
	u8	devctl = musb_readb(musb->mregs, MUSB_DEVCTL);

	

	if (!epnum) {
#ifndef CONFIG_USB_TUSB_OMAP_DMA
		if (!is_cppi_enabled()) {
			
			if (devctl & MUSB_DEVCTL_HM)
				musb_h_ep0_irq(musb);
			else
				musb_g_ep0_irq(musb);
		}
#endif
	} else {
		
		if (transmit) {
			if (devctl & MUSB_DEVCTL_HM) {
				if (is_host_capable())
					musb_host_tx(musb, epnum);
			} else {
				if (is_peripheral_capable())
					musb_g_tx(musb, epnum);
			}
		} else {
			
			if (devctl & MUSB_DEVCTL_HM) {
				if (is_host_capable())
					musb_host_rx(musb, epnum);
			} else {
				if (is_peripheral_capable())
					musb_g_rx(musb, epnum);
			}
		}
	}
}

#else
#define use_dma			0
#endif



#ifdef CONFIG_SYSFS

static ssize_t
musb_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct musb *musb = dev_to_musb(dev);
	unsigned long flags;
	int ret = -EINVAL;

	spin_lock_irqsave(&musb->lock, flags);
	ret = sprintf(buf, "%s\n", otg_state_string(musb));
	spin_unlock_irqrestore(&musb->lock, flags);

	return ret;
}

static ssize_t
musb_mode_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct musb	*musb = dev_to_musb(dev);
	unsigned long	flags;
	int		status;

	spin_lock_irqsave(&musb->lock, flags);
	if (sysfs_streq(buf, "host"))
		status = musb_platform_set_mode(musb, MUSB_HOST);
	else if (sysfs_streq(buf, "peripheral"))
		status = musb_platform_set_mode(musb, MUSB_PERIPHERAL);
	else if (sysfs_streq(buf, "otg"))
		status = musb_platform_set_mode(musb, MUSB_OTG);
	else
		status = -EINVAL;
	spin_unlock_irqrestore(&musb->lock, flags);

	return (status == 0) ? n : status;
}
static DEVICE_ATTR(mode, 0644, musb_mode_show, musb_mode_store);

static ssize_t
musb_vbus_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct musb	*musb = dev_to_musb(dev);
	unsigned long	flags;
	unsigned long	val;

	if (sscanf(buf, "%lu", &val) < 1) {
		printk(KERN_ERR "Invalid VBUS timeout ms value\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&musb->lock, flags);
	
	musb->a_wait_bcon = val ? max_t(int, val, OTG_TIME_A_WAIT_BCON) : 0 ;
	if (musb->xceiv->state == OTG_STATE_A_WAIT_BCON)
		musb->is_active = 0;
	musb_platform_try_idle(musb, jiffies + msecs_to_jiffies(val));
	spin_unlock_irqrestore(&musb->lock, flags);

	return n;
}

static ssize_t
musb_vbus_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct musb	*musb = dev_to_musb(dev);
	unsigned long	flags;
	unsigned long	val;
	int		vbus;

	spin_lock_irqsave(&musb->lock, flags);
	val = musb->a_wait_bcon;
	
	vbus = musb_platform_get_vbus_status(musb);
	spin_unlock_irqrestore(&musb->lock, flags);

	return sprintf(buf, "Vbus %s, timeout %lu msec\n",
			vbus ? "on" : "off", val);
}
static DEVICE_ATTR(vbus, 0644, musb_vbus_show, musb_vbus_store);

#ifdef CONFIG_USB_GADGET_MUSB_HDRC


static ssize_t
musb_srp_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t n)
{
	struct musb	*musb = dev_to_musb(dev);
	unsigned short	srp;

	if (sscanf(buf, "%hu", &srp) != 1
			|| (srp != 1)) {
		printk(KERN_ERR "SRP: Value must be 1\n");
		return -EINVAL;
	}

	if (srp == 1)
		musb_g_wakeup(musb);

	return n;
}
static DEVICE_ATTR(srp, 0644, NULL, musb_srp_store);

#endif 

#endif	


static void musb_irq_work(struct work_struct *data)
{
	struct musb *musb = container_of(data, struct musb, irq_work);
	static int old_state;

	if (musb->xceiv->state != old_state) {
		old_state = musb->xceiv->state;
		sysfs_notify(&musb->controller->kobj, NULL, "mode");
	}
}



static struct musb *__init
allocate_instance(struct device *dev,
		struct musb_hdrc_config *config, void __iomem *mbase)
{
	struct musb		*musb;
	struct musb_hw_ep	*ep;
	int			epnum;
#ifdef CONFIG_USB_MUSB_HDRC_HCD
	struct usb_hcd	*hcd;

	hcd = usb_create_hcd(&musb_hc_driver, dev, dev_name(dev));
	if (!hcd)
		return NULL;
	

	musb = hcd_to_musb(hcd);
	INIT_LIST_HEAD(&musb->control);
	INIT_LIST_HEAD(&musb->in_bulk);
	INIT_LIST_HEAD(&musb->out_bulk);

	hcd->uses_new_polling = 1;

	musb->vbuserr_retry = VBUSERR_RETRY_COUNT;
	musb->a_wait_bcon = OTG_TIME_A_WAIT_BCON;
#else
	musb = kzalloc(sizeof *musb, GFP_KERNEL);
	if (!musb)
		return NULL;
	dev_set_drvdata(dev, musb);

#endif

	musb->mregs = mbase;
	musb->ctrl_base = mbase;
	musb->nIrq = -ENODEV;
	musb->config = config;
	BUG_ON(musb->config->num_eps > MUSB_C_NUM_EPS);
	for (epnum = 0, ep = musb->endpoints;
			epnum < musb->config->num_eps;
			epnum++, ep++) {
		ep->musb = musb;
		ep->epnum = epnum;
	}

	musb->controller = dev;
	return musb;
}

static void musb_free(struct musb *musb)
{
	

#ifdef CONFIG_SYSFS
	device_remove_file(musb->controller, &dev_attr_mode);
	device_remove_file(musb->controller, &dev_attr_vbus);
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	device_remove_file(musb->controller, &dev_attr_srp);
#endif
#endif

#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	musb_gadget_cleanup(musb);
#endif

	if (musb->nIrq >= 0) {
		if (musb->irq_wake)
			disable_irq_wake(musb->nIrq);
		free_irq(musb->nIrq, musb);
	}
	if (is_dma_capable() && musb->dma_controller) {
		struct dma_controller	*c = musb->dma_controller;

		(void) c->stop(c);
		dma_controller_destroy(c);
	}

#ifdef CONFIG_USB_MUSB_OTG
	put_device(musb->xceiv->dev);
#endif

	musb_writeb(musb->mregs, MUSB_DEVCTL, 0);
	musb_platform_exit(musb);
	musb_writeb(musb->mregs, MUSB_DEVCTL, 0);

	if (musb->clock) {
		clk_disable(musb->clock);
		clk_put(musb->clock);
	}

#ifdef CONFIG_USB_MUSB_HDRC_HCD
	usb_put_hcd(musb_to_hcd(musb));
#else
	kfree(musb);
#endif
}


static int __init
musb_init_controller(struct device *dev, int nIrq, void __iomem *ctrl)
{
	int			status;
	struct musb		*musb;
	struct musb_hdrc_platform_data *plat = dev->platform_data;

	
	if (!plat) {
		dev_dbg(dev, "no platform_data?\n");
		return -ENODEV;
	}
	switch (plat->mode) {
	case MUSB_HOST:
#ifdef CONFIG_USB_MUSB_HDRC_HCD
		break;
#else
		goto bad_config;
#endif
	case MUSB_PERIPHERAL:
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
		break;
#else
		goto bad_config;
#endif
	case MUSB_OTG:
#ifdef CONFIG_USB_MUSB_OTG
		break;
#else
bad_config:
#endif
	default:
		dev_err(dev, "incompatible Kconfig role setting\n");
		return -EINVAL;
	}

	
	musb = allocate_instance(dev, plat->config, ctrl);
	if (!musb)
		return -ENOMEM;

	spin_lock_init(&musb->lock);
	musb->board_mode = plat->mode;
	musb->board_set_power = plat->set_power;
	musb->set_clock = plat->set_clock;
	musb->min_power = plat->min_power;

	
	if (plat->clock) {
		musb->clock = clk_get(dev, plat->clock);
		if (IS_ERR(musb->clock)) {
			status = PTR_ERR(musb->clock);
			musb->clock = NULL;
			goto fail;
		}
	}

	
	musb->isr = generic_interrupt;
	status = musb_platform_init(musb);

	if (status < 0)
		goto fail;
	if (!musb->isr) {
		status = -ENODEV;
		goto fail2;
	}

#ifndef CONFIG_MUSB_PIO_ONLY
	if (use_dma && dev->dma_mask) {
		struct dma_controller	*c;

		c = dma_controller_create(musb, musb->mregs);
		musb->dma_controller = c;
		if (c)
			(void) c->start(c);
	}
#endif
	
	if (!is_dma_capable() || !musb->dma_controller)
		dev->dma_mask = NULL;

	
	musb_platform_disable(musb);
	musb_generic_disable(musb);

	
	status = musb_core_init(plat->config->multipoint
			? MUSB_CONTROLLER_MHDRC
			: MUSB_CONTROLLER_HDRC, musb);
	if (status < 0)
		goto fail2;

#ifdef CONFIG_USB_MUSB_OTG
	setup_timer(&musb->otg_timer, musb_otg_timer_func, (unsigned long) musb);
#endif

	
	INIT_WORK(&musb->irq_work, musb_irq_work);

	
	if (request_irq(nIrq, musb->isr, 0, dev_name(dev), musb)) {
		dev_err(dev, "request_irq %d failed!\n", nIrq);
		status = -ENODEV;
		goto fail2;
	}
	musb->nIrq = nIrq;

	if (enable_irq_wake(nIrq) == 0) {
		musb->irq_wake = 1;
		device_init_wakeup(dev, 1);
	} else {
		musb->irq_wake = 0;
	}

	pr_info("%s: USB %s mode controller at %p using %s, IRQ %d\n",
			musb_driver_name,
			({char *s;
			switch (musb->board_mode) {
			case MUSB_HOST:		s = "Host"; break;
			case MUSB_PERIPHERAL:	s = "Peripheral"; break;
			default:		s = "OTG"; break;
			}; s; }),
			ctrl,
			(is_dma_capable() && musb->dma_controller)
				? "DMA" : "PIO",
			musb->nIrq);

	
	if (is_host_enabled(musb)) {
		struct usb_hcd	*hcd = musb_to_hcd(musb);

		otg_set_host(musb->xceiv, &hcd->self);

		if (is_otg_enabled(musb))
			hcd->self.otg_port = 1;
		musb->xceiv->host = &hcd->self;
		hcd->power_budget = 2 * (plat->power ? : 250);
	}

	
	if (!is_otg_enabled(musb) && is_host_enabled(musb)) {
		MUSB_HST_MODE(musb);
		musb->xceiv->default_a = 1;
		musb->xceiv->state = OTG_STATE_A_IDLE;

		status = usb_add_hcd(musb_to_hcd(musb), -1, 0);
		if (status)
			goto fail;

		DBG(1, "%s mode, status %d, devctl %02x %c\n",
			"HOST", status,
			musb_readb(musb->mregs, MUSB_DEVCTL),
			(musb_readb(musb->mregs, MUSB_DEVCTL)
					& MUSB_DEVCTL_BDEVICE
				? 'B' : 'A'));

	} else  {
		MUSB_DEV_MODE(musb);
		musb->xceiv->default_a = 0;
		musb->xceiv->state = OTG_STATE_B_IDLE;

		status = musb_gadget_setup(musb);
		if (status)
			goto fail;

		DBG(1, "%s mode, status %d, dev%02x\n",
			is_otg_enabled(musb) ? "OTG" : "PERIPHERAL",
			status,
			musb_readb(musb->mregs, MUSB_DEVCTL));

	}

#ifdef CONFIG_SYSFS
	status = device_create_file(dev, &dev_attr_mode);
	status = device_create_file(dev, &dev_attr_vbus);
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	status = device_create_file(dev, &dev_attr_srp);
#endif 
	status = 0;
#endif
	if (status)
		goto fail2;

	return 0;

fail2:
#ifdef CONFIG_SYSFS
	device_remove_file(musb->controller, &dev_attr_mode);
	device_remove_file(musb->controller, &dev_attr_vbus);
#ifdef CONFIG_USB_GADGET_MUSB_HDRC
	device_remove_file(musb->controller, &dev_attr_srp);
#endif
#endif
	musb_platform_exit(musb);
fail:
	dev_err(musb->controller,
		"musb_init_controller failed with status %d\n", status);

	if (musb->clock)
		clk_put(musb->clock);
	device_init_wakeup(dev, 0);
	musb_free(musb);

	return status;

}





#ifndef CONFIG_MUSB_PIO_ONLY
static u64	*orig_dma_mask;
#endif

static int __init musb_probe(struct platform_device *pdev)
{
	struct device	*dev = &pdev->dev;
	int		irq = platform_get_irq(pdev, 0);
	struct resource	*iomem;
	void __iomem	*base;

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem || irq == 0)
		return -ENODEV;

	base = ioremap(iomem->start, iomem->end - iomem->start + 1);
	if (!base) {
		dev_err(dev, "ioremap failed\n");
		return -ENOMEM;
	}

#ifndef CONFIG_MUSB_PIO_ONLY
	
	orig_dma_mask = dev->dma_mask;
#endif
	return musb_init_controller(dev, irq, base);
}

static int __devexit musb_remove(struct platform_device *pdev)
{
	struct musb	*musb = dev_to_musb(&pdev->dev);
	void __iomem	*ctrl_base = musb->ctrl_base;

	
	musb_shutdown(pdev);
#ifdef CONFIG_USB_MUSB_HDRC_HCD
	if (musb->board_mode == MUSB_HOST)
		usb_remove_hcd(musb_to_hcd(musb));
#endif
	musb_free(musb);
	iounmap(ctrl_base);
	device_init_wakeup(&pdev->dev, 0);
#ifndef CONFIG_MUSB_PIO_ONLY
	pdev->dev.dma_mask = orig_dma_mask;
#endif
	return 0;
}

#ifdef	CONFIG_PM

static int musb_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	unsigned long	flags;
	struct musb	*musb = dev_to_musb(&pdev->dev);

	if (!musb->clock)
		return 0;

	spin_lock_irqsave(&musb->lock, flags);

	if (is_peripheral_active(musb)) {
		
	} else if (is_host_active(musb)) {
		
	}

	if (musb->set_clock)
		musb->set_clock(musb->clock, 0);
	else
		clk_disable(musb->clock);
	spin_unlock_irqrestore(&musb->lock, flags);
	return 0;
}

static int musb_resume_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct musb	*musb = dev_to_musb(&pdev->dev);

	if (!musb->clock)
		return 0;

	if (musb->set_clock)
		musb->set_clock(musb->clock, 1);
	else
		clk_enable(musb->clock);

	
	return 0;
}

static struct dev_pm_ops musb_dev_pm_ops = {
	.suspend	= musb_suspend,
	.resume_noirq	= musb_resume_noirq,
};

#define MUSB_DEV_PM_OPS (&musb_dev_pm_ops)
#else
#define	MUSB_DEV_PM_OPS	NULL
#endif

static struct platform_driver musb_driver = {
	.driver = {
		.name		= (char *)musb_driver_name,
		.bus		= &platform_bus_type,
		.owner		= THIS_MODULE,
		.pm		= MUSB_DEV_PM_OPS,
	},
	.remove		= __devexit_p(musb_remove),
	.shutdown	= musb_shutdown,
};



static int __init musb_init(void)
{
#ifdef CONFIG_USB_MUSB_HDRC_HCD
	if (usb_disabled())
		return 0;
#endif

	pr_info("%s: version " MUSB_VERSION ", "
#ifdef CONFIG_MUSB_PIO_ONLY
		"pio"
#elif defined(CONFIG_USB_TI_CPPI_DMA)
		"cppi-dma"
#elif defined(CONFIG_USB_INVENTRA_DMA)
		"musb-dma"
#elif defined(CONFIG_USB_TUSB_OMAP_DMA)
		"tusb-omap-dma"
#else
		"?dma?"
#endif
		", "
#ifdef CONFIG_USB_MUSB_OTG
		"otg (peripheral+host)"
#elif defined(CONFIG_USB_GADGET_MUSB_HDRC)
		"peripheral"
#elif defined(CONFIG_USB_MUSB_HDRC_HCD)
		"host"
#endif
		", debug=%d\n",
		musb_driver_name, musb_debug);
	return platform_driver_probe(&musb_driver, musb_probe);
}


fs_initcall(musb_init);

static void __exit musb_cleanup(void)
{
	platform_driver_unregister(&musb_driver);
}
module_exit(musb_cleanup);
