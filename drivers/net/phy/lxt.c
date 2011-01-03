
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
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>



#define MII_LXT970_IER       17  

#define MII_LXT970_IER_IEN	0x0002

#define MII_LXT970_ISR       18  

#define MII_LXT970_CONFIG    19  





#define MII_LXT971_IER		18  
#define MII_LXT971_IER_IEN	0x00f2

#define MII_LXT971_ISR		19  


MODULE_DESCRIPTION("Intel LXT PHY driver");
MODULE_AUTHOR("Andy Fleming");
MODULE_LICENSE("GPL");

static int lxt970_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, MII_BMSR);

	if (err < 0)
		return err;

	err = phy_read(phydev, MII_LXT970_ISR);

	if (err < 0)
		return err;

	return 0;
}

static int lxt970_config_intr(struct phy_device *phydev)
{
	int err;

	if(phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, MII_LXT970_IER, MII_LXT970_IER_IEN);
	else
		err = phy_write(phydev, MII_LXT970_IER, 0);

	return err;
}

static int lxt970_config_init(struct phy_device *phydev)
{
	int err;

	err = phy_write(phydev, MII_LXT970_CONFIG, 0);

	return err;
}


static int lxt971_ack_interrupt(struct phy_device *phydev)
{
	int err = phy_read(phydev, MII_LXT971_ISR);

	if (err < 0)
		return err;

	return 0;
}

static int lxt971_config_intr(struct phy_device *phydev)
{
	int err;

	if(phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, MII_LXT971_IER, MII_LXT971_IER_IEN);
	else
		err = phy_write(phydev, MII_LXT971_IER, 0);

	return err;
}

static struct phy_driver lxt970_driver = {
	.phy_id		= 0x78100000,
	.name		= "LXT970",
	.phy_id_mask	= 0xfffffff0,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= lxt970_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= lxt970_ack_interrupt,
	.config_intr	= lxt970_config_intr,
	.driver 	= { .owner = THIS_MODULE,},
};

static struct phy_driver lxt971_driver = {
	.phy_id		= 0x001378e0,
	.name		= "LXT971",
	.phy_id_mask	= 0xfffffff0,
	.features	= PHY_BASIC_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= lxt971_ack_interrupt,
	.config_intr	= lxt971_config_intr,
	.driver 	= { .owner = THIS_MODULE,},
};

static int __init lxt_init(void)
{
	int ret;

	ret = phy_driver_register(&lxt970_driver);
	if (ret)
		goto err1;

	ret = phy_driver_register(&lxt971_driver);
	if (ret)
		goto err2;
	return 0;

 err2:	
	phy_driver_unregister(&lxt970_driver);
 err1:
	return ret;
}

static void __exit lxt_exit(void)
{
	phy_driver_unregister(&lxt970_driver);
	phy_driver_unregister(&lxt971_driver);
}

module_init(lxt_init);
module_exit(lxt_exit);
