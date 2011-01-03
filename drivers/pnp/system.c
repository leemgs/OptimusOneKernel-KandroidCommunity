

#include <linux/pnp.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/ioport.h>

static const struct pnp_device_id pnp_dev_table[] = {
	
	{"PNP0c02", 0},
	
	{"PNP0c01", 0},
	{"", 0}
};

static void reserve_range(struct pnp_dev *dev, resource_size_t start,
			  resource_size_t end, int port)
{
	char *regionid;
	const char *pnpid = dev_name(&dev->dev);
	struct resource *res;

	regionid = kmalloc(16, GFP_KERNEL);
	if (!regionid)
		return;

	snprintf(regionid, 16, "pnp %s", pnpid);
	if (port)
		res = request_region(start, end - start + 1, regionid);
	else
		res = request_mem_region(start, end - start + 1, regionid);
	if (res)
		res->flags &= ~IORESOURCE_BUSY;
	else
		kfree(regionid);

	
	dev_info(&dev->dev, "%s range 0x%llx-0x%llx %s reserved\n",
		port ? "ioport" : "iomem",
		(unsigned long long) start, (unsigned long long) end,
		res ? "has been" : "could not be");
}

static void reserve_resources_of_dev(struct pnp_dev *dev)
{
	struct resource *res;
	int i;

	for (i = 0; (res = pnp_get_resource(dev, IORESOURCE_IO, i)); i++) {
		if (res->flags & IORESOURCE_DISABLED)
			continue;
		if (res->start == 0)
			continue;	
		if (res->start < 0x100)
			
			continue;
		if (res->end < res->start)
			continue;	

		reserve_range(dev, res->start, res->end, 1);
	}

	for (i = 0; (res = pnp_get_resource(dev, IORESOURCE_MEM, i)); i++) {
		if (res->flags & IORESOURCE_DISABLED)
			continue;

		reserve_range(dev, res->start, res->end, 0);
	}
}

static int system_pnp_probe(struct pnp_dev *dev,
			    const struct pnp_device_id *dev_id)
{
	reserve_resources_of_dev(dev);
	return 0;
}

static struct pnp_driver system_pnp_driver = {
	.name     = "system",
	.id_table = pnp_dev_table,
	.flags    = PNP_DRIVER_RES_DO_NOT_CHANGE,
	.probe    = system_pnp_probe,
};

static int __init pnp_system_init(void)
{
	return pnp_register_driver(&system_pnp_driver);
}


fs_initcall(pnp_system_init);
