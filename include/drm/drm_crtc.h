
#ifndef __DRM_CRTC_H__
#define __DRM_CRTC_H__

#include <linux/i2c.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/idr.h>

#include <linux/fb.h>

struct drm_device;
struct drm_mode_set;
struct drm_framebuffer;


#define DRM_MODE_OBJECT_CRTC 0xcccccccc
#define DRM_MODE_OBJECT_CONNECTOR 0xc0c0c0c0
#define DRM_MODE_OBJECT_ENCODER 0xe0e0e0e0
#define DRM_MODE_OBJECT_MODE 0xdededede
#define DRM_MODE_OBJECT_PROPERTY 0xb0b0b0b0
#define DRM_MODE_OBJECT_FB 0xfbfbfbfb
#define DRM_MODE_OBJECT_BLOB 0xbbbbbbbb

struct drm_mode_object {
	uint32_t id;
	uint32_t type;
};



enum drm_mode_status {
    MODE_OK	= 0,	
    MODE_HSYNC,		
    MODE_VSYNC,		
    MODE_H_ILLEGAL,	
    MODE_V_ILLEGAL,	
    MODE_BAD_WIDTH,	
    MODE_NOMODE,	
    MODE_NO_INTERLACE,	
    MODE_NO_DBLESCAN,	
    MODE_NO_VSCAN,	
    MODE_MEM,		
    MODE_VIRTUAL_X,	
    MODE_VIRTUAL_Y,	
    MODE_MEM_VIRT,	
    MODE_NOCLOCK,	
    MODE_CLOCK_HIGH,	
    MODE_CLOCK_LOW,	
    MODE_CLOCK_RANGE,	
    MODE_BAD_HVALUE,	
    MODE_BAD_VVALUE,	
    MODE_BAD_VSCAN,	
    MODE_HSYNC_NARROW,	
    MODE_HSYNC_WIDE,	
    MODE_HBLANK_NARROW,	
    MODE_HBLANK_WIDE,	
    MODE_VSYNC_NARROW,	
    MODE_VSYNC_WIDE,	
    MODE_VBLANK_NARROW,	
    MODE_VBLANK_WIDE,	
    MODE_PANEL,         
    MODE_INTERLACE_WIDTH, 
    MODE_ONE_WIDTH,     
    MODE_ONE_HEIGHT,    
    MODE_ONE_SIZE,      
    MODE_NO_REDUCED,    
    MODE_UNVERIFIED = -3, 
    MODE_BAD = -2,	
    MODE_ERROR	= -1	
};

#define DRM_MODE_TYPE_CLOCK_CRTC_C (DRM_MODE_TYPE_CLOCK_C | \
				    DRM_MODE_TYPE_CRTC_C)

#define DRM_MODE(nm, t, c, hd, hss, hse, ht, hsk, vd, vss, vse, vt, vs, f) \
	.name = nm, .status = 0, .type = (t), .clock = (c), \
	.hdisplay = (hd), .hsync_start = (hss), .hsync_end = (hse), \
	.htotal = (ht), .hskew = (hsk), .vdisplay = (vd), \
	.vsync_start = (vss), .vsync_end = (vse), .vtotal = (vt), \
	.vscan = (vs), .flags = (f), .vrefresh = 0

#define CRTC_INTERLACE_HALVE_V 0x1 

struct drm_display_mode {
	
	struct list_head head;
	struct drm_mode_object base;

	char name[DRM_DISPLAY_MODE_LEN];

	int connector_count;
	enum drm_mode_status status;
	int type;

	
	int clock;
	int hdisplay;
	int hsync_start;
	int hsync_end;
	int htotal;
	int hskew;
	int vdisplay;
	int vsync_start;
	int vsync_end;
	int vtotal;
	int vscan;
	unsigned int flags;

	
	int width_mm;
	int height_mm;

	
	int clock_index;
	int synth_clock;
	int crtc_hdisplay;
	int crtc_hblank_start;
	int crtc_hblank_end;
	int crtc_hsync_start;
	int crtc_hsync_end;
	int crtc_htotal;
	int crtc_hskew;
	int crtc_vdisplay;
	int crtc_vblank_start;
	int crtc_vblank_end;
	int crtc_vsync_start;
	int crtc_vsync_end;
	int crtc_vtotal;
	int crtc_hadjusted;
	int crtc_vadjusted;

	
	int private_size;
	int *private;
	int private_flags;

	int vrefresh;
	float hsync;
};

enum drm_connector_status {
	connector_status_connected = 1,
	connector_status_disconnected = 2,
	connector_status_unknown = 3,
};

enum subpixel_order {
	SubPixelUnknown = 0,
	SubPixelHorizontalRGB,
	SubPixelHorizontalBGR,
	SubPixelVerticalRGB,
	SubPixelVerticalBGR,
	SubPixelNone,
};



struct drm_display_info {
	char name[DRM_DISPLAY_INFO_LEN];
	
	bool serration_vsync;
	bool sync_on_green;
	bool composite_sync;
	bool separate_syncs;
	bool blank_to_black;
	unsigned char video_level;
	bool digital;
	
        unsigned int width_mm;
	unsigned int height_mm;

	
	unsigned char gamma; 
	bool gtf_supported;
	bool standard_color;
	enum {
		monochrome = 0,
		rgb,
		other,
		unknown,
	} display_type;
	bool active_off_supported;
	bool suspend_supported;
	bool standby_supported;

	
	unsigned short redx, redy;
	unsigned short greenx, greeny;
	unsigned short bluex, bluey;
	unsigned short whitex, whitey;

	
	unsigned int min_vfreq, max_vfreq;
	unsigned int min_hfreq, max_hfreq;
	unsigned int pixel_clock;

	
	unsigned int wpx1, wpy1;
	unsigned int wpgamma1;
	unsigned int wpx2, wpy2;
	unsigned int wpgamma2;

	enum subpixel_order subpixel_order;

	char *raw_edid; 
};

struct drm_framebuffer_funcs {
	void (*destroy)(struct drm_framebuffer *framebuffer);
	int (*create_handle)(struct drm_framebuffer *fb,
			     struct drm_file *file_priv,
			     unsigned int *handle);
};

struct drm_framebuffer {
	struct drm_device *dev;
	struct list_head head;
	struct drm_mode_object base;
	const struct drm_framebuffer_funcs *funcs;
	unsigned int pitch;
	unsigned int width;
	unsigned int height;
	
	unsigned int depth;
	int bits_per_pixel;
	int flags;
	void *fbdev;
	u32 pseudo_palette[17];
	struct list_head filp_head;
	
	void *helper_private;
};

struct drm_property_blob {
	struct drm_mode_object base;
	struct list_head head;
	unsigned int length;
	void *data;
};

struct drm_property_enum {
	uint64_t value;
	struct list_head head;
	char name[DRM_PROP_NAME_LEN];
};

struct drm_property {
	struct list_head head;
	struct drm_mode_object base;
	uint32_t flags;
	char name[DRM_PROP_NAME_LEN];
	uint32_t num_values;
	uint64_t *values;

	struct list_head enum_blob_list;
};

struct drm_crtc;
struct drm_connector;
struct drm_encoder;


struct drm_crtc_funcs {
	
	void (*save)(struct drm_crtc *crtc); 
	
	void (*restore)(struct drm_crtc *crtc); 

	
	int (*cursor_set)(struct drm_crtc *crtc, struct drm_file *file_priv,
			  uint32_t handle, uint32_t width, uint32_t height);
	int (*cursor_move)(struct drm_crtc *crtc, int x, int y);

	
	void (*gamma_set)(struct drm_crtc *crtc, u16 *r, u16 *g, u16 *b,
			  uint32_t size);
	
	void (*destroy)(struct drm_crtc *crtc);

	int (*set_config)(struct drm_mode_set *set);
};


struct drm_crtc {
	struct drm_device *dev;
	struct list_head head;

	struct drm_mode_object base;

	
	struct drm_framebuffer *fb;

	bool enabled;

	struct drm_display_mode mode;

	int x, y;
	struct drm_display_mode *desired_mode;
	int desired_x, desired_y;
	const struct drm_crtc_funcs *funcs;

	
	uint32_t gamma_size;
	uint16_t *gamma_store;

	
	void *helper_private;
};



struct drm_connector_funcs {
	void (*dpms)(struct drm_connector *connector, int mode);
	void (*save)(struct drm_connector *connector);
	void (*restore)(struct drm_connector *connector);
	enum drm_connector_status (*detect)(struct drm_connector *connector);
	int (*fill_modes)(struct drm_connector *connector, uint32_t max_width, uint32_t max_height);
	int (*set_property)(struct drm_connector *connector, struct drm_property *property,
			     uint64_t val);
	void (*destroy)(struct drm_connector *connector);
	void (*force)(struct drm_connector *connector);
};

struct drm_encoder_funcs {
	void (*destroy)(struct drm_encoder *encoder);
};

#define DRM_CONNECTOR_MAX_UMODES 16
#define DRM_CONNECTOR_MAX_PROPERTY 16
#define DRM_CONNECTOR_LEN 32
#define DRM_CONNECTOR_MAX_ENCODER 2


struct drm_encoder {
	struct drm_device *dev;
	struct list_head head;

	struct drm_mode_object base;
	int encoder_type;
	uint32_t possible_crtcs;
	uint32_t possible_clones;

	struct drm_crtc *crtc;
	const struct drm_encoder_funcs *funcs;
	void *helper_private;
};

enum drm_connector_force {
	DRM_FORCE_UNSPECIFIED,
	DRM_FORCE_OFF,
	DRM_FORCE_ON,         
	DRM_FORCE_ON_DIGITAL, 
};


struct drm_connector {
	struct drm_device *dev;
	struct device kdev;
	struct device_attribute *attr;
	struct list_head head;

	struct drm_mode_object base;

	int connector_type;
	int connector_type_id;
	bool interlace_allowed;
	bool doublescan_allowed;
	struct list_head modes; 

	int initial_x, initial_y;
	enum drm_connector_status status;

	
	struct list_head probed_modes;

	struct drm_display_info display_info;
	const struct drm_connector_funcs *funcs;

	struct list_head user_modes;
	struct drm_property_blob *edid_blob_ptr;
	u32 property_ids[DRM_CONNECTOR_MAX_PROPERTY];
	uint64_t property_values[DRM_CONNECTOR_MAX_PROPERTY];

	
	int dpms;

	void *helper_private;

	
	enum drm_connector_force force;
	uint32_t encoder_ids[DRM_CONNECTOR_MAX_ENCODER];
	uint32_t force_encoder_id;
	struct drm_encoder *encoder; 
	void *fb_helper_private;
};


struct drm_mode_set {
	struct list_head head;

	struct drm_framebuffer *fb;
	struct drm_crtc *crtc;
	struct drm_display_mode *mode;

	uint32_t x;
	uint32_t y;

	struct drm_connector **connectors;
	size_t num_connectors;
};


struct drm_mode_config_funcs {
	struct drm_framebuffer *(*fb_create)(struct drm_device *dev, struct drm_file *file_priv, struct drm_mode_fb_cmd *mode_cmd);
	int (*fb_changed)(struct drm_device *dev);
};

struct drm_mode_group {
	uint32_t num_crtcs;
	uint32_t num_encoders;
	uint32_t num_connectors;

	
	uint32_t *id_list;
};


struct drm_mode_config {
	struct mutex mutex; 
	struct mutex idr_mutex; 
	struct idr crtc_idr; 
	
	int num_fb;
	struct list_head fb_list;
	int num_connector;
	struct list_head connector_list;
	int num_encoder;
	struct list_head encoder_list;

	int num_crtc;
	struct list_head crtc_list;

	struct list_head property_list;

	
	struct list_head fb_kernel_list;

	int min_width, min_height;
	int max_width, max_height;
	struct drm_mode_config_funcs *funcs;
	resource_size_t fb_base;

	
	struct list_head property_blob_list;
	struct drm_property *edid_property;
	struct drm_property *dpms_property;

	
	struct drm_property *dvi_i_subconnector_property;
	struct drm_property *dvi_i_select_subconnector_property;

	
	struct drm_property *tv_subconnector_property;
	struct drm_property *tv_select_subconnector_property;
	struct drm_property *tv_mode_property;
	struct drm_property *tv_left_margin_property;
	struct drm_property *tv_right_margin_property;
	struct drm_property *tv_top_margin_property;
	struct drm_property *tv_bottom_margin_property;
	struct drm_property *tv_brightness_property;
	struct drm_property *tv_contrast_property;
	struct drm_property *tv_flicker_reduction_property;
	struct drm_property *tv_overscan_property;
	struct drm_property *tv_saturation_property;
	struct drm_property *tv_hue_property;

	
	struct drm_property *scaling_mode_property;
	struct drm_property *dithering_mode_property;
};

#define obj_to_crtc(x) container_of(x, struct drm_crtc, base)
#define obj_to_connector(x) container_of(x, struct drm_connector, base)
#define obj_to_encoder(x) container_of(x, struct drm_encoder, base)
#define obj_to_mode(x) container_of(x, struct drm_display_mode, base)
#define obj_to_fb(x) container_of(x, struct drm_framebuffer, base)
#define obj_to_property(x) container_of(x, struct drm_property, base)
#define obj_to_blob(x) container_of(x, struct drm_property_blob, base)


extern void drm_crtc_init(struct drm_device *dev,
			  struct drm_crtc *crtc,
			  const struct drm_crtc_funcs *funcs);
extern void drm_crtc_cleanup(struct drm_crtc *crtc);

extern void drm_connector_init(struct drm_device *dev,
			    struct drm_connector *connector,
			    const struct drm_connector_funcs *funcs,
			    int connector_type);

extern void drm_connector_cleanup(struct drm_connector *connector);

extern void drm_encoder_init(struct drm_device *dev,
			     struct drm_encoder *encoder,
			     const struct drm_encoder_funcs *funcs,
			     int encoder_type);

extern void drm_encoder_cleanup(struct drm_encoder *encoder);

extern char *drm_get_connector_name(struct drm_connector *connector);
extern char *drm_get_dpms_name(int val);
extern char *drm_get_dvi_i_subconnector_name(int val);
extern char *drm_get_dvi_i_select_name(int val);
extern char *drm_get_tv_subconnector_name(int val);
extern char *drm_get_tv_select_name(int val);
extern void drm_fb_release(struct drm_file *file_priv);
extern int drm_mode_group_init_legacy_group(struct drm_device *dev, struct drm_mode_group *group);
extern struct edid *drm_get_edid(struct drm_connector *connector,
				 struct i2c_adapter *adapter);
extern int drm_do_probe_ddc_edid(struct i2c_adapter *adapter,
				 unsigned char *buf, int len);
extern int drm_add_edid_modes(struct drm_connector *connector, struct edid *edid);
extern void drm_mode_probed_add(struct drm_connector *connector, struct drm_display_mode *mode);
extern void drm_mode_remove(struct drm_connector *connector, struct drm_display_mode *mode);
extern struct drm_display_mode *drm_mode_duplicate(struct drm_device *dev,
						   struct drm_display_mode *mode);
extern void drm_mode_debug_printmodeline(struct drm_display_mode *mode);
extern void drm_mode_config_init(struct drm_device *dev);
extern void drm_mode_config_cleanup(struct drm_device *dev);
extern void drm_mode_set_name(struct drm_display_mode *mode);
extern bool drm_mode_equal(struct drm_display_mode *mode1, struct drm_display_mode *mode2);
extern int drm_mode_width(struct drm_display_mode *mode);
extern int drm_mode_height(struct drm_display_mode *mode);


extern int drm_mode_attachmode_crtc(struct drm_device *dev,
				    struct drm_crtc *crtc,
				    struct drm_display_mode *mode);
extern int drm_mode_detachmode_crtc(struct drm_device *dev, struct drm_display_mode *mode);

extern struct drm_display_mode *drm_mode_create(struct drm_device *dev);
extern void drm_mode_destroy(struct drm_device *dev, struct drm_display_mode *mode);
extern void drm_mode_list_concat(struct list_head *head,
				 struct list_head *new);
extern void drm_mode_validate_size(struct drm_device *dev,
				   struct list_head *mode_list,
				   int maxX, int maxY, int maxPitch);
extern void drm_mode_prune_invalid(struct drm_device *dev,
				   struct list_head *mode_list, bool verbose);
extern void drm_mode_sort(struct list_head *mode_list);
extern int drm_mode_vrefresh(struct drm_display_mode *mode);
extern void drm_mode_set_crtcinfo(struct drm_display_mode *p,
				  int adjust_flags);
extern void drm_mode_connector_list_update(struct drm_connector *connector);
extern int drm_mode_connector_update_edid_property(struct drm_connector *connector,
						struct edid *edid);
extern int drm_connector_property_set_value(struct drm_connector *connector,
					 struct drm_property *property,
					 uint64_t value);
extern int drm_connector_property_get_value(struct drm_connector *connector,
					 struct drm_property *property,
					 uint64_t *value);
extern struct drm_display_mode *drm_crtc_mode_create(struct drm_device *dev);
extern void drm_framebuffer_set_object(struct drm_device *dev,
				       unsigned long handle);
extern int drm_framebuffer_init(struct drm_device *dev,
				struct drm_framebuffer *fb,
				const struct drm_framebuffer_funcs *funcs);
extern void drm_framebuffer_cleanup(struct drm_framebuffer *fb);
extern int drmfb_probe(struct drm_device *dev, struct drm_crtc *crtc);
extern int drmfb_remove(struct drm_device *dev, struct drm_framebuffer *fb);
extern void drm_crtc_probe_connector_modes(struct drm_device *dev, int maxX, int maxY);
extern bool drm_crtc_in_use(struct drm_crtc *crtc);

extern int drm_connector_attach_property(struct drm_connector *connector,
				      struct drm_property *property, uint64_t init_val);
extern struct drm_property *drm_property_create(struct drm_device *dev, int flags,
						const char *name, int num_values);
extern void drm_property_destroy(struct drm_device *dev, struct drm_property *property);
extern int drm_property_add_enum(struct drm_property *property, int index,
				 uint64_t value, const char *name);
extern int drm_mode_create_dvi_i_properties(struct drm_device *dev);
extern int drm_mode_create_tv_properties(struct drm_device *dev, int num_formats,
				     char *formats[]);
extern int drm_mode_create_scaling_mode_property(struct drm_device *dev);
extern int drm_mode_create_dithering_property(struct drm_device *dev);
extern char *drm_get_encoder_name(struct drm_encoder *encoder);

extern int drm_mode_connector_attach_encoder(struct drm_connector *connector,
					     struct drm_encoder *encoder);
extern void drm_mode_connector_detach_encoder(struct drm_connector *connector,
					   struct drm_encoder *encoder);
extern bool drm_mode_crtc_set_gamma_size(struct drm_crtc *crtc,
					 int gamma_size);
extern void *drm_mode_object_find(struct drm_device *dev, uint32_t id, uint32_t type);

extern int drm_mode_getresources(struct drm_device *dev,
				 void *data, struct drm_file *file_priv);

extern int drm_mode_getcrtc(struct drm_device *dev,
			    void *data, struct drm_file *file_priv);
extern int drm_mode_getconnector(struct drm_device *dev,
			      void *data, struct drm_file *file_priv);
extern int drm_mode_setcrtc(struct drm_device *dev,
			    void *data, struct drm_file *file_priv);
extern int drm_mode_cursor_ioctl(struct drm_device *dev,
				void *data, struct drm_file *file_priv);
extern int drm_mode_addfb(struct drm_device *dev,
			  void *data, struct drm_file *file_priv);
extern int drm_mode_rmfb(struct drm_device *dev,
			 void *data, struct drm_file *file_priv);
extern int drm_mode_getfb(struct drm_device *dev,
			  void *data, struct drm_file *file_priv);
extern int drm_mode_addmode_ioctl(struct drm_device *dev,
				  void *data, struct drm_file *file_priv);
extern int drm_mode_rmmode_ioctl(struct drm_device *dev,
				 void *data, struct drm_file *file_priv);
extern int drm_mode_attachmode_ioctl(struct drm_device *dev,
				     void *data, struct drm_file *file_priv);
extern int drm_mode_detachmode_ioctl(struct drm_device *dev,
				     void *data, struct drm_file *file_priv);

extern int drm_mode_getproperty_ioctl(struct drm_device *dev,
				      void *data, struct drm_file *file_priv);
extern int drm_mode_getblob_ioctl(struct drm_device *dev,
				  void *data, struct drm_file *file_priv);
extern int drm_mode_connector_property_set_ioctl(struct drm_device *dev,
					      void *data, struct drm_file *file_priv);
extern int drm_mode_hotplug_ioctl(struct drm_device *dev,
				  void *data, struct drm_file *file_priv);
extern int drm_mode_replacefb(struct drm_device *dev,
			      void *data, struct drm_file *file_priv);
extern int drm_mode_getencoder(struct drm_device *dev,
			       void *data, struct drm_file *file_priv);
extern int drm_mode_gamma_get_ioctl(struct drm_device *dev,
				    void *data, struct drm_file *file_priv);
extern int drm_mode_gamma_set_ioctl(struct drm_device *dev,
				    void *data, struct drm_file *file_priv);
extern bool drm_detect_hdmi_monitor(struct edid *edid);
extern struct drm_display_mode *drm_cvt_mode(struct drm_device *dev,
				int hdisplay, int vdisplay, int vrefresh,
				bool reduced, bool interlaced, bool margins);
extern struct drm_display_mode *drm_gtf_mode(struct drm_device *dev,
				int hdisplay, int vdisplay, int vrefresh,
				bool interlaced, int margins);
extern int drm_add_modes_noedid(struct drm_connector *connector,
				int hdisplay, int vdisplay);
#endif 
