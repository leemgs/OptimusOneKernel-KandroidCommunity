

#include <linux/i2c.h>
#include <linux/fb.h>
#include "drmP.h"
#include "intel_drv.h"
#include "i915_drv.h"


bool intel_ddc_probe(struct intel_output *intel_output)
{
	u8 out_buf[] = { 0x0, 0x0};
	u8 buf[2];
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = 0x50,
			.flags = 0,
			.len = 1,
			.buf = out_buf,
		},
		{
			.addr = 0x50,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = buf,
		}
	};

	intel_i2c_quirk_set(intel_output->base.dev, true);
	ret = i2c_transfer(intel_output->ddc_bus, msgs, 2);
	intel_i2c_quirk_set(intel_output->base.dev, false);
	if (ret == 2)
		return true;

	return false;
}


int intel_ddc_get_modes(struct intel_output *intel_output)
{
	struct edid *edid;
	int ret = 0;

	intel_i2c_quirk_set(intel_output->base.dev, true);
	edid = drm_get_edid(&intel_output->base, intel_output->ddc_bus);
	intel_i2c_quirk_set(intel_output->base.dev, false);
	if (edid) {
		drm_mode_connector_update_edid_property(&intel_output->base,
							edid);
		ret = drm_add_edid_modes(&intel_output->base, edid);
		intel_output->base.display_info.raw_edid = NULL;
		kfree(edid);
	}

	return ret;
}
