

#ifndef _INTEL_DVO_H
#define _INTEL_DVO_H

#include <linux/i2c.h>
#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "intel_drv.h"

struct intel_dvo_device {
	char *name;
	int type;
	
	u32 dvo_reg;
	
	u32 gpio;
	int slave_addr;
	struct i2c_adapter *i2c_bus;

	const struct intel_dvo_dev_ops *dev_ops;
	void *dev_priv;

	struct drm_display_mode *panel_fixed_mode;
	bool panel_wants_dither;
};

struct intel_dvo_dev_ops {
	
	bool (*init)(struct intel_dvo_device *dvo,
		     struct i2c_adapter *i2cbus);

	
	void (*create_resources)(struct intel_dvo_device *dvo);

	
	void (*dpms)(struct intel_dvo_device *dvo, int mode);

	
	void (*save)(struct intel_dvo_device *dvo);

	
	void (*restore)(struct intel_dvo_device *dvo);

	
	int (*mode_valid)(struct intel_dvo_device *dvo,
			  struct drm_display_mode *mode);

	
	bool (*mode_fixup)(struct intel_dvo_device *dvo,
			   struct drm_display_mode *mode,
			   struct drm_display_mode *adjusted_mode);

	
	void (*prepare)(struct intel_dvo_device *dvo);

	
	void (*commit)(struct intel_dvo_device *dvo);

	
	void (*mode_set)(struct intel_dvo_device *dvo,
			 struct drm_display_mode *mode,
			 struct drm_display_mode *adjusted_mode);

	
	enum drm_connector_status (*detect)(struct intel_dvo_device *dvo);

	
	struct drm_display_mode *(*get_modes)(struct intel_dvo_device *dvo);

	
	void (*destroy) (struct intel_dvo_device *dvo);

	
	void (*dump_regs)(struct intel_dvo_device *dvo);
};

extern struct intel_dvo_dev_ops sil164_ops;
extern struct intel_dvo_dev_ops ch7xxx_ops;
extern struct intel_dvo_dev_ops ivch_ops;
extern struct intel_dvo_dev_ops tfp410_ops;
extern struct intel_dvo_dev_ops ch7017_ops;

#endif 
