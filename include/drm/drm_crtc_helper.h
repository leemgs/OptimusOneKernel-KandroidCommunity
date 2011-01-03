



#ifndef __DRM_CRTC_HELPER_H__
#define __DRM_CRTC_HELPER_H__

#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/idr.h>

#include <linux/fb.h>

#include "drm_fb_helper.h"
struct drm_crtc_helper_funcs {
	
	void (*dpms)(struct drm_crtc *crtc, int mode);
	void (*prepare)(struct drm_crtc *crtc);
	void (*commit)(struct drm_crtc *crtc);

	
	bool (*mode_fixup)(struct drm_crtc *crtc,
			   struct drm_display_mode *mode,
			   struct drm_display_mode *adjusted_mode);
	
	int (*mode_set)(struct drm_crtc *crtc, struct drm_display_mode *mode,
			struct drm_display_mode *adjusted_mode, int x, int y,
			struct drm_framebuffer *old_fb);

	
	int (*mode_set_base)(struct drm_crtc *crtc, int x, int y,
			     struct drm_framebuffer *old_fb);

	
	void (*load_lut)(struct drm_crtc *crtc);
};

struct drm_encoder_helper_funcs {
	void (*dpms)(struct drm_encoder *encoder, int mode);
	void (*save)(struct drm_encoder *encoder);
	void (*restore)(struct drm_encoder *encoder);

	bool (*mode_fixup)(struct drm_encoder *encoder,
			   struct drm_display_mode *mode,
			   struct drm_display_mode *adjusted_mode);
	void (*prepare)(struct drm_encoder *encoder);
	void (*commit)(struct drm_encoder *encoder);
	void (*mode_set)(struct drm_encoder *encoder,
			 struct drm_display_mode *mode,
			 struct drm_display_mode *adjusted_mode);
	struct drm_crtc *(*get_crtc)(struct drm_encoder *encoder);
	
	enum drm_connector_status (*detect)(struct drm_encoder *encoder,
					    struct drm_connector *connector);
	
	void (*disable)(struct drm_encoder *encoder);
};

struct drm_connector_helper_funcs {
	int (*get_modes)(struct drm_connector *connector);
	int (*mode_valid)(struct drm_connector *connector,
			  struct drm_display_mode *mode);
	struct drm_encoder *(*best_encoder)(struct drm_connector *connector);
};

extern int drm_helper_probe_single_connector_modes(struct drm_connector *connector, uint32_t maxX, uint32_t maxY);
extern void drm_helper_disable_unused_functions(struct drm_device *dev);
extern int drm_helper_hotplug_stage_two(struct drm_device *dev);
extern bool drm_helper_initial_config(struct drm_device *dev);
extern int drm_crtc_helper_set_config(struct drm_mode_set *set);
extern bool drm_crtc_helper_set_mode(struct drm_crtc *crtc,
				     struct drm_display_mode *mode,
				     int x, int y,
				     struct drm_framebuffer *old_fb);
extern bool drm_helper_crtc_in_use(struct drm_crtc *crtc);
extern bool drm_helper_encoder_in_use(struct drm_encoder *encoder);

extern void drm_helper_connector_dpms(struct drm_connector *connector, int mode);

extern int drm_helper_mode_fill_fb_struct(struct drm_framebuffer *fb,
					  struct drm_mode_fb_cmd *mode_cmd);

static inline void drm_crtc_helper_add(struct drm_crtc *crtc,
				       const struct drm_crtc_helper_funcs *funcs)
{
	crtc->helper_private = (void *)funcs;
}

static inline void drm_encoder_helper_add(struct drm_encoder *encoder,
					  const struct drm_encoder_helper_funcs *funcs)
{
	encoder->helper_private = (void *)funcs;
}

static inline int drm_connector_helper_add(struct drm_connector *connector,
					    const struct drm_connector_helper_funcs *funcs)
{
	connector->helper_private = (void *)funcs;
	return drm_fb_helper_add_connector(connector);
}

extern int drm_helper_resume_force_mode(struct drm_device *dev);
#endif
