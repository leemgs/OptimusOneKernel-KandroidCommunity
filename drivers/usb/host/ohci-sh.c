

#include <linux/platform_device.h>

static int ohci_sh_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);

	ohci_hcd_init(ohci);
	ohci_init(ohci);
	ohci_run(ohci);
	hcd->state = HC_STATE_RUNNING;
	return 0;
}

static const struct hc_driver ohci_sh_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"SuperH OHCI",
	.hcd_priv_size =	sizeof(struct ohci_hcd),

	
	.irq =			ohci_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	
	.start =		ohci_sh_start,
	.stop =			ohci_stop,
	.shutdown =		ohci_shutdown,

	
	.urb_enqueue =		ohci_urb_enqueue,
	.urb_dequeue =		ohci_urb_dequeue,
	.endpoint_disable =	ohci_endpoint_disable,

	
	.get_frame_number =	ohci_get_frame,

	
	.hub_status_data =	ohci_hub_status_data,
	.hub_control =		ohci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend =		ohci_bus_suspend,
	.bus_resume =		ohci_bus_resume,
#endif
	.start_port_reset =	ohci_start_port_reset,
};



#define resource_len(r) (((r)->end - (r)->start) + 1)
static int ohci_hcd_sh_probe(struct platform_device *pdev)
{
	struct resource *res = NULL;
	struct usb_hcd *hcd = NULL;
	int irq = -1;
	int ret;

	if (usb_disabled())
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err("platform_get_resource error.");
		return -ENODEV;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err("platform_get_irq error.");
		return -ENODEV;
	}

	
	hcd = usb_create_hcd(&ohci_sh_hc_driver, &pdev->dev, (char *)hcd_name);
	if (!hcd) {
		err("Failed to create hcd");
		return -ENOMEM;
	}

	hcd->regs = (void __iomem *)res->start;
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_len(res);
	ret = usb_add_hcd(hcd, irq, IRQF_DISABLED);
	if (ret != 0) {
		err("Failed to add hcd");
		usb_put_hcd(hcd);
		return ret;
	}

	return ret;
}

static int ohci_hcd_sh_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);

	return 0;
}

static struct platform_driver ohci_hcd_sh_driver = {
	.probe		= ohci_hcd_sh_probe,
	.remove		= ohci_hcd_sh_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver		= {
		.name	= "sh_ohci",
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:sh_ohci");
