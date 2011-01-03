


#include <linux/timer.h>
#include <linux/delay.h>
#include "efx.h"
#include "mdio_10g.h"
#include "phy.h"
#include "falcon.h"

#define XFP_REQUIRED_DEVS (MDIO_DEVS_PCS |	\
			   MDIO_DEVS_PMAPMD |	\
			   MDIO_DEVS_PHYXS)

#define XFP_LOOPBACKS ((1 << LOOPBACK_PCS) |		\
		       (1 << LOOPBACK_PMAPMD) |		\
		       (1 << LOOPBACK_NETWORK))



#define MDIO_QUAKE_LED0_REG	(0xD006)


#define PCS_FW_HEARTBEAT_REG	0xd7ee
#define PCS_FW_HEARTB_LBN	0
#define PCS_FW_HEARTB_WIDTH	8
#define PCS_UC8051_STATUS_REG	0xd7fd
#define PCS_UC_STATUS_LBN	0
#define PCS_UC_STATUS_WIDTH	8
#define PCS_UC_STATUS_FW_SAVE	0x20
#define PMA_PMD_FTX_CTRL2_REG	0xc309
#define PMA_PMD_FTX_STATIC_LBN	13
#define PMA_PMD_VEND1_REG	0xc001
#define PMA_PMD_VEND1_LBTXD_LBN	15
#define PCS_VEND1_REG	   	0xc000
#define PCS_VEND1_LBTXD_LBN	5

void xfp_set_led(struct efx_nic *p, int led, int mode)
{
	int addr = MDIO_QUAKE_LED0_REG + led;
	efx_mdio_write(p, MDIO_MMD_PMAPMD, addr, mode);
}

struct xfp_phy_data {
	enum efx_phy_mode phy_mode;
};

#define XFP_MAX_RESET_TIME 500
#define XFP_RESET_WAIT 10

static int qt2025c_wait_reset(struct efx_nic *efx)
{
	unsigned long timeout = jiffies + 10 * HZ;
	int reg, old_counter = 0;

	
	for (;;) {
		int counter;
		reg = efx_mdio_read(efx, MDIO_MMD_PCS, PCS_FW_HEARTBEAT_REG);
		if (reg < 0)
			return reg;
		counter = ((reg >> PCS_FW_HEARTB_LBN) &
			    ((1 << PCS_FW_HEARTB_WIDTH) - 1));
		if (old_counter == 0)
			old_counter = counter;
		else if (counter != old_counter)
			break;
		if (time_after(jiffies, timeout))
			return -ETIMEDOUT;
		msleep(10);
	}

	
	for (;;) {
		reg = efx_mdio_read(efx, MDIO_MMD_PCS, PCS_UC8051_STATUS_REG);
		if (reg < 0)
			return reg;
		if ((reg &
		     ((1 << PCS_UC_STATUS_WIDTH) - 1) << PCS_UC_STATUS_LBN) >=
		    PCS_UC_STATUS_FW_SAVE)
			break;
		if (time_after(jiffies, timeout))
			return -ETIMEDOUT;
		msleep(100);
	}

	return 0;
}

static int xfp_reset_phy(struct efx_nic *efx)
{
	int rc;

	if (efx->phy_type == PHY_TYPE_QT2025C) {
		
		rc = qt2025c_wait_reset(efx);
		if (rc < 0)
			goto fail;
	} else {
		
		rc = efx_mdio_reset_mmd(efx, MDIO_MMD_PHYXS,
					XFP_MAX_RESET_TIME / XFP_RESET_WAIT,
					XFP_RESET_WAIT);
		if (rc < 0)
			goto fail;
	}

	
	msleep(250);

	
	rc = efx_mdio_check_mmds(efx, XFP_REQUIRED_DEVS, MDIO_DEVS_PHYXS);
	if (rc < 0)
		goto fail;

	efx->board_info.init_leds(efx);

	return rc;

 fail:
	EFX_ERR(efx, "PHY reset timed out\n");
	return rc;
}

static int xfp_phy_init(struct efx_nic *efx)
{
	struct xfp_phy_data *phy_data;
	u32 devid = efx_mdio_read_id(efx, MDIO_MMD_PHYXS);
	int rc;

	phy_data = kzalloc(sizeof(struct xfp_phy_data), GFP_KERNEL);
	if (!phy_data)
		return -ENOMEM;
	efx->phy_data = phy_data;

	EFX_INFO(efx, "PHY ID reg %x (OUI %06x model %02x revision %x)\n",
		 devid, efx_mdio_id_oui(devid), efx_mdio_id_model(devid),
		 efx_mdio_id_rev(devid));

	phy_data->phy_mode = efx->phy_mode;

	rc = xfp_reset_phy(efx);

	EFX_INFO(efx, "PHY init %s.\n",
		 rc ? "failed" : "successful");
	if (rc < 0)
		goto fail;

	return 0;

 fail:
	kfree(efx->phy_data);
	efx->phy_data = NULL;
	return rc;
}

static void xfp_phy_clear_interrupt(struct efx_nic *efx)
{
	
	efx_mdio_read(efx, MDIO_MMD_PMAPMD, MDIO_PMA_LASI_STAT);
}

static int xfp_link_ok(struct efx_nic *efx)
{
	return efx_mdio_links_ok(efx, XFP_REQUIRED_DEVS);
}

static void xfp_phy_poll(struct efx_nic *efx)
{
	int link_up = xfp_link_ok(efx);
	
	if (link_up != efx->link_up)
		falcon_sim_phy_event(efx);
}

static void xfp_phy_reconfigure(struct efx_nic *efx)
{
	struct xfp_phy_data *phy_data = efx->phy_data;

	if (efx->phy_type == PHY_TYPE_QT2025C) {
		
		mdio_set_flag(
			&efx->mdio, efx->mdio.prtad, MDIO_MMD_PMAPMD,
			PMA_PMD_FTX_CTRL2_REG, 1 << PMA_PMD_FTX_STATIC_LBN,
			efx->phy_mode & PHY_MODE_TX_DISABLED ||
			efx->phy_mode & PHY_MODE_LOW_POWER ||
			efx->loopback_mode == LOOPBACK_PCS ||
			efx->loopback_mode == LOOPBACK_PMAPMD);
	} else {
		
		if (!(efx->phy_mode & PHY_MODE_TX_DISABLED) &&
		    (phy_data->phy_mode & PHY_MODE_TX_DISABLED))
			xfp_reset_phy(efx);

		efx_mdio_transmit_disable(efx);
	}

	efx_mdio_phy_reconfigure(efx);

	phy_data->phy_mode = efx->phy_mode;
	efx->link_up = xfp_link_ok(efx);
	efx->link_speed = 10000;
	efx->link_fd = true;
	efx->link_fc = efx->wanted_fc;
}

static void xfp_phy_get_settings(struct efx_nic *efx, struct ethtool_cmd *ecmd)
{
	mdio45_ethtool_gset(&efx->mdio, ecmd);
}

static void xfp_phy_fini(struct efx_nic *efx)
{
	
	efx->board_info.blink(efx, false);

	
	kfree(efx->phy_data);
	efx->phy_data = NULL;
}

struct efx_phy_operations falcon_xfp_phy_ops = {
	.macs		 = EFX_XMAC,
	.init            = xfp_phy_init,
	.reconfigure     = xfp_phy_reconfigure,
	.poll            = xfp_phy_poll,
	.fini            = xfp_phy_fini,
	.clear_interrupt = xfp_phy_clear_interrupt,
	.get_settings    = xfp_phy_get_settings,
	.set_settings	 = efx_mdio_set_settings,
	.mmds            = XFP_REQUIRED_DEVS,
	.loopbacks       = XFP_LOOPBACKS,
};
