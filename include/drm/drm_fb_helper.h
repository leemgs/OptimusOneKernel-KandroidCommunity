
#ifndef DRM_FB_HELPER_H
#define DRM_FB_HELPER_H

struct drm_fb_helper_crtc {
	uint32_t crtc_id;
	struct drm_mode_set mode_set;
};


struct drm_fb_helper_funcs {
	void (*gamma_set)(struct drm_crtc *crtc, u16 red, u16 green,
			  u16 blue, int regno);
	void (*gamma_get)(struct drm_crtc *crtc, u16 *red, u16 *green,
			  u16 *blue, int regno);
};


struct drm_fb_helper_cmdline_mode {
	bool specified;
	bool refresh_specified;
	bool bpp_specified;
	int xres, yres;
	int bpp;
	int refresh;
	bool rb;
	bool interlace;
	bool cvt;
	bool margins;
};

struct drm_fb_helper_connector {
	struct drm_fb_helper_cmdline_mode cmdline_mode;
};

struct drm_fb_helper {
	struct drm_framebuffer *fb;
	struct drm_device *dev;
	struct drm_display_mode *mode;
	int crtc_count;
	struct drm_fb_helper_crtc *crtc_info;
	struct drm_fb_helper_funcs *funcs;
	int conn_limit;
	struct list_head kernel_fb_list;
};

int drm_fb_helper_single_fb_probe(struct drm_device *dev,
				  int preferred_bpp,
				  int (*fb_create)(struct drm_device *dev,
						   uint32_t fb_width,
						   uint32_t fb_height,
						   uint32_t surface_width,
						   uint32_t surface_height,
						   uint32_t surface_depth,
						   uint32_t surface_bpp,
						   struct drm_framebuffer **fb_ptr));
int drm_fb_helper_init_crtc_count(struct drm_fb_helper *helper, int crtc_count,
				  int max_conn);
void drm_fb_helper_free(struct drm_fb_helper *helper);
int drm_fb_helper_blank(int blank, struct fb_info *info);
int drm_fb_helper_pan_display(struct fb_var_screeninfo *var,
			      struct fb_info *info);
int drm_fb_helper_set_par(struct fb_info *info);
int drm_fb_helper_check_var(struct fb_var_screeninfo *var,
			    struct fb_info *info);
int drm_fb_helper_setcolreg(unsigned regno,
			    unsigned red,
			    unsigned green,
			    unsigned blue,
			    unsigned transp,
			    struct fb_info *info);

void drm_fb_helper_restore(void);
void drm_fb_helper_fill_var(struct fb_info *info, struct drm_framebuffer *fb,
			    uint32_t fb_width, uint32_t fb_height);
void drm_fb_helper_fill_fix(struct fb_info *info, uint32_t pitch,
			    uint32_t depth);

int drm_fb_helper_add_connector(struct drm_connector *connector);
int drm_fb_helper_parse_command_line(struct drm_device *dev);
int drm_fb_helper_setcmap(struct fb_cmap *cmap, struct fb_info *info);

#endif
