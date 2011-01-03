

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/map.h>
#include <plat/devs.h>

static struct resource s3c_nand_resource[] = {
	[0] = {
		.start = S3C_PA_NAND,
		.end   = S3C_PA_NAND + SZ_1M,
		.flags = IORESOURCE_MEM,
	}
};

struct platform_device s3c_device_nand = {
	.name		  = "s3c2410-nand",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_nand_resource),
	.resource	  = s3c_nand_resource,
};

EXPORT_SYMBOL(s3c_device_nand);
