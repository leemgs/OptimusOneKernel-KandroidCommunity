

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/ucc.h>

#include "gianfar.h"
#include "fsl_pq_mdio.h"


int fsl_pq_local_mdio_write(struct fsl_pq_mdio __iomem *regs, int mii_id,
		int regnum, u16 value)
{
	
	out_be32(&regs->miimadd, (mii_id << 8) | regnum);

	
	out_be32(&regs->miimcon, value);

	
	while (in_be32(&regs->miimind) & MIIMIND_BUSY)
		cpu_relax();

	return 0;
}


int fsl_pq_local_mdio_read(struct fsl_pq_mdio __iomem *regs,
		int mii_id, int regnum)
{
	u16 value;

	
	out_be32(&regs->miimadd, (mii_id << 8) | regnum);

	
	out_be32(&regs->miimcom, 0);
	out_be32(&regs->miimcom, MII_READ_COMMAND);

	
	while (in_be32(&regs->miimind) & (MIIMIND_NOTVALID | MIIMIND_BUSY))
		cpu_relax();

	
	value = in_be32(&regs->miimstat);

	return value;
}


int fsl_pq_mdio_write(struct mii_bus *bus, int mii_id, int regnum, u16 value)
{
	struct fsl_pq_mdio __iomem *regs = (void __iomem *)bus->priv;

	
	return(fsl_pq_local_mdio_write(regs, mii_id, regnum, value));
}


int fsl_pq_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	struct fsl_pq_mdio __iomem *regs = (void __iomem *)bus->priv;

	
	return(fsl_pq_local_mdio_read(regs, mii_id, regnum));
}


static int fsl_pq_mdio_reset(struct mii_bus *bus)
{
	struct fsl_pq_mdio __iomem *regs = (void __iomem *)bus->priv;
	int timeout = PHY_INIT_TIMEOUT;

	mutex_lock(&bus->mdio_lock);

	
	out_be32(&regs->miimcfg, MIIMCFG_RESET);

	
	out_be32(&regs->miimcfg, MIIMCFG_INIT_VALUE);

	
	while ((in_be32(&regs->miimind) & MIIMIND_BUSY) && timeout--)
		cpu_relax();

	mutex_unlock(&bus->mdio_lock);

	if (timeout < 0) {
		printk(KERN_ERR "%s: The MII Bus is stuck!\n",
				bus->name);
		return -EBUSY;
	}

	return 0;
}

void fsl_pq_mdio_bus_name(char *name, struct device_node *np)
{
	const u32 *addr;
	u64 taddr = OF_BAD_ADDR;

	addr = of_get_address(np, 0, NULL, NULL);
	if (addr)
		taddr = of_translate_address(np, addr);

	snprintf(name, MII_BUS_ID_SIZE, "%s@%llx", np->name,
		(unsigned long long)taddr);
}
EXPORT_SYMBOL_GPL(fsl_pq_mdio_bus_name);


static int fsl_pq_mdio_find_free(struct mii_bus *new_bus)
{
	int i;

	for (i = PHY_MAX_ADDR; i > 0; i--) {
		u32 phy_id;

		if (get_phy_id(new_bus, i, &phy_id))
			return -1;

		if (phy_id == 0xffffffff)
			break;
	}

	return i;
}


#if defined(CONFIG_GIANFAR) || defined(CONFIG_GIANFAR_MODULE)
static u32 __iomem *get_gfar_tbipa(struct fsl_pq_mdio __iomem *regs)
{
	struct gfar __iomem *enet_regs;

	
	enet_regs = (struct gfar __iomem *)
		((char __iomem *)regs - offsetof(struct gfar, gfar_mii_regs));

	return &enet_regs->tbipa;
}
#endif


#if defined(CONFIG_UCC_GETH) || defined(CONFIG_UCC_GETH_MODULE)
static int get_ucc_id_for_range(u64 start, u64 end, u32 *ucc_id)
{
	struct device_node *np = NULL;
	int err = 0;

	for_each_compatible_node(np, NULL, "ucc_geth") {
		struct resource tempres;

		err = of_address_to_resource(np, 0, &tempres);
		if (err)
			continue;

		
		if ((start >= tempres.start) && (end <= tempres.end)) {
			
			const u32 *id;

			id = of_get_property(np, "cell-index", NULL);
			if (!id) {
				id = of_get_property(np, "device-id", NULL);
				if (!id)
					continue;
			}

			*ucc_id = *id;

			return 0;
		}
	}

	if (err)
		return err;
	else
		return -EINVAL;
}
#endif


static int fsl_pq_mdio_probe(struct of_device *ofdev,
		const struct of_device_id *match)
{
	struct device_node *np = ofdev->node;
	struct device_node *tbi;
	struct fsl_pq_mdio __iomem *regs;
	u32 __iomem *tbipa;
	struct mii_bus *new_bus;
	int tbiaddr = -1;
	u64 addr, size;
	int err = 0;

	new_bus = mdiobus_alloc();
	if (NULL == new_bus)
		return -ENOMEM;

	new_bus->name = "Freescale PowerQUICC MII Bus",
	new_bus->read = &fsl_pq_mdio_read,
	new_bus->write = &fsl_pq_mdio_write,
	new_bus->reset = &fsl_pq_mdio_reset,
	fsl_pq_mdio_bus_name(new_bus->id, np);

	
	addr = of_translate_address(np, of_get_address(np, 0, &size, NULL));
	regs = ioremap(addr, size);

	if (NULL == regs) {
		err = -ENOMEM;
		goto err_free_bus;
	}

	new_bus->priv = (void __force *)regs;

	new_bus->irq = kcalloc(PHY_MAX_ADDR, sizeof(int), GFP_KERNEL);

	if (NULL == new_bus->irq) {
		err = -ENOMEM;
		goto err_unmap_regs;
	}

	new_bus->parent = &ofdev->dev;
	dev_set_drvdata(&ofdev->dev, new_bus);

	if (of_device_is_compatible(np, "fsl,gianfar-mdio") ||
			of_device_is_compatible(np, "fsl,gianfar-tbi") ||
			of_device_is_compatible(np, "gianfar")) {
#if defined(CONFIG_GIANFAR) || defined(CONFIG_GIANFAR_MODULE)
		tbipa = get_gfar_tbipa(regs);
#else
		err = -ENODEV;
		goto err_free_irqs;
#endif
	} else if (of_device_is_compatible(np, "fsl,ucc-mdio") ||
			of_device_is_compatible(np, "ucc_geth_phy")) {
#if defined(CONFIG_UCC_GETH) || defined(CONFIG_UCC_GETH_MODULE)
		u32 id;
		static u32 mii_mng_master;

		tbipa = &regs->utbipar;

		if ((err = get_ucc_id_for_range(addr, addr + size, &id)))
			goto err_free_irqs;

		if (!mii_mng_master) {
			mii_mng_master = id;
			ucc_set_qe_mux_mii_mng(id - 1);
		}
#else
		err = -ENODEV;
		goto err_free_irqs;
#endif
	} else {
		err = -ENODEV;
		goto err_free_irqs;
	}

	for_each_child_of_node(np, tbi) {
		if (!strncmp(tbi->type, "tbi-phy", 8))
			break;
	}

	if (tbi) {
		const u32 *prop = of_get_property(tbi, "reg", NULL);

		if (prop)
			tbiaddr = *prop;
	}

	if (tbiaddr == -1) {
		out_be32(tbipa, 0);

		tbiaddr = fsl_pq_mdio_find_free(new_bus);
	}

	
	if (tbiaddr == 0) {
		err = -EBUSY;

		goto err_free_irqs;
	}

	out_be32(tbipa, tbiaddr);

	err = of_mdiobus_register(new_bus, np);
	if (err) {
		printk (KERN_ERR "%s: Cannot register as MDIO bus\n",
				new_bus->name);
		goto err_free_irqs;
	}

	return 0;

err_free_irqs:
	kfree(new_bus->irq);
err_unmap_regs:
	iounmap(regs);
err_free_bus:
	kfree(new_bus);

	return err;
}


static int fsl_pq_mdio_remove(struct of_device *ofdev)
{
	struct device *device = &ofdev->dev;
	struct mii_bus *bus = dev_get_drvdata(device);

	mdiobus_unregister(bus);

	dev_set_drvdata(device, NULL);

	iounmap((void __iomem *)bus->priv);
	bus->priv = NULL;
	mdiobus_free(bus);

	return 0;
}

static struct of_device_id fsl_pq_mdio_match[] = {
	{
		.type = "mdio",
		.compatible = "ucc_geth_phy",
	},
	{
		.type = "mdio",
		.compatible = "gianfar",
	},
	{
		.compatible = "fsl,ucc-mdio",
	},
	{
		.compatible = "fsl,gianfar-tbi",
	},
	{
		.compatible = "fsl,gianfar-mdio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, fsl_pq_mdio_match);

static struct of_platform_driver fsl_pq_mdio_driver = {
	.name = "fsl-pq_mdio",
	.probe = fsl_pq_mdio_probe,
	.remove = fsl_pq_mdio_remove,
	.match_table = fsl_pq_mdio_match,
};

int __init fsl_pq_mdio_init(void)
{
	return of_register_platform_driver(&fsl_pq_mdio_driver);
}
module_init(fsl_pq_mdio_init);

void fsl_pq_mdio_exit(void)
{
	of_unregister_platform_driver(&fsl_pq_mdio_driver);
}
module_exit(fsl_pq_mdio_exit);
MODULE_LICENSE("GPL");
