

#include "dvo.h"




#define VR00		0x00
# define VR00_BASE_ADDRESS_MASK		0x007f


#define VR01		0x01


# define VR01_PANEL_FIT_ENABLE		(1 << 3)

# define VR01_LCD_ENABLE		(1 << 2)

# define VR01_DVO_BYPASS_ENABLE		(1 << 1)

# define VR01_DVO_ENABLE		(1 << 0)


#define VR10		0x10

# define VR10_LVDS_ENABLE		(1 << 4)

# define VR10_INTERFACE_1X18		(0 << 2)

# define VR10_INTERFACE_1X24		(1 << 2)

# define VR10_INTERFACE_2X18		(2 << 2)

# define VR10_INTERFACE_2X24		(3 << 2)


#define VR20	0x20


#define VR21	0x20


#define VR30		0x30

# define VR30_PANEL_ON			(1 << 15)

#define VR40		0x40
# define VR40_STALL_ENABLE		(1 << 13)
# define VR40_VERTICAL_INTERP_ENABLE	(1 << 12)
# define VR40_ENHANCED_PANEL_FITTING	(1 << 11)
# define VR40_HORIZONTAL_INTERP_ENABLE	(1 << 10)
# define VR40_AUTO_RATIO_ENABLE		(1 << 9)
# define VR40_CLOCK_GATING_ENABLE	(1 << 8)


#define VR41		0x41


#define VR42		0x42


#define VR43		0x43


#define VR80	    0x80
#define VR81	    0x81
#define VR82	    0x82
#define VR83	    0x83
#define VR84	    0x84
#define VR85	    0x85
#define VR86	    0x86
#define VR87	    0x87


#define VR88	    0x88


#define VR8E	    0x8E
# define VR8E_PANEL_TYPE_MASK		(0xf << 0)
# define VR8E_PANEL_INTERFACE_CMOS	(0 << 4)
# define VR8E_PANEL_INTERFACE_LVDS	(1 << 4)
# define VR8E_FORCE_DEFAULT_PANEL	(1 << 5)


#define VR8F	    0x8F
# define VR8F_VCH_PRESENT		(1 << 0)
# define VR8F_DISPLAY_CONN		(1 << 1)
# define VR8F_POWER_MASK		(0x3c)
# define VR8F_POWER_POS			(2)


struct ivch_priv {
	bool quiet;

	uint16_t width, height;

	uint16_t save_VR01;
	uint16_t save_VR40;
};


static void ivch_dump_regs(struct intel_dvo_device *dvo);


static bool ivch_read(struct intel_dvo_device *dvo, int addr, uint16_t *data)
{
	struct ivch_priv *priv = dvo->dev_priv;
	struct i2c_adapter *adapter = dvo->i2c_bus;
	struct intel_i2c_chan *i2cbus = container_of(adapter, struct intel_i2c_chan, adapter);
	u8 out_buf[1];
	u8 in_buf[2];

	struct i2c_msg msgs[] = {
		{
			.addr = dvo->slave_addr,
			.flags = I2C_M_RD,
			.len = 0,
		},
		{
			.addr = 0,
			.flags = I2C_M_NOSTART,
			.len = 1,
			.buf = out_buf,
		},
		{
			.addr = dvo->slave_addr,
			.flags = I2C_M_RD | I2C_M_NOSTART,
			.len = 2,
			.buf = in_buf,
		}
	};

	out_buf[0] = addr;

	if (i2c_transfer(&i2cbus->adapter, msgs, 3) == 3) {
		*data = (in_buf[1] << 8) | in_buf[0];
		return true;
	};

	if (!priv->quiet) {
		DRM_DEBUG("Unable to read register 0x%02x from %s:%02x.\n",
			  addr, i2cbus->adapter.name, dvo->slave_addr);
	}
	return false;
}


static bool ivch_write(struct intel_dvo_device *dvo, int addr, uint16_t data)
{
	struct ivch_priv *priv = dvo->dev_priv;
	struct i2c_adapter *adapter = dvo->i2c_bus;
	struct intel_i2c_chan *i2cbus = container_of(adapter, struct intel_i2c_chan, adapter);
	u8 out_buf[3];
	struct i2c_msg msg = {
		.addr = dvo->slave_addr,
		.flags = 0,
		.len = 3,
		.buf = out_buf,
	};

	out_buf[0] = addr;
	out_buf[1] = data & 0xff;
	out_buf[2] = data >> 8;

	if (i2c_transfer(&i2cbus->adapter, &msg, 1) == 1)
		return true;

	if (!priv->quiet) {
		DRM_DEBUG("Unable to write register 0x%02x to %s:%d.\n",
			  addr, i2cbus->adapter.name, dvo->slave_addr);
	}

	return false;
}


static bool ivch_init(struct intel_dvo_device *dvo,
		      struct i2c_adapter *adapter)
{
	struct ivch_priv *priv;
	uint16_t temp;

	priv = kzalloc(sizeof(struct ivch_priv), GFP_KERNEL);
	if (priv == NULL)
		return false;

	dvo->i2c_bus = adapter;
	dvo->dev_priv = priv;
	priv->quiet = true;

	if (!ivch_read(dvo, VR00, &temp))
		goto out;
	priv->quiet = false;

	
	if ((temp & VR00_BASE_ADDRESS_MASK) != dvo->slave_addr) {
		DRM_DEBUG("ivch detect failed due to address mismatch "
			  "(%d vs %d)\n",
			  (temp & VR00_BASE_ADDRESS_MASK), dvo->slave_addr);
		goto out;
	}

	ivch_read(dvo, VR20, &priv->width);
	ivch_read(dvo, VR21, &priv->height);

	return true;

out:
	kfree(priv);
	return false;
}

static enum drm_connector_status ivch_detect(struct intel_dvo_device *dvo)
{
	return connector_status_connected;
}

static enum drm_mode_status ivch_mode_valid(struct intel_dvo_device *dvo,
					    struct drm_display_mode *mode)
{
	if (mode->clock > 112000)
		return MODE_CLOCK_HIGH;

	return MODE_OK;
}


static void ivch_dpms(struct intel_dvo_device *dvo, int mode)
{
	int i;
	uint16_t vr01, vr30, backlight;

	
	if (!ivch_read(dvo, VR01, &vr01))
		return;

	if (mode == DRM_MODE_DPMS_ON)
		backlight = 1;
	else
		backlight = 0;
	ivch_write(dvo, VR80, backlight);

	if (mode == DRM_MODE_DPMS_ON)
		vr01 |= VR01_LCD_ENABLE | VR01_DVO_ENABLE;
	else
		vr01 &= ~(VR01_LCD_ENABLE | VR01_DVO_ENABLE);

	ivch_write(dvo, VR01, vr01);

	
	for (i = 0; i < 100; i++) {
		if (!ivch_read(dvo, VR30, &vr30))
			break;

		if (((vr30 & VR30_PANEL_ON) != 0) == (mode == DRM_MODE_DPMS_ON))
			break;
		udelay(1000);
	}
	
	udelay(16 * 1000);
}

static void ivch_mode_set(struct intel_dvo_device *dvo,
			  struct drm_display_mode *mode,
			  struct drm_display_mode *adjusted_mode)
{
	uint16_t vr40 = 0;
	uint16_t vr01;

	vr01 = 0;
	vr40 = (VR40_STALL_ENABLE | VR40_VERTICAL_INTERP_ENABLE |
		VR40_HORIZONTAL_INTERP_ENABLE);

	if (mode->hdisplay != adjusted_mode->hdisplay ||
	    mode->vdisplay != adjusted_mode->vdisplay) {
		uint16_t x_ratio, y_ratio;

		vr01 |= VR01_PANEL_FIT_ENABLE;
		vr40 |= VR40_CLOCK_GATING_ENABLE;
		x_ratio = (((mode->hdisplay - 1) << 16) /
			   (adjusted_mode->hdisplay - 1)) >> 2;
		y_ratio = (((mode->vdisplay - 1) << 16) /
			   (adjusted_mode->vdisplay - 1)) >> 2;
		ivch_write (dvo, VR42, x_ratio);
		ivch_write (dvo, VR41, y_ratio);
	} else {
		vr01 &= ~VR01_PANEL_FIT_ENABLE;
		vr40 &= ~VR40_CLOCK_GATING_ENABLE;
	}
	vr40 &= ~VR40_AUTO_RATIO_ENABLE;

	ivch_write(dvo, VR01, vr01);
	ivch_write(dvo, VR40, vr40);

	ivch_dump_regs(dvo);
}

static void ivch_dump_regs(struct intel_dvo_device *dvo)
{
	uint16_t val;

	ivch_read(dvo, VR00, &val);
	DRM_DEBUG("VR00: 0x%04x\n", val);
	ivch_read(dvo, VR01, &val);
	DRM_DEBUG("VR01: 0x%04x\n", val);
	ivch_read(dvo, VR30, &val);
	DRM_DEBUG("VR30: 0x%04x\n", val);
	ivch_read(dvo, VR40, &val);
	DRM_DEBUG("VR40: 0x%04x\n", val);

	
	ivch_read(dvo, VR80, &val);
	DRM_DEBUG("VR80: 0x%04x\n", val);
	ivch_read(dvo, VR81, &val);
	DRM_DEBUG("VR81: 0x%04x\n", val);
	ivch_read(dvo, VR82, &val);
	DRM_DEBUG("VR82: 0x%04x\n", val);
	ivch_read(dvo, VR83, &val);
	DRM_DEBUG("VR83: 0x%04x\n", val);
	ivch_read(dvo, VR84, &val);
	DRM_DEBUG("VR84: 0x%04x\n", val);
	ivch_read(dvo, VR85, &val);
	DRM_DEBUG("VR85: 0x%04x\n", val);
	ivch_read(dvo, VR86, &val);
	DRM_DEBUG("VR86: 0x%04x\n", val);
	ivch_read(dvo, VR87, &val);
	DRM_DEBUG("VR87: 0x%04x\n", val);
	ivch_read(dvo, VR88, &val);
	DRM_DEBUG("VR88: 0x%04x\n", val);

	
	ivch_read(dvo, VR8E, &val);
	DRM_DEBUG("VR8E: 0x%04x\n", val);

	
	ivch_read(dvo, VR8F, &val);
	DRM_DEBUG("VR8F: 0x%04x\n", val);
}

static void ivch_save(struct intel_dvo_device *dvo)
{
	struct ivch_priv *priv = dvo->dev_priv;

	ivch_read(dvo, VR01, &priv->save_VR01);
	ivch_read(dvo, VR40, &priv->save_VR40);
}

static void ivch_restore(struct intel_dvo_device *dvo)
{
	struct ivch_priv *priv = dvo->dev_priv;

	ivch_write(dvo, VR01, priv->save_VR01);
	ivch_write(dvo, VR40, priv->save_VR40);
}

static void ivch_destroy(struct intel_dvo_device *dvo)
{
	struct ivch_priv *priv = dvo->dev_priv;

	if (priv) {
		kfree(priv);
		dvo->dev_priv = NULL;
	}
}

struct intel_dvo_dev_ops ivch_ops= {
	.init = ivch_init,
	.dpms = ivch_dpms,
	.save = ivch_save,
	.restore = ivch_restore,
	.mode_valid = ivch_mode_valid,
	.mode_set = ivch_mode_set,
	.detect = ivch_detect,
	.dump_regs = ivch_dump_regs,
	.destroy = ivch_destroy,
};
