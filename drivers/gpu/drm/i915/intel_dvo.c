
#include <linux/i2c.h>
#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "intel_drv.h"
#include "i915_drm.h"
#include "i915_drv.h"
#include "dvo.h"

#define SIL164_ADDR	0x38
#define CH7xxx_ADDR	0x76
#define TFP410_ADDR	0x38

static struct intel_dvo_device intel_dvo_devices[] = {
	{
		.type = INTEL_DVO_CHIP_TMDS,
		.name = "sil164",
		.dvo_reg = DVOC,
		.slave_addr = SIL164_ADDR,
		.dev_ops = &sil164_ops,
	},
	{
		.type = INTEL_DVO_CHIP_TMDS,
		.name = "ch7xxx",
		.dvo_reg = DVOC,
		.slave_addr = CH7xxx_ADDR,
		.dev_ops = &ch7xxx_ops,
	},
	{
		.type = INTEL_DVO_CHIP_LVDS,
		.name = "ivch",
		.dvo_reg = DVOA,
		.slave_addr = 0x02, 
		.dev_ops = &ivch_ops,
	},
	{
		.type = INTEL_DVO_CHIP_TMDS,
		.name = "tfp410",
		.dvo_reg = DVOC,
		.slave_addr = TFP410_ADDR,
		.dev_ops = &tfp410_ops,
	},
	{
		.type = INTEL_DVO_CHIP_LVDS,
		.name = "ch7017",
		.dvo_reg = DVOC,
		.slave_addr = 0x75,
		.gpio = GPIOE,
		.dev_ops = &ch7017_ops,
	}
};

static void intel_dvo_dpms(struct drm_encoder *encoder, int mode)
{
	struct drm_i915_private *dev_priv = encoder->dev->dev_private;
	struct intel_output *intel_output = enc_to_intel_output(encoder);
	struct intel_dvo_device *dvo = intel_output->dev_priv;
	u32 dvo_reg = dvo->dvo_reg;
	u32 temp = I915_READ(dvo_reg);

	if (mode == DRM_MODE_DPMS_ON) {
		I915_WRITE(dvo_reg, temp | DVO_ENABLE);
		I915_READ(dvo_reg);
		dvo->dev_ops->dpms(dvo, mode);
	} else {
		dvo->dev_ops->dpms(dvo, mode);
		I915_WRITE(dvo_reg, temp & ~DVO_ENABLE);
		I915_READ(dvo_reg);
	}
}

static void intel_dvo_save(struct drm_connector *connector)
{
	struct drm_i915_private *dev_priv = connector->dev->dev_private;
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	
	dev_priv->saveDVOA = I915_READ(DVOA);
	dev_priv->saveDVOB = I915_READ(DVOB);
	dev_priv->saveDVOC = I915_READ(DVOC);

	dvo->dev_ops->save(dvo);
}

static void intel_dvo_restore(struct drm_connector *connector)
{
	struct drm_i915_private *dev_priv = connector->dev->dev_private;
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	dvo->dev_ops->restore(dvo);

	I915_WRITE(DVOA, dev_priv->saveDVOA);
	I915_WRITE(DVOB, dev_priv->saveDVOB);
	I915_WRITE(DVOC, dev_priv->saveDVOC);
}

static int intel_dvo_mode_valid(struct drm_connector *connector,
				struct drm_display_mode *mode)
{
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
		return MODE_NO_DBLESCAN;

	

	if (dvo->panel_fixed_mode) {
		if (mode->hdisplay > dvo->panel_fixed_mode->hdisplay)
			return MODE_PANEL;
		if (mode->vdisplay > dvo->panel_fixed_mode->vdisplay)
			return MODE_PANEL;
	}

	return dvo->dev_ops->mode_valid(dvo, mode);
}

static bool intel_dvo_mode_fixup(struct drm_encoder *encoder,
				 struct drm_display_mode *mode,
				 struct drm_display_mode *adjusted_mode)
{
	struct intel_output *intel_output = enc_to_intel_output(encoder);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	
	if (dvo->panel_fixed_mode != NULL) {
#define C(x) adjusted_mode->x = dvo->panel_fixed_mode->x
		C(hdisplay);
		C(hsync_start);
		C(hsync_end);
		C(htotal);
		C(vdisplay);
		C(vsync_start);
		C(vsync_end);
		C(vtotal);
		C(clock);
		drm_mode_set_crtcinfo(adjusted_mode, CRTC_INTERLACE_HALVE_V);
#undef C
	}

	if (dvo->dev_ops->mode_fixup)
		return dvo->dev_ops->mode_fixup(dvo, mode, adjusted_mode);

	return true;
}

static void intel_dvo_mode_set(struct drm_encoder *encoder,
			       struct drm_display_mode *mode,
			       struct drm_display_mode *adjusted_mode)
{
	struct drm_device *dev = encoder->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_crtc *intel_crtc = to_intel_crtc(encoder->crtc);
	struct intel_output *intel_output = enc_to_intel_output(encoder);
	struct intel_dvo_device *dvo = intel_output->dev_priv;
	int pipe = intel_crtc->pipe;
	u32 dvo_val;
	u32 dvo_reg = dvo->dvo_reg, dvo_srcdim_reg;
	int dpll_reg = (pipe == 0) ? DPLL_A : DPLL_B;

	switch (dvo_reg) {
	case DVOA:
	default:
		dvo_srcdim_reg = DVOA_SRCDIM;
		break;
	case DVOB:
		dvo_srcdim_reg = DVOB_SRCDIM;
		break;
	case DVOC:
		dvo_srcdim_reg = DVOC_SRCDIM;
		break;
	}

	dvo->dev_ops->mode_set(dvo, mode, adjusted_mode);

	
	dvo_val = I915_READ(dvo_reg) &
		  (DVO_PRESERVE_MASK | DVO_DATA_ORDER_GBRG);
	dvo_val |= DVO_DATA_ORDER_FP | DVO_BORDER_ENABLE |
		   DVO_BLANK_ACTIVE_HIGH;

	if (pipe == 1)
		dvo_val |= DVO_PIPE_B_SELECT;
	dvo_val |= DVO_PIPE_STALL;
	if (adjusted_mode->flags & DRM_MODE_FLAG_PHSYNC)
		dvo_val |= DVO_HSYNC_ACTIVE_HIGH;
	if (adjusted_mode->flags & DRM_MODE_FLAG_PVSYNC)
		dvo_val |= DVO_VSYNC_ACTIVE_HIGH;

	I915_WRITE(dpll_reg, I915_READ(dpll_reg) | DPLL_DVO_HIGH_SPEED);

	
	I915_WRITE(dvo_srcdim_reg,
		   (adjusted_mode->hdisplay << DVO_SRCDIM_HORIZONTAL_SHIFT) |
		   (adjusted_mode->vdisplay << DVO_SRCDIM_VERTICAL_SHIFT));
	
	I915_WRITE(dvo_reg, dvo_val);
}


static enum drm_connector_status intel_dvo_detect(struct drm_connector *connector)
{
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	return dvo->dev_ops->detect(dvo);
}

static int intel_dvo_get_modes(struct drm_connector *connector)
{
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	
	intel_ddc_get_modes(intel_output);
	if (!list_empty(&connector->probed_modes))
		return 1;


	if (dvo->panel_fixed_mode != NULL) {
		struct drm_display_mode *mode;
		mode = drm_mode_duplicate(connector->dev, dvo->panel_fixed_mode);
		if (mode) {
			drm_mode_probed_add(connector, mode);
			return 1;
		}
	}
	return 0;
}

static void intel_dvo_destroy (struct drm_connector *connector)
{
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;

	if (dvo) {
		if (dvo->dev_ops->destroy)
			dvo->dev_ops->destroy(dvo);
		if (dvo->panel_fixed_mode)
			kfree(dvo->panel_fixed_mode);
		
		
	}
	if (intel_output->i2c_bus)
		intel_i2c_destroy(intel_output->i2c_bus);
	if (intel_output->ddc_bus)
		intel_i2c_destroy(intel_output->ddc_bus);
	drm_sysfs_connector_remove(connector);
	drm_connector_cleanup(connector);
	kfree(intel_output);
}

#ifdef RANDR_GET_CRTC_INTERFACE
static struct drm_crtc *intel_dvo_get_crtc(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;
	int pipe = !!(I915_READ(dvo->dvo_reg) & SDVO_PIPE_B_SELECT);

	return intel_pipe_to_crtc(pScrn, pipe);
}
#endif

static const struct drm_encoder_helper_funcs intel_dvo_helper_funcs = {
	.dpms = intel_dvo_dpms,
	.mode_fixup = intel_dvo_mode_fixup,
	.prepare = intel_encoder_prepare,
	.mode_set = intel_dvo_mode_set,
	.commit = intel_encoder_commit,
};

static const struct drm_connector_funcs intel_dvo_connector_funcs = {
	.dpms = drm_helper_connector_dpms,
	.save = intel_dvo_save,
	.restore = intel_dvo_restore,
	.detect = intel_dvo_detect,
	.destroy = intel_dvo_destroy,
	.fill_modes = drm_helper_probe_single_connector_modes,
};

static const struct drm_connector_helper_funcs intel_dvo_connector_helper_funcs = {
	.mode_valid = intel_dvo_mode_valid,
	.get_modes = intel_dvo_get_modes,
	.best_encoder = intel_best_encoder,
};

static void intel_dvo_enc_destroy(struct drm_encoder *encoder)
{
	drm_encoder_cleanup(encoder);
}

static const struct drm_encoder_funcs intel_dvo_enc_funcs = {
	.destroy = intel_dvo_enc_destroy,
};



static struct drm_display_mode *
intel_dvo_get_current_mode (struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_output *intel_output = to_intel_output(connector);
	struct intel_dvo_device *dvo = intel_output->dev_priv;
	uint32_t dvo_reg = dvo->dvo_reg;
	uint32_t dvo_val = I915_READ(dvo_reg);
	struct drm_display_mode *mode = NULL;

	
	if (dvo_val & DVO_ENABLE) {
		struct drm_crtc *crtc;
		int pipe = (dvo_val & DVO_PIPE_B_SELECT) ? 1 : 0;

		crtc = intel_get_crtc_from_pipe(dev, pipe);
		if (crtc) {
			mode = intel_crtc_mode_get(dev, crtc);

			if (mode) {
				mode->type |= DRM_MODE_TYPE_PREFERRED;
				if (dvo_val & DVO_HSYNC_ACTIVE_HIGH)
					mode->flags |= DRM_MODE_FLAG_PHSYNC;
				if (dvo_val & DVO_VSYNC_ACTIVE_HIGH)
					mode->flags |= DRM_MODE_FLAG_PVSYNC;
			}
		}
	}
	return mode;
}

void intel_dvo_init(struct drm_device *dev)
{
	struct intel_output *intel_output;
	struct intel_dvo_device *dvo;
	struct i2c_adapter *i2cbus = NULL;
	int ret = 0;
	int i;
	int encoder_type = DRM_MODE_ENCODER_NONE;
	intel_output = kzalloc (sizeof(struct intel_output), GFP_KERNEL);
	if (!intel_output)
		return;

	
	intel_output->ddc_bus = intel_i2c_create(dev, GPIOD, "DVODDC_D");
	if (!intel_output->ddc_bus)
		goto free_intel;

	
	for (i = 0; i < ARRAY_SIZE(intel_dvo_devices); i++) {
		struct drm_connector *connector = &intel_output->base;
		int gpio;

		dvo = &intel_dvo_devices[i];

		
		if (dvo->gpio != 0)
			gpio = dvo->gpio;
		else if (dvo->type == INTEL_DVO_CHIP_LVDS)
			gpio = GPIOB;
		else
			gpio = GPIOE;

		
		if (i2cbus != NULL)
			intel_i2c_destroy(i2cbus);
		if (!(i2cbus = intel_i2c_create(dev, gpio,
			gpio == GPIOB ? "DVOI2C_B" : "DVOI2C_E"))) {
			continue;
		}

		if (dvo->dev_ops!= NULL)
			ret = dvo->dev_ops->init(dvo, i2cbus);
		else
			ret = false;

		if (!ret)
			continue;

		intel_output->type = INTEL_OUTPUT_DVO;
		intel_output->crtc_mask = (1 << 0) | (1 << 1);
		switch (dvo->type) {
		case INTEL_DVO_CHIP_TMDS:
			intel_output->clone_mask =
				(1 << INTEL_DVO_TMDS_CLONE_BIT) |
				(1 << INTEL_ANALOG_CLONE_BIT);
			drm_connector_init(dev, connector,
					   &intel_dvo_connector_funcs,
					   DRM_MODE_CONNECTOR_DVII);
			encoder_type = DRM_MODE_ENCODER_TMDS;
			break;
		case INTEL_DVO_CHIP_LVDS:
			intel_output->clone_mask =
				(1 << INTEL_DVO_LVDS_CLONE_BIT);
			drm_connector_init(dev, connector,
					   &intel_dvo_connector_funcs,
					   DRM_MODE_CONNECTOR_LVDS);
			encoder_type = DRM_MODE_ENCODER_LVDS;
			break;
		}

		drm_connector_helper_add(connector,
					 &intel_dvo_connector_helper_funcs);
		connector->display_info.subpixel_order = SubPixelHorizontalRGB;
		connector->interlace_allowed = false;
		connector->doublescan_allowed = false;

		intel_output->dev_priv = dvo;
		intel_output->i2c_bus = i2cbus;

		drm_encoder_init(dev, &intel_output->enc,
				 &intel_dvo_enc_funcs, encoder_type);
		drm_encoder_helper_add(&intel_output->enc,
				       &intel_dvo_helper_funcs);

		drm_mode_connector_attach_encoder(&intel_output->base,
						  &intel_output->enc);
		if (dvo->type == INTEL_DVO_CHIP_LVDS) {
			
			dvo->panel_fixed_mode =
				intel_dvo_get_current_mode(connector);
			dvo->panel_wants_dither = true;
		}

		drm_sysfs_connector_add(connector);
		return;
	}

	intel_i2c_destroy(intel_output->ddc_bus);
	
	if (i2cbus != NULL)
		intel_i2c_destroy(i2cbus);
free_intel:
	kfree(intel_output);
}
