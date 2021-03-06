
#include <linux/module.h>
#include <linux/phy.h>

#define MII_BCM63XX_IR		0x1a	
#define MII_BCM63XX_IR_EN	0x4000	
#define MII_BCM63XX_IR_DUPLEX	0x0800	
#define MII_BCM63XX_IR_SPEED	0x0400	
#define MII_BCM63XX_IR_LINK	0x0200	
#define MII_BCM63XX_IR_GMASK	0x0100	

MODULE_DESCRIPTION("Broadcom 63xx internal PHY driver");
MODULE_AUTHOR("Maxime Bizon <mbizon@freebox.fr>");
MODULE_LICENSE("GPL");

static int bcm63xx_config_init(struct phy_device *phydev)
{
	int reg, err;

	reg = phy_read(phydev, MII_BCM63XX_IR);
	if (reg < 0)
		return reg;

	
	reg |= MII_BCM63XX_IR_GMASK;
	err = phy_write(phydev, MII_BCM63XX_IR, reg);
	if (err < 0)
		return err;

	
	reg = ~(MII_BCM63XX_IR_DUPLEX |
		MII_BCM63XX_IR_SPEED |
		MII_BCM63XX_IR_LINK) |
		MII_BCM63XX_IR_EN;
	err = phy_write(phydev, MII_BCM63XX_IR, reg);
	if (err < 0)
		return err;
	return 0;
}

static int bcm63xx_ack_interrupt(struct phy_device *phydev)
{
	int reg;

	
	reg = phy_read(phydev, MII_BCM63XX_IR);
	if (reg < 0)
		return reg;

	return 0;
}

static int bcm63xx_config_intr(struct phy_device *phydev)
{
	int reg, err;

	reg = phy_read(phydev, MII_BCM63XX_IR);
	if (reg < 0)
		return reg;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		reg &= ~MII_BCM63XX_IR_GMASK;
	else
		reg |= MII_BCM63XX_IR_GMASK;

	err = phy_write(phydev, MII_BCM63XX_IR, reg);
	return err;
}

static struct phy_driver bcm63xx_1_driver = {
	.phy_id		= 0x00406000,
	.phy_id_mask	= 0xfffffc00,
	.name		= "Broadcom BCM63XX (1)",
	
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= bcm63xx_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= bcm63xx_ack_interrupt,
	.config_intr	= bcm63xx_config_intr,
	.driver		= { .owner = THIS_MODULE },
};


static struct phy_driver bcm63xx_2_driver = {
	.phy_id		= 0x002bdc00,
	.phy_id_mask	= 0xfffffc00,
	.name		= "Broadcom BCM63XX (2)",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= bcm63xx_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= bcm63xx_ack_interrupt,
	.config_intr	= bcm63xx_config_intr,
	.driver		= { .owner = THIS_MODULE },
};

static int __init bcm63xx_phy_init(void)
{
	int ret;

	ret = phy_driver_register(&bcm63xx_1_driver);
	if (ret)
		goto out_63xx_1;
	ret = phy_driver_register(&bcm63xx_2_driver);
	if (ret)
		goto out_63xx_2;
	return ret;

out_63xx_2:
	phy_driver_unregister(&bcm63xx_1_driver);
out_63xx_1:
	return ret;
}

static void __exit bcm63xx_phy_exit(void)
{
	phy_driver_unregister(&bcm63xx_1_driver);
	phy_driver_unregister(&bcm63xx_2_driver);
}

module_init(bcm63xx_phy_init);
module_exit(bcm63xx_phy_exit);
