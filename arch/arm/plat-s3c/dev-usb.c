

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/map.h>

#include <plat/devs.h>


static struct resource s3c_usb_resource[] = {
	[0] = {
		.start = S3C_PA_USBHOST,
		.end   = S3C_PA_USBHOST + 0x100 - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_USBH,
		.end   = IRQ_USBH,
		.flags = IORESOURCE_IRQ,
	}
};

static u64 s3c_device_usb_dmamask = 0xffffffffUL;

struct platform_device s3c_device_usb = {
	.name		  = "s3c2410-ohci",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_usb_resource),
	.resource	  = s3c_usb_resource,
	.dev              = {
		.dma_mask = &s3c_device_usb_dmamask,
		.coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(s3c_device_usb);
