
#include <linux/kernel.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <net/dst.h>

#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"
#include "ethernet-mdio.h"

#include "cvmx-helper-board.h"

#include "cvmx-smix-defs.h"

DECLARE_MUTEX(mdio_sem);


static int cvm_oct_mdio_read(struct net_device *dev, int phy_id, int location)
{
	union cvmx_smix_cmd smi_cmd;
	union cvmx_smix_rd_dat smi_rd;

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 1;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = location;
	cvmx_write_csr(CVMX_SMIX_CMD(0), smi_cmd.u64);

	do {
		if (!in_interrupt())
			yield();
		smi_rd.u64 = cvmx_read_csr(CVMX_SMIX_RD_DAT(0));
	} while (smi_rd.s.pending);

	if (smi_rd.s.val)
		return smi_rd.s.dat;
	else
		return 0;
}

static int cvm_oct_mdio_dummy_read(struct net_device *dev, int phy_id,
				   int location)
{
	return 0xffff;
}


static void cvm_oct_mdio_write(struct net_device *dev, int phy_id, int location,
			       int val)
{
	union cvmx_smix_cmd smi_cmd;
	union cvmx_smix_wr_dat smi_wr;

	smi_wr.u64 = 0;
	smi_wr.s.dat = val;
	cvmx_write_csr(CVMX_SMIX_WR_DAT(0), smi_wr.u64);

	smi_cmd.u64 = 0;
	smi_cmd.s.phy_op = 0;
	smi_cmd.s.phy_adr = phy_id;
	smi_cmd.s.reg_adr = location;
	cvmx_write_csr(CVMX_SMIX_CMD(0), smi_cmd.u64);

	do {
		if (!in_interrupt())
			yield();
		smi_wr.u64 = cvmx_read_csr(CVMX_SMIX_WR_DAT(0));
	} while (smi_wr.s.pending);
}

static void cvm_oct_mdio_dummy_write(struct net_device *dev, int phy_id,
				     int location, int val)
{
}

static void cvm_oct_get_drvinfo(struct net_device *dev,
				struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "cavium-ethernet");
	strcpy(info->version, OCTEON_ETHERNET_VERSION);
	strcpy(info->bus_info, "Builtin");
}

static int cvm_oct_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int ret;

	down(&mdio_sem);
	ret = mii_ethtool_gset(&priv->mii_info, cmd);
	up(&mdio_sem);

	return ret;
}

static int cvm_oct_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int ret;

	down(&mdio_sem);
	ret = mii_ethtool_sset(&priv->mii_info, cmd);
	up(&mdio_sem);

	return ret;
}

static int cvm_oct_nway_reset(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int ret;

	down(&mdio_sem);
	ret = mii_nway_restart(&priv->mii_info);
	up(&mdio_sem);

	return ret;
}

static u32 cvm_oct_get_link(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	u32 ret;

	down(&mdio_sem);
	ret = mii_link_ok(&priv->mii_info);
	up(&mdio_sem);

	return ret;
}

const struct ethtool_ops cvm_oct_ethtool_ops = {
	.get_drvinfo = cvm_oct_get_drvinfo,
	.get_settings = cvm_oct_get_settings,
	.set_settings = cvm_oct_set_settings,
	.nway_reset = cvm_oct_nway_reset,
	.get_link = cvm_oct_get_link,
	.get_sg = ethtool_op_get_sg,
	.get_tx_csum = ethtool_op_get_tx_csum,
};


int cvm_oct_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	struct mii_ioctl_data *data = if_mii(rq);
	unsigned int duplex_chg;
	int ret;

	down(&mdio_sem);
	ret = generic_mii_ioctl(&priv->mii_info, data, cmd, &duplex_chg);
	up(&mdio_sem);

	return ret;
}


int cvm_oct_mdio_setup_device(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	int phy_id = cvmx_helper_board_get_mii_address(priv->port);
	if (phy_id != -1) {
		priv->mii_info.dev = dev;
		priv->mii_info.phy_id = phy_id;
		priv->mii_info.phy_id_mask = 0xff;
		priv->mii_info.supports_gmii = 1;
		priv->mii_info.reg_num_mask = 0x1f;
		priv->mii_info.mdio_read = cvm_oct_mdio_read;
		priv->mii_info.mdio_write = cvm_oct_mdio_write;
	} else {
		
		priv->mii_info.mdio_read = cvm_oct_mdio_dummy_read;
		priv->mii_info.mdio_write = cvm_oct_mdio_dummy_write;
	}
	return 0;
}
