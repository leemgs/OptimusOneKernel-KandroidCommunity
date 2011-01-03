

#ifndef ISX006_H
#define ISX006_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct isx006_reg isx006_regs;

enum isx006_width {
	BYTE_LEN = 1,
	WORD_LEN = 2,
	QUAD_LEN = 4,
};

struct isx006_register_address_value_pair {
	uint16_t register_address;
	uint32_t register_value;
	enum isx006_width register_length;
};

struct isx006_reg {
	const struct isx006_register_address_value_pair *init_reg_settings;
	uint16_t init_reg_settings_size;
	const struct isx006_register_address_value_pair *tuning_reg_settings;
	uint16_t tuning_reg_settings_size;
	
	const struct isx006_register_address_value_pair *prev_reg_settings;
	uint16_t prev_reg_settings_size;
	const struct isx006_register_address_value_pair *snap_reg_settings;
	uint16_t snap_reg_settings_size;
	
	const struct isx006_register_address_value_pair *af_normal_reg_settings;
	uint16_t af_normal_reg_settings_size;
	const struct isx006_register_address_value_pair *af_macro_reg_settings;
	uint16_t af_macro_reg_settings_size;
	const struct isx006_register_address_value_pair *af_manual_reg_settings;
	uint16_t af_manual_reg_settings_size;
	
	const struct isx006_register_address_value_pair *af_start_reg_settings;
	uint16_t af_start_reg_settings_size;

	const struct isx006_register_address_value_pair 
		*scene_auto_reg_settings;
	uint16_t scene_auto_reg_settings_size;	
	const struct isx006_register_address_value_pair 
		*scene_portrait_reg_settings;
	uint16_t scene_portrait_reg_settings_size;
	const struct isx006_register_address_value_pair 
		*scene_landscape_reg_settings;
	uint16_t scene_landscape_reg_settings_size;
	const struct isx006_register_address_value_pair 
		*scene_sports_reg_settings;
	uint16_t scene_sports_reg_settings_size;
	const struct isx006_register_address_value_pair 
		*scene_sunset_reg_settings;
	uint16_t scene_sunset_reg_settings_size;
	const struct isx006_register_address_value_pair 
		*scene_night_reg_settings;
	uint16_t scene_night_reg_settings_size;
  
	const struct isx006_register_address_value_pair *preview_addition_AP003_reg_settings;
	uint16_t preview_addition_AP003_reg_settings_size;
	const struct isx006_register_address_value_pair *preview_addition_AP001_reg_settings;
	uint16_t preview_addition_AP001_reg_settings_size;  
};


enum isx006_focus_mode {
	FOCUS_NORMAL,
	FOCUS_MACRO,
	FOCUS_AUTO,
	FOCUS_MANUAL,
};


enum isx006_wb_type {
	CAMERA_WB_MIN_MINUS_1,
	CAMERA_WB_AUTO = 1,  
	CAMERA_WB_CUSTOM,
	CAMERA_WB_INCANDESCENT,
	CAMERA_WB_FLUORESCENT,
	CAMERA_WB_DAYLIGHT,
	CAMERA_WB_CLOUDY_DAYLIGHT,
	CAMERA_WB_TWILIGHT,
	CAMERA_WB_SHADE,
	CAMERA_WB_MAX_PLUS_1
};

enum isx006_antibanding_type {
	CAMERA_ANTIBANDING_OFF,
	CAMERA_ANTIBANDING_60HZ,
	CAMERA_ANTIBANDING_50HZ,
	CAMERA_ANTIBANDING_AUTO,
	CAMERA_MAX_ANTIBANDING,
};


enum isx006_iso_value {
	CAMERA_ISO_AUTO = 0,
	CAMERA_ISO_DEBLUR,
	CAMERA_ISO_100,
	CAMERA_ISO_200,
	CAMERA_ISO_400,
	CAMERA_ISO_800,
	CAMERA_ISO_MAX
};


enum {
	CAMERA_SCENE_AUTO = 1,
	CAMERA_SCENE_PORTRAIT,
	CAMERA_SCENE_LANDSCAPE,
	CAMERA_SCENE_SPORTS,
	CAMERA_SCENE_NIGHT,
	CAMERA_SCENE_SUNSET,
};

#endif 
