

#include "drmP.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "drm_fb_helper.h"

static void drm_mode_validate_flag(struct drm_connector *connector,
				   int flags)
{
	struct drm_display_mode *mode, *t;

	if (flags == (DRM_MODE_FLAG_DBLSCAN | DRM_MODE_FLAG_INTERLACE))
		return;

	list_for_each_entry_safe(mode, t, &connector->modes, head) {
		if ((mode->flags & DRM_MODE_FLAG_INTERLACE) &&
				!(flags & DRM_MODE_FLAG_INTERLACE))
			mode->status = MODE_NO_INTERLACE;
		if ((mode->flags & DRM_MODE_FLAG_DBLSCAN) &&
				!(flags & DRM_MODE_FLAG_DBLSCAN))
			mode->status = MODE_NO_DBLESCAN;
	}

	return;
}


int drm_helper_probe_single_connector_modes(struct drm_connector *connector,
					    uint32_t maxX, uint32_t maxY)
{
	struct drm_device *dev = connector->dev;
	struct drm_display_mode *mode, *t;
	struct drm_connector_helper_funcs *connector_funcs =
		connector->helper_private;
	int count = 0;
	int mode_flags = 0;

	DRM_DEBUG_KMS("%s\n", drm_get_connector_name(connector));
	
	list_for_each_entry_safe(mode, t, &connector->modes, head)
		mode->status = MODE_UNVERIFIED;

	if (connector->force) {
		if (connector->force == DRM_FORCE_ON)
			connector->status = connector_status_connected;
		else
			connector->status = connector_status_disconnected;
		if (connector->funcs->force)
			connector->funcs->force(connector);
	} else
		connector->status = connector->funcs->detect(connector);

	if (connector->status == connector_status_disconnected) {
		DRM_DEBUG_KMS("%s is disconnected\n",
			  drm_get_connector_name(connector));
		goto prune;
	}

	count = (*connector_funcs->get_modes)(connector);
	if (!count) {
		count = drm_add_modes_noedid(connector, 800, 600);
		if (!count)
			return 0;
	}

	drm_mode_connector_list_update(connector);

	if (maxX && maxY)
		drm_mode_validate_size(dev, &connector->modes, maxX,
				       maxY, 0);

	if (connector->interlace_allowed)
		mode_flags |= DRM_MODE_FLAG_INTERLACE;
	if (connector->doublescan_allowed)
		mode_flags |= DRM_MODE_FLAG_DBLSCAN;
	drm_mode_validate_flag(connector, mode_flags);

	list_for_each_entry_safe(mode, t, &connector->modes, head) {
		if (mode->status == MODE_OK)
			mode->status = connector_funcs->mode_valid(connector,
								   mode);
	}

prune:
	drm_mode_prune_invalid(dev, &connector->modes, true);

	if (list_empty(&connector->modes))
		return 0;

	drm_mode_sort(&connector->modes);

	DRM_DEBUG_KMS("Probed modes for %s\n",
				drm_get_connector_name(connector));
	list_for_each_entry_safe(mode, t, &connector->modes, head) {
		mode->vrefresh = drm_mode_vrefresh(mode);

		drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);
		drm_mode_debug_printmodeline(mode);
	}

	return count;
}
EXPORT_SYMBOL(drm_helper_probe_single_connector_modes);

int drm_helper_probe_connector_modes(struct drm_device *dev, uint32_t maxX,
				      uint32_t maxY)
{
	struct drm_connector *connector;
	int count = 0;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		count += drm_helper_probe_single_connector_modes(connector,
								 maxX, maxY);
	}

	return count;
}
EXPORT_SYMBOL(drm_helper_probe_connector_modes);


bool drm_helper_encoder_in_use(struct drm_encoder *encoder)
{
	struct drm_connector *connector;
	struct drm_device *dev = encoder->dev;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head)
		if (connector->encoder == encoder)
			return true;
	return false;
}
EXPORT_SYMBOL(drm_helper_encoder_in_use);


bool drm_helper_crtc_in_use(struct drm_crtc *crtc)
{
	struct drm_encoder *encoder;
	struct drm_device *dev = crtc->dev;
	
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head)
		if (encoder->crtc == crtc && drm_helper_encoder_in_use(encoder))
			return true;
	return false;
}
EXPORT_SYMBOL(drm_helper_crtc_in_use);


void drm_helper_disable_unused_functions(struct drm_device *dev)
{
	struct drm_encoder *encoder;
	struct drm_connector *connector;
	struct drm_encoder_helper_funcs *encoder_funcs;
	struct drm_crtc *crtc;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (!connector->encoder)
			continue;
		if (connector->status == connector_status_disconnected)
			connector->encoder = NULL;
	}

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		encoder_funcs = encoder->helper_private;
		if (!drm_helper_encoder_in_use(encoder)) {
			if (encoder_funcs->disable)
				(*encoder_funcs->disable)(encoder);
			else
				(*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);
			
			encoder->crtc = NULL;
		}
	}

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
		crtc->enabled = drm_helper_crtc_in_use(crtc);
		if (!crtc->enabled) {
			crtc_funcs->dpms(crtc, DRM_MODE_DPMS_OFF);
			crtc->fb = NULL;
		}
	}
}
EXPORT_SYMBOL(drm_helper_disable_unused_functions);

static struct drm_display_mode *drm_has_preferred_mode(struct drm_connector *connector, int width, int height)
{
	struct drm_display_mode *mode;

	list_for_each_entry(mode, &connector->modes, head) {
		if (drm_mode_width(mode) > width ||
		    drm_mode_height(mode) > height)
			continue;
		if (mode->type & DRM_MODE_TYPE_PREFERRED)
			return mode;
	}
	return NULL;
}

static bool drm_has_cmdline_mode(struct drm_connector *connector)
{
	struct drm_fb_helper_connector *fb_help_conn = connector->fb_helper_private;
	struct drm_fb_helper_cmdline_mode *cmdline_mode;

	if (!fb_help_conn)
		return false;

	cmdline_mode = &fb_help_conn->cmdline_mode;
	return cmdline_mode->specified;
}

static struct drm_display_mode *drm_pick_cmdline_mode(struct drm_connector *connector, int width, int height)
{
	struct drm_fb_helper_connector *fb_help_conn = connector->fb_helper_private;
	struct drm_fb_helper_cmdline_mode *cmdline_mode;
	struct drm_display_mode *mode = NULL;

	if (!fb_help_conn)
		return mode;

	cmdline_mode = &fb_help_conn->cmdline_mode;
	if (cmdline_mode->specified == false)
		return mode;

	
	if (cmdline_mode->rb || cmdline_mode->margins)
		goto create_mode;

	list_for_each_entry(mode, &connector->modes, head) {
		
		if (mode->hdisplay != cmdline_mode->xres ||
		    mode->vdisplay != cmdline_mode->yres)
			continue;

		if (cmdline_mode->refresh_specified) {
			if (mode->vrefresh != cmdline_mode->refresh)
				continue;
		}

		if (cmdline_mode->interlace) {
			if (!(mode->flags & DRM_MODE_FLAG_INTERLACE))
				continue;
		}
		return mode;
	}

create_mode:
	mode = drm_cvt_mode(connector->dev, cmdline_mode->xres,
			    cmdline_mode->yres,
			    cmdline_mode->refresh_specified ? cmdline_mode->refresh : 60,
			    cmdline_mode->rb, cmdline_mode->interlace,
			    cmdline_mode->margins);
	drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);
	list_add(&mode->head, &connector->modes);
	return mode;
}

static bool drm_connector_enabled(struct drm_connector *connector, bool strict)
{
	bool enable;

	if (strict) {
		enable = connector->status == connector_status_connected;
	} else {
		enable = connector->status != connector_status_disconnected;
	}
	return enable;
}

static void drm_enable_connectors(struct drm_device *dev, bool *enabled)
{
	bool any_enabled = false;
	struct drm_connector *connector;
	int i = 0;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		enabled[i] = drm_connector_enabled(connector, true);
		DRM_DEBUG_KMS("connector %d enabled? %s\n", connector->base.id,
			  enabled[i] ? "yes" : "no");
		any_enabled |= enabled[i];
		i++;
	}

	if (any_enabled)
		return;

	i = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		enabled[i] = drm_connector_enabled(connector, false);
		i++;
	}
}

static bool drm_target_preferred(struct drm_device *dev,
				 struct drm_display_mode **modes,
				 bool *enabled, int width, int height)
{
	struct drm_connector *connector;
	int i = 0;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {

		if (enabled[i] == false) {
			i++;
			continue;
		}

		DRM_DEBUG_KMS("looking for cmdline mode on connector %d\n",
			      connector->base.id);

		
		modes[i] = drm_pick_cmdline_mode(connector, width, height);
		if (!modes[i]) {
			DRM_DEBUG_KMS("looking for preferred mode on connector %d\n",
				      connector->base.id);
			modes[i] = drm_has_preferred_mode(connector, width, height);
		}
		
		if (!modes[i] && !list_empty(&connector->modes)) {
			list_for_each_entry(modes[i], &connector->modes, head)
				break;
		}
		DRM_DEBUG_KMS("found mode %s\n", modes[i] ? modes[i]->name :
			  "none");
		i++;
	}
	return true;
}

static int drm_pick_crtcs(struct drm_device *dev,
			  struct drm_crtc **best_crtcs,
			  struct drm_display_mode **modes,
			  int n, int width, int height)
{
	int c, o;
	struct drm_connector *connector;
	struct drm_connector_helper_funcs *connector_funcs;
	struct drm_encoder *encoder;
	struct drm_crtc *best_crtc;
	int my_score, best_score, score;
	struct drm_crtc **crtcs, *crtc;

	if (n == dev->mode_config.num_connector)
		return 0;
	c = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (c == n)
			break;
		c++;
	}

	best_crtcs[n] = NULL;
	best_crtc = NULL;
	best_score = drm_pick_crtcs(dev, best_crtcs, modes, n+1, width, height);
	if (modes[n] == NULL)
		return best_score;

	crtcs = kmalloc(dev->mode_config.num_connector *
			sizeof(struct drm_crtc *), GFP_KERNEL);
	if (!crtcs)
		return best_score;

	my_score = 1;
	if (connector->status == connector_status_connected)
		my_score++;
	if (drm_has_cmdline_mode(connector))
		my_score++;
	if (drm_has_preferred_mode(connector, width, height))
		my_score++;

	connector_funcs = connector->helper_private;
	encoder = connector_funcs->best_encoder(connector);
	if (!encoder)
		goto out;

	connector->encoder = encoder;

	
	c = 0;
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {

		if ((encoder->possible_crtcs & (1 << c)) == 0) {
			c++;
			continue;
		}

		for (o = 0; o < n; o++)
			if (best_crtcs[o] == crtc)
				break;

		if (o < n) {
			
			c++;
			continue;
		}

		crtcs[n] = crtc;
		memcpy(crtcs, best_crtcs, n * sizeof(struct drm_crtc *));
		score = my_score + drm_pick_crtcs(dev, crtcs, modes, n + 1,
						  width, height);
		if (score > best_score) {
			best_crtc = crtc;
			best_score = score;
			memcpy(best_crtcs, crtcs,
			       dev->mode_config.num_connector *
			       sizeof(struct drm_crtc *));
		}
		c++;
	}
out:
	kfree(crtcs);
	return best_score;
}

static void drm_setup_crtcs(struct drm_device *dev)
{
	struct drm_crtc **crtcs;
	struct drm_display_mode **modes;
	struct drm_encoder *encoder;
	struct drm_connector *connector;
	bool *enabled;
	int width, height;
	int i, ret;

	DRM_DEBUG_KMS("\n");

	width = dev->mode_config.max_width;
	height = dev->mode_config.max_height;

	
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		encoder->crtc = NULL;
	}

	crtcs = kcalloc(dev->mode_config.num_connector,
			sizeof(struct drm_crtc *), GFP_KERNEL);
	modes = kcalloc(dev->mode_config.num_connector,
			sizeof(struct drm_display_mode *), GFP_KERNEL);
	enabled = kcalloc(dev->mode_config.num_connector,
			  sizeof(bool), GFP_KERNEL);

	drm_enable_connectors(dev, enabled);

	ret = drm_target_preferred(dev, modes, enabled, width, height);
	if (!ret)
		DRM_ERROR("Unable to find initial modes\n");

	DRM_DEBUG_KMS("picking CRTCs for %dx%d config\n", width, height);

	drm_pick_crtcs(dev, crtcs, modes, 0, width, height);

	i = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		struct drm_display_mode *mode = modes[i];
		struct drm_crtc *crtc = crtcs[i];

		if (connector->encoder == NULL) {
			i++;
			continue;
		}

		if (mode && crtc) {
			DRM_DEBUG_KMS("desired mode %s set on crtc %d\n",
				  mode->name, crtc->base.id);
			crtc->desired_mode = mode;
			connector->encoder->crtc = crtc;
		} else {
			connector->encoder->crtc = NULL;
			connector->encoder = NULL;
		}
		i++;
	}

	kfree(crtcs);
	kfree(modes);
	kfree(enabled);
}


static bool drm_encoder_crtc_ok(struct drm_encoder *encoder,
				struct drm_crtc *crtc)
{
	struct drm_device *dev;
	struct drm_crtc *tmp;
	int crtc_mask = 1;

	WARN(!crtc, "checking null crtc?");

	dev = crtc->dev;

	list_for_each_entry(tmp, &dev->mode_config.crtc_list, head) {
		if (tmp == crtc)
			break;
		crtc_mask <<= 1;
	}

	if (encoder->possible_crtcs & crtc_mask)
		return true;
	return false;
}


static void
drm_crtc_prepare_encoders(struct drm_device *dev)
{
	struct drm_encoder_helper_funcs *encoder_funcs;
	struct drm_encoder *encoder;

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		encoder_funcs = encoder->helper_private;
		
		if (encoder->crtc == NULL)
			(*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);
		
		if (encoder_funcs->get_crtc &&
		    encoder->crtc != (*encoder_funcs->get_crtc)(encoder))
			(*encoder_funcs->dpms)(encoder, DRM_MODE_DPMS_OFF);
	}
}


bool drm_crtc_helper_set_mode(struct drm_crtc *crtc,
			      struct drm_display_mode *mode,
			      int x, int y,
			      struct drm_framebuffer *old_fb)
{
	struct drm_device *dev = crtc->dev;
	struct drm_display_mode *adjusted_mode, saved_mode;
	struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
	struct drm_encoder_helper_funcs *encoder_funcs;
	int saved_x, saved_y;
	struct drm_encoder *encoder;
	bool ret = true;

	adjusted_mode = drm_mode_duplicate(dev, mode);

	crtc->enabled = drm_helper_crtc_in_use(crtc);

	if (!crtc->enabled)
		return true;

	saved_mode = crtc->mode;
	saved_x = crtc->x;
	saved_y = crtc->y;

	
	crtc->mode = *mode;
	crtc->x = x;
	crtc->y = y;

	
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {

		if (encoder->crtc != crtc)
			continue;
		encoder_funcs = encoder->helper_private;
		if (!(ret = encoder_funcs->mode_fixup(encoder, mode,
						      adjusted_mode))) {
			goto done;
		}
	}

	if (!(ret = crtc_funcs->mode_fixup(crtc, mode, adjusted_mode))) {
		goto done;
	}

	
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {

		if (encoder->crtc != crtc)
			continue;
		encoder_funcs = encoder->helper_private;
		
		encoder_funcs->prepare(encoder);
	}

	drm_crtc_prepare_encoders(dev);

	crtc_funcs->prepare(crtc);

	
	ret = !crtc_funcs->mode_set(crtc, mode, adjusted_mode, x, y, old_fb);
	if (!ret)
	    goto done;

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {

		if (encoder->crtc != crtc)
			continue;

		DRM_INFO("%s: set mode %s %x\n", drm_get_encoder_name(encoder),
			 mode->name, mode->base.id);
		encoder_funcs = encoder->helper_private;
		encoder_funcs->mode_set(encoder, mode, adjusted_mode);
	}

	
	crtc_funcs->commit(crtc);

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {

		if (encoder->crtc != crtc)
			continue;

		encoder_funcs = encoder->helper_private;
		encoder_funcs->commit(encoder);

	}

	
	drm_mode_destroy(dev, adjusted_mode);
	
done:
	if (!ret) {
		crtc->mode = saved_mode;
		crtc->x = saved_x;
		crtc->y = saved_y;
	}

	return ret;
}
EXPORT_SYMBOL(drm_crtc_helper_set_mode);



int drm_crtc_helper_set_config(struct drm_mode_set *set)
{
	struct drm_device *dev;
	struct drm_crtc *save_crtcs, *new_crtc, *crtc;
	struct drm_encoder *save_encoders, *new_encoder, *encoder;
	struct drm_framebuffer *old_fb = NULL;
	bool mode_changed = false; 
	bool fb_changed = false; 
	struct drm_connector *save_connectors, *connector;
	int count = 0, ro, fail = 0;
	struct drm_crtc_helper_funcs *crtc_funcs;
	int ret = 0;

	DRM_DEBUG_KMS("\n");

	if (!set)
		return -EINVAL;

	if (!set->crtc)
		return -EINVAL;

	if (!set->crtc->helper_private)
		return -EINVAL;

	crtc_funcs = set->crtc->helper_private;

	DRM_DEBUG_KMS("crtc: %p %d fb: %p connectors: %p num_connectors:"
			" %d (x, y) (%i, %i)\n",
		  set->crtc, set->crtc->base.id, set->fb, set->connectors,
		  (int)set->num_connectors, set->x, set->y);

	dev = set->crtc->dev;

	
	save_crtcs = kzalloc(dev->mode_config.num_crtc *
			     sizeof(struct drm_crtc), GFP_KERNEL);
	if (!save_crtcs)
		return -ENOMEM;

	save_encoders = kzalloc(dev->mode_config.num_encoder *
				sizeof(struct drm_encoder), GFP_KERNEL);
	if (!save_encoders) {
		kfree(save_crtcs);
		return -ENOMEM;
	}

	save_connectors = kzalloc(dev->mode_config.num_connector *
				sizeof(struct drm_connector), GFP_KERNEL);
	if (!save_connectors) {
		kfree(save_crtcs);
		kfree(save_encoders);
		return -ENOMEM;
	}

	
	count = 0;
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		save_crtcs[count++] = *crtc;
	}

	count = 0;
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		save_encoders[count++] = *encoder;
	}

	count = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		save_connectors[count++] = *connector;
	}

	
	if (set->crtc->fb != set->fb) {
		
		if (set->crtc->fb == NULL) {
			DRM_DEBUG_KMS("crtc has no fb, full mode set\n");
			mode_changed = true;
		} else if (set->fb == NULL) {
			mode_changed = true;
		} else if ((set->fb->bits_per_pixel !=
			 set->crtc->fb->bits_per_pixel) ||
			 set->fb->depth != set->crtc->fb->depth)
			fb_changed = true;
		else
			fb_changed = true;
	}

	if (set->x != set->crtc->x || set->y != set->crtc->y)
		fb_changed = true;

	if (set->mode && !drm_mode_equal(set->mode, &set->crtc->mode)) {
		DRM_DEBUG_KMS("modes are different, full mode set\n");
		drm_mode_debug_printmodeline(&set->crtc->mode);
		drm_mode_debug_printmodeline(set->mode);
		mode_changed = true;
	}

	
	count = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		struct drm_connector_helper_funcs *connector_funcs =
			connector->helper_private;
		new_encoder = connector->encoder;
		for (ro = 0; ro < set->num_connectors; ro++) {
			if (set->connectors[ro] == connector) {
				new_encoder = connector_funcs->best_encoder(connector);
				
				if (new_encoder == NULL)
					
					fail = 1;
				break;
			}
		}

		if (new_encoder != connector->encoder) {
			DRM_DEBUG_KMS("encoder changed, full mode switch\n");
			mode_changed = true;
			
			if (connector->encoder)
				connector->encoder->crtc = NULL;
			connector->encoder = new_encoder;
		}
	}

	if (fail) {
		ret = -EINVAL;
		goto fail;
	}

	count = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (!connector->encoder)
			continue;

		if (connector->encoder->crtc == set->crtc)
			new_crtc = NULL;
		else
			new_crtc = connector->encoder->crtc;

		for (ro = 0; ro < set->num_connectors; ro++) {
			if (set->connectors[ro] == connector)
				new_crtc = set->crtc;
		}

		
		if (new_crtc &&
		    !drm_encoder_crtc_ok(connector->encoder, new_crtc)) {
			ret = -EINVAL;
			goto fail;
		}
		if (new_crtc != connector->encoder->crtc) {
			DRM_DEBUG_KMS("crtc changed, full mode switch\n");
			mode_changed = true;
			connector->encoder->crtc = new_crtc;
		}
		DRM_DEBUG_KMS("setting connector %d crtc to %p\n",
			  connector->base.id, new_crtc);
	}

	
	if (fb_changed && !crtc_funcs->mode_set_base)
		mode_changed = true;

	if (mode_changed) {
		old_fb = set->crtc->fb;
		set->crtc->fb = set->fb;
		set->crtc->enabled = (set->mode != NULL);
		if (set->mode != NULL) {
			DRM_DEBUG_KMS("attempting to set mode from"
					" userspace\n");
			drm_mode_debug_printmodeline(set->mode);
			if (!drm_crtc_helper_set_mode(set->crtc, set->mode,
						      set->x, set->y,
						      old_fb)) {
				DRM_ERROR("failed to set mode on crtc %p\n",
					  set->crtc);
				ret = -EINVAL;
				goto fail;
			}
			
			set->crtc->desired_x = set->x;
			set->crtc->desired_y = set->y;
			set->crtc->desired_mode = set->mode;
		}
		drm_helper_disable_unused_functions(dev);
	} else if (fb_changed) {
		set->crtc->x = set->x;
		set->crtc->y = set->y;

		old_fb = set->crtc->fb;
		if (set->crtc->fb != set->fb)
			set->crtc->fb = set->fb;
		ret = crtc_funcs->mode_set_base(set->crtc,
						set->x, set->y, old_fb);
		if (ret != 0)
			goto fail;
	}

	kfree(save_connectors);
	kfree(save_encoders);
	kfree(save_crtcs);
	return 0;

fail:
	
	count = 0;
	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {
		*crtc = save_crtcs[count++];
	}

	count = 0;
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		*encoder = save_encoders[count++];
	}

	count = 0;
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		*connector = save_connectors[count++];
	}

	kfree(save_connectors);
	kfree(save_encoders);
	kfree(save_crtcs);
	return ret;
}
EXPORT_SYMBOL(drm_crtc_helper_set_config);

bool drm_helper_plugged_event(struct drm_device *dev)
{
	DRM_DEBUG_KMS("\n");

	drm_helper_probe_connector_modes(dev, dev->mode_config.max_width,
					 dev->mode_config.max_height);

	drm_setup_crtcs(dev);

	
	dev->mode_config.funcs->fb_changed(dev);

	
	return true;
}

bool drm_helper_initial_config(struct drm_device *dev)
{
	int count = 0;

	
	drm_helper_disable_unused_functions(dev);

	drm_fb_helper_parse_command_line(dev);

	count = drm_helper_probe_connector_modes(dev,
						 dev->mode_config.max_width,
						 dev->mode_config.max_height);

	
	WARN(!count, "No connectors reported connected with modes\n");

	drm_setup_crtcs(dev);

	
	dev->mode_config.funcs->fb_changed(dev);

	return 0;
}
EXPORT_SYMBOL(drm_helper_initial_config);

static int drm_helper_choose_encoder_dpms(struct drm_encoder *encoder)
{
	int dpms = DRM_MODE_DPMS_OFF;
	struct drm_connector *connector;
	struct drm_device *dev = encoder->dev;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head)
		if (connector->encoder == encoder)
			if (connector->dpms < dpms)
				dpms = connector->dpms;
	return dpms;
}

static int drm_helper_choose_crtc_dpms(struct drm_crtc *crtc)
{
	int dpms = DRM_MODE_DPMS_OFF;
	struct drm_connector *connector;
	struct drm_device *dev = crtc->dev;

	list_for_each_entry(connector, &dev->mode_config.connector_list, head)
		if (connector->encoder && connector->encoder->crtc == crtc)
			if (connector->dpms < dpms)
				dpms = connector->dpms;
	return dpms;
}


void drm_helper_connector_dpms(struct drm_connector *connector, int mode)
{
	struct drm_encoder *encoder = connector->encoder;
	struct drm_crtc *crtc = encoder ? encoder->crtc : NULL;
	int old_dpms;

	if (mode == connector->dpms)
		return;

	old_dpms = connector->dpms;
	connector->dpms = mode;

	
	if (mode < old_dpms) {
		if (crtc) {
			struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
			if (crtc_funcs->dpms)
				(*crtc_funcs->dpms) (crtc,
						     drm_helper_choose_crtc_dpms(crtc));
		}
		if (encoder) {
			struct drm_encoder_helper_funcs *encoder_funcs = encoder->helper_private;
			if (encoder_funcs->dpms)
				(*encoder_funcs->dpms) (encoder,
							drm_helper_choose_encoder_dpms(encoder));
		}
	}

	
	if (mode > old_dpms) {
		if (encoder) {
			struct drm_encoder_helper_funcs *encoder_funcs = encoder->helper_private;
			if (encoder_funcs->dpms)
				(*encoder_funcs->dpms) (encoder,
							drm_helper_choose_encoder_dpms(encoder));
		}
		if (crtc) {
			struct drm_crtc_helper_funcs *crtc_funcs = crtc->helper_private;
			if (crtc_funcs->dpms)
				(*crtc_funcs->dpms) (crtc,
						     drm_helper_choose_crtc_dpms(crtc));
		}
	}

	return;
}
EXPORT_SYMBOL(drm_helper_connector_dpms);


int drm_helper_hotplug_stage_two(struct drm_device *dev)
{
	drm_helper_plugged_event(dev);

	return 0;
}
EXPORT_SYMBOL(drm_helper_hotplug_stage_two);

int drm_helper_mode_fill_fb_struct(struct drm_framebuffer *fb,
				   struct drm_mode_fb_cmd *mode_cmd)
{
	fb->width = mode_cmd->width;
	fb->height = mode_cmd->height;
	fb->pitch = mode_cmd->pitch;
	fb->bits_per_pixel = mode_cmd->bpp;
	fb->depth = mode_cmd->depth;

	return 0;
}
EXPORT_SYMBOL(drm_helper_mode_fill_fb_struct);

int drm_helper_resume_force_mode(struct drm_device *dev)
{
	struct drm_crtc *crtc;
	int ret;

	list_for_each_entry(crtc, &dev->mode_config.crtc_list, head) {

		if (!crtc->enabled)
			continue;

		ret = drm_crtc_helper_set_mode(crtc, &crtc->mode,
					       crtc->x, crtc->y, crtc->fb);

		if (ret == false)
			DRM_ERROR("failed to set mode on crtc %p\n", crtc);
	}
	
	drm_helper_disable_unused_functions(dev);
	return 0;
}
EXPORT_SYMBOL(drm_helper_resume_force_mode);
