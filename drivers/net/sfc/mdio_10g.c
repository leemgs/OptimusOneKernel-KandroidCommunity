

#include <linux/types.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include "net_driver.h"
#include "mdio_10g.h"
#include "boards.h"
#include "workarounds.h"

unsigned efx_mdio_id_oui(u32 id)
{
	unsigned oui = 0;
	int i;

	
	for (i = 0; i < 22; ++i)
		if (id & (1 << (i + 10)))
			oui |= 1 << (i ^ 7);

	return oui;
}

int efx_mdio_reset_mmd(struct efx_nic *port, int mmd,
			    int spins, int spintime)
{
	u32 ctrl;

	
	EFX_BUG_ON_PARANOID(spins * spintime >= 5000);

	efx_mdio_write(port, mmd, MDIO_CTRL1, MDIO_CTRL1_RESET);
	
	do {
		msleep(spintime);
		ctrl = efx_mdio_read(port, mmd, MDIO_CTRL1);
		spins--;

	} while (spins && (ctrl & MDIO_CTRL1_RESET));

	return spins ? spins : -ETIMEDOUT;
}

static int efx_mdio_check_mmd(struct efx_nic *efx, int mmd, int fault_fatal)
{
	int status;

	if (LOOPBACK_INTERNAL(efx))
		return 0;

	if (mmd != MDIO_MMD_AN) {
		
		status = efx_mdio_read(efx, mmd, MDIO_STAT2);
		if ((status & MDIO_STAT2_DEVPRST) != MDIO_STAT2_DEVPRST_VAL) {
			EFX_ERR(efx, "PHY MMD %d not responding.\n", mmd);
			return -EIO;
		}
	}

	
	status = efx_mdio_read(efx, mmd, MDIO_STAT1);
	if (status & MDIO_STAT1_FAULT) {
		if (fault_fatal) {
			EFX_ERR(efx, "PHY MMD %d reporting fatal"
				" fault: status %x\n", mmd, status);
			return -EIO;
		} else {
			EFX_LOG(efx, "PHY MMD %d reporting status"
				" %x (expected)\n", mmd, status);
		}
	}
	return 0;
}


#define MDIO45_RESET_TIME	1000 
#define MDIO45_RESET_ITERS	100

int efx_mdio_wait_reset_mmds(struct efx_nic *efx, unsigned int mmd_mask)
{
	const int spintime = MDIO45_RESET_TIME / MDIO45_RESET_ITERS;
	int tries = MDIO45_RESET_ITERS;
	int rc = 0;
	int in_reset;

	while (tries) {
		int mask = mmd_mask;
		int mmd = 0;
		int stat;
		in_reset = 0;
		while (mask) {
			if (mask & 1) {
				stat = efx_mdio_read(efx, mmd, MDIO_CTRL1);
				if (stat < 0) {
					EFX_ERR(efx, "failed to read status of"
						" MMD %d\n", mmd);
					return -EIO;
				}
				if (stat & MDIO_CTRL1_RESET)
					in_reset |= (1 << mmd);
			}
			mask = mask >> 1;
			mmd++;
		}
		if (!in_reset)
			break;
		tries--;
		msleep(spintime);
	}
	if (in_reset != 0) {
		EFX_ERR(efx, "not all MMDs came out of reset in time."
			" MMDs still in reset: %x\n", in_reset);
		rc = -ETIMEDOUT;
	}
	return rc;
}

int efx_mdio_check_mmds(struct efx_nic *efx,
			unsigned int mmd_mask, unsigned int fatal_mask)
{
	int mmd = 0, probe_mmd, devs1, devs2;
	u32 devices;

	
	probe_mmd = (mmd_mask & MDIO_DEVS_PHYXS) ? MDIO_MMD_PHYXS :
	    __ffs(mmd_mask);

	
	devs1 = efx_mdio_read(efx, probe_mmd, MDIO_DEVS1);
	devs2 = efx_mdio_read(efx, probe_mmd, MDIO_DEVS2);
	if (devs1 < 0 || devs2 < 0) {
		EFX_ERR(efx, "failed to read devices present\n");
		return -EIO;
	}
	devices = devs1 | (devs2 << 16);
	if ((devices & mmd_mask) != mmd_mask) {
		EFX_ERR(efx, "required MMDs not present: got %x, "
			"wanted %x\n", devices, mmd_mask);
		return -ENODEV;
	}
	EFX_TRACE(efx, "Devices present: %x\n", devices);

	
	while (mmd_mask) {
		if (mmd_mask & 1) {
			int fault_fatal = fatal_mask & 1;
			if (efx_mdio_check_mmd(efx, mmd, fault_fatal))
				return -EIO;
		}
		mmd_mask = mmd_mask >> 1;
		fatal_mask = fatal_mask >> 1;
		mmd++;
	}

	return 0;
}

bool efx_mdio_links_ok(struct efx_nic *efx, unsigned int mmd_mask)
{
	
	if (LOOPBACK_INTERNAL(efx))
		return true;
	else if (efx->loopback_mode == LOOPBACK_NETWORK)
		return false;
	else if (efx_phy_mode_disabled(efx->phy_mode))
		return false;
	else if (efx->loopback_mode == LOOPBACK_PHYXS)
		mmd_mask &= ~(MDIO_DEVS_PHYXS |
			      MDIO_DEVS_PCS |
			      MDIO_DEVS_PMAPMD |
			      MDIO_DEVS_AN);
	else if (efx->loopback_mode == LOOPBACK_PCS)
		mmd_mask &= ~(MDIO_DEVS_PCS |
			      MDIO_DEVS_PMAPMD |
			      MDIO_DEVS_AN);
	else if (efx->loopback_mode == LOOPBACK_PMAPMD)
		mmd_mask &= ~(MDIO_DEVS_PMAPMD |
			      MDIO_DEVS_AN);

	return mdio45_links_ok(&efx->mdio, mmd_mask);
}

void efx_mdio_transmit_disable(struct efx_nic *efx)
{
	efx_mdio_set_flag(efx, MDIO_MMD_PMAPMD,
			  MDIO_PMA_TXDIS, MDIO_PMD_TXDIS_GLOBAL,
			  efx->phy_mode & PHY_MODE_TX_DISABLED);
}

void efx_mdio_phy_reconfigure(struct efx_nic *efx)
{
	efx_mdio_set_flag(efx, MDIO_MMD_PMAPMD,
			  MDIO_CTRL1, MDIO_PMA_CTRL1_LOOPBACK,
			  efx->loopback_mode == LOOPBACK_PMAPMD);
	efx_mdio_set_flag(efx, MDIO_MMD_PCS,
			  MDIO_CTRL1, MDIO_PCS_CTRL1_LOOPBACK,
			  efx->loopback_mode == LOOPBACK_PCS);
	efx_mdio_set_flag(efx, MDIO_MMD_PHYXS,
			  MDIO_CTRL1, MDIO_PHYXS_CTRL1_LOOPBACK,
			  efx->loopback_mode == LOOPBACK_NETWORK);
}

static void efx_mdio_set_mmd_lpower(struct efx_nic *efx,
				    int lpower, int mmd)
{
	int stat = efx_mdio_read(efx, mmd, MDIO_STAT1);

	EFX_TRACE(efx, "Setting low power mode for MMD %d to %d\n",
		  mmd, lpower);

	if (stat & MDIO_STAT1_LPOWERABLE) {
		efx_mdio_set_flag(efx, mmd, MDIO_CTRL1,
				  MDIO_CTRL1_LPOWER, lpower);
	}
}

void efx_mdio_set_mmds_lpower(struct efx_nic *efx,
			      int low_power, unsigned int mmd_mask)
{
	int mmd = 0;
	mmd_mask &= ~MDIO_DEVS_AN;
	while (mmd_mask) {
		if (mmd_mask & 1)
			efx_mdio_set_mmd_lpower(efx, low_power, mmd);
		mmd_mask = (mmd_mask >> 1);
		mmd++;
	}
}


int efx_mdio_set_settings(struct efx_nic *efx, struct ethtool_cmd *ecmd)
{
	struct ethtool_cmd prev;
	u32 required;
	int reg;

	efx->phy_op->get_settings(efx, &prev);

	if (ecmd->advertising == prev.advertising &&
	    ecmd->speed == prev.speed &&
	    ecmd->duplex == prev.duplex &&
	    ecmd->port == prev.port &&
	    ecmd->autoneg == prev.autoneg)
		return 0;

	
	if (prev.port != PORT_TP || ecmd->port != PORT_TP)
		return -EINVAL;

	
	if (ecmd->autoneg) {
		required = SUPPORTED_Autoneg;
	} else if (ecmd->duplex) {
		switch (ecmd->speed) {
		case SPEED_10:  required = SUPPORTED_10baseT_Full;  break;
		case SPEED_100: required = SUPPORTED_100baseT_Full; break;
		default:        return -EINVAL;
		}
	} else {
		switch (ecmd->speed) {
		case SPEED_10:  required = SUPPORTED_10baseT_Half;  break;
		case SPEED_100: required = SUPPORTED_100baseT_Half; break;
		default:        return -EINVAL;
		}
	}
	required |= ecmd->advertising;
	if (required & ~prev.supported)
		return -EINVAL;

	if (ecmd->autoneg) {
		bool xnp = (ecmd->advertising & ADVERTISED_10000baseT_Full
			    || EFX_WORKAROUND_13204(efx));

		
		reg = ADVERTISE_CSMA;
		if (ecmd->advertising & ADVERTISED_10baseT_Half)
			reg |= ADVERTISE_10HALF;
		if (ecmd->advertising & ADVERTISED_10baseT_Full)
			reg |= ADVERTISE_10FULL;
		if (ecmd->advertising & ADVERTISED_100baseT_Half)
			reg |= ADVERTISE_100HALF;
		if (ecmd->advertising & ADVERTISED_100baseT_Full)
			reg |= ADVERTISE_100FULL;
		if (xnp)
			reg |= ADVERTISE_RESV;
		else if (ecmd->advertising & (ADVERTISED_1000baseT_Half |
					      ADVERTISED_1000baseT_Full))
			reg |= ADVERTISE_NPAGE;
		reg |= mii_advertise_flowctrl(efx->wanted_fc);
		efx_mdio_write(efx, MDIO_MMD_AN, MDIO_AN_ADVERTISE, reg);

		
		if (efx->phy_op->set_npage_adv)
			efx->phy_op->set_npage_adv(efx, ecmd->advertising);

		
		reg = efx_mdio_read(efx, MDIO_MMD_AN, MDIO_CTRL1);
		reg |= MDIO_AN_CTRL1_ENABLE;
		if (!(EFX_WORKAROUND_15195(efx) &&
		      LOOPBACK_MASK(efx) & efx->phy_op->loopbacks))
			reg |= MDIO_AN_CTRL1_RESTART;
		if (xnp)
			reg |= MDIO_AN_CTRL1_XNP;
		else
			reg &= ~MDIO_AN_CTRL1_XNP;
		efx_mdio_write(efx, MDIO_MMD_AN, MDIO_CTRL1, reg);
	} else {
		
		efx_mdio_set_flag(efx, MDIO_MMD_AN, MDIO_CTRL1,
				  MDIO_AN_CTRL1_ENABLE, false);

		
		reg = efx_mdio_read(efx, MDIO_MMD_PMAPMD, MDIO_CTRL1);
		reg &= ~(MDIO_CTRL1_SPEEDSEL | MDIO_CTRL1_FULLDPLX);
		if (ecmd->speed == SPEED_100)
			reg |= MDIO_PMA_CTRL1_SPEED100;
		if (ecmd->duplex)
			reg |= MDIO_CTRL1_FULLDPLX;
		efx_mdio_write(efx, MDIO_MMD_PMAPMD, MDIO_CTRL1, reg);
	}

	return 0;
}

enum efx_fc_type efx_mdio_get_pause(struct efx_nic *efx)
{
	int lpa;

	if (!(efx->phy_op->mmds & MDIO_DEVS_AN))
		return efx->wanted_fc;
	lpa = efx_mdio_read(efx, MDIO_MMD_AN, MDIO_AN_LPA);
	return efx_fc_resolve(efx->wanted_fc, lpa);
}
