
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include "snddev_icodec.h"
#include <mach/qdsp5v2/audio_dev_ctl.h>
#include <mach/vreg.h>
#include <mach/pmic.h>
#include <mach/debug_mm.h>
#include <linux/wakelock.h>
#include <linux/mfd/marimba-codec.h>

#define SNDDEV_ICODEC_PCM_SZ 32 
#define SNDDEV_ICODEC_MUL_FACTOR 3 
#define SNDDEV_ICODEC_CLK_RATE(freq) \
	(((freq) * (SNDDEV_ICODEC_PCM_SZ)) << (SNDDEV_ICODEC_MUL_FACTOR))

#ifdef CONFIG_DEBUG_FS
static struct adie_codec_action_unit debug_rx_actions[] = {
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_OFF},
	{ ADIE_CODEC_ACTION_DELAY_WAIT, 0xbb8},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x02, 0x02)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x02, 0x00)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_READY },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x24, 0x6F, 0x44)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x04, 0x5F, 0xBC)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x81, 0xFF, 0x4E)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x25, 0x0F, 0x0E)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x26, 0xfc, 0xfc)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x36, 0xc0, 0x80)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x3A, 0xFF, 0x2B)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x3d, 0xFF, 0xD5)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x83, 0x21, 0x21)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x33, 0x80, 0x80)},
	{ ADIE_CODEC_ACTION_DELAY_WAIT,  0x2710},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x33, 0x40, 0x40)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x84, 0xff, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x05, 0x04)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_ANALOG_READY},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8a, 0x01, 0x01)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x36, 0xc0, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x33, 0x40, 0x00)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED,  ADIE_CODEC_ANALOG_OFF},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x33, 0x80, 0x00)}
};

static struct adie_codec_action_unit debug_tx_lb_actions[] = {
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_OFF },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x01)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x00) },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x30, 0x30)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x11, 0xfc, 0xfc)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x13, 0xfc, 0x58)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x14, 0xff, 0x65)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x15, 0xff, 0x64)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x82, 0xff, 0x5C)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_READY },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x0D, 0xF0, 0xd0)},
	{ ADIE_CODEC_ACTION_DELAY_WAIT, 0xbb8},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x14)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x86, 0xff, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x50, 0x40)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x91, 0xFF, 0x01)}, 
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_ANALOG_READY},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x10, 0x30)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x0D, 0xFF, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x00)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_ANALOG_OFF},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x11, 0xff, 0x00)}
};

static struct adie_codec_action_unit debug_tx_actions[] = {
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_OFF },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x01)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x80, 0x01, 0x00) },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x30, 0x30)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x11, 0xfc, 0xfc)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x13, 0xfc, 0x58)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x14, 0xff, 0x65)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x15, 0xff, 0x64)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x82, 0xff, 0x5C)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_READY },
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x0D, 0xF0, 0xd0)},
	{ ADIE_CODEC_ACTION_DELAY_WAIT, 0xbb8},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x14)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x86, 0xff, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x50, 0x40)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_DIGITAL_ANALOG_READY},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x8A, 0x10, 0x30)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x0D, 0xFF, 0x00)},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x83, 0x14, 0x00)},
	{ ADIE_CODEC_ACTION_STAGE_REACHED, ADIE_CODEC_ANALOG_OFF},
	{ ADIE_CODEC_ACTION_ENTRY,
	ADIE_CODEC_PACK_ENTRY(0x11, 0xff, 0x00)}
};

static struct adie_codec_hwsetting_entry debug_rx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = debug_rx_actions,
		.action_sz = ARRAY_SIZE(debug_rx_actions),
	}
};

static struct adie_codec_hwsetting_entry debug_tx_lb_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = debug_tx_lb_actions,
		.action_sz = ARRAY_SIZE(debug_tx_lb_actions),
	}
};

static struct adie_codec_hwsetting_entry debug_tx_settings[] = {
	{
		.freq_plan = 8000,
		.osr = 256,
		.actions = debug_tx_actions,
		.action_sz = ARRAY_SIZE(debug_tx_actions),
	}
};

static struct adie_codec_dev_profile debug_rx_profile = {
	.path_type = ADIE_CODEC_RX,
	.settings = debug_rx_settings,
	.setting_sz = ARRAY_SIZE(debug_rx_settings),
};

static struct adie_codec_dev_profile debug_tx_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = debug_tx_settings,
	.setting_sz = ARRAY_SIZE(debug_tx_settings),
};

static struct adie_codec_dev_profile debug_tx_lb_profile = {
	.path_type = ADIE_CODEC_TX,
	.settings = debug_tx_lb_settings,
	.setting_sz = ARRAY_SIZE(debug_tx_lb_settings),
};
#endif 


struct snddev_icodec_state {
	struct snddev_icodec_data *data;
	struct adie_codec_path *adie_path;
	u32 sample_rate;
	u32 enabled;
};


struct snddev_icodec_drv_state {
	struct mutex rx_lock;
	struct mutex tx_lock;
	u32 rx_active; 
	u32 tx_active; 
	struct clk *rx_mclk;
	struct clk *rx_sclk;
	struct clk *tx_mclk;
	struct clk *tx_sclk;
	struct clk *lpa_codec_clk;
	struct clk *lpa_core_clk;
	struct clk *lpa_p_clk;
	struct lpa_drv *lpa;

	struct wake_lock rx_idlelock;
	struct wake_lock tx_idlelock;
};

static struct snddev_icodec_drv_state snddev_icodec_drv;

static int snddev_icodec_open_rx(struct snddev_icodec_state *icodec)
{
	int trc;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	wake_lock(&drv->rx_idlelock);

	
	
	trc = clk_set_rate(drv->rx_mclk,
		SNDDEV_ICODEC_CLK_RATE(icodec->sample_rate));
	if (IS_ERR_VALUE(trc))
		goto error_invalid_freq;
	clk_enable(drv->rx_mclk);
	clk_enable(drv->rx_sclk);
	 
	clk_enable(drv->lpa_p_clk);
	clk_enable(drv->lpa_codec_clk);
	clk_enable(drv->lpa_core_clk);

	
	trc = adie_codec_open(icodec->data->profile, &icodec->adie_path);
	if (IS_ERR_VALUE(trc))
		goto error_adie;
	
	adie_codec_setpath(icodec->adie_path, icodec->sample_rate, 256);

	
	adie_codec_proceed_stage(icodec->adie_path, ADIE_CODEC_DIGITAL_READY);
	adie_codec_proceed_stage(icodec->adie_path,
					ADIE_CODEC_DIGITAL_ANALOG_READY);

	
	if (icodec->data->pamp_on)
		icodec->data->pamp_on();

	icodec->enabled = 1;

	wake_unlock(&drv->rx_idlelock);
	return 0;

error_adie:
	clk_disable(drv->lpa_p_clk);
	clk_disable(drv->lpa_codec_clk);
	clk_disable(drv->lpa_core_clk);
	clk_disable(drv->rx_sclk);
	clk_disable(drv->rx_mclk);
error_invalid_freq:

	pr_err("%s: encounter error\n", __func__);

	wake_unlock(&drv->rx_idlelock);
	return -ENODEV;
}

static int snddev_icodec_open_tx(struct snddev_icodec_state *icodec)
{
	int trc;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;;

	wake_lock(&drv->tx_idlelock);

	
	if (icodec->data->pamp_on)
		icodec->data->pamp_on();

	
	
	trc = clk_set_rate(drv->tx_mclk,
		SNDDEV_ICODEC_CLK_RATE(icodec->sample_rate));
	if (IS_ERR_VALUE(trc))
		goto error_invalid_freq;
	clk_enable(drv->tx_mclk);
	clk_enable(drv->tx_sclk);

	
	trc = adie_codec_open(icodec->data->profile, &icodec->adie_path);
	if (IS_ERR_VALUE(trc))
		goto error_adie;
	
	adie_codec_setpath(icodec->adie_path, icodec->sample_rate, 256);
	adie_codec_proceed_stage(icodec->adie_path, ADIE_CODEC_DIGITAL_READY);
	adie_codec_proceed_stage(icodec->adie_path,
	ADIE_CODEC_DIGITAL_ANALOG_READY);


	icodec->enabled = 1;

	wake_unlock(&drv->tx_idlelock);
	return 0;

error_adie:
	clk_disable(drv->tx_sclk);
	clk_disable(drv->tx_mclk);
error_invalid_freq:

	if (icodec->data->pamp_off)
		icodec->data->pamp_off();

	pr_err("%s: encounter error\n", __func__);

	wake_unlock(&drv->tx_idlelock);
	return -ENODEV;
}

static int snddev_icodec_close_rx(struct snddev_icodec_state *icodec)
{
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	wake_lock(&drv->rx_idlelock);

	
	if (icodec->data->pamp_off)
		icodec->data->pamp_off();

	
	adie_codec_proceed_stage(icodec->adie_path, ADIE_CODEC_DIGITAL_OFF);
	adie_codec_close(icodec->adie_path);
	icodec->adie_path = NULL;

	
	clk_disable(drv->lpa_p_clk);
	clk_disable(drv->lpa_codec_clk);
	clk_disable(drv->lpa_core_clk);

	
	
	clk_disable(drv->rx_sclk);
	clk_disable(drv->rx_mclk);

	icodec->enabled = 0;

	wake_unlock(&drv->rx_idlelock);
	return 0;
}

static int snddev_icodec_close_tx(struct snddev_icodec_state *icodec)
{
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	wake_lock(&drv->tx_idlelock);

	
	adie_codec_proceed_stage(icodec->adie_path, ADIE_CODEC_DIGITAL_OFF);
	adie_codec_close(icodec->adie_path);
	icodec->adie_path = NULL;

	
	
	clk_disable(drv->tx_sclk);
	clk_disable(drv->tx_mclk);

	
	if (icodec->data->pamp_off)
		icodec->data->pamp_off();

	icodec->enabled = 0;

	wake_unlock(&drv->tx_idlelock);
	return 0;
}

static int snddev_icodec_set_device_volume_impl(
		struct msm_snddev_info *dev_info, u32 volume)
{
	struct snddev_icodec_state *icodec;

	int rc = 0;

	icodec = dev_info->private_data;

	if (icodec->data->dev_vol_type & SNDDEV_DEV_VOL_DIGITAL) {

		rc = adie_codec_set_device_digital_volume(icodec->adie_path,
				icodec->data->channel_mode, volume);
		if (rc < 0) {
			pr_err("%s: unable to set_device_digital_volume for"
				"%s volume in percentage = %u\n",
				__func__, dev_info->name, volume);
			return rc;
		}

	} else if (icodec->data->dev_vol_type & SNDDEV_DEV_VOL_ANALOG)
		rc = adie_codec_set_device_analog_volume(icodec->adie_path,
				icodec->data->channel_mode, volume);
		if (rc < 0) {
			pr_err("%s: unable to set_device_analog_volume for"
				"%s volume in percentage = %u\n",
				__func__, dev_info->name, volume);
			return rc;
		}
	else {
		pr_err("%s: Invalid device volume control\n", __func__);
		return -EPERM;
	}
}

static int snddev_icodec_open(struct msm_snddev_info *dev_info)
{
	int rc = 0;
	struct snddev_icodec_state *icodec;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	if (!dev_info) {
		rc = -EINVAL;
		goto error;
	}

	icodec = dev_info->private_data;

	if (icodec->data->capability & SNDDEV_CAP_RX) {
		mutex_lock(&drv->rx_lock);
		if (drv->rx_active) {
			mutex_unlock(&drv->rx_lock);
			rc = -EBUSY;
			goto error;
		}
		rc = snddev_icodec_open_rx(icodec);

		if (!IS_ERR_VALUE(rc)) {
			drv->rx_active = 1;
			if ((icodec->data->dev_vol_type & (
				SNDDEV_DEV_VOL_DIGITAL |
				SNDDEV_DEV_VOL_ANALOG)))
				rc = snddev_icodec_set_device_volume_impl(
						dev_info, dev_info->dev_volume);
		}
		mutex_unlock(&drv->rx_lock);
	} else {
		mutex_lock(&drv->tx_lock);
		if (drv->tx_active) {
			mutex_unlock(&drv->tx_lock);
			rc = -EBUSY;
			goto error;
		}
		rc = snddev_icodec_open_tx(icodec);

		if (!IS_ERR_VALUE(rc)) {
			drv->tx_active = 1;
			if ((icodec->data->dev_vol_type & (
				SNDDEV_DEV_VOL_DIGITAL |
				SNDDEV_DEV_VOL_ANALOG)))
				rc = snddev_icodec_set_device_volume_impl(
						dev_info, dev_info->dev_volume);
		}
		mutex_unlock(&drv->tx_lock);
	}
error:
	return rc;
}

static int snddev_icodec_close(struct msm_snddev_info *dev_info)
{
	int rc = 0;
	struct snddev_icodec_state *icodec;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;
	if (!dev_info) {
		rc = -EINVAL;
		goto error;
	}

	icodec = dev_info->private_data;

	if (icodec->data->capability & SNDDEV_CAP_RX) {
		mutex_lock(&drv->rx_lock);
		if (!drv->rx_active) {
			mutex_unlock(&drv->rx_lock);
			rc = -EPERM;
			goto error;
		}
		rc = snddev_icodec_close_rx(icodec);
		if (!IS_ERR_VALUE(rc))
			drv->rx_active = 0;
		mutex_unlock(&drv->rx_lock);
	} else {
		mutex_lock(&drv->tx_lock);
		if (!drv->tx_active) {
			mutex_unlock(&drv->tx_lock);
			rc = -EPERM;
			goto error;
		}
		rc = snddev_icodec_close_tx(icodec);
		if (!IS_ERR_VALUE(rc))
			drv->tx_active = 0;
		mutex_unlock(&drv->tx_lock);
	}

error:
	return rc;
}

static int snddev_icodec_check_freq(u32 req_freq)
{
	int rc = -EINVAL;

	if ((req_freq != 0) && (req_freq >= 8000) && (req_freq <= 48000)) {
		if ((req_freq == 8000) || (req_freq == 11025) ||
			(req_freq == 12000) || (req_freq == 16000) ||
			(req_freq == 22050) || (req_freq == 24000) ||
			(req_freq == 32000) || (req_freq == 44100) ||
			(req_freq == 48000)) {
				rc = 0;
		} else
			pr_info("%s: Unsupported Frequency:%d\n", __func__,
								req_freq);
	}
	return rc;
}

static int snddev_icodec_set_freq(struct msm_snddev_info *dev_info, u32 rate)
{
	int rc;
	struct snddev_icodec_state *icodec;

	if (!dev_info) {
		rc = -EINVAL;
		goto error;
	}

	icodec = dev_info->private_data;
	if (adie_codec_freq_supported(icodec->data->profile, rate) != 0) {
		rc = -EINVAL;
		goto error;
	} else {
		if (snddev_icodec_check_freq(rate) != 0) {
			rc = -EINVAL;
			goto error;
		} else
			icodec->sample_rate = rate;
	}

	if (icodec->enabled) {
		snddev_icodec_close(dev_info);
		snddev_icodec_open(dev_info);
	}

	return icodec->sample_rate;

error:
	return rc;
}

static int snddev_icodec_enable_sidetone(struct msm_snddev_info *dev_info,
	u32 enable)
{
	int rc = 0;
	struct snddev_icodec_state *icodec;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	if (!dev_info) {
		MM_ERR("invalid dev_info\n");
		rc = -EINVAL;
		goto error;
	}

	icodec = dev_info->private_data;

	if (icodec->data->capability & SNDDEV_CAP_RX) {
		mutex_lock(&drv->rx_lock);
		if (!drv->rx_active || !dev_info->opened) {
			MM_ERR("dev not active\n");
			rc = -EPERM;
			mutex_unlock(&drv->rx_lock);
			goto error;
		}
		rc = adie_codec_enable_sidetone(icodec->adie_path, enable);
		mutex_unlock(&drv->rx_lock);
	} else {
		rc = -EINVAL;
		MM_ERR("rx device only\n");
	}

error:
	return rc;

}

int snddev_icodec_set_device_volume(struct msm_snddev_info *dev_info,
		u32 volume)
{
	struct snddev_icodec_state *icodec;
	struct mutex *lock;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;
	int rc = -EPERM;

	if (!dev_info) {
		pr_info("%s : device not intilized.\n", __func__);
		return  -EINVAL;
	}

	icodec = dev_info->private_data;

	if (!(icodec->data->dev_vol_type & (SNDDEV_DEV_VOL_DIGITAL
				| SNDDEV_DEV_VOL_ANALOG))) {

		pr_info("%s : device %s does not support device volume "
				"control.", __func__, dev_info->name);
		return -EPERM;
	}
	dev_info->dev_volume =  volume;

	if (icodec->data->capability & SNDDEV_CAP_RX)
		lock = &drv->rx_lock;
	else
		lock = &drv->tx_lock;

	mutex_lock(lock);

	rc = snddev_icodec_set_device_volume_impl(dev_info,
			dev_info->dev_volume);
	mutex_unlock(lock);
	return rc;
}

static int snddev_icodec_probe(struct platform_device *pdev)
{
	int rc = 0, i;
	struct snddev_icodec_data *pdata;
	struct msm_snddev_info *dev_info;
	struct snddev_icodec_state *icodec;

	if (!pdev || !pdev->dev.platform_data) {
		printk(KERN_ALERT "Invalid caller \n");
		rc = -1;
		goto error;
	}
	pdata = pdev->dev.platform_data;
	if ((pdata->capability & SNDDEV_CAP_RX) &&
	   (pdata->capability & SNDDEV_CAP_TX)) {
		pr_err("%s: invalid device data either RX or TX\n", __func__);
		goto error;
	}
	icodec = kzalloc(sizeof(struct snddev_icodec_state), GFP_KERNEL);
	if (!icodec) {
		rc = -ENOMEM;
		goto error;
	}
	dev_info = kmalloc(sizeof(struct msm_snddev_info), GFP_KERNEL);
	if (!dev_info) {
		kfree(icodec);
		rc = -ENOMEM;
		goto error;
	}

	dev_info->name = pdata->name;
	dev_info->copp_id = pdata->copp_id;
	dev_info->acdb_id = pdata->acdb_id;
	dev_info->private_data = (void *) icodec;
	dev_info->dev_ops.open = snddev_icodec_open;
	dev_info->dev_ops.close = snddev_icodec_close;
	dev_info->dev_ops.set_freq = snddev_icodec_set_freq;
	dev_info->dev_ops.set_device_volume = snddev_icodec_set_device_volume;
	dev_info->capability = pdata->capability;
	dev_info->opened = 0;
	msm_snddev_register(dev_info);
	icodec->data = pdata;
	icodec->sample_rate = pdata->default_sample_rate;
	dev_info->sample_rate = pdata->default_sample_rate;
	if (pdata->capability & SNDDEV_CAP_RX) {
		for (i = 0; i < VOC_RX_VOL_ARRAY_NUM; i++) {
			dev_info->max_voc_rx_vol[i] =
				pdata->max_voice_rx_vol[i];
			dev_info->min_voc_rx_vol[i] =
				pdata->min_voice_rx_vol[i];
		}
		dev_info->dev_ops.enable_sidetone =
		snddev_icodec_enable_sidetone;
	} else {
		dev_info->dev_ops.enable_sidetone = NULL;
	}

error:
	return rc;
}

static int snddev_icodec_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver snddev_icodec_driver = {
  .probe = snddev_icodec_probe,
  .remove = snddev_icodec_remove,
  .driver = { .name = "snddev_icodec" }
};

#ifdef CONFIG_DEBUG_FS
static struct dentry *debugfs_sdev_dent;
static struct dentry *debugfs_afelb;
static struct dentry *debugfs_adielb;
static struct adie_codec_path *debugfs_rx_adie;
static struct adie_codec_path *debugfs_tx_adie;

static int snddev_icodec_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	pr_info("snddev_icodec: debug intf %s\n", (char *) file->private_data);
	return 0;
}

static void debugfs_adie_loopback(u32 loop)
{
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	if (loop) {

		
		
		clk_set_rate(drv->rx_mclk,
			SNDDEV_ICODEC_CLK_RATE(8000));
		clk_enable(drv->rx_mclk);
		clk_enable(drv->rx_sclk);

		pr_info("%s: configure ADIE RX path\n", __func__);
		
		adie_codec_open(&debug_rx_profile, &debugfs_rx_adie);
		adie_codec_setpath(debugfs_rx_adie, 8000, 256);
		adie_codec_proceed_stage(debugfs_rx_adie,
		ADIE_CODEC_DIGITAL_ANALOG_READY);

		pr_info("%s: Enable Handset Mic bias\n", __func__);
		
		
		clk_set_rate(drv->tx_mclk,
			SNDDEV_ICODEC_CLK_RATE(8000));
		clk_enable(drv->tx_mclk);
		clk_enable(drv->tx_sclk);

		pr_info("%s: configure ADIE TX path\n", __func__);
		
		adie_codec_open(&debug_tx_lb_profile, &debugfs_tx_adie);
		adie_codec_setpath(debugfs_tx_adie, 8000, 256);
		adie_codec_proceed_stage(debugfs_tx_adie,
		ADIE_CODEC_DIGITAL_ANALOG_READY);
	} else {
		
		adie_codec_proceed_stage(debugfs_rx_adie,
		ADIE_CODEC_DIGITAL_OFF);
		adie_codec_close(debugfs_rx_adie);
		adie_codec_proceed_stage(debugfs_tx_adie,
		ADIE_CODEC_DIGITAL_OFF);
		adie_codec_close(debugfs_tx_adie);


		
		
		clk_disable(drv->rx_sclk);
		clk_disable(drv->rx_mclk);

		
		
		clk_disable(drv->tx_sclk);
		clk_disable(drv->tx_mclk);
	}
}

static void debugfs_afe_loopback(u32 loop)
{
	int trc;
	struct snddev_icodec_drv_state *drv = &snddev_icodec_drv;

	if (loop) {

		
		
		trc = clk_set_rate(drv->rx_mclk,
		SNDDEV_ICODEC_CLK_RATE(8000));
		if (IS_ERR_VALUE(trc))
			pr_err("%s: failed to set clk rate\n", __func__);
		clk_enable(drv->rx_mclk);
		clk_enable(drv->rx_sclk);
		clk_enable(drv->lpa_codec_clk);
		clk_enable(drv->lpa_core_clk);
		clk_enable(drv->lpa_p_clk);
		pr_info("%s: configure ADIE RX path\n", __func__);
		
		adie_codec_open(&debug_rx_profile, &debugfs_rx_adie);
		adie_codec_setpath(debugfs_rx_adie, 8000, 256);
		adie_codec_proceed_stage(debugfs_rx_adie,
		ADIE_CODEC_DIGITAL_ANALOG_READY);

		pr_info("%s: Enable Handset Mic bias\n", __func__);
		
		
		clk_set_rate(drv->tx_mclk,
			SNDDEV_ICODEC_CLK_RATE(8000));
		clk_enable(drv->tx_mclk);
		clk_enable(drv->tx_sclk);
		pr_info("%s: configure ADIE TX path\n", __func__);
		
		adie_codec_open(&debug_tx_profile, &debugfs_tx_adie);
		adie_codec_setpath(debugfs_tx_adie, 8000, 256);
		adie_codec_proceed_stage(debugfs_tx_adie,
		ADIE_CODEC_DIGITAL_ANALOG_READY);
	} else {
		
		adie_codec_proceed_stage(debugfs_rx_adie,
		ADIE_CODEC_DIGITAL_OFF);
		adie_codec_close(debugfs_rx_adie);
		adie_codec_proceed_stage(debugfs_tx_adie,
		ADIE_CODEC_DIGITAL_OFF);
		adie_codec_close(debugfs_tx_adie);


		
		
		clk_disable(drv->rx_sclk);
		clk_disable(drv->rx_mclk);

		
		
		clk_disable(drv->tx_sclk);
		clk_disable(drv->tx_mclk);
	}
}

static ssize_t snddev_icodec_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char *lb_str = filp->private_data;
	char cmd;

	if (get_user(cmd, ubuf))
		return -EFAULT;

	pr_info("%s: %s %c\n", __func__, lb_str, cmd);

	if (!strcmp(lb_str, "adie_loopback")) {
		switch (cmd) {
		case '1':
			debugfs_adie_loopback(1);
			break;
		case '0':
			debugfs_adie_loopback(0);
			break;
		}
	} else if (!strcmp(lb_str, "afe_loopback")) {
		switch (cmd) {
		case '1':
			debugfs_afe_loopback(1);
			break;
		case '0':
			debugfs_afe_loopback(0);
			break;
		}
	}

	return cnt;
}

static const struct file_operations snddev_icodec_debug_fops = {
	.open = snddev_icodec_debug_open,
	.write = snddev_icodec_debug_write
};
#endif

static int __init snddev_icodec_init(void)
{
	s32 rc;
	struct snddev_icodec_drv_state *icodec_drv = &snddev_icodec_drv;

	rc = platform_driver_register(&snddev_icodec_driver);
	if (IS_ERR_VALUE(rc))
		goto error_platform_driver;
	icodec_drv->rx_mclk = clk_get(NULL, "mi2s_codec_rx_m_clk");
	if (IS_ERR(icodec_drv->rx_mclk))
		goto error_rx_mclk;
	icodec_drv->rx_sclk = clk_get(NULL, "mi2s_codec_rx_s_clk");
	if (IS_ERR(icodec_drv->rx_sclk))
		goto error_rx_sclk;
	icodec_drv->tx_mclk = clk_get(NULL, "mi2s_codec_tx_m_clk");
	if (IS_ERR(icodec_drv->tx_mclk))
		goto error_tx_mclk;
	icodec_drv->tx_sclk = clk_get(NULL, "mi2s_codec_tx_s_clk");
	if (IS_ERR(icodec_drv->tx_sclk))
		goto error_tx_sclk;
	icodec_drv->lpa_codec_clk = clk_get(NULL, "lpa_codec_clk");
	if (IS_ERR(icodec_drv->lpa_codec_clk))
		goto error_lpa_codec_clk;
	icodec_drv->lpa_core_clk = clk_get(NULL, "lpa_core_clk");
	if (IS_ERR(icodec_drv->lpa_core_clk))
		goto error_lpa_core_clk;
	icodec_drv->lpa_p_clk = clk_get(NULL, "lpa_pclk");
	if (IS_ERR(icodec_drv->lpa_p_clk))
		goto error_lpa_p_clk;

#ifdef CONFIG_DEBUG_FS
	debugfs_sdev_dent = debugfs_create_dir("snddev_icodec", 0);
	if (!IS_ERR(debugfs_sdev_dent)) {
		debugfs_afelb = debugfs_create_file("afe_loopback",
		S_IFREG | S_IRUGO, debugfs_sdev_dent,
		(void *) "afe_loopback", &snddev_icodec_debug_fops);
		debugfs_adielb = debugfs_create_file("adie_loopback",
		S_IFREG | S_IRUGO, debugfs_sdev_dent,
		(void *) "adie_loopback", &snddev_icodec_debug_fops);
	}
#endif
	mutex_init(&icodec_drv->rx_lock);
	mutex_init(&icodec_drv->tx_lock);
	icodec_drv->rx_active = 0;
	icodec_drv->tx_active = 0;
	icodec_drv->lpa = NULL;
	wake_lock_init(&icodec_drv->tx_idlelock, WAKE_LOCK_IDLE,
			"snddev_tx_idle");
	wake_lock_init(&icodec_drv->rx_idlelock, WAKE_LOCK_IDLE,
			"snddev_rx_idle");
	return 0;

error_lpa_p_clk:
	clk_put(icodec_drv->lpa_core_clk);
error_lpa_core_clk:
	clk_put(icodec_drv->lpa_codec_clk);
error_lpa_codec_clk:
	clk_put(icodec_drv->tx_sclk);
error_tx_sclk:
	clk_put(icodec_drv->tx_mclk);
error_tx_mclk:
	clk_put(icodec_drv->rx_sclk);
error_rx_sclk:
	clk_put(icodec_drv->rx_mclk);
error_rx_mclk:
	platform_driver_unregister(&snddev_icodec_driver);
error_platform_driver:

	pr_err("%s: encounter error\n", __func__);
	return -ENODEV;
}

static void __exit snddev_icodec_exit(void)
{
	struct snddev_icodec_drv_state *icodec_drv = &snddev_icodec_drv;

#ifdef CONFIG_DEBUG_FS
	debugfs_remove(debugfs_afelb);
	debugfs_remove(debugfs_adielb);
	debugfs_remove(debugfs_sdev_dent);
#endif
	platform_driver_unregister(&snddev_icodec_driver);

	clk_put(icodec_drv->rx_sclk);
	clk_put(icodec_drv->rx_mclk);
	clk_put(icodec_drv->tx_sclk);
	clk_put(icodec_drv->tx_mclk);
	return;
}

module_init(snddev_icodec_init);
module_exit(snddev_icodec_exit);

MODULE_DESCRIPTION("ICodec Sound Device driver");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL v2");
