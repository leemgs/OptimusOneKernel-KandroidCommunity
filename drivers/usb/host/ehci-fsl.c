

#include <linux/platform_device.h>
#include <linux/fsl_devices.h>

#include "ehci-fsl.h"


#undef CONFIG_PM






int usb_hcd_fsl_probe(const struct hc_driver *driver,
		      struct platform_device *pdev)
{
	struct fsl_usb2_platform_data *pdata;
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int retval;
	unsigned int temp;

	pr_debug("initializing FSL-SOC USB Controller\n");

	
	pdata = (struct fsl_usb2_platform_data *)pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev,
			"No platform data for %s.\n", dev_name(&pdev->dev));
		return -ENODEV;
	}

	
	if (!((pdata->operating_mode == FSL_USB2_DR_HOST) ||
	      (pdata->operating_mode == FSL_USB2_MPH_HOST) ||
	      (pdata->operating_mode == FSL_USB2_DR_OTG))) {
		dev_err(&pdev->dev,
			"Non Host Mode configured for %s. Wrong driver linked.\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}
	irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto err1;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto err2;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto err2;
	}
	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);

	if (hcd->regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto err3;
	}

	
	temp = in_be32(hcd->regs + 0x500);
	out_be32(hcd->regs + 0x500, temp | 0x4);

	
	temp = in_le32(hcd->regs + 0x1a8);
	out_le32(hcd->regs + 0x1a8, temp | 0x3);

	retval = usb_add_hcd(hcd, irq, IRQF_DISABLED | IRQF_SHARED);
	if (retval != 0)
		goto err4;
	return retval;

      err4:
	iounmap(hcd->regs);
      err3:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
      err2:
	usb_put_hcd(hcd);
      err1:
	dev_err(&pdev->dev, "init %s fail, %d\n", dev_name(&pdev->dev), retval);
	return retval;
}





void usb_hcd_fsl_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static void mpc83xx_setup_phy(struct ehci_hcd *ehci,
			      enum fsl_usb2_phy_modes phy_mode,
			      unsigned int port_offset)
{
	u32 portsc = 0;
	switch (phy_mode) {
	case FSL_USB2_PHY_ULPI:
		portsc |= PORT_PTS_ULPI;
		break;
	case FSL_USB2_PHY_SERIAL:
		portsc |= PORT_PTS_SERIAL;
		break;
	case FSL_USB2_PHY_UTMI_WIDE:
		portsc |= PORT_PTS_PTW;
		
	case FSL_USB2_PHY_UTMI:
		portsc |= PORT_PTS_UTMI;
		break;
	case FSL_USB2_PHY_NONE:
		break;
	}
	ehci_writel(ehci, portsc, &ehci->regs->port_status[port_offset]);
}

static void mpc83xx_usb_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	struct fsl_usb2_platform_data *pdata;
	void __iomem *non_ehci = hcd->regs;
	u32 temp;

	pdata =
	    (struct fsl_usb2_platform_data *)hcd->self.controller->
	    platform_data;
	
	temp = in_be32(non_ehci + FSL_SOC_USB_CTRL);
	out_be32(non_ehci + FSL_SOC_USB_CTRL, temp | 0x00000004);
	out_be32(non_ehci + FSL_SOC_USB_SNOOP1, 0x0000001b);

#if defined(CONFIG_PPC32) && !defined(CONFIG_NOT_COHERENT_CACHE)
	

	
	
	out_be32(non_ehci + FSL_SOC_USB_SNOOP1, 0x0 | SNOOP_SIZE_2GB);
	
	out_be32(non_ehci + FSL_SOC_USB_SNOOP2, 0x80000000 | SNOOP_SIZE_2GB);
#endif

	if ((pdata->operating_mode == FSL_USB2_DR_HOST) ||
			(pdata->operating_mode == FSL_USB2_DR_OTG))
		mpc83xx_setup_phy(ehci, pdata->phy_mode, 0);

	if (pdata->operating_mode == FSL_USB2_MPH_HOST) {
		unsigned int chip, rev, svr;

		svr = mfspr(SPRN_SVR);
		chip = svr >> 16;
		rev = (svr >> 4) & 0xf;

		
		if ((rev == 1) && (chip >= 0x8050) && (chip <= 0x8055))
			ehci->has_fsl_port_bug = 1;

		if (pdata->port_enables & FSL_USB2_PORT0_ENABLED)
			mpc83xx_setup_phy(ehci, pdata->phy_mode, 0);
		if (pdata->port_enables & FSL_USB2_PORT1_ENABLED)
			mpc83xx_setup_phy(ehci, pdata->phy_mode, 1);
	}

	
	ehci_writel(ehci, 0x00000003, non_ehci + FSL_SOC_USB_USBMODE);
#ifdef CONFIG_PPC_85xx
	out_be32(non_ehci + FSL_SOC_USB_PRICTRL, 0x00000008);
	out_be32(non_ehci + FSL_SOC_USB_AGECNTTHRSH, 0x00000080);
#else
	out_be32(non_ehci + FSL_SOC_USB_PRICTRL, 0x0000000c);
	out_be32(non_ehci + FSL_SOC_USB_AGECNTTHRSH, 0x00000040);
#endif
	out_be32(non_ehci + FSL_SOC_USB_SICTRL, 0x00000001);
}


static int ehci_fsl_reinit(struct ehci_hcd *ehci)
{
	mpc83xx_usb_setup(ehci_to_hcd(ehci));
	ehci_port_power(ehci, 0);

	return 0;
}


static int ehci_fsl_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval;

	
	ehci->caps = hcd->regs + 0x100;
	ehci->regs = hcd->regs + 0x100 +
	    HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	dbg_hcs_params(ehci, "reset");
	dbg_hcc_params(ehci, "reset");

	
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	hcd->has_tt = 1;

	ehci->sbrn = 0x20;

	ehci_reset(ehci);

	retval = ehci_fsl_reinit(ehci);
	return retval;
}

static const struct hc_driver ehci_fsl_hc_driver = {
	.description = hcd_name,
	.product_desc = "Freescale On-Chip EHCI Host Controller",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	
	.irq = ehci_irq,
	.flags = HCD_USB2,

	
	.reset = ehci_fsl_setup,
	.start = ehci_run,
	.stop = ehci_stop,
	.shutdown = ehci_shutdown,

	
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,
	.endpoint_reset = ehci_endpoint_reset,

	
	.get_frame_number = ehci_get_frame,

	
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
	.relinquish_port = ehci_relinquish_port,
	.port_handed_over = ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

static int ehci_fsl_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	
	return usb_hcd_fsl_probe(&ehci_fsl_hc_driver, pdev);
}

static int ehci_fsl_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	
	usb_hcd_fsl_remove(hcd, pdev);
	return 0;
}

MODULE_ALIAS("platform:fsl-ehci");

static struct platform_driver ehci_fsl_driver = {
	.probe = ehci_fsl_drv_probe,
	.remove = ehci_fsl_drv_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver = {
		   .name = "fsl-ehci",
	},
};
