



#include <linux/delay.h>
#include <linux/rtnetlink.h>
#include "net_driver.h"
#include "efx.h"
#include "phy.h"
#include "boards.h"
#include "falcon.h"
#include "falcon_hwdefs.h"
#include "falcon_io.h"
#include "mac.h"
#include "workarounds.h"


#define	PCA9539 0x74

#define	P0_IN 0x00
#define	P0_OUT 0x02
#define	P0_INVERT 0x04
#define	P0_CONFIG 0x06

#define	P0_EN_1V0X_LBN 0
#define	P0_EN_1V0X_WIDTH 1
#define	P0_EN_1V2_LBN 1
#define	P0_EN_1V2_WIDTH 1
#define	P0_EN_2V5_LBN 2
#define	P0_EN_2V5_WIDTH 1
#define	P0_EN_3V3X_LBN 3
#define	P0_EN_3V3X_WIDTH 1
#define	P0_EN_5V_LBN 4
#define	P0_EN_5V_WIDTH 1
#define	P0_SHORTEN_JTAG_LBN 5
#define	P0_SHORTEN_JTAG_WIDTH 1
#define	P0_X_TRST_LBN 6
#define	P0_X_TRST_WIDTH 1
#define	P0_DSP_RESET_LBN 7
#define	P0_DSP_RESET_WIDTH 1

#define	P1_IN 0x01
#define	P1_OUT 0x03
#define	P1_INVERT 0x05
#define	P1_CONFIG 0x07

#define	P1_AFE_PWD_LBN 0
#define	P1_AFE_PWD_WIDTH 1
#define	P1_DSP_PWD25_LBN 1
#define	P1_DSP_PWD25_WIDTH 1
#define	P1_RESERVED_LBN 2
#define	P1_RESERVED_WIDTH 2
#define	P1_SPARE_LBN 4
#define	P1_SPARE_WIDTH 4


#define MAX664X_REG_RSL		0x02
#define MAX664X_REG_WLHO	0x0B

static void sfe4001_poweroff(struct efx_nic *efx)
{
	struct i2c_client *ioexp_client = efx->board_info.ioexp_client;
	struct i2c_client *hwmon_client = efx->board_info.hwmon_client;

	
	i2c_smbus_write_byte_data(ioexp_client, P0_OUT, 0xff);
	i2c_smbus_write_byte_data(ioexp_client, P1_CONFIG, 0xff);
	i2c_smbus_write_byte_data(ioexp_client, P0_CONFIG, 0xff);

	
	i2c_smbus_read_byte_data(hwmon_client, MAX664X_REG_RSL);
}

static int sfe4001_poweron(struct efx_nic *efx)
{
	struct i2c_client *hwmon_client = efx->board_info.hwmon_client;
	struct i2c_client *ioexp_client = efx->board_info.ioexp_client;
	unsigned int i, j;
	int rc;
	u8 out;

	
	rc = i2c_smbus_read_byte_data(hwmon_client, MAX664X_REG_RSL);
	if (rc < 0)
		return rc;

	
	rc = i2c_smbus_write_byte_data(ioexp_client, P0_CONFIG, 0x00);
	if (rc)
		return rc;
	rc = i2c_smbus_write_byte_data(ioexp_client, P1_CONFIG,
				       0xff & ~(1 << P1_SPARE_LBN));
	if (rc)
		goto fail_on;

	
	rc = i2c_smbus_read_byte_data(ioexp_client, P0_OUT);
	if (rc < 0)
		goto fail_on;
	out = 0xff & ~((0 << P0_EN_1V2_LBN) | (0 << P0_EN_2V5_LBN) |
		       (0 << P0_EN_3V3X_LBN) | (0 << P0_EN_5V_LBN) |
		       (0 << P0_EN_1V0X_LBN));
	if (rc != out) {
		EFX_INFO(efx, "power-cycling PHY\n");
		rc = i2c_smbus_write_byte_data(ioexp_client, P0_OUT, out);
		if (rc)
			goto fail_on;
		schedule_timeout_uninterruptible(HZ);
	}

	for (i = 0; i < 20; ++i) {
		
		out = 0xff & ~((1 << P0_EN_1V2_LBN) | (1 << P0_EN_2V5_LBN) |
			       (1 << P0_EN_3V3X_LBN) | (1 << P0_EN_5V_LBN) |
			       (1 << P0_X_TRST_LBN));
		if (efx->phy_mode & PHY_MODE_SPECIAL)
			out |= 1 << P0_EN_3V3X_LBN;

		rc = i2c_smbus_write_byte_data(ioexp_client, P0_OUT, out);
		if (rc)
			goto fail_on;
		msleep(10);

		
		out &= ~(1 << P0_EN_1V0X_LBN);
		rc = i2c_smbus_write_byte_data(ioexp_client, P0_OUT, out);
		if (rc)
			goto fail_on;

		EFX_INFO(efx, "waiting for DSP boot (attempt %d)...\n", i);

		
		if (efx->phy_mode & PHY_MODE_SPECIAL) {
			schedule_timeout_uninterruptible(HZ);
			return 0;
		}

		for (j = 0; j < 10; ++j) {
			msleep(100);

			
			rc = i2c_smbus_read_byte_data(ioexp_client, P1_IN);
			if (rc < 0)
				goto fail_on;
			if (rc & (1 << P1_AFE_PWD_LBN))
				return 0;
		}
	}

	EFX_INFO(efx, "timed out waiting for DSP boot\n");
	rc = -ETIMEDOUT;
fail_on:
	sfe4001_poweroff(efx);
	return rc;
}

static int sfn4111t_reset(struct efx_nic *efx)
{
	efx_oword_t reg;

	
	i2c_lock_adapter(&efx->i2c_adap);

	
	falcon_read(efx, &reg, GPIO_CTL_REG_KER);
	EFX_SET_OWORD_FIELD(reg, GPIO2_OEN, true);
	falcon_write(efx, &reg, GPIO_CTL_REG_KER);
	msleep(1000);
	EFX_SET_OWORD_FIELD(reg, GPIO2_OEN, false);
	EFX_SET_OWORD_FIELD(reg, GPIO3_OEN,
			    !!(efx->phy_mode & PHY_MODE_SPECIAL));
	falcon_write(efx, &reg, GPIO_CTL_REG_KER);
	msleep(1);

	i2c_unlock_adapter(&efx->i2c_adap);

	ssleep(1);
	return 0;
}

static ssize_t show_phy_flash_cfg(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct efx_nic *efx = pci_get_drvdata(to_pci_dev(dev));
	return sprintf(buf, "%d\n", !!(efx->phy_mode & PHY_MODE_SPECIAL));
}

static ssize_t set_phy_flash_cfg(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct efx_nic *efx = pci_get_drvdata(to_pci_dev(dev));
	enum efx_phy_mode old_mode, new_mode;
	int err;

	rtnl_lock();
	old_mode = efx->phy_mode;
	if (count == 0 || *buf == '0')
		new_mode = old_mode & ~PHY_MODE_SPECIAL;
	else
		new_mode = PHY_MODE_SPECIAL;
	if (old_mode == new_mode) {
		err = 0;
	} else if (efx->state != STATE_RUNNING || netif_running(efx->net_dev)) {
		err = -EBUSY;
	} else {
		
		efx->phy_mode = new_mode;
		if (new_mode & PHY_MODE_SPECIAL)
			efx_stats_disable(efx);
		if (efx->board_info.type == EFX_BOARD_SFE4001)
			err = sfe4001_poweron(efx);
		else
			err = sfn4111t_reset(efx);
		efx_reconfigure_port(efx);
		if (!(new_mode & PHY_MODE_SPECIAL))
			efx_stats_enable(efx);
	}
	rtnl_unlock();

	return err ? err : count;
}

static DEVICE_ATTR(phy_flash_cfg, 0644, show_phy_flash_cfg, set_phy_flash_cfg);

static void sfe4001_fini(struct efx_nic *efx)
{
	EFX_INFO(efx, "%s\n", __func__);

	device_remove_file(&efx->pci_dev->dev, &dev_attr_phy_flash_cfg);
	sfe4001_poweroff(efx);
	i2c_unregister_device(efx->board_info.ioexp_client);
	i2c_unregister_device(efx->board_info.hwmon_client);
}

static int sfe4001_check_hw(struct efx_nic *efx)
{
	s32 status;

	
	if (EFX_WORKAROUND_7884(efx) && efx->mac_up)
		return 0;

	
	status = i2c_smbus_read_byte_data(efx->board_info.ioexp_client, P1_IN);
	if (status >= 0 &&
	    (status & ((1 << P1_AFE_PWD_LBN) | (1 << P1_DSP_PWD25_LBN))) != 0)
		return 0;

	
	sfe4001_poweroff(efx);
	efx->phy_mode = PHY_MODE_OFF;

	return (status < 0) ? -EIO : -ERANGE;
}

static struct i2c_board_info sfe4001_hwmon_info = {
	I2C_BOARD_INFO("max6647", 0x4e),
};


int sfe4001_init(struct efx_nic *efx)
{
	int rc;

#if defined(CONFIG_SENSORS_LM90) || defined(CONFIG_SENSORS_LM90_MODULE)
	efx->board_info.hwmon_client =
		i2c_new_device(&efx->i2c_adap, &sfe4001_hwmon_info);
#else
	efx->board_info.hwmon_client =
		i2c_new_dummy(&efx->i2c_adap, sfe4001_hwmon_info.addr);
#endif
	if (!efx->board_info.hwmon_client)
		return -EIO;

	
	rc = i2c_smbus_write_byte_data(efx->board_info.hwmon_client,
				       MAX664X_REG_WLHO, 90);
	if (rc)
		goto fail_hwmon;

	efx->board_info.ioexp_client = i2c_new_dummy(&efx->i2c_adap, PCA9539);
	if (!efx->board_info.ioexp_client) {
		rc = -EIO;
		goto fail_hwmon;
	}

	
	efx->board_info.blink = tenxpress_phy_blink;

	efx->board_info.monitor = sfe4001_check_hw;
	efx->board_info.fini = sfe4001_fini;

	if (efx->phy_mode & PHY_MODE_SPECIAL) {
		
		efx_stats_disable(efx);
	}
	rc = sfe4001_poweron(efx);
	if (rc)
		goto fail_ioexp;

	rc = device_create_file(&efx->pci_dev->dev, &dev_attr_phy_flash_cfg);
	if (rc)
		goto fail_on;

	EFX_INFO(efx, "PHY is powered on\n");
	return 0;

fail_on:
	sfe4001_poweroff(efx);
fail_ioexp:
	i2c_unregister_device(efx->board_info.ioexp_client);
fail_hwmon:
	i2c_unregister_device(efx->board_info.hwmon_client);
	return rc;
}

static int sfn4111t_check_hw(struct efx_nic *efx)
{
	s32 status;

	
	if (EFX_WORKAROUND_7884(efx) && efx->mac_up)
		return 0;

	
	status = i2c_smbus_read_byte_data(efx->board_info.hwmon_client,
					  MAX664X_REG_RSL);
	if (status < 0)
		return -EIO;
	if (status & 0x57)
		return -ERANGE;
	return 0;
}

static void sfn4111t_fini(struct efx_nic *efx)
{
	EFX_INFO(efx, "%s\n", __func__);

	device_remove_file(&efx->pci_dev->dev, &dev_attr_phy_flash_cfg);
	i2c_unregister_device(efx->board_info.hwmon_client);
}

static struct i2c_board_info sfn4111t_a0_hwmon_info = {
	I2C_BOARD_INFO("max6647", 0x4e),
};

static struct i2c_board_info sfn4111t_r5_hwmon_info = {
	I2C_BOARD_INFO("max6646", 0x4d),
};

int sfn4111t_init(struct efx_nic *efx)
{
	int i = 0;
	int rc;

	efx->board_info.hwmon_client =
		i2c_new_device(&efx->i2c_adap,
			       (efx->board_info.minor < 5) ?
			       &sfn4111t_a0_hwmon_info :
			       &sfn4111t_r5_hwmon_info);
	if (!efx->board_info.hwmon_client)
		return -EIO;

	efx->board_info.blink = tenxpress_phy_blink;
	efx->board_info.monitor = sfn4111t_check_hw;
	efx->board_info.fini = sfn4111t_fini;

	rc = device_create_file(&efx->pci_dev->dev, &dev_attr_phy_flash_cfg);
	if (rc)
		goto fail_hwmon;

	do {
		if (efx->phy_mode & PHY_MODE_SPECIAL) {
			
			efx_stats_disable(efx);
			sfn4111t_reset(efx);
		}
		rc = sft9001_wait_boot(efx);
		if (rc == 0)
			return 0;
		efx->phy_mode = PHY_MODE_SPECIAL;
	} while (rc == -EINVAL && ++i < 2);

	device_remove_file(&efx->pci_dev->dev, &dev_attr_phy_flash_cfg);
fail_hwmon:
	i2c_unregister_device(efx->board_info.hwmon_client);
	return rc;
}
