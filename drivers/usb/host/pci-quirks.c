

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include "pci-quirks.h"
#include "xhci-ext-caps.h"


#define UHCI_USBLEGSUP		0xc0		
#define UHCI_USBCMD		0		
#define UHCI_USBINTR		4		
#define UHCI_USBLEGSUP_RWC	0x8f00		
#define UHCI_USBLEGSUP_RO	0x5040		
#define UHCI_USBCMD_RUN		0x0001		
#define UHCI_USBCMD_HCRESET	0x0002		
#define UHCI_USBCMD_EGSM	0x0008		
#define UHCI_USBCMD_CONFIGURE	0x0040		
#define UHCI_USBINTR_RESUME	0x0002		

#define OHCI_CONTROL		0x04
#define OHCI_CMDSTATUS		0x08
#define OHCI_INTRSTATUS		0x0c
#define OHCI_INTRENABLE		0x10
#define OHCI_INTRDISABLE	0x14
#define OHCI_OCR		(1 << 3)	
#define OHCI_CTRL_RWC		(1 << 9)	
#define OHCI_CTRL_IR		(1 << 8)	
#define OHCI_INTR_OC		(1 << 30)	

#define EHCI_HCC_PARAMS		0x08		
#define EHCI_USBCMD		0		
#define EHCI_USBCMD_RUN		(1 << 0)	
#define EHCI_USBSTS		4		
#define EHCI_USBSTS_HALTED	(1 << 12)	
#define EHCI_USBINTR		8		
#define EHCI_CONFIGFLAG		0x40		
#define EHCI_USBLEGSUP		0		
#define EHCI_USBLEGSUP_BIOS	(1 << 16)	
#define EHCI_USBLEGSUP_OS	(1 << 24)	
#define EHCI_USBLEGCTLSTS	4		
#define EHCI_USBLEGCTLSTS_SOOE	(1 << 13)	



void uhci_reset_hc(struct pci_dev *pdev, unsigned long base)
{
	
	pci_write_config_word(pdev, UHCI_USBLEGSUP, UHCI_USBLEGSUP_RWC);

	
	outw(UHCI_USBCMD_HCRESET, base + UHCI_USBCMD);
	mb();
	udelay(5);
	if (inw(base + UHCI_USBCMD) & UHCI_USBCMD_HCRESET)
		dev_warn(&pdev->dev, "HCRESET not completed yet!\n");

	
	outw(0, base + UHCI_USBINTR);
	outw(0, base + UHCI_USBCMD);
}
EXPORT_SYMBOL_GPL(uhci_reset_hc);


int uhci_check_and_reset_hc(struct pci_dev *pdev, unsigned long base)
{
	u16 legsup;
	unsigned int cmd, intr;

	
	pci_read_config_word(pdev, UHCI_USBLEGSUP, &legsup);
	if (legsup & ~(UHCI_USBLEGSUP_RO | UHCI_USBLEGSUP_RWC)) {
		dev_dbg(&pdev->dev, "%s: legsup = 0x%04x\n",
				__func__, legsup);
		goto reset_needed;
	}

	cmd = inw(base + UHCI_USBCMD);
	if ((cmd & UHCI_USBCMD_RUN) || !(cmd & UHCI_USBCMD_CONFIGURE) ||
			!(cmd & UHCI_USBCMD_EGSM)) {
		dev_dbg(&pdev->dev, "%s: cmd = 0x%04x\n",
				__func__, cmd);
		goto reset_needed;
	}

	intr = inw(base + UHCI_USBINTR);
	if (intr & (~UHCI_USBINTR_RESUME)) {
		dev_dbg(&pdev->dev, "%s: intr = 0x%04x\n",
				__func__, intr);
		goto reset_needed;
	}
	return 0;

reset_needed:
	dev_dbg(&pdev->dev, "Performing full reset\n");
	uhci_reset_hc(pdev, base);
	return 1;
}
EXPORT_SYMBOL_GPL(uhci_check_and_reset_hc);

static inline int io_type_enabled(struct pci_dev *pdev, unsigned int mask)
{
	u16 cmd;
	return !pci_read_config_word(pdev, PCI_COMMAND, &cmd) && (cmd & mask);
}

#define pio_enabled(dev) io_type_enabled(dev, PCI_COMMAND_IO)
#define mmio_enabled(dev) io_type_enabled(dev, PCI_COMMAND_MEMORY)

static void __devinit quirk_usb_handoff_uhci(struct pci_dev *pdev)
{
	unsigned long base = 0;
	int i;

	if (!pio_enabled(pdev))
		return;

	for (i = 0; i < PCI_ROM_RESOURCE; i++)
		if ((pci_resource_flags(pdev, i) & IORESOURCE_IO)) {
			base = pci_resource_start(pdev, i);
			break;
		}

	if (base)
		uhci_check_and_reset_hc(pdev, base);
}

static int __devinit mmio_resource_enabled(struct pci_dev *pdev, int idx)
{
	return pci_resource_start(pdev, idx) && mmio_enabled(pdev);
}

static void __devinit quirk_usb_handoff_ohci(struct pci_dev *pdev)
{
	void __iomem *base;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = pci_ioremap_bar(pdev, 0);
	if (base == NULL)
		return;


#ifndef __hppa__
{
	u32 control = readl(base + OHCI_CONTROL);
	if (control & OHCI_CTRL_IR) {
		int wait_time = 500; 
		writel(OHCI_INTR_OC, base + OHCI_INTRENABLE);
		writel(OHCI_OCR, base + OHCI_CMDSTATUS);
		while (wait_time > 0 &&
				readl(base + OHCI_CONTROL) & OHCI_CTRL_IR) {
			wait_time -= 10;
			msleep(10);
		}
		if (wait_time <= 0)
			dev_warn(&pdev->dev, "OHCI: BIOS handoff failed"
					" (BIOS bug?) %08x\n",
					readl(base + OHCI_CONTROL));

		
		writel(control & OHCI_CTRL_RWC, base + OHCI_CONTROL);
	}
}
#endif

	
	writel(~(u32)0, base + OHCI_INTRDISABLE);
	writel(~(u32)0, base + OHCI_INTRSTATUS);

	iounmap(base);
}

static void __devinit quirk_usb_disable_ehci(struct pci_dev *pdev)
{
	int wait_time, delta;
	void __iomem *base, *op_reg_base;
	u32	hcc_params, val;
	u8	offset, cap_length;
	int	count = 256/4;
	int	tried_handoff = 0;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = pci_ioremap_bar(pdev, 0);
	if (base == NULL)
		return;

	cap_length = readb(base);
	op_reg_base = base + cap_length;

	
	hcc_params = readl(base + EHCI_HCC_PARAMS);
	offset = (hcc_params >> 8) & 0xff;
	while (offset && --count) {
		u32		cap;
		int		msec;

		pci_read_config_dword(pdev, offset, &cap);
		switch (cap & 0xff) {
		case 1:			
			if ((cap & EHCI_USBLEGSUP_BIOS)) {
				dev_dbg(&pdev->dev, "EHCI: BIOS handoff\n");

#if 0

				
				pci_read_config_dword(pdev,
						offset + EHCI_USBLEGCTLSTS,
						&val);
				pci_write_config_dword(pdev,
						offset + EHCI_USBLEGCTLSTS,
						val | EHCI_USBLEGCTLSTS_SOOE);
#endif

				
				pci_write_config_byte(pdev, offset + 3, 1);
			}

			
			msec = 1000;
			while ((cap & EHCI_USBLEGSUP_BIOS) && (msec > 0)) {
				tried_handoff = 1;
				msleep(10);
				msec -= 10;
				pci_read_config_dword(pdev, offset, &cap);
			}

			if (cap & EHCI_USBLEGSUP_BIOS) {
				
				dev_warn(&pdev->dev, "EHCI: BIOS handoff failed"
						" (BIOS bug?) %08x\n", cap);
				pci_write_config_byte(pdev, offset + 2, 0);
			}

			
			pci_write_config_dword(pdev,
					offset + EHCI_USBLEGCTLSTS,
					0);

			
			if (tried_handoff)
				writel(0, op_reg_base + EHCI_CONFIGFLAG);
			break;
		case 0:			
			cap = 0;
			
		default:
			dev_warn(&pdev->dev, "EHCI: unrecognized capability "
					"%02x\n", cap & 0xff);
			break;
		}
		offset = (cap >> 8) & 0xff;
	}
	if (!count)
		dev_printk(KERN_DEBUG, &pdev->dev, "EHCI: capability loop?\n");

	
	val = readl(op_reg_base + EHCI_USBSTS);
	if ((val & EHCI_USBSTS_HALTED) == 0) {
		val = readl(op_reg_base + EHCI_USBCMD);
		val &= ~EHCI_USBCMD_RUN;
		writel(val, op_reg_base + EHCI_USBCMD);

		wait_time = 2000;
		delta = 100;
		do {
			writel(0x3f, op_reg_base + EHCI_USBSTS);
			udelay(delta);
			wait_time -= delta;
			val = readl(op_reg_base + EHCI_USBSTS);
			if ((val == ~(u32)0) || (val & EHCI_USBSTS_HALTED)) {
				break;
			}
		} while (wait_time > 0);
	}
	writel(0, op_reg_base + EHCI_USBINTR);
	writel(0x3f, op_reg_base + EHCI_USBSTS);

	iounmap(base);

	return;
}


static int handshake(void __iomem *ptr, u32 mask, u32 done,
		int wait_usec, int delay_usec)
{
	u32	result;

	do {
		result = readl(ptr);
		result &= mask;
		if (result == done)
			return 0;
		udelay(delay_usec);
		wait_usec -= delay_usec;
	} while (wait_usec > 0);
	return -ETIMEDOUT;
}


static void __devinit quirk_usb_handoff_xhci(struct pci_dev *pdev)
{
	void __iomem *base;
	int ext_cap_offset;
	void __iomem *op_reg_base;
	u32 val;
	int timeout;

	if (!mmio_resource_enabled(pdev, 0))
		return;

	base = ioremap_nocache(pci_resource_start(pdev, 0),
				pci_resource_len(pdev, 0));
	if (base == NULL)
		return;

	
	ext_cap_offset = xhci_find_next_cap_offset(base, XHCI_HCC_PARAMS_OFFSET);
	do {
		if (!ext_cap_offset)
			
			goto hc_init;
		val = readl(base + ext_cap_offset);
		if (XHCI_EXT_CAPS_ID(val) == XHCI_EXT_CAPS_LEGACY)
			break;
		ext_cap_offset = xhci_find_next_cap_offset(base, ext_cap_offset);
	} while (1);

	
	if (val & XHCI_HC_BIOS_OWNED) {
		writel(val & XHCI_HC_OS_OWNED, base + ext_cap_offset);

		
		timeout = handshake(base + ext_cap_offset, XHCI_HC_BIOS_OWNED,
				0, 5000, 10);

		
		if (timeout) {
			dev_warn(&pdev->dev, "xHCI BIOS handoff failed"
					" (BIOS bug ?) %08x\n", val);
			writel(val & ~XHCI_HC_BIOS_OWNED, base + ext_cap_offset);
		}
	}

	
	writel(XHCI_LEGACY_DISABLE_SMI,
			base + ext_cap_offset + XHCI_LEGACY_CONTROL_OFFSET);

hc_init:
	op_reg_base = base + XHCI_HC_LENGTH(readl(base));

	
	timeout = handshake(op_reg_base + XHCI_STS_OFFSET, XHCI_STS_CNR, 0,
			5000, 10);
	
	if (timeout) {
		val = readl(op_reg_base + XHCI_STS_OFFSET);
		dev_warn(&pdev->dev,
				"xHCI HW not ready after 5 sec (HC bug?) "
				"status = 0x%x\n", val);
	}

	
	val = readl(op_reg_base + XHCI_CMD_OFFSET);
	val &= ~(XHCI_CMD_RUN | XHCI_IRQS);
	writel(val, op_reg_base + XHCI_CMD_OFFSET);

	
	timeout = handshake(op_reg_base + XHCI_STS_OFFSET, XHCI_STS_HALT, 1,
			XHCI_MAX_HALT_USEC, 125);
	if (timeout) {
		val = readl(op_reg_base + XHCI_STS_OFFSET);
		dev_warn(&pdev->dev,
				"xHCI HW did not halt within %d usec "
				"status = 0x%x\n", XHCI_MAX_HALT_USEC, val);
	}

	iounmap(base);
}

static void __devinit quirk_usb_early_handoff(struct pci_dev *pdev)
{
	if (pdev->class == PCI_CLASS_SERIAL_USB_UHCI)
		quirk_usb_handoff_uhci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_OHCI)
		quirk_usb_handoff_ohci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_EHCI)
		quirk_usb_disable_ehci(pdev);
	else if (pdev->class == PCI_CLASS_SERIAL_USB_XHCI)
		quirk_usb_handoff_xhci(pdev);
}
DECLARE_PCI_FIXUP_FINAL(PCI_ANY_ID, PCI_ANY_ID, quirk_usb_early_handoff);
