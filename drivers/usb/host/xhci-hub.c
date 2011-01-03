

#include <asm/unaligned.h>

#include "xhci.h"

static void xhci_hub_descriptor(struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{
	int ports;
	u16 temp;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	
	desc->bDescriptorType = 0x29;
	desc->bPwrOn2PwrGood = 10;	
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = ports;
	temp = 1 + (ports / 8);
	desc->bDescLength = 7 + 2 * temp;

	
	memset(&desc->DeviceRemovable[0], 0, temp);
	memset(&desc->DeviceRemovable[temp], 0xff, temp);

	
	
	temp = 0;
	
	if (HCC_PPC(xhci->hcc_params))
		temp |= 0x0001;
	else
		temp |= 0x0002;
	
	
	temp |= 0x0008;
	
	
	desc->wHubCharacteristics = (__force __u16) cpu_to_le16(temp);
}

static unsigned int xhci_port_speed(unsigned int port_status)
{
	if (DEV_LOWSPEED(port_status))
		return 1 << USB_PORT_FEAT_LOWSPEED;
	if (DEV_HIGHSPEED(port_status))
		return 1 << USB_PORT_FEAT_HIGHSPEED;
	if (DEV_SUPERSPEED(port_status))
		return 1 << USB_PORT_FEAT_SUPERSPEED;
	
	return 0;
}


#define	XHCI_PORT_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))

#define XHCI_PORT_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))

#define	XHCI_PORT_RW1S	((1<<4))

#define XHCI_PORT_RW1CS	((1<<1) | (0x7f<<17))

#define	XHCI_PORT_RW	((1<<16))

#define	XHCI_PORT_RZ	((1<<2) | (1<<24) | (0xf<<28))


static u32 xhci_port_state_to_neutral(u32 state)
{
	
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

int xhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buf, u16 wLength)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	unsigned long flags;
	u32 temp, status;
	int retval = 0;
	u32 __iomem *addr;
	char *port_change_bit;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	spin_lock_irqsave(&xhci->lock, flags);
	switch (typeReq) {
	case GetHubStatus:
		
		memset(buf, 0, 4);
		break;
	case GetHubDescriptor:
		xhci_hub_descriptor(xhci, (struct usb_hub_descriptor *) buf);
		break;
	case GetPortStatus:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		status = 0;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		xhci_dbg(xhci, "get port status, actual port %d status  = 0x%x\n", wIndex, temp);

		
		if (temp & PORT_CSC)
			status |= 1 << USB_PORT_FEAT_C_CONNECTION;
		if (temp & PORT_PEC)
			status |= 1 << USB_PORT_FEAT_C_ENABLE;
		if ((temp & PORT_OCC))
			status |= 1 << USB_PORT_FEAT_C_OVER_CURRENT;
		
		if (temp & PORT_CONNECT) {
			status |= 1 << USB_PORT_FEAT_CONNECTION;
			status |= xhci_port_speed(temp);
		}
		if (temp & PORT_PE)
			status |= 1 << USB_PORT_FEAT_ENABLE;
		if (temp & PORT_OC)
			status |= 1 << USB_PORT_FEAT_OVER_CURRENT;
		if (temp & PORT_RESET)
			status |= 1 << USB_PORT_FEAT_RESET;
		if (temp & PORT_POWER)
			status |= 1 << USB_PORT_FEAT_POWER;
		xhci_dbg(xhci, "Get port status returned 0x%x\n", status);
		put_unaligned(cpu_to_le32(status), (__le32 *) buf);
		break;
	case SetPortFeature:
		wIndex &= 0xff;
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_POWER:
			
			xhci_writel(xhci, temp | PORT_POWER, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port power, actual port %d status  = 0x%x\n", wIndex, temp);
			break;
		case USB_PORT_FEAT_RESET:
			temp = (temp | PORT_RESET);
			xhci_writel(xhci, temp, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port reset, actual port %d status  = 0x%x\n", wIndex, temp);
			break;
		default:
			goto error;
		}
		temp = xhci_readl(xhci, addr); 
		break;
	case ClearPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_C_RESET:
			status = PORT_RC;
			port_change_bit = "reset";
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			status = PORT_CSC;
			port_change_bit = "connect";
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
			status = PORT_OCC;
			port_change_bit = "over-current";
			break;
		default:
			goto error;
		}
		
		xhci_writel(xhci, temp | status, addr);
		temp = xhci_readl(xhci, addr);
		xhci_dbg(xhci, "clear port %s change, actual port %d status  = 0x%x\n",
				port_change_bit, wIndex, temp);
		temp = xhci_readl(xhci, addr); 
		break;
	default:
error:
		
		retval = -EPIPE;
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return retval;
}


int xhci_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	unsigned long flags;
	u32 temp, status;
	int i, retval;
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	u32 __iomem *addr;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	
	buf[0] = 0;
	status = 0;
	if (ports > 7) {
		buf[1] = 0;
		retval = 2;
	} else {
		retval = 1;
	}

	spin_lock_irqsave(&xhci->lock, flags);
	
	for (i = 0; i < ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*i;
		temp = xhci_readl(xhci, addr);
		if (temp & (PORT_CSC | PORT_PEC | PORT_OCC)) {
			if (i < 7)
				buf[0] |= 1 << (i + 1);
			else
				buf[1] |= 1 << (i - 7);
			status = 1;
		}
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return status ? retval : 0;
}
