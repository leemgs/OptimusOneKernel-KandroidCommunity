

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/netdevice.h>

#define MII_LAN83C185_ISF 29 
#define MII_LAN83C185_IM  30 

#define MII_LAN83C185_ISF_INT1 (1<<1) 
#define MII_LAN83C185_ISF_INT2 (1<<2) 
#define MII_LAN83C185_ISF_INT3 (1<<3) 
#define MII_LAN83C185_ISF_INT4 (1<<4) 
#define MII_LAN83C185_ISF_INT5 (1<<5) 
#define MII_LAN83C185_ISF_INT6 (1<<6) 
#define MII_LAN83C185_ISF_INT7 (1<<7) 

#define MII_LAN83C185_ISF_INT_ALL (0x0e)

#define MII_LAN83C185_ISF_INT_PHYLIB_EVENTS \
	(MII_LAN83C185_ISF_INT6 | MII_LAN83C185_ISF_INT4)


static int smsc_phy_config_intr(struct phy_device *phydev)
{
	int rc = phy_write (phydev, MII_LAN83C185_IM,
			((PHY_INTERRUPT_ENABLED == phydev->interrupts)
			? MII_LAN83C185_ISF_INT_PHYLIB_EVENTS
			: 0));

	return rc < 0 ? rc : 0;
}

static int smsc_phy_ack_interrupt(struct phy_device *phydev)
{
	int rc = phy_read (phydev, MII_LAN83C185_ISF);

	return rc < 0 ? rc : 0;
}

static int smsc_phy_config_init(struct phy_device *phydev)
{
	return smsc_phy_ack_interrupt (phydev);
}


static struct phy_driver lan83c185_driver = {
	.phy_id		= 0x0007c0a0, 
	.phy_id_mask	= 0xfffffff0,
	.name		= "SMSC LAN83C185",

	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause
				| SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT | PHY_HAS_MAGICANEG,

	
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.config_init	= smsc_phy_config_init,

	
	.ack_interrupt	= smsc_phy_ack_interrupt,
	.config_intr	= smsc_phy_config_intr,

	.suspend	= genphy_suspend,
	.resume		= genphy_resume,

	.driver		= { .owner = THIS_MODULE, }
};

static struct phy_driver lan8187_driver = {
	.phy_id		= 0x0007c0b0, 
	.phy_id_mask	= 0xfffffff0,
	.name		= "SMSC LAN8187",

	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause
				| SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT | PHY_HAS_MAGICANEG,

	
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.config_init	= smsc_phy_config_init,

	
	.ack_interrupt	= smsc_phy_ack_interrupt,
	.config_intr	= smsc_phy_config_intr,

	.suspend	= genphy_suspend,
	.resume		= genphy_resume,

	.driver		= { .owner = THIS_MODULE, }
};

static struct phy_driver lan8700_driver = {
	.phy_id		= 0x0007c0c0, 
	.phy_id_mask	= 0xfffffff0,
	.name		= "SMSC LAN8700",

	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause
				| SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT | PHY_HAS_MAGICANEG,

	
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.config_init	= smsc_phy_config_init,

	
	.ack_interrupt	= smsc_phy_ack_interrupt,
	.config_intr	= smsc_phy_config_intr,

	.suspend	= genphy_suspend,
	.resume		= genphy_resume,

	.driver		= { .owner = THIS_MODULE, }
};

static struct phy_driver lan911x_int_driver = {
	.phy_id		= 0x0007c0d0, 
	.phy_id_mask	= 0xfffffff0,
	.name		= "SMSC LAN911x Internal PHY",

	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause
				| SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT | PHY_HAS_MAGICANEG,

	
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.config_init	= smsc_phy_config_init,

	
	.ack_interrupt	= smsc_phy_ack_interrupt,
	.config_intr	= smsc_phy_config_intr,

	.suspend	= genphy_suspend,
	.resume		= genphy_resume,

	.driver		= { .owner = THIS_MODULE, }
};

static struct phy_driver lan8710_driver = {
	.phy_id		= 0x0007c0f0, 
	.phy_id_mask	= 0xfffffff0,
	.name		= "SMSC LAN8710/LAN8720",

	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause
				| SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT | PHY_HAS_MAGICANEG,

	
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.config_init	= smsc_phy_config_init,

	
	.ack_interrupt	= smsc_phy_ack_interrupt,
	.config_intr	= smsc_phy_config_intr,

	.suspend	= genphy_suspend,
	.resume		= genphy_resume,

	.driver		= { .owner = THIS_MODULE, }
};

static int __init smsc_init(void)
{
	int ret;

	ret = phy_driver_register (&lan83c185_driver);
	if (ret)
		goto err1;

	ret = phy_driver_register (&lan8187_driver);
	if (ret)
		goto err2;

	ret = phy_driver_register (&lan8700_driver);
	if (ret)
		goto err3;

	ret = phy_driver_register (&lan911x_int_driver);
	if (ret)
		goto err4;

	ret = phy_driver_register (&lan8710_driver);
	if (ret)
		goto err5;

	return 0;

err5:
	phy_driver_unregister (&lan911x_int_driver);
err4:
	phy_driver_unregister (&lan8700_driver);
err3:
	phy_driver_unregister (&lan8187_driver);
err2:
	phy_driver_unregister (&lan83c185_driver);
err1:
	return ret;
}

static void __exit smsc_exit(void)
{
	phy_driver_unregister (&lan8710_driver);
	phy_driver_unregister (&lan911x_int_driver);
	phy_driver_unregister (&lan8700_driver);
	phy_driver_unregister (&lan8187_driver);
	phy_driver_unregister (&lan83c185_driver);
}

MODULE_DESCRIPTION("SMSC PHY driver");
MODULE_AUTHOR("Herbert Valerio Riedel");
MODULE_LICENSE("GPL");

module_init(smsc_init);
module_exit(smsc_exit);
