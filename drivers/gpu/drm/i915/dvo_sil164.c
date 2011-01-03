

#include "dvo.h"

#define SIL164_VID 0x0001
#define SIL164_DID 0x0006

#define SIL164_VID_LO 0x00
#define SIL164_VID_HI 0x01
#define SIL164_DID_LO 0x02
#define SIL164_DID_HI 0x03
#define SIL164_REV    0x04
#define SIL164_RSVD   0x05
#define SIL164_FREQ_LO 0x06
#define SIL164_FREQ_HI 0x07

#define SIL164_REG8 0x08
#define SIL164_8_VEN (1<<5)
#define SIL164_8_HEN (1<<4)
#define SIL164_8_DSEL (1<<3)
#define SIL164_8_BSEL (1<<2)
#define SIL164_8_EDGE (1<<1)
#define SIL164_8_PD   (1<<0)

#define SIL164_REG9 0x09
#define SIL164_9_VLOW (1<<7)
#define SIL164_9_MSEL_MASK (0x7<<4)
#define SIL164_9_TSEL (1<<3)
#define SIL164_9_RSEN (1<<2)
#define SIL164_9_HTPLG (1<<1)
#define SIL164_9_MDI (1<<0)

#define SIL164_REGC 0x0c

struct sil164_save_rec {
	uint8_t reg8;
	uint8_t reg9;
	uint8_t regc;
};

struct sil164_priv {
	
	bool quiet;
	struct sil164_save_rec save_regs;
	struct sil164_save_rec mode_regs;
};

#define SILPTR(d) ((SIL164Ptr)(d->DriverPrivate.ptr))

static bool sil164_readb(struct intel_dvo_device *dvo, int addr, uint8_t *ch)
{
	struct sil164_priv *sil = dvo->dev_priv;
	struct i2c_adapter *adapter = dvo->i2c_bus;
	struct intel_i2c_chan *i2cbus = container_of(adapter, struct intel_i2c_chan, adapter);
	u8 out_buf[2];
	u8 in_buf[2];

	struct i2c_msg msgs[] = {
		{
			.addr = dvo->slave_addr,
			.flags = 0,
			.len = 1,
			.buf = out_buf,
		},
		{
			.addr = dvo->slave_addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = in_buf,
		}
	};

	out_buf[0] = addr;
	out_buf[1] = 0;

	if (i2c_transfer(&i2cbus->adapter, msgs, 2) == 2) {
		*ch = in_buf[0];
		return true;
	};

	if (!sil->quiet) {
		DRM_DEBUG("Unable to read register 0x%02x from %s:%02x.\n",
			  addr, i2cbus->adapter.name, dvo->slave_addr);
	}
	return false;
}

static bool sil164_writeb(struct intel_dvo_device *dvo, int addr, uint8_t ch)
{
	struct sil164_priv *sil= dvo->dev_priv;
	struct i2c_adapter *adapter = dvo->i2c_bus;
	struct intel_i2c_chan *i2cbus = container_of(adapter, struct intel_i2c_chan, adapter);
	uint8_t out_buf[2];
	struct i2c_msg msg = {
		.addr = dvo->slave_addr,
		.flags = 0,
		.len = 2,
		.buf = out_buf,
	};

	out_buf[0] = addr;
	out_buf[1] = ch;

	if (i2c_transfer(&i2cbus->adapter, &msg, 1) == 1)
		return true;

	if (!sil->quiet) {
		DRM_DEBUG("Unable to write register 0x%02x to %s:%d.\n",
			  addr, i2cbus->adapter.name, dvo->slave_addr);
	}

	return false;
}


static bool sil164_init(struct intel_dvo_device *dvo,
			struct i2c_adapter *adapter)
{
	
	struct sil164_priv *sil;
	unsigned char ch;

	sil = kzalloc(sizeof(struct sil164_priv), GFP_KERNEL);
	if (sil == NULL)
		return false;

	dvo->i2c_bus = adapter;
	dvo->dev_priv = sil;
	sil->quiet = true;

	if (!sil164_readb(dvo, SIL164_VID_LO, &ch))
		goto out;

	if (ch != (SIL164_VID & 0xff)) {
		DRM_DEBUG("sil164 not detected got %d: from %s Slave %d.\n",
			  ch, adapter->name, dvo->slave_addr);
		goto out;
	}

	if (!sil164_readb(dvo, SIL164_DID_LO, &ch))
		goto out;

	if (ch != (SIL164_DID & 0xff)) {
		DRM_DEBUG("sil164 not detected got %d: from %s Slave %d.\n",
			  ch, adapter->name, dvo->slave_addr);
		goto out;
	}
	sil->quiet = false;

	DRM_DEBUG("init sil164 dvo controller successfully!\n");
	return true;

out:
	kfree(sil);
	return false;
}

static enum drm_connector_status sil164_detect(struct intel_dvo_device *dvo)
{
	uint8_t reg9;

	sil164_readb(dvo, SIL164_REG9, &reg9);

	if (reg9 & SIL164_9_HTPLG)
		return connector_status_connected;
	else
		return connector_status_disconnected;
}

static enum drm_mode_status sil164_mode_valid(struct intel_dvo_device *dvo,
					      struct drm_display_mode *mode)
{
	return MODE_OK;
}

static void sil164_mode_set(struct intel_dvo_device *dvo,
			    struct drm_display_mode *mode,
			    struct drm_display_mode *adjusted_mode)
{
	
	
	
	
	return;
}


static void sil164_dpms(struct intel_dvo_device *dvo, int mode)
{
	int ret;
	unsigned char ch;

	ret = sil164_readb(dvo, SIL164_REG8, &ch);
	if (ret == false)
		return;

	if (mode == DRM_MODE_DPMS_ON)
		ch |= SIL164_8_PD;
	else
		ch &= ~SIL164_8_PD;

	sil164_writeb(dvo, SIL164_REG8, ch);
	return;
}

static void sil164_dump_regs(struct intel_dvo_device *dvo)
{
	uint8_t val;

	sil164_readb(dvo, SIL164_FREQ_LO, &val);
	DRM_DEBUG("SIL164_FREQ_LO: 0x%02x\n", val);
	sil164_readb(dvo, SIL164_FREQ_HI, &val);
	DRM_DEBUG("SIL164_FREQ_HI: 0x%02x\n", val);
	sil164_readb(dvo, SIL164_REG8, &val);
	DRM_DEBUG("SIL164_REG8: 0x%02x\n", val);
	sil164_readb(dvo, SIL164_REG9, &val);
	DRM_DEBUG("SIL164_REG9: 0x%02x\n", val);
	sil164_readb(dvo, SIL164_REGC, &val);
	DRM_DEBUG("SIL164_REGC: 0x%02x\n", val);
}

static void sil164_save(struct intel_dvo_device *dvo)
{
	struct sil164_priv *sil= dvo->dev_priv;

	if (!sil164_readb(dvo, SIL164_REG8, &sil->save_regs.reg8))
		return;

	if (!sil164_readb(dvo, SIL164_REG9, &sil->save_regs.reg9))
		return;

	if (!sil164_readb(dvo, SIL164_REGC, &sil->save_regs.regc))
		return;

	return;
}

static void sil164_restore(struct intel_dvo_device *dvo)
{
	struct sil164_priv *sil = dvo->dev_priv;

	
	sil164_writeb(dvo, SIL164_REG8, sil->save_regs.reg8 & ~0x1);

	sil164_writeb(dvo, SIL164_REG9, sil->save_regs.reg9);
	sil164_writeb(dvo, SIL164_REGC, sil->save_regs.regc);
	sil164_writeb(dvo, SIL164_REG8, sil->save_regs.reg8);
}

static void sil164_destroy(struct intel_dvo_device *dvo)
{
	struct sil164_priv *sil = dvo->dev_priv;

	if (sil) {
		kfree(sil);
		dvo->dev_priv = NULL;
	}
}

struct intel_dvo_dev_ops sil164_ops = {
	.init = sil164_init,
	.detect = sil164_detect,
	.mode_valid = sil164_mode_valid,
	.mode_set = sil164_mode_set,
	.dpms = sil164_dpms,
	.dump_regs = sil164_dump_regs,
	.save = sil164_save,
	.restore = sil164_restore,
	.destroy = sil164_destroy,
};
