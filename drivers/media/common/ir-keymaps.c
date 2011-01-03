


#include <linux/module.h>

#include <linux/input.h>
#include <media/ir-common.h>


static struct ir_scancode ir_codes_empty[] = {
	{ 0x2a, KEY_COFFEE },
};

struct ir_scancode_table ir_codes_empty_table = {
	.scan = ir_codes_empty,
	.size = ARRAY_SIZE(ir_codes_empty),
};
EXPORT_SYMBOL_GPL(ir_codes_empty_table);


static struct ir_scancode ir_codes_proteus_2309[] = {
	
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x5c, KEY_POWER },		
	{ 0x20, KEY_ZOOM },		
	{ 0x0f, KEY_BACKSPACE },	
	{ 0x1b, KEY_ENTER },		
	{ 0x41, KEY_RECORD },		
	{ 0x43, KEY_STOP },		
	{ 0x16, KEY_S },
	{ 0x1a, KEY_POWER2 },		
	{ 0x2e, KEY_RED },
	{ 0x1f, KEY_CHANNELDOWN },	
	{ 0x1c, KEY_CHANNELUP },	
	{ 0x10, KEY_VOLUMEDOWN },	
	{ 0x1e, KEY_VOLUMEUP },		
	{ 0x14, KEY_F1 },
};

struct ir_scancode_table ir_codes_proteus_2309_table = {
	.scan = ir_codes_proteus_2309,
	.size = ARRAY_SIZE(ir_codes_proteus_2309),
};
EXPORT_SYMBOL_GPL(ir_codes_proteus_2309_table);


static struct ir_scancode ir_codes_avermedia_dvbt[] = {
	{ 0x28, KEY_0 },		
	{ 0x22, KEY_1 },		
	{ 0x12, KEY_2 },		
	{ 0x32, KEY_3 },		
	{ 0x24, KEY_4 },		
	{ 0x14, KEY_5 },		
	{ 0x34, KEY_6 },		
	{ 0x26, KEY_7 },		
	{ 0x16, KEY_8 },		
	{ 0x36, KEY_9 },		

	{ 0x20, KEY_LIST },		
	{ 0x10, KEY_TEXT },		
	{ 0x00, KEY_POWER },		
	{ 0x04, KEY_AUDIO },		
	{ 0x06, KEY_ZOOM },		
	{ 0x18, KEY_VIDEO },		
	{ 0x38, KEY_SEARCH },		
	{ 0x08, KEY_INFO },		
	{ 0x2a, KEY_REWIND },		
	{ 0x1a, KEY_FASTFORWARD },	
	{ 0x3a, KEY_RECORD },		
	{ 0x0a, KEY_MUTE },		
	{ 0x2c, KEY_RECORD },		
	{ 0x1c, KEY_PAUSE },		
	{ 0x3c, KEY_STOP },		
	{ 0x0c, KEY_PLAY },		
	{ 0x2e, KEY_RED },		
	{ 0x01, KEY_BLUE },		
	{ 0x0e, KEY_YELLOW },		
	{ 0x21, KEY_GREEN },		
	{ 0x11, KEY_CHANNELDOWN },	
	{ 0x31, KEY_CHANNELUP },	
	{ 0x1e, KEY_VOLUMEDOWN },	
	{ 0x3e, KEY_VOLUMEUP },		
};

struct ir_scancode_table ir_codes_avermedia_dvbt_table = {
	.scan = ir_codes_avermedia_dvbt,
	.size = ARRAY_SIZE(ir_codes_avermedia_dvbt),
};
EXPORT_SYMBOL_GPL(ir_codes_avermedia_dvbt_table);


static struct ir_scancode ir_codes_avermedia_m135a[] = {
	{ 0x00, KEY_POWER2 },
	{ 0x2e, KEY_DOT },		
	{ 0x01, KEY_MODE },		

	{ 0x05, KEY_1 },
	{ 0x06, KEY_2 },
	{ 0x07, KEY_3 },
	{ 0x09, KEY_4 },
	{ 0x0a, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x0d, KEY_7 },
	{ 0x0e, KEY_8 },
	{ 0x0f, KEY_9 },
	{ 0x11, KEY_0 },

	{ 0x13, KEY_RIGHT },		
	{ 0x12, KEY_LEFT },		

	{ 0x17, KEY_SLEEP },		
	{ 0x10, KEY_SHUFFLE },		

	

	{ 0x43, KEY_CHANNELUP },
	{ 0x42, KEY_CHANNELDOWN },
	{ 0x1f, KEY_VOLUMEUP },
	{ 0x1e, KEY_VOLUMEDOWN },
	{ 0x0c, KEY_ENTER },

	{ 0x14, KEY_MUTE },
	{ 0x08, KEY_AUDIO },

	{ 0x03, KEY_TEXT },
	{ 0x04, KEY_EPG },
	{ 0x2b, KEY_TV2 },		

	{ 0x1d, KEY_RED },
	{ 0x1c, KEY_YELLOW },
	{ 0x41, KEY_GREEN },
	{ 0x40, KEY_BLUE },

	{ 0x1a, KEY_PLAYPAUSE },
	{ 0x19, KEY_RECORD },
	{ 0x18, KEY_PLAY },
	{ 0x1b, KEY_STOP },
};

struct ir_scancode_table ir_codes_avermedia_m135a_table = {
	.scan = ir_codes_avermedia_m135a,
	.size = ARRAY_SIZE(ir_codes_avermedia_m135a),
};
EXPORT_SYMBOL_GPL(ir_codes_avermedia_m135a_table);


static struct ir_scancode ir_codes_avermedia_cardbus[] = {
	{ 0x00, KEY_POWER },
	{ 0x01, KEY_TUNER },		
	{ 0x03, KEY_TEXT },		
	{ 0x04, KEY_EPG },
	{ 0x05, KEY_1 },
	{ 0x06, KEY_2 },
	{ 0x07, KEY_3 },
	{ 0x08, KEY_AUDIO },
	{ 0x09, KEY_4 },
	{ 0x0a, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x0c, KEY_ZOOM },		
	{ 0x0d, KEY_7 },
	{ 0x0e, KEY_8 },
	{ 0x0f, KEY_9 },
	{ 0x10, KEY_PAGEUP },		
	{ 0x11, KEY_0 },
	{ 0x12, KEY_INFO },
	{ 0x13, KEY_AGAIN },		
	{ 0x14, KEY_MUTE },
	{ 0x15, KEY_EDIT },		
	{ 0x17, KEY_SAVE },		
	{ 0x18, KEY_PLAYPAUSE },
	{ 0x19, KEY_RECORD },
	{ 0x1a, KEY_PLAY },
	{ 0x1b, KEY_STOP },
	{ 0x1c, KEY_FASTFORWARD },
	{ 0x1d, KEY_REWIND },
	{ 0x1e, KEY_VOLUMEDOWN },
	{ 0x1f, KEY_VOLUMEUP },
	{ 0x22, KEY_SLEEP },		
	{ 0x23, KEY_ZOOM },		
	{ 0x26, KEY_SCREEN },		
	{ 0x27, KEY_ANGLE },		
	{ 0x28, KEY_SELECT },		
	{ 0x29, KEY_BLUE },		
	{ 0x2a, KEY_BACKSPACE },	
	{ 0x2b, KEY_MEDIA },		
	{ 0x2c, KEY_DOWN },
	{ 0x2e, KEY_DOT },
	{ 0x2f, KEY_TV },		
	{ 0x32, KEY_LEFT },
	{ 0x33, KEY_CLEAR },		
	{ 0x35, KEY_RED },		
	{ 0x36, KEY_UP },
	{ 0x37, KEY_HOME },		
	{ 0x39, KEY_GREEN },		
	{ 0x3d, KEY_YELLOW },		
	{ 0x3e, KEY_OK },		
	{ 0x3f, KEY_RIGHT },
	{ 0x40, KEY_NEXT },		
	{ 0x41, KEY_PREVIOUS },		
	{ 0x42, KEY_CHANNELDOWN },	
	{ 0x43, KEY_CHANNELUP },	
};

struct ir_scancode_table ir_codes_avermedia_cardbus_table = {
	.scan = ir_codes_avermedia_cardbus,
	.size = ARRAY_SIZE(ir_codes_avermedia_cardbus),
};
EXPORT_SYMBOL_GPL(ir_codes_avermedia_cardbus_table);


static struct ir_scancode ir_codes_apac_viewcomp[] = {

	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },
	{ 0x00, KEY_0 },
	{ 0x17, KEY_LAST },		
	{ 0x0a, KEY_LIST },		


	{ 0x1c, KEY_TUNER },		
	{ 0x15, KEY_SEARCH },		
	{ 0x12, KEY_POWER },		
	{ 0x1f, KEY_VOLUMEDOWN },	
	{ 0x1b, KEY_VOLUMEUP },		
	{ 0x1e, KEY_CHANNELDOWN },	
	{ 0x1a, KEY_CHANNELUP },	

	{ 0x11, KEY_VIDEO },		
	{ 0x0f, KEY_ZOOM },		
	{ 0x13, KEY_MUTE },		
	{ 0x10, KEY_TEXT },		

	{ 0x0d, KEY_STOP },		
	{ 0x0e, KEY_RECORD },		
	{ 0x1d, KEY_PLAYPAUSE },	
	{ 0x19, KEY_PLAY },		

	{ 0x16, KEY_GOTO },		
	{ 0x14, KEY_REFRESH },		
	{ 0x0c, KEY_KPPLUS },		
	{ 0x18, KEY_KPMINUS },		
};

struct ir_scancode_table ir_codes_apac_viewcomp_table = {
	.scan = ir_codes_apac_viewcomp,
	.size = ARRAY_SIZE(ir_codes_apac_viewcomp),
};
EXPORT_SYMBOL_GPL(ir_codes_apac_viewcomp_table);



static struct ir_scancode ir_codes_pixelview[] = {

	{ 0x1e, KEY_POWER },	
	{ 0x07, KEY_MEDIA },	
	{ 0x1c, KEY_SEARCH },	


	{ 0x03, KEY_TUNER },		

	{ 0x00, KEY_RECORD },
	{ 0x08, KEY_STOP },
	{ 0x11, KEY_PLAY },

	{ 0x1a, KEY_PLAYPAUSE },	
	{ 0x19, KEY_ZOOM },		
	{ 0x0f, KEY_TEXT },		

	{ 0x01, KEY_1 },
	{ 0x0b, KEY_2 },
	{ 0x1b, KEY_3 },
	{ 0x05, KEY_4 },
	{ 0x09, KEY_5 },
	{ 0x15, KEY_6 },
	{ 0x06, KEY_7 },
	{ 0x0a, KEY_8 },
	{ 0x12, KEY_9 },
	{ 0x02, KEY_0 },
	{ 0x10, KEY_LAST },		
	{ 0x13, KEY_LIST },		

	{ 0x1f, KEY_CHANNELUP },	
	{ 0x17, KEY_CHANNELDOWN },	
	{ 0x16, KEY_VOLUMEUP },		
	{ 0x14, KEY_VOLUMEDOWN },	

	{ 0x04, KEY_KPMINUS },		
	{ 0x0e, KEY_SETUP },		
	{ 0x0c, KEY_KPPLUS },		

	{ 0x0d, KEY_GOTO },		
	{ 0x1d, KEY_REFRESH },		
	{ 0x18, KEY_MUTE },		
};

struct ir_scancode_table ir_codes_pixelview_table = {
	.scan = ir_codes_pixelview,
	.size = ARRAY_SIZE(ir_codes_pixelview),
};
EXPORT_SYMBOL_GPL(ir_codes_pixelview_table);


static struct ir_scancode ir_codes_pixelview_new[] = {
	{ 0x3c, KEY_TIME },		
	{ 0x12, KEY_POWER },

	{ 0x3d, KEY_1 },
	{ 0x38, KEY_2 },
	{ 0x18, KEY_3 },
	{ 0x35, KEY_4 },
	{ 0x39, KEY_5 },
	{ 0x15, KEY_6 },
	{ 0x36, KEY_7 },
	{ 0x3a, KEY_8 },
	{ 0x1e, KEY_9 },
	{ 0x3e, KEY_0 },

	{ 0x1c, KEY_AGAIN },		
	{ 0x3f, KEY_MEDIA },		
	{ 0x1f, KEY_LAST },		
	{ 0x1b, KEY_MUTE },

	{ 0x17, KEY_CHANNELDOWN },
	{ 0x16, KEY_CHANNELUP },
	{ 0x10, KEY_VOLUMEUP },
	{ 0x14, KEY_VOLUMEDOWN },
	{ 0x13, KEY_ZOOM },

	{ 0x19, KEY_CAMERA },		
	{ 0x1a, KEY_SEARCH },		

	{ 0x37, KEY_REWIND },		
	{ 0x32, KEY_RECORD },		
	{ 0x33, KEY_FORWARD },		
	{ 0x11, KEY_STOP },		
	{ 0x3b, KEY_PLAY },		
	{ 0x30, KEY_PLAYPAUSE },	

	{ 0x31, KEY_TV },
	{ 0x34, KEY_RADIO },
};

struct ir_scancode_table ir_codes_pixelview_new_table = {
	.scan = ir_codes_pixelview_new,
	.size = ARRAY_SIZE(ir_codes_pixelview_new),
};
EXPORT_SYMBOL_GPL(ir_codes_pixelview_new_table);

static struct ir_scancode ir_codes_nebula[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },
	{ 0x0a, KEY_TV },
	{ 0x0b, KEY_AUX },
	{ 0x0c, KEY_DVD },
	{ 0x0d, KEY_POWER },
	{ 0x0e, KEY_MHP },	
	{ 0x0f, KEY_AUDIO },
	{ 0x10, KEY_INFO },
	{ 0x11, KEY_F13 },	
	{ 0x12, KEY_F14 },	
	{ 0x13, KEY_EPG },
	{ 0x14, KEY_EXIT },
	{ 0x15, KEY_MENU },
	{ 0x16, KEY_UP },
	{ 0x17, KEY_DOWN },
	{ 0x18, KEY_LEFT },
	{ 0x19, KEY_RIGHT },
	{ 0x1a, KEY_ENTER },
	{ 0x1b, KEY_CHANNELUP },
	{ 0x1c, KEY_CHANNELDOWN },
	{ 0x1d, KEY_VOLUMEUP },
	{ 0x1e, KEY_VOLUMEDOWN },
	{ 0x1f, KEY_RED },
	{ 0x20, KEY_GREEN },
	{ 0x21, KEY_YELLOW },
	{ 0x22, KEY_BLUE },
	{ 0x23, KEY_SUBTITLE },
	{ 0x24, KEY_F15 },	
	{ 0x25, KEY_TEXT },
	{ 0x26, KEY_MUTE },
	{ 0x27, KEY_REWIND },
	{ 0x28, KEY_STOP },
	{ 0x29, KEY_PLAY },
	{ 0x2a, KEY_FASTFORWARD },
	{ 0x2b, KEY_F16 },	
	{ 0x2c, KEY_PAUSE },
	{ 0x2d, KEY_PLAY },
	{ 0x2e, KEY_RECORD },
	{ 0x2f, KEY_F17 },	
	{ 0x30, KEY_KPPLUS },	
	{ 0x31, KEY_KPMINUS },	
	{ 0x32, KEY_F18 },	
	{ 0x33, KEY_F19 },	
	{ 0x34, KEY_EMAIL },
	{ 0x35, KEY_PHONE },
	{ 0x36, KEY_PC },
};

struct ir_scancode_table ir_codes_nebula_table = {
	.scan = ir_codes_nebula,
	.size = ARRAY_SIZE(ir_codes_nebula),
};
EXPORT_SYMBOL_GPL(ir_codes_nebula_table);


static struct ir_scancode ir_codes_dntv_live_dvb_t[] = {
	{ 0x00, KEY_ESC },		
	
	{ 0x0a, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0b, KEY_TUNER },		
	{ 0x0c, KEY_SEARCH },		
	{ 0x0d, KEY_STOP },
	{ 0x0e, KEY_PAUSE },
	{ 0x0f, KEY_LIST },		

	{ 0x10, KEY_MUTE },
	{ 0x11, KEY_REWIND },		
	{ 0x12, KEY_POWER },
	{ 0x13, KEY_CAMERA },		
	{ 0x14, KEY_AUDIO },		
	{ 0x15, KEY_CLEAR },		
	{ 0x16, KEY_PLAY },
	{ 0x17, KEY_ENTER },
	{ 0x18, KEY_ZOOM },		
	{ 0x19, KEY_FASTFORWARD },	
	{ 0x1a, KEY_CHANNELUP },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x1c, KEY_INFO },		
	{ 0x1d, KEY_RECORD },		
	{ 0x1e, KEY_CHANNELDOWN },
	{ 0x1f, KEY_VOLUMEDOWN },
};

struct ir_scancode_table ir_codes_dntv_live_dvb_t_table = {
	.scan = ir_codes_dntv_live_dvb_t,
	.size = ARRAY_SIZE(ir_codes_dntv_live_dvb_t),
};
EXPORT_SYMBOL_GPL(ir_codes_dntv_live_dvb_t_table);




static struct ir_scancode ir_codes_iodata_bctv7e[] = {
	{ 0x40, KEY_TV },
	{ 0x20, KEY_RADIO },		
	{ 0x60, KEY_EPG },
	{ 0x00, KEY_POWER },

	
	{ 0x44, KEY_0 },		
	{ 0x50, KEY_1 },
	{ 0x30, KEY_2 },
	{ 0x70, KEY_3 },
	{ 0x48, KEY_4 },
	{ 0x28, KEY_5 },
	{ 0x68, KEY_6 },
	{ 0x58, KEY_7 },
	{ 0x38, KEY_8 },
	{ 0x78, KEY_9 },

	{ 0x10, KEY_L },		
	{ 0x08, KEY_TIME },		

	{ 0x18, KEY_PLAYPAUSE },	

	{ 0x24, KEY_ENTER },		
	{ 0x64, KEY_ESC },		
	{ 0x04, KEY_M },		

	{ 0x54, KEY_VIDEO },
	{ 0x34, KEY_CHANNELUP },
	{ 0x74, KEY_VOLUMEUP },
	{ 0x14, KEY_MUTE },

	{ 0x4c, KEY_VCR },		
	{ 0x2c, KEY_CHANNELDOWN },
	{ 0x6c, KEY_VOLUMEDOWN },
	{ 0x0c, KEY_ZOOM },

	{ 0x5c, KEY_PAUSE },
	{ 0x3c, KEY_RED },		
	{ 0x7c, KEY_RECORD },		
	{ 0x1c, KEY_STOP },

	{ 0x41, KEY_REWIND },		
	{ 0x21, KEY_PLAY },
	{ 0x61, KEY_FASTFORWARD },	
	{ 0x01, KEY_NEXT },		
};

struct ir_scancode_table ir_codes_iodata_bctv7e_table = {
	.scan = ir_codes_iodata_bctv7e,
	.size = ARRAY_SIZE(ir_codes_iodata_bctv7e),
};
EXPORT_SYMBOL_GPL(ir_codes_iodata_bctv7e_table);




static struct ir_scancode ir_codes_adstech_dvb_t_pci[] = {
	
	{ 0x4d, KEY_0 },
	{ 0x57, KEY_1 },
	{ 0x4f, KEY_2 },
	{ 0x53, KEY_3 },
	{ 0x56, KEY_4 },
	{ 0x4e, KEY_5 },
	{ 0x5e, KEY_6 },
	{ 0x54, KEY_7 },
	{ 0x4c, KEY_8 },
	{ 0x5c, KEY_9 },

	{ 0x5b, KEY_POWER },
	{ 0x5f, KEY_MUTE },
	{ 0x55, KEY_GOTO },
	{ 0x5d, KEY_SEARCH },
	{ 0x17, KEY_EPG },		
	{ 0x1f, KEY_MENU },
	{ 0x0f, KEY_UP },
	{ 0x46, KEY_DOWN },
	{ 0x16, KEY_LEFT },
	{ 0x1e, KEY_RIGHT },
	{ 0x0e, KEY_SELECT },		
	{ 0x5a, KEY_INFO },
	{ 0x52, KEY_EXIT },
	{ 0x59, KEY_PREVIOUS },
	{ 0x51, KEY_NEXT },
	{ 0x58, KEY_REWIND },
	{ 0x50, KEY_FORWARD },
	{ 0x44, KEY_PLAYPAUSE },
	{ 0x07, KEY_STOP },
	{ 0x1b, KEY_RECORD },
	{ 0x13, KEY_TUNER },		
	{ 0x0a, KEY_A },
	{ 0x12, KEY_B },
	{ 0x03, KEY_PROG1 },		
	{ 0x01, KEY_PROG2 },		
	{ 0x00, KEY_PROG3 },		
	{ 0x06, KEY_DVD },
	{ 0x48, KEY_AUX },		
	{ 0x40, KEY_VIDEO },
	{ 0x19, KEY_AUDIO },		
	{ 0x0b, KEY_CHANNELUP },
	{ 0x08, KEY_CHANNELDOWN },
	{ 0x15, KEY_VOLUMEUP },
	{ 0x1c, KEY_VOLUMEDOWN },
};

struct ir_scancode_table ir_codes_adstech_dvb_t_pci_table = {
	.scan = ir_codes_adstech_dvb_t_pci,
	.size = ARRAY_SIZE(ir_codes_adstech_dvb_t_pci),
};
EXPORT_SYMBOL_GPL(ir_codes_adstech_dvb_t_pci_table);





static struct ir_scancode ir_codes_msi_tvanywhere[] = {
	
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0c, KEY_MUTE },
	{ 0x0f, KEY_SCREEN },		
	{ 0x10, KEY_FN },		
	{ 0x11, KEY_TIME },		
	{ 0x12, KEY_POWER },
	{ 0x13, KEY_MEDIA },		
	{ 0x14, KEY_SLOW },
	{ 0x16, KEY_REWIND },		
	{ 0x17, KEY_ENTER },		
	{ 0x18, KEY_FASTFORWARD },	
	{ 0x1a, KEY_CHANNELUP },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x1e, KEY_CHANNELDOWN },
	{ 0x1f, KEY_VOLUMEDOWN },
};

struct ir_scancode_table ir_codes_msi_tvanywhere_table = {
	.scan = ir_codes_msi_tvanywhere,
	.size = ARRAY_SIZE(ir_codes_msi_tvanywhere),
};
EXPORT_SYMBOL_GPL(ir_codes_msi_tvanywhere_table);





static struct ir_scancode ir_codes_msi_tvanywhere_plus[] = {



	{ 0x01, KEY_1 },		
	{ 0x0b, KEY_2 },		
	{ 0x1b, KEY_3 },		
	{ 0x05, KEY_4 },		
	{ 0x09, KEY_5 },		
	{ 0x15, KEY_6 },		
	{ 0x06, KEY_7 },		
	{ 0x0a, KEY_8 },		
	{ 0x12, KEY_9 },		
	{ 0x02, KEY_0 },		
	{ 0x10, KEY_KPPLUS },		
	{ 0x13, KEY_AGAIN },		

	{ 0x1e, KEY_POWER },		
	{ 0x07, KEY_TUNER },		
	{ 0x1c, KEY_SEARCH },		
	{ 0x18, KEY_MUTE },		

	{ 0x03, KEY_RADIO },		
	
	{ 0x3f, KEY_RIGHT },		
	{ 0x37, KEY_LEFT },		
	{ 0x2c, KEY_UP },		
	{ 0x24, KEY_DOWN },		

	{ 0x00, KEY_RECORD },		
	{ 0x08, KEY_STOP },		
	{ 0x11, KEY_PLAY },		

	{ 0x0f, KEY_CLOSE },		
	{ 0x19, KEY_ZOOM },		
	{ 0x1a, KEY_CAMERA },		
	{ 0x0d, KEY_LANGUAGE },		

	{ 0x14, KEY_VOLUMEDOWN },	
	{ 0x16, KEY_VOLUMEUP },		
	{ 0x17, KEY_CHANNELDOWN },	
	{ 0x1f, KEY_CHANNELUP },	

	{ 0x04, KEY_REWIND },		
	{ 0x0e, KEY_MENU },		
	{ 0x0c, KEY_FASTFORWARD },	
	{ 0x1d, KEY_RESTART },		
};

struct ir_scancode_table ir_codes_msi_tvanywhere_plus_table = {
	.scan = ir_codes_msi_tvanywhere_plus,
	.size = ARRAY_SIZE(ir_codes_msi_tvanywhere_plus),
};
EXPORT_SYMBOL_GPL(ir_codes_msi_tvanywhere_plus_table);




static struct ir_scancode ir_codes_cinergy_1400[] = {
	{ 0x01, KEY_POWER },
	{ 0x02, KEY_1 },
	{ 0x03, KEY_2 },
	{ 0x04, KEY_3 },
	{ 0x05, KEY_4 },
	{ 0x06, KEY_5 },
	{ 0x07, KEY_6 },
	{ 0x08, KEY_7 },
	{ 0x09, KEY_8 },
	{ 0x0a, KEY_9 },
	{ 0x0c, KEY_0 },

	{ 0x0b, KEY_VIDEO },
	{ 0x0d, KEY_REFRESH },
	{ 0x0e, KEY_SELECT },
	{ 0x0f, KEY_EPG },
	{ 0x10, KEY_UP },
	{ 0x11, KEY_LEFT },
	{ 0x12, KEY_OK },
	{ 0x13, KEY_RIGHT },
	{ 0x14, KEY_DOWN },
	{ 0x15, KEY_TEXT },
	{ 0x16, KEY_INFO },

	{ 0x17, KEY_RED },
	{ 0x18, KEY_GREEN },
	{ 0x19, KEY_YELLOW },
	{ 0x1a, KEY_BLUE },

	{ 0x1b, KEY_CHANNELUP },
	{ 0x1c, KEY_VOLUMEUP },
	{ 0x1d, KEY_MUTE },
	{ 0x1e, KEY_VOLUMEDOWN },
	{ 0x1f, KEY_CHANNELDOWN },

	{ 0x40, KEY_PAUSE },
	{ 0x4c, KEY_PLAY },
	{ 0x58, KEY_RECORD },
	{ 0x54, KEY_PREVIOUS },
	{ 0x48, KEY_STOP },
	{ 0x5c, KEY_NEXT },
};

struct ir_scancode_table ir_codes_cinergy_1400_table = {
	.scan = ir_codes_cinergy_1400,
	.size = ARRAY_SIZE(ir_codes_cinergy_1400),
};
EXPORT_SYMBOL_GPL(ir_codes_cinergy_1400_table);




static struct ir_scancode ir_codes_avertv_303[] = {
	{ 0x2a, KEY_1 },
	{ 0x32, KEY_2 },
	{ 0x3a, KEY_3 },
	{ 0x4a, KEY_4 },
	{ 0x52, KEY_5 },
	{ 0x5a, KEY_6 },
	{ 0x6a, KEY_7 },
	{ 0x72, KEY_8 },
	{ 0x7a, KEY_9 },
	{ 0x0e, KEY_0 },

	{ 0x02, KEY_POWER },
	{ 0x22, KEY_VIDEO },
	{ 0x42, KEY_AUDIO },
	{ 0x62, KEY_ZOOM },
	{ 0x0a, KEY_TV },
	{ 0x12, KEY_CD },
	{ 0x1a, KEY_TEXT },

	{ 0x16, KEY_SUBTITLE },
	{ 0x1e, KEY_REWIND },
	{ 0x06, KEY_PRINT },

	{ 0x2e, KEY_SEARCH },
	{ 0x36, KEY_SLEEP },
	{ 0x3e, KEY_SHUFFLE },
	{ 0x26, KEY_MUTE },

	{ 0x4e, KEY_RECORD },
	{ 0x56, KEY_PAUSE },
	{ 0x5e, KEY_STOP },
	{ 0x46, KEY_PLAY },

	{ 0x6e, KEY_RED },
	{ 0x0b, KEY_GREEN },
	{ 0x66, KEY_YELLOW },
	{ 0x03, KEY_BLUE },

	{ 0x76, KEY_LEFT },
	{ 0x7e, KEY_RIGHT },
	{ 0x13, KEY_DOWN },
	{ 0x1b, KEY_UP },
};

struct ir_scancode_table ir_codes_avertv_303_table = {
	.scan = ir_codes_avertv_303,
	.size = ARRAY_SIZE(ir_codes_avertv_303),
};
EXPORT_SYMBOL_GPL(ir_codes_avertv_303_table);




static struct ir_scancode ir_codes_dntv_live_dvbt_pro[] = {
	{ 0x16, KEY_POWER },
	{ 0x5b, KEY_HOME },

	{ 0x55, KEY_TV },		
	{ 0x58, KEY_TUNER },		
	{ 0x5a, KEY_RADIO },		
	{ 0x59, KEY_DVD },		
	{ 0x03, KEY_1 },
	{ 0x01, KEY_2 },
	{ 0x06, KEY_3 },
	{ 0x09, KEY_4 },
	{ 0x1d, KEY_5 },
	{ 0x1f, KEY_6 },
	{ 0x0d, KEY_7 },
	{ 0x19, KEY_8 },
	{ 0x1b, KEY_9 },
	{ 0x0c, KEY_CANCEL },
	{ 0x15, KEY_0 },
	{ 0x4a, KEY_CLEAR },
	{ 0x13, KEY_BACK },
	{ 0x00, KEY_TAB },
	{ 0x4b, KEY_UP },
	{ 0x4e, KEY_LEFT },
	{ 0x4f, KEY_OK },
	{ 0x52, KEY_RIGHT },
	{ 0x51, KEY_DOWN },
	{ 0x1e, KEY_VOLUMEUP },
	{ 0x0a, KEY_VOLUMEDOWN },
	{ 0x02, KEY_CHANNELDOWN },
	{ 0x05, KEY_CHANNELUP },
	{ 0x11, KEY_RECORD },
	{ 0x14, KEY_PLAY },
	{ 0x4c, KEY_PAUSE },
	{ 0x1a, KEY_STOP },
	{ 0x40, KEY_REWIND },
	{ 0x12, KEY_FASTFORWARD },
	{ 0x41, KEY_PREVIOUSSONG },	
	{ 0x42, KEY_NEXTSONG },		
	{ 0x54, KEY_CAMERA },		
	{ 0x50, KEY_LANGUAGE },		
	{ 0x47, KEY_TV2 },		
	{ 0x4d, KEY_SCREEN },
	{ 0x43, KEY_SUBTITLE },
	{ 0x10, KEY_MUTE },
	{ 0x49, KEY_AUDIO },		
	{ 0x07, KEY_SLEEP },
	{ 0x08, KEY_VIDEO },		
	{ 0x0e, KEY_PREVIOUS },		
	{ 0x45, KEY_ZOOM },		
	{ 0x46, KEY_ANGLE },		
	{ 0x56, KEY_RED },
	{ 0x57, KEY_GREEN },
	{ 0x5c, KEY_YELLOW },
	{ 0x5d, KEY_BLUE },
};

struct ir_scancode_table ir_codes_dntv_live_dvbt_pro_table = {
	.scan = ir_codes_dntv_live_dvbt_pro,
	.size = ARRAY_SIZE(ir_codes_dntv_live_dvbt_pro),
};
EXPORT_SYMBOL_GPL(ir_codes_dntv_live_dvbt_pro_table);

static struct ir_scancode ir_codes_em_terratec[] = {
	{ 0x01, KEY_CHANNEL },
	{ 0x02, KEY_SELECT },
	{ 0x03, KEY_MUTE },
	{ 0x04, KEY_POWER },
	{ 0x05, KEY_1 },
	{ 0x06, KEY_2 },
	{ 0x07, KEY_3 },
	{ 0x08, KEY_CHANNELUP },
	{ 0x09, KEY_4 },
	{ 0x0a, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x0c, KEY_CHANNELDOWN },
	{ 0x0d, KEY_7 },
	{ 0x0e, KEY_8 },
	{ 0x0f, KEY_9 },
	{ 0x10, KEY_VOLUMEUP },
	{ 0x11, KEY_0 },
	{ 0x12, KEY_MENU },
	{ 0x13, KEY_PRINT },
	{ 0x14, KEY_VOLUMEDOWN },
	{ 0x16, KEY_PAUSE },
	{ 0x18, KEY_RECORD },
	{ 0x19, KEY_REWIND },
	{ 0x1a, KEY_PLAY },
	{ 0x1b, KEY_FORWARD },
	{ 0x1c, KEY_BACKSPACE },
	{ 0x1e, KEY_STOP },
	{ 0x40, KEY_ZOOM },
};

struct ir_scancode_table ir_codes_em_terratec_table = {
	.scan = ir_codes_em_terratec,
	.size = ARRAY_SIZE(ir_codes_em_terratec),
};
EXPORT_SYMBOL_GPL(ir_codes_em_terratec_table);

static struct ir_scancode ir_codes_pinnacle_grey[] = {
	{ 0x3a, KEY_0 },
	{ 0x31, KEY_1 },
	{ 0x32, KEY_2 },
	{ 0x33, KEY_3 },
	{ 0x34, KEY_4 },
	{ 0x35, KEY_5 },
	{ 0x36, KEY_6 },
	{ 0x37, KEY_7 },
	{ 0x38, KEY_8 },
	{ 0x39, KEY_9 },

	{ 0x2f, KEY_POWER },

	{ 0x2e, KEY_P },
	{ 0x1f, KEY_L },
	{ 0x2b, KEY_I },

	{ 0x2d, KEY_SCREEN },
	{ 0x1e, KEY_ZOOM },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x0f, KEY_VOLUMEDOWN },
	{ 0x17, KEY_CHANNELUP },
	{ 0x1c, KEY_CHANNELDOWN },
	{ 0x25, KEY_INFO },

	{ 0x3c, KEY_MUTE },

	{ 0x3d, KEY_LEFT },
	{ 0x3b, KEY_RIGHT },

	{ 0x3f, KEY_UP },
	{ 0x3e, KEY_DOWN },
	{ 0x1a, KEY_ENTER },

	{ 0x1d, KEY_MENU },
	{ 0x19, KEY_AGAIN },
	{ 0x16, KEY_PREVIOUSSONG },
	{ 0x13, KEY_NEXTSONG },
	{ 0x15, KEY_PAUSE },
	{ 0x0e, KEY_REWIND },
	{ 0x0d, KEY_PLAY },
	{ 0x0b, KEY_STOP },
	{ 0x07, KEY_FORWARD },
	{ 0x27, KEY_RECORD },
	{ 0x26, KEY_TUNER },
	{ 0x29, KEY_TEXT },
	{ 0x2a, KEY_MEDIA },
	{ 0x18, KEY_EPG },
};

struct ir_scancode_table ir_codes_pinnacle_grey_table = {
	.scan = ir_codes_pinnacle_grey,
	.size = ARRAY_SIZE(ir_codes_pinnacle_grey),
};
EXPORT_SYMBOL_GPL(ir_codes_pinnacle_grey_table);

static struct ir_scancode ir_codes_flyvideo[] = {
	{ 0x0f, KEY_0 },
	{ 0x03, KEY_1 },
	{ 0x04, KEY_2 },
	{ 0x05, KEY_3 },
	{ 0x07, KEY_4 },
	{ 0x08, KEY_5 },
	{ 0x09, KEY_6 },
	{ 0x0b, KEY_7 },
	{ 0x0c, KEY_8 },
	{ 0x0d, KEY_9 },

	{ 0x0e, KEY_MODE },	
	{ 0x11, KEY_VIDEO },	
	{ 0x15, KEY_AUDIO },	
	{ 0x00, KEY_POWER },	
	{ 0x18, KEY_TUNER },	
	{ 0x02, KEY_ZOOM },	
	{ 0x1a, KEY_LANGUAGE },	
	{ 0x1b, KEY_MUTE },	
	{ 0x14, KEY_VOLUMEUP },	
	{ 0x17, KEY_VOLUMEDOWN },
	{ 0x12, KEY_CHANNELUP },
	{ 0x13, KEY_CHANNELDOWN },
	{ 0x06, KEY_AGAIN },	
	{ 0x10, KEY_ENTER },	

	{ 0x19, KEY_BACK },	
	{ 0x1f, KEY_FORWARD },	
	{ 0x0a, KEY_ANGLE },	
};

struct ir_scancode_table ir_codes_flyvideo_table = {
	.scan = ir_codes_flyvideo,
	.size = ARRAY_SIZE(ir_codes_flyvideo),
};
EXPORT_SYMBOL_GPL(ir_codes_flyvideo_table);

static struct ir_scancode ir_codes_flydvb[] = {
	{ 0x01, KEY_ZOOM },		
	{ 0x00, KEY_POWER },		

	{ 0x03, KEY_1 },
	{ 0x04, KEY_2 },
	{ 0x05, KEY_3 },
	{ 0x07, KEY_4 },
	{ 0x08, KEY_5 },
	{ 0x09, KEY_6 },
	{ 0x0b, KEY_7 },
	{ 0x0c, KEY_8 },
	{ 0x0d, KEY_9 },
	{ 0x06, KEY_AGAIN },		
	{ 0x0f, KEY_0 },
	{ 0x10, KEY_MUTE },		
	{ 0x02, KEY_RADIO },		
	{ 0x1b, KEY_LANGUAGE },		

	{ 0x14, KEY_VOLUMEUP },		
	{ 0x17, KEY_VOLUMEDOWN },	
	{ 0x12, KEY_CHANNELUP },	
	{ 0x13, KEY_CHANNELDOWN },	
	{ 0x1d, KEY_ENTER },		

	{ 0x1a, KEY_MODE },		
	{ 0x18, KEY_TUNER },		

	{ 0x1e, KEY_RECORD },		
	{ 0x15, KEY_ANGLE },		
	{ 0x1c, KEY_PAUSE },		
	{ 0x19, KEY_BACK },		
	{ 0x0a, KEY_PLAYPAUSE },	
	{ 0x1f, KEY_FORWARD },		
	{ 0x16, KEY_PREVIOUS },		
	{ 0x11, KEY_STOP },		
	{ 0x0e, KEY_NEXT },		
};

struct ir_scancode_table ir_codes_flydvb_table = {
	.scan = ir_codes_flydvb,
	.size = ARRAY_SIZE(ir_codes_flydvb),
};
EXPORT_SYMBOL_GPL(ir_codes_flydvb_table);

static struct ir_scancode ir_codes_cinergy[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0a, KEY_POWER },
	{ 0x0b, KEY_PROG1 },		
	{ 0x0c, KEY_ZOOM },		
	{ 0x0d, KEY_CHANNELUP },	
	{ 0x0e, KEY_CHANNELDOWN },	
	{ 0x0f, KEY_VOLUMEUP },
	{ 0x10, KEY_VOLUMEDOWN },
	{ 0x11, KEY_TUNER },		
	{ 0x12, KEY_NUMLOCK },		
	{ 0x13, KEY_AUDIO },		
	{ 0x14, KEY_MUTE },
	{ 0x15, KEY_UP },
	{ 0x16, KEY_DOWN },
	{ 0x17, KEY_LEFT },
	{ 0x18, KEY_RIGHT },
	{ 0x19, BTN_LEFT, },
	{ 0x1a, BTN_RIGHT, },
	{ 0x1b, KEY_WWW },		
	{ 0x1c, KEY_REWIND },
	{ 0x1d, KEY_FORWARD },
	{ 0x1e, KEY_RECORD },
	{ 0x1f, KEY_PLAY },
	{ 0x20, KEY_PREVIOUSSONG },
	{ 0x21, KEY_NEXTSONG },
	{ 0x22, KEY_PAUSE },
	{ 0x23, KEY_STOP },
};

struct ir_scancode_table ir_codes_cinergy_table = {
	.scan = ir_codes_cinergy,
	.size = ARRAY_SIZE(ir_codes_cinergy),
};
EXPORT_SYMBOL_GPL(ir_codes_cinergy_table);


static struct ir_scancode ir_codes_eztv[] = {
	{ 0x12, KEY_POWER },
	{ 0x01, KEY_TV },	
	{ 0x15, KEY_DVD },	
	{ 0x17, KEY_AUDIO },	
				

	{ 0x1b, KEY_MUTE },	
	{ 0x02, KEY_LANGUAGE },	
	{ 0x1e, KEY_SUBTITLE },	
	{ 0x16, KEY_ZOOM },	
	{ 0x1c, KEY_VIDEO },	
	{ 0x1d, KEY_RESTART },	
	{ 0x2f, KEY_SEARCH },	
	{ 0x30, KEY_CHANNEL },	

	{ 0x31, KEY_HELP },	
	{ 0x32, KEY_MODE },	
	{ 0x33, KEY_ESC },	

	{ 0x0c, KEY_UP },	
	{ 0x10, KEY_DOWN },	
	{ 0x08, KEY_LEFT },	
	{ 0x04, KEY_RIGHT },	
	{ 0x03, KEY_SELECT },	

	{ 0x1f, KEY_REWIND },	
	{ 0x20, KEY_PLAYPAUSE },
	{ 0x29, KEY_FORWARD },	
	{ 0x14, KEY_AGAIN },	
	{ 0x2b, KEY_RECORD },	
	{ 0x2c, KEY_STOP },	
	{ 0x2d, KEY_PLAY },	
	{ 0x2e, KEY_CAMERA },	

	{ 0x00, KEY_0 },
	{ 0x05, KEY_1 },
	{ 0x06, KEY_2 },
	{ 0x07, KEY_3 },
	{ 0x09, KEY_4 },
	{ 0x0a, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x0d, KEY_7 },
	{ 0x0e, KEY_8 },
	{ 0x0f, KEY_9 },

	{ 0x2a, KEY_VOLUMEUP },
	{ 0x11, KEY_VOLUMEDOWN },
	{ 0x18, KEY_CHANNELUP },
	{ 0x19, KEY_CHANNELDOWN },

	{ 0x13, KEY_ENTER },	
	{ 0x21, KEY_DOT },	
};

struct ir_scancode_table ir_codes_eztv_table = {
	.scan = ir_codes_eztv,
	.size = ARRAY_SIZE(ir_codes_eztv),
};
EXPORT_SYMBOL_GPL(ir_codes_eztv_table);


static struct ir_scancode ir_codes_avermedia[] = {
	{ 0x28, KEY_1 },
	{ 0x18, KEY_2 },
	{ 0x38, KEY_3 },
	{ 0x24, KEY_4 },
	{ 0x14, KEY_5 },
	{ 0x34, KEY_6 },
	{ 0x2c, KEY_7 },
	{ 0x1c, KEY_8 },
	{ 0x3c, KEY_9 },
	{ 0x22, KEY_0 },

	{ 0x20, KEY_TV },		
	{ 0x10, KEY_CD },		
	{ 0x30, KEY_TEXT },		
	{ 0x00, KEY_POWER },		

	{ 0x08, KEY_VIDEO },		
	{ 0x04, KEY_AUDIO },		
	{ 0x0c, KEY_ZOOM },		

	{ 0x12, KEY_SUBTITLE },		
	{ 0x32, KEY_REWIND },		
	{ 0x02, KEY_PRINT },		

	{ 0x2a, KEY_SEARCH },		
	{ 0x1a, KEY_SLEEP },		
	{ 0x3a, KEY_CAMERA },		
	{ 0x0a, KEY_MUTE },		

	{ 0x26, KEY_RECORD },		
	{ 0x16, KEY_PAUSE },		
	{ 0x36, KEY_STOP },		
	{ 0x06, KEY_PLAY },		

	{ 0x2e, KEY_RED },		
	{ 0x21, KEY_GREEN },		
	{ 0x0e, KEY_YELLOW },		
	{ 0x01, KEY_BLUE },		

	{ 0x1e, KEY_VOLUMEDOWN },	
	{ 0x3e, KEY_VOLUMEUP },		
	{ 0x11, KEY_CHANNELDOWN },	
	{ 0x31, KEY_CHANNELUP }		
};

struct ir_scancode_table ir_codes_avermedia_table = {
	.scan = ir_codes_avermedia,
	.size = ARRAY_SIZE(ir_codes_avermedia),
};
EXPORT_SYMBOL_GPL(ir_codes_avermedia_table);

static struct ir_scancode ir_codes_videomate_tv_pvr[] = {
	{ 0x14, KEY_MUTE },
	{ 0x24, KEY_ZOOM },

	{ 0x01, KEY_DVD },
	{ 0x23, KEY_RADIO },
	{ 0x00, KEY_TV },

	{ 0x0a, KEY_REWIND },
	{ 0x08, KEY_PLAYPAUSE },
	{ 0x0f, KEY_FORWARD },

	{ 0x02, KEY_PREVIOUS },
	{ 0x07, KEY_STOP },
	{ 0x06, KEY_NEXT },

	{ 0x0c, KEY_UP },
	{ 0x0e, KEY_DOWN },
	{ 0x0b, KEY_LEFT },
	{ 0x0d, KEY_RIGHT },
	{ 0x11, KEY_OK },

	{ 0x03, KEY_MENU },
	{ 0x09, KEY_SETUP },
	{ 0x05, KEY_VIDEO },
	{ 0x22, KEY_CHANNEL },

	{ 0x12, KEY_VOLUMEUP },
	{ 0x15, KEY_VOLUMEDOWN },
	{ 0x10, KEY_CHANNELUP },
	{ 0x13, KEY_CHANNELDOWN },

	{ 0x04, KEY_RECORD },

	{ 0x16, KEY_1 },
	{ 0x17, KEY_2 },
	{ 0x18, KEY_3 },
	{ 0x19, KEY_4 },
	{ 0x1a, KEY_5 },
	{ 0x1b, KEY_6 },
	{ 0x1c, KEY_7 },
	{ 0x1d, KEY_8 },
	{ 0x1e, KEY_9 },
	{ 0x1f, KEY_0 },

	{ 0x20, KEY_LANGUAGE },
	{ 0x21, KEY_SLEEP },
};

struct ir_scancode_table ir_codes_videomate_tv_pvr_table = {
	.scan = ir_codes_videomate_tv_pvr,
	.size = ARRAY_SIZE(ir_codes_videomate_tv_pvr),
};
EXPORT_SYMBOL_GPL(ir_codes_videomate_tv_pvr_table);


static struct ir_scancode ir_codes_manli[] = {

	
	{ 0x1c, KEY_RADIO },	
	{ 0x12, KEY_POWER },

	
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	
	{ 0x0a, KEY_AGAIN },	
	{ 0x00, KEY_0 },
	{ 0x17, KEY_DIGITS },	

	
	{ 0x14, KEY_MENU },
	{ 0x10, KEY_INFO },

	
	{ 0x0b, KEY_UP },
	{ 0x18, KEY_LEFT },
	{ 0x16, KEY_OK },	
	{ 0x0c, KEY_RIGHT },
	{ 0x15, KEY_DOWN },

	
	{ 0x11, KEY_TV },	
	{ 0x0d, KEY_MODE },	

	
	{ 0x0f, KEY_AUDIO },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x1a, KEY_CHANNELUP },
	{ 0x0e, KEY_TIME },
	{ 0x1f, KEY_VOLUMEDOWN },
	{ 0x1e, KEY_CHANNELDOWN },

	
	{ 0x13, KEY_MUTE },
	{ 0x19, KEY_CAMERA },

	
};

struct ir_scancode_table ir_codes_manli_table = {
	.scan = ir_codes_manli,
	.size = ARRAY_SIZE(ir_codes_manli),
};
EXPORT_SYMBOL_GPL(ir_codes_manli_table);


static struct ir_scancode ir_codes_gotview7135[] = {

	{ 0x11, KEY_POWER },
	{ 0x35, KEY_TV },
	{ 0x1b, KEY_0 },
	{ 0x29, KEY_1 },
	{ 0x19, KEY_2 },
	{ 0x39, KEY_3 },
	{ 0x1f, KEY_4 },
	{ 0x2c, KEY_5 },
	{ 0x21, KEY_6 },
	{ 0x24, KEY_7 },
	{ 0x18, KEY_8 },
	{ 0x2b, KEY_9 },
	{ 0x3b, KEY_AGAIN },	
	{ 0x06, KEY_AUDIO },
	{ 0x31, KEY_PRINT },	
	{ 0x3e, KEY_VIDEO },
	{ 0x10, KEY_CHANNELUP },
	{ 0x20, KEY_CHANNELDOWN },
	{ 0x0c, KEY_VOLUMEDOWN },
	{ 0x28, KEY_VOLUMEUP },
	{ 0x08, KEY_MUTE },
	{ 0x26, KEY_SEARCH },	
	{ 0x3f, KEY_CAMERA },	
	{ 0x12, KEY_RECORD },
	{ 0x32, KEY_STOP },
	{ 0x3c, KEY_PLAY },
	{ 0x1d, KEY_REWIND },
	{ 0x2d, KEY_PAUSE },
	{ 0x0d, KEY_FORWARD },
	{ 0x05, KEY_ZOOM },	

	{ 0x2a, KEY_F21 },	
	{ 0x0e, KEY_F22 },	
	{ 0x1e, KEY_TIME },	
	{ 0x38, KEY_F24 },	
};

struct ir_scancode_table ir_codes_gotview7135_table = {
	.scan = ir_codes_gotview7135,
	.size = ARRAY_SIZE(ir_codes_gotview7135),
};
EXPORT_SYMBOL_GPL(ir_codes_gotview7135_table);

static struct ir_scancode ir_codes_purpletv[] = {
	{ 0x03, KEY_POWER },
	{ 0x6f, KEY_MUTE },
	{ 0x10, KEY_BACKSPACE },	

	{ 0x11, KEY_0 },
	{ 0x04, KEY_1 },
	{ 0x05, KEY_2 },
	{ 0x06, KEY_3 },
	{ 0x08, KEY_4 },
	{ 0x09, KEY_5 },
	{ 0x0a, KEY_6 },
	{ 0x0c, KEY_7 },
	{ 0x0d, KEY_8 },
	{ 0x0e, KEY_9 },
	{ 0x12, KEY_DOT },	

	{ 0x07, KEY_VOLUMEUP },
	{ 0x0b, KEY_VOLUMEDOWN },
	{ 0x1a, KEY_KPPLUS },
	{ 0x18, KEY_KPMINUS },
	{ 0x15, KEY_UP },
	{ 0x1d, KEY_DOWN },
	{ 0x0f, KEY_CHANNELUP },
	{ 0x13, KEY_CHANNELDOWN },
	{ 0x48, KEY_ZOOM },

	{ 0x1b, KEY_VIDEO },	
	{ 0x1f, KEY_CAMERA },	
	{ 0x49, KEY_LANGUAGE },	
	{ 0x19, KEY_SEARCH },	

	{ 0x4b, KEY_RECORD },
	{ 0x46, KEY_PLAY },
	{ 0x45, KEY_PAUSE },	
	{ 0x44, KEY_STOP },
	{ 0x43, KEY_TIME },	
	{ 0x17, KEY_CHANNEL },	
	{ 0x40, KEY_FORWARD },	
	{ 0x42, KEY_REWIND },	

};

struct ir_scancode_table ir_codes_purpletv_table = {
	.scan = ir_codes_purpletv,
	.size = ARRAY_SIZE(ir_codes_purpletv),
};
EXPORT_SYMBOL_GPL(ir_codes_purpletv_table);


static struct ir_scancode ir_codes_pctv_sedna[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0a, KEY_AGAIN },	
	{ 0x0b, KEY_CHANNELUP },
	{ 0x0c, KEY_VOLUMEUP },
	{ 0x0d, KEY_MODE },	
	{ 0x0e, KEY_STOP },
	{ 0x0f, KEY_PREVIOUSSONG },
	{ 0x10, KEY_ZOOM },
	{ 0x11, KEY_TUNER },	
	{ 0x12, KEY_POWER },
	{ 0x13, KEY_MUTE },
	{ 0x15, KEY_CHANNELDOWN },
	{ 0x18, KEY_VOLUMEDOWN },
	{ 0x19, KEY_CAMERA },	
	{ 0x1a, KEY_NEXTSONG },
	{ 0x1b, KEY_TIME },	
	{ 0x1c, KEY_RADIO },	
	{ 0x1d, KEY_RECORD },
	{ 0x1e, KEY_PAUSE },
	
	{ 0x14, KEY_INFO },	
	{ 0x16, KEY_OK },	
	{ 0x17, KEY_DIGITS },	
	{ 0x1f, KEY_PLAY },	
};

struct ir_scancode_table ir_codes_pctv_sedna_table = {
	.scan = ir_codes_pctv_sedna,
	.size = ARRAY_SIZE(ir_codes_pctv_sedna),
};
EXPORT_SYMBOL_GPL(ir_codes_pctv_sedna_table);


static struct ir_scancode ir_codes_pv951[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x12, KEY_POWER },
	{ 0x10, KEY_MUTE },
	{ 0x1f, KEY_VOLUMEDOWN },
	{ 0x1b, KEY_VOLUMEUP },
	{ 0x1a, KEY_CHANNELUP },
	{ 0x1e, KEY_CHANNELDOWN },
	{ 0x0e, KEY_PAGEUP },
	{ 0x1d, KEY_PAGEDOWN },
	{ 0x13, KEY_SOUND },

	{ 0x18, KEY_KPPLUSMINUS },	
	{ 0x16, KEY_SUBTITLE },		
	{ 0x0d, KEY_TEXT },		
	{ 0x0b, KEY_TV },		
	{ 0x11, KEY_PC },		
	{ 0x17, KEY_OK },		
	{ 0x19, KEY_MODE },		
	{ 0x0c, KEY_SEARCH },		

	
	{ 0x0f, KEY_SELECT },		
	{ 0x0a, KEY_KPPLUS },		
	{ 0x14, KEY_EQUAL },		
	{ 0x1c, KEY_MEDIA },		
};

struct ir_scancode_table ir_codes_pv951_table = {
	.scan = ir_codes_pv951,
	.size = ARRAY_SIZE(ir_codes_pv951),
};
EXPORT_SYMBOL_GPL(ir_codes_pv951_table);




static struct ir_scancode ir_codes_rc5_tv[] = {
	
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0b, KEY_CHANNEL },		
	{ 0x0c, KEY_POWER },		
	{ 0x0d, KEY_MUTE },		
	{ 0x0f, KEY_TV },		
	{ 0x10, KEY_VOLUMEUP },
	{ 0x11, KEY_VOLUMEDOWN },
	{ 0x12, KEY_BRIGHTNESSUP },
	{ 0x13, KEY_BRIGHTNESSDOWN },
	{ 0x1e, KEY_SEARCH },		
	{ 0x20, KEY_CHANNELUP },	
	{ 0x21, KEY_CHANNELDOWN },	
	{ 0x22, KEY_CHANNEL },		
	{ 0x23, KEY_LANGUAGE },		
	{ 0x26, KEY_SLEEP },		
	{ 0x2e, KEY_MENU },		
	{ 0x30, KEY_PAUSE },
	{ 0x32, KEY_REWIND },
	{ 0x33, KEY_GOTO },
	{ 0x35, KEY_PLAY },
	{ 0x36, KEY_STOP },
	{ 0x37, KEY_RECORD },		
	{ 0x3c, KEY_TEXT },		
	{ 0x3d, KEY_SUSPEND },		

};

struct ir_scancode_table ir_codes_rc5_tv_table = {
	.scan = ir_codes_rc5_tv,
	.size = ARRAY_SIZE(ir_codes_rc5_tv),
};
EXPORT_SYMBOL_GPL(ir_codes_rc5_tv_table);


static struct ir_scancode ir_codes_winfast[] = {
	
	{ 0x12, KEY_0 },
	{ 0x05, KEY_1 },
	{ 0x06, KEY_2 },
	{ 0x07, KEY_3 },
	{ 0x09, KEY_4 },
	{ 0x0a, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x0d, KEY_7 },
	{ 0x0e, KEY_8 },
	{ 0x0f, KEY_9 },

	{ 0x00, KEY_POWER },
	{ 0x1b, KEY_AUDIO },		
	{ 0x02, KEY_TUNER },		
	{ 0x1e, KEY_VIDEO },		
	{ 0x16, KEY_INFO },		
	{ 0x04, KEY_VOLUMEUP },
	{ 0x08, KEY_VOLUMEDOWN },
	{ 0x0c, KEY_CHANNELUP },
	{ 0x10, KEY_CHANNELDOWN },
	{ 0x03, KEY_ZOOM },		
	{ 0x1f, KEY_TEXT },		
	{ 0x20, KEY_SLEEP },
	{ 0x29, KEY_CLEAR },		
	{ 0x14, KEY_MUTE },
	{ 0x2b, KEY_RED },
	{ 0x2c, KEY_GREEN },
	{ 0x2d, KEY_YELLOW },
	{ 0x2e, KEY_BLUE },
	{ 0x18, KEY_KPPLUS },		
	{ 0x19, KEY_KPMINUS },		
	{ 0x2a, KEY_MEDIA },		
	{ 0x21, KEY_DOT },
	{ 0x13, KEY_ENTER },
	{ 0x11, KEY_LAST },		
	{ 0x22, KEY_PREVIOUS },
	{ 0x23, KEY_PLAYPAUSE },
	{ 0x24, KEY_NEXT },
	{ 0x25, KEY_TIME },		
	{ 0x26, KEY_STOP },
	{ 0x27, KEY_RECORD },
	{ 0x28, KEY_SAVE },		
	{ 0x2f, KEY_MENU },
	{ 0x30, KEY_CANCEL },
	{ 0x31, KEY_CHANNEL },		
	{ 0x32, KEY_SUBTITLE },
	{ 0x33, KEY_LANGUAGE },
	{ 0x34, KEY_REWIND },
	{ 0x35, KEY_FASTFORWARD },
	{ 0x36, KEY_TV },
	{ 0x37, KEY_RADIO },		
	{ 0x38, KEY_DVD },

	{ 0x3e, KEY_F21 },		
	{ 0x3a, KEY_F22 },		
	{ 0x3b, KEY_F23 },		
	{ 0x3f, KEY_F24 }		
};

struct ir_scancode_table ir_codes_winfast_table = {
	.scan = ir_codes_winfast,
	.size = ARRAY_SIZE(ir_codes_winfast),
};
EXPORT_SYMBOL_GPL(ir_codes_winfast_table);

static struct ir_scancode ir_codes_pinnacle_color[] = {
	{ 0x59, KEY_MUTE },
	{ 0x4a, KEY_POWER },

	{ 0x18, KEY_TEXT },
	{ 0x26, KEY_TV },
	{ 0x3d, KEY_PRINT },

	{ 0x48, KEY_RED },
	{ 0x04, KEY_GREEN },
	{ 0x11, KEY_YELLOW },
	{ 0x00, KEY_BLUE },

	{ 0x2d, KEY_VOLUMEUP },
	{ 0x1e, KEY_VOLUMEDOWN },

	{ 0x49, KEY_MENU },

	{ 0x16, KEY_CHANNELUP },
	{ 0x17, KEY_CHANNELDOWN },

	{ 0x20, KEY_UP },
	{ 0x21, KEY_DOWN },
	{ 0x22, KEY_LEFT },
	{ 0x23, KEY_RIGHT },
	{ 0x0d, KEY_SELECT },

	{ 0x08, KEY_BACK },
	{ 0x07, KEY_REFRESH },

	{ 0x2f, KEY_ZOOM },
	{ 0x29, KEY_RECORD },

	{ 0x4b, KEY_PAUSE },
	{ 0x4d, KEY_REWIND },
	{ 0x2e, KEY_PLAY },
	{ 0x4e, KEY_FORWARD },
	{ 0x53, KEY_PREVIOUS },
	{ 0x4c, KEY_STOP },
	{ 0x54, KEY_NEXT },

	{ 0x69, KEY_0 },
	{ 0x6a, KEY_1 },
	{ 0x6b, KEY_2 },
	{ 0x6c, KEY_3 },
	{ 0x6d, KEY_4 },
	{ 0x6e, KEY_5 },
	{ 0x6f, KEY_6 },
	{ 0x70, KEY_7 },
	{ 0x71, KEY_8 },
	{ 0x72, KEY_9 },

	{ 0x74, KEY_CHANNEL },
	{ 0x0a, KEY_BACKSPACE },
};

struct ir_scancode_table ir_codes_pinnacle_color_table = {
	.scan = ir_codes_pinnacle_color,
	.size = ARRAY_SIZE(ir_codes_pinnacle_color),
};
EXPORT_SYMBOL_GPL(ir_codes_pinnacle_color_table);


static struct ir_scancode ir_codes_hauppauge_new[] = {
	
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	{ 0x0a, KEY_TEXT },		
	{ 0x0b, KEY_RED },		
	{ 0x0c, KEY_RADIO },
	{ 0x0d, KEY_MENU },
	{ 0x0e, KEY_SUBTITLE },		
	{ 0x0f, KEY_MUTE },
	{ 0x10, KEY_VOLUMEUP },
	{ 0x11, KEY_VOLUMEDOWN },
	{ 0x12, KEY_PREVIOUS },		
	{ 0x14, KEY_UP },
	{ 0x15, KEY_DOWN },
	{ 0x16, KEY_LEFT },
	{ 0x17, KEY_RIGHT },
	{ 0x18, KEY_VIDEO },		
	{ 0x19, KEY_AUDIO },		
	
	{ 0x1a, KEY_MHP },

	{ 0x1b, KEY_EPG },		
	{ 0x1c, KEY_TV },
	{ 0x1e, KEY_NEXTSONG },		
	{ 0x1f, KEY_EXIT },		
	{ 0x20, KEY_CHANNELUP },	
	{ 0x21, KEY_CHANNELDOWN },	
	{ 0x22, KEY_CHANNEL },		
	{ 0x24, KEY_PREVIOUSSONG },	
	{ 0x25, KEY_ENTER },		
	{ 0x26, KEY_SLEEP },		
	{ 0x29, KEY_BLUE },		
	{ 0x2e, KEY_GREEN },		
	{ 0x30, KEY_PAUSE },		
	{ 0x32, KEY_REWIND },		
	{ 0x34, KEY_FASTFORWARD },	
	{ 0x35, KEY_PLAY },
	{ 0x36, KEY_STOP },
	{ 0x37, KEY_RECORD },		
	{ 0x38, KEY_YELLOW },		
	{ 0x3b, KEY_SELECT },		
	{ 0x3c, KEY_ZOOM },		
	{ 0x3d, KEY_POWER },		
};

struct ir_scancode_table ir_codes_hauppauge_new_table = {
	.scan = ir_codes_hauppauge_new,
	.size = ARRAY_SIZE(ir_codes_hauppauge_new),
};
EXPORT_SYMBOL_GPL(ir_codes_hauppauge_new_table);

static struct ir_scancode ir_codes_npgtech[] = {
	{ 0x1d, KEY_SWITCHVIDEOMODE },	
	{ 0x2a, KEY_FRONT },

	{ 0x3e, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x06, KEY_3 },
	{ 0x0a, KEY_4 },
	{ 0x0e, KEY_5 },
	{ 0x12, KEY_6 },
	{ 0x16, KEY_7 },
	{ 0x1a, KEY_8 },
	{ 0x1e, KEY_9 },
	{ 0x3a, KEY_0 },
	{ 0x22, KEY_NUMLOCK },		
	{ 0x20, KEY_REFRESH },

	{ 0x03, KEY_BRIGHTNESSDOWN },
	{ 0x28, KEY_AUDIO },
	{ 0x3c, KEY_CHANNELUP },
	{ 0x3f, KEY_VOLUMEDOWN },
	{ 0x2e, KEY_MUTE },
	{ 0x3b, KEY_VOLUMEUP },
	{ 0x00, KEY_CHANNELDOWN },
	{ 0x07, KEY_BRIGHTNESSUP },
	{ 0x2c, KEY_TEXT },

	{ 0x37, KEY_RECORD },
	{ 0x17, KEY_PLAY },
	{ 0x13, KEY_PAUSE },
	{ 0x26, KEY_STOP },
	{ 0x18, KEY_FASTFORWARD },
	{ 0x14, KEY_REWIND },
	{ 0x33, KEY_ZOOM },
	{ 0x32, KEY_KEYBOARD },
	{ 0x30, KEY_GOTO },		
	{ 0x36, KEY_MACRO },		
	{ 0x0b, KEY_RADIO },
	{ 0x10, KEY_POWER },

};

struct ir_scancode_table ir_codes_npgtech_table = {
	.scan = ir_codes_npgtech,
	.size = ARRAY_SIZE(ir_codes_npgtech),
};
EXPORT_SYMBOL_GPL(ir_codes_npgtech_table);


static struct ir_scancode ir_codes_norwood[] = {
	
	{ 0x20, KEY_0 },
	{ 0x21, KEY_1 },
	{ 0x22, KEY_2 },
	{ 0x23, KEY_3 },
	{ 0x24, KEY_4 },
	{ 0x25, KEY_5 },
	{ 0x26, KEY_6 },
	{ 0x27, KEY_7 },
	{ 0x28, KEY_8 },
	{ 0x29, KEY_9 },

	{ 0x78, KEY_TUNER },		
	{ 0x2c, KEY_EXIT },		
	{ 0x2a, KEY_SELECT },		
	{ 0x69, KEY_AGAIN },		

	{ 0x32, KEY_BRIGHTNESSUP },	
	{ 0x33, KEY_BRIGHTNESSDOWN },	
	{ 0x6b, KEY_KPPLUS },		
	{ 0x6c, KEY_KPMINUS },		

	{ 0x2d, KEY_MUTE },		
	{ 0x30, KEY_VOLUMEUP },		
	{ 0x31, KEY_VOLUMEDOWN },	
	{ 0x60, KEY_CHANNELUP },	
	{ 0x61, KEY_CHANNELDOWN },	

	{ 0x3f, KEY_RECORD },		
	{ 0x37, KEY_PLAY },		
	{ 0x36, KEY_PAUSE },		
	{ 0x2b, KEY_STOP },		
	{ 0x67, KEY_FASTFORWARD },	
	{ 0x66, KEY_REWIND },		
	{ 0x3e, KEY_SEARCH },		
	{ 0x2e, KEY_CAMERA },		
	{ 0x6d, KEY_MENU },		
	{ 0x2f, KEY_ZOOM },		
	{ 0x34, KEY_RADIO },		
	{ 0x65, KEY_POWER },		
};

struct ir_scancode_table ir_codes_norwood_table = {
	.scan = ir_codes_norwood,
	.size = ARRAY_SIZE(ir_codes_norwood),
};
EXPORT_SYMBOL_GPL(ir_codes_norwood_table);


static struct ir_scancode ir_codes_budget_ci_old[] = {
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },
	{ 0x0a, KEY_ENTER },
	{ 0x0b, KEY_RED },
	{ 0x0c, KEY_POWER },		
	{ 0x0d, KEY_MUTE },
	{ 0x0f, KEY_A },		
	{ 0x10, KEY_VOLUMEUP },
	{ 0x11, KEY_VOLUMEDOWN },
	{ 0x14, KEY_B },
	{ 0x1c, KEY_UP },
	{ 0x1d, KEY_DOWN },
	{ 0x1e, KEY_OPTION },		
	{ 0x1f, KEY_BREAK },
	{ 0x20, KEY_CHANNELUP },
	{ 0x21, KEY_CHANNELDOWN },
	{ 0x22, KEY_PREVIOUS },		
	{ 0x24, KEY_RESTART },
	{ 0x25, KEY_OK },
	{ 0x26, KEY_CYCLEWINDOWS },	
	{ 0x28, KEY_ENTER },		
	{ 0x29, KEY_PAUSE },
	{ 0x2b, KEY_RIGHT },
	{ 0x2c, KEY_LEFT },
	{ 0x2e, KEY_MENU },		
	{ 0x30, KEY_SLOW },
	{ 0x31, KEY_PREVIOUS },		
	{ 0x32, KEY_REWIND },
	{ 0x34, KEY_FASTFORWARD },
	{ 0x35, KEY_PLAY },
	{ 0x36, KEY_STOP },
	{ 0x37, KEY_RECORD },
	{ 0x38, KEY_TUNER },		
	{ 0x3a, KEY_C },
	{ 0x3c, KEY_EXIT },
	{ 0x3d, KEY_POWER2 },
	{ 0x3e, KEY_TUNER },
};

struct ir_scancode_table ir_codes_budget_ci_old_table = {
	.scan = ir_codes_budget_ci_old,
	.size = ARRAY_SIZE(ir_codes_budget_ci_old),
};
EXPORT_SYMBOL_GPL(ir_codes_budget_ci_old_table);


static struct ir_scancode ir_codes_asus_pc39[] = {
	
	{ 0x15, KEY_0 },
	{ 0x29, KEY_1 },
	{ 0x2d, KEY_2 },
	{ 0x2b, KEY_3 },
	{ 0x09, KEY_4 },
	{ 0x0d, KEY_5 },
	{ 0x0b, KEY_6 },
	{ 0x31, KEY_7 },
	{ 0x35, KEY_8 },
	{ 0x33, KEY_9 },

	{ 0x3e, KEY_RADIO },		
	{ 0x03, KEY_MENU },		
	{ 0x2a, KEY_VOLUMEUP },
	{ 0x19, KEY_VOLUMEDOWN },
	{ 0x37, KEY_UP },
	{ 0x3b, KEY_DOWN },
	{ 0x27, KEY_LEFT },
	{ 0x2f, KEY_RIGHT },
	{ 0x25, KEY_VIDEO },		
	{ 0x39, KEY_AUDIO },		

	{ 0x21, KEY_TV },		
	{ 0x1d, KEY_EXIT },		
	{ 0x0a, KEY_CHANNELUP },	
	{ 0x1b, KEY_CHANNELDOWN },	
	{ 0x1a, KEY_ENTER },		

	{ 0x06, KEY_PAUSE },		
	{ 0x1e, KEY_PREVIOUS },		
	{ 0x26, KEY_NEXT },		
	{ 0x0e, KEY_REWIND },		
	{ 0x3a, KEY_FASTFORWARD },	
	{ 0x36, KEY_STOP },
	{ 0x2e, KEY_RECORD },		
	{ 0x16, KEY_POWER },		

	{ 0x11, KEY_ZOOM },		
	{ 0x13, KEY_MACRO },		
	{ 0x23, KEY_HOME },		
	{ 0x05, KEY_PVR },		
	{ 0x3d, KEY_MUTE },		
	{ 0x01, KEY_DVD },		
};

struct ir_scancode_table ir_codes_asus_pc39_table = {
	.scan = ir_codes_asus_pc39,
	.size = ARRAY_SIZE(ir_codes_asus_pc39),
};
EXPORT_SYMBOL_GPL(ir_codes_asus_pc39_table);



static struct ir_scancode ir_codes_encore_enltv[] = {

	
	{ 0x0d, KEY_MUTE },

	{ 0x1e, KEY_TV },
	{ 0x00, KEY_VIDEO },
	{ 0x01, KEY_AUDIO },		
	{ 0x02, KEY_MHP },		

	{ 0x1f, KEY_1 },
	{ 0x03, KEY_2 },
	{ 0x04, KEY_3 },
	{ 0x05, KEY_4 },
	{ 0x1c, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x1d, KEY_9 },
	{ 0x0a, KEY_0 },

	{ 0x09, KEY_LIST },		
	{ 0x0b, KEY_LAST },		

	{ 0x14, KEY_HOME },		
	{ 0x15, KEY_EXIT },		
	{ 0x16, KEY_CHANNELUP },	
	{ 0x12, KEY_CHANNELDOWN },	
	{ 0x0c, KEY_VOLUMEUP },		
	{ 0x17, KEY_VOLUMEDOWN },	

	{ 0x18, KEY_ENTER },		

	{ 0x0e, KEY_ESC },
	{ 0x13, KEY_CYCLEWINDOWS },	
	{ 0x11, KEY_TAB },
	{ 0x19, KEY_SWITCHVIDEOMODE },	

	{ 0x1a, KEY_MENU },
	{ 0x1b, KEY_ZOOM },		
	{ 0x44, KEY_TIME },		
	{ 0x40, KEY_MODE },		

	{ 0x5a, KEY_RECORD },
	{ 0x42, KEY_PLAY },		
	{ 0x45, KEY_STOP },
	{ 0x43, KEY_CAMERA },		

	{ 0x48, KEY_REWIND },
	{ 0x4a, KEY_FASTFORWARD },
	{ 0x49, KEY_PREVIOUS },
	{ 0x4b, KEY_NEXT },

	{ 0x4c, KEY_FAVORITES },	
	{ 0x4d, KEY_SOUND },		
	{ 0x4e, KEY_LANGUAGE },		
	{ 0x4f, KEY_TEXT },		

	{ 0x50, KEY_SLEEP },		
	{ 0x51, KEY_MODE },		
	{ 0x52, KEY_SELECT },		
	{ 0x53, KEY_PROG1 },		


	{ 0x59, KEY_RED },		
	{ 0x41, KEY_GREEN },		
	{ 0x47, KEY_YELLOW },		
	{ 0x57, KEY_BLUE },		
};

struct ir_scancode_table ir_codes_encore_enltv_table = {
	.scan = ir_codes_encore_enltv,
	.size = ARRAY_SIZE(ir_codes_encore_enltv),
};
EXPORT_SYMBOL_GPL(ir_codes_encore_enltv_table);


static struct ir_scancode ir_codes_encore_enltv2[] = {
	{ 0x4c, KEY_POWER2 },
	{ 0x4a, KEY_TUNER },
	{ 0x40, KEY_1 },
	{ 0x60, KEY_2 },
	{ 0x50, KEY_3 },
	{ 0x70, KEY_4 },
	{ 0x48, KEY_5 },
	{ 0x68, KEY_6 },
	{ 0x58, KEY_7 },
	{ 0x78, KEY_8 },
	{ 0x44, KEY_9 },
	{ 0x54, KEY_0 },

	{ 0x64, KEY_LAST },		
	{ 0x4e, KEY_AGAIN },		

	{ 0x6c, KEY_SWITCHVIDEOMODE },	
	{ 0x5e, KEY_MENU },
	{ 0x56, KEY_SCREEN },
	{ 0x7a, KEY_SETUP },

	{ 0x46, KEY_MUTE },
	{ 0x5c, KEY_MODE },		
	{ 0x74, KEY_INFO },
	{ 0x7c, KEY_CLEAR },

	{ 0x55, KEY_UP },
	{ 0x49, KEY_DOWN },
	{ 0x7e, KEY_LEFT },
	{ 0x59, KEY_RIGHT },
	{ 0x6a, KEY_ENTER },

	{ 0x42, KEY_VOLUMEUP },
	{ 0x62, KEY_VOLUMEDOWN },
	{ 0x52, KEY_CHANNELUP },
	{ 0x72, KEY_CHANNELDOWN },

	{ 0x41, KEY_RECORD },
	{ 0x51, KEY_CAMERA },		
	{ 0x75, KEY_TIME },		
	{ 0x71, KEY_TV2 },		

	{ 0x45, KEY_REWIND },
	{ 0x6f, KEY_PAUSE },
	{ 0x7d, KEY_FORWARD },
	{ 0x79, KEY_STOP },
};

struct ir_scancode_table ir_codes_encore_enltv2_table = {
	.scan = ir_codes_encore_enltv2,
	.size = ARRAY_SIZE(ir_codes_encore_enltv2),
};
EXPORT_SYMBOL_GPL(ir_codes_encore_enltv2_table);


static struct ir_scancode ir_codes_tt_1500[] = {
	{ 0x01, KEY_POWER },
	{ 0x02, KEY_SHUFFLE },		
	{ 0x03, KEY_1 },
	{ 0x04, KEY_2 },
	{ 0x05, KEY_3 },
	{ 0x06, KEY_4 },
	{ 0x07, KEY_5 },
	{ 0x08, KEY_6 },
	{ 0x09, KEY_7 },
	{ 0x0a, KEY_8 },
	{ 0x0b, KEY_9 },
	{ 0x0c, KEY_0 },
	{ 0x0d, KEY_UP },
	{ 0x0e, KEY_LEFT },
	{ 0x0f, KEY_OK },
	{ 0x10, KEY_RIGHT },
	{ 0x11, KEY_DOWN },
	{ 0x12, KEY_INFO },
	{ 0x13, KEY_EXIT },
	{ 0x14, KEY_RED },
	{ 0x15, KEY_GREEN },
	{ 0x16, KEY_YELLOW },
	{ 0x17, KEY_BLUE },
	{ 0x18, KEY_MUTE },
	{ 0x19, KEY_TEXT },
	{ 0x1a, KEY_MODE },		
	{ 0x21, KEY_OPTION },
	{ 0x22, KEY_EPG },
	{ 0x23, KEY_CHANNELUP },
	{ 0x24, KEY_CHANNELDOWN },
	{ 0x25, KEY_VOLUMEUP },
	{ 0x26, KEY_VOLUMEDOWN },
	{ 0x27, KEY_SETUP },
	{ 0x3a, KEY_RECORD },		
	{ 0x3b, KEY_PLAY },
	{ 0x3c, KEY_STOP },
	{ 0x3d, KEY_REWIND },
	{ 0x3e, KEY_PAUSE },
	{ 0x3f, KEY_FORWARD },
};

struct ir_scancode_table ir_codes_tt_1500_table = {
	.scan = ir_codes_tt_1500,
	.size = ARRAY_SIZE(ir_codes_tt_1500),
};
EXPORT_SYMBOL_GPL(ir_codes_tt_1500_table);


static struct ir_scancode ir_codes_fusionhdtv_mce[] = {

	{ 0x0b, KEY_1 },
	{ 0x17, KEY_2 },
	{ 0x1b, KEY_3 },
	{ 0x07, KEY_4 },
	{ 0x50, KEY_5 },
	{ 0x54, KEY_6 },
	{ 0x48, KEY_7 },
	{ 0x4c, KEY_8 },
	{ 0x58, KEY_9 },
	{ 0x03, KEY_0 },

	{ 0x5e, KEY_OK },
	{ 0x51, KEY_UP },
	{ 0x53, KEY_DOWN },
	{ 0x5b, KEY_LEFT },
	{ 0x5f, KEY_RIGHT },

	{ 0x02, KEY_TV },		
	{ 0x0e, KEY_MP3 },
	{ 0x1a, KEY_DVD },
	{ 0x1e, KEY_FAVORITES },	
	{ 0x16, KEY_SETUP },
	{ 0x46, KEY_POWER2 },		
	{ 0x0a, KEY_EPG },		

	{ 0x49, KEY_BACK },
	{ 0x59, KEY_INFO },		
	{ 0x4d, KEY_MENU },		
	{ 0x55, KEY_CYCLEWINDOWS },	

	{ 0x0f, KEY_PREVIOUSSONG },	
	{ 0x12, KEY_NEXTSONG },		
	{ 0x42, KEY_ENTER },		

	{ 0x15, KEY_VOLUMEUP },
	{ 0x05, KEY_VOLUMEDOWN },
	{ 0x11, KEY_CHANNELUP },
	{ 0x09, KEY_CHANNELDOWN },

	{ 0x52, KEY_CAMERA },
	{ 0x5a, KEY_TUNER },
	{ 0x19, KEY_OPEN },

	{ 0x13, KEY_MODE },		
	{ 0x1f, KEY_ZOOM },

	{ 0x43, KEY_REWIND },
	{ 0x47, KEY_PLAYPAUSE },
	{ 0x4f, KEY_FASTFORWARD },
	{ 0x57, KEY_MUTE },
	{ 0x0d, KEY_STOP },
	{ 0x01, KEY_RECORD },
	{ 0x4e, KEY_POWER },
};

struct ir_scancode_table ir_codes_fusionhdtv_mce_table = {
	.scan = ir_codes_fusionhdtv_mce,
	.size = ARRAY_SIZE(ir_codes_fusionhdtv_mce),
};
EXPORT_SYMBOL_GPL(ir_codes_fusionhdtv_mce_table);


static struct ir_scancode ir_codes_pinnacle_pctv_hd[] = {

	{ 0x0f, KEY_1 },
	{ 0x15, KEY_2 },
	{ 0x10, KEY_3 },
	{ 0x18, KEY_4 },
	{ 0x1b, KEY_5 },
	{ 0x1e, KEY_6 },
	{ 0x11, KEY_7 },
	{ 0x21, KEY_8 },
	{ 0x12, KEY_9 },
	{ 0x27, KEY_0 },

	{ 0x24, KEY_ZOOM },
	{ 0x2a, KEY_SUBTITLE },

	{ 0x00, KEY_MUTE },
	{ 0x01, KEY_ENTER },	
	{ 0x39, KEY_POWER },

	{ 0x03, KEY_VOLUMEUP },
	{ 0x09, KEY_VOLUMEDOWN },
	{ 0x06, KEY_CHANNELUP },
	{ 0x0c, KEY_CHANNELDOWN },

	{ 0x2d, KEY_REWIND },
	{ 0x30, KEY_PLAYPAUSE },
	{ 0x33, KEY_FASTFORWARD },
	{ 0x3c, KEY_STOP },
	{ 0x36, KEY_RECORD },
	{ 0x3f, KEY_EPG },	
};

struct ir_scancode_table ir_codes_pinnacle_pctv_hd_table = {
	.scan = ir_codes_pinnacle_pctv_hd,
	.size = ARRAY_SIZE(ir_codes_pinnacle_pctv_hd),
};
EXPORT_SYMBOL_GPL(ir_codes_pinnacle_pctv_hd_table);


static struct ir_scancode ir_codes_behold[] = {

	
	{ 0x1c, KEY_TUNER },	
	{ 0x12, KEY_POWER },

	
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },

	
	{ 0x0a, KEY_AGAIN },
	{ 0x00, KEY_0 },
	{ 0x17, KEY_MODE },

	
	{ 0x14, KEY_SCREEN },
	{ 0x10, KEY_ZOOM },

	
	{ 0x0b, KEY_CHANNELUP },
	{ 0x18, KEY_VOLUMEDOWN },
	{ 0x16, KEY_OK },		
	{ 0x0c, KEY_VOLUMEUP },
	{ 0x15, KEY_CHANNELDOWN },

	
	{ 0x11, KEY_MUTE },
	{ 0x0d, KEY_INFO },

	
	{ 0x0f, KEY_RECORD },
	{ 0x1b, KEY_PLAYPAUSE },
	{ 0x1a, KEY_STOP },
	{ 0x0e, KEY_TEXT },
	{ 0x1f, KEY_RED },	
	{ 0x1e, KEY_YELLOW },	

	
	{ 0x1d, KEY_SLEEP },
	{ 0x13, KEY_GREEN },
	{ 0x19, KEY_BLUE },	

	
	{ 0x58, KEY_SLOW },
	{ 0x5c, KEY_CAMERA },

};

struct ir_scancode_table ir_codes_behold_table = {
	.scan = ir_codes_behold,
	.size = ARRAY_SIZE(ir_codes_behold),
};
EXPORT_SYMBOL_GPL(ir_codes_behold_table);


static struct ir_scancode ir_codes_behold_columbus[] = {

	

	{ 0x13, KEY_MUTE },
	{ 0x11, KEY_PROPS },
	{ 0x1C, KEY_TUNER },	
	{ 0x12, KEY_POWER },

	
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x0D, KEY_SETUP },	  
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x19, KEY_CAMERA },	
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },
	{ 0x10, KEY_ZOOM },

	
	{ 0x0A, KEY_AGAIN },
	{ 0x00, KEY_0 },
	{ 0x0B, KEY_CHANNELUP },
	{ 0x0C, KEY_VOLUMEUP },

	

	{ 0x1B, KEY_TIME },
	{ 0x1D, KEY_RECORD },
	{ 0x15, KEY_CHANNELDOWN },
	{ 0x18, KEY_VOLUMEDOWN },

	

	{ 0x0E, KEY_STOP },
	{ 0x1E, KEY_PAUSE },
	{ 0x0F, KEY_PREVIOUS },
	{ 0x1A, KEY_NEXT },

};

struct ir_scancode_table ir_codes_behold_columbus_table = {
	.scan = ir_codes_behold_columbus,
	.size = ARRAY_SIZE(ir_codes_behold_columbus),
};
EXPORT_SYMBOL_GPL(ir_codes_behold_columbus_table);


static struct ir_scancode ir_codes_genius_tvgo_a11mce[] = {
	
	{ 0x48, KEY_0 },
	{ 0x09, KEY_1 },
	{ 0x1d, KEY_2 },
	{ 0x1f, KEY_3 },
	{ 0x19, KEY_4 },
	{ 0x1b, KEY_5 },
	{ 0x11, KEY_6 },
	{ 0x17, KEY_7 },
	{ 0x12, KEY_8 },
	{ 0x16, KEY_9 },

	{ 0x54, KEY_RECORD },		
	{ 0x06, KEY_MUTE },		
	{ 0x10, KEY_POWER },
	{ 0x40, KEY_LAST },		
	{ 0x4c, KEY_CHANNELUP },	
	{ 0x00, KEY_CHANNELDOWN },	
	{ 0x0d, KEY_VOLUMEUP },
	{ 0x15, KEY_VOLUMEDOWN },
	{ 0x4d, KEY_OK },		
	{ 0x1c, KEY_ZOOM },		
	{ 0x02, KEY_MODE },		
	{ 0x04, KEY_LIST },		
	
	{ 0x1a, KEY_NEXT },		
	{ 0x0e, KEY_PREVIOUS },		
	
	{ 0x1e, KEY_UP },		
	{ 0x0a, KEY_DOWN },		
	{ 0x05, KEY_CAMERA },		
	{ 0x0c, KEY_RIGHT },		
	
	{ 0x49, KEY_RED },
	{ 0x0b, KEY_GREEN },
	{ 0x13, KEY_YELLOW },
	{ 0x50, KEY_BLUE },
};

struct ir_scancode_table ir_codes_genius_tvgo_a11mce_table = {
	.scan = ir_codes_genius_tvgo_a11mce,
	.size = ARRAY_SIZE(ir_codes_genius_tvgo_a11mce),
};
EXPORT_SYMBOL_GPL(ir_codes_genius_tvgo_a11mce_table);


static struct ir_scancode ir_codes_powercolor_real_angel[] = {
	{ 0x38, KEY_SWITCHVIDEOMODE },	
	{ 0x0c, KEY_MEDIA },		
	{ 0x00, KEY_0 },
	{ 0x01, KEY_1 },
	{ 0x02, KEY_2 },
	{ 0x03, KEY_3 },
	{ 0x04, KEY_4 },
	{ 0x05, KEY_5 },
	{ 0x06, KEY_6 },
	{ 0x07, KEY_7 },
	{ 0x08, KEY_8 },
	{ 0x09, KEY_9 },
	{ 0x0a, KEY_DIGITS },		
	{ 0x29, KEY_PREVIOUS },		
	{ 0x12, KEY_BRIGHTNESSUP },
	{ 0x13, KEY_BRIGHTNESSDOWN },
	{ 0x2b, KEY_MODE },		
	{ 0x2c, KEY_TEXT },		
	{ 0x20, KEY_CHANNELUP },	
	{ 0x21, KEY_CHANNELDOWN },	
	{ 0x10, KEY_VOLUMEUP },		
	{ 0x11, KEY_VOLUMEDOWN },	
	{ 0x0d, KEY_MUTE },
	{ 0x1f, KEY_RECORD },
	{ 0x17, KEY_PLAY },
	{ 0x16, KEY_PAUSE },
	{ 0x0b, KEY_STOP },
	{ 0x27, KEY_FASTFORWARD },
	{ 0x26, KEY_REWIND },
	{ 0x1e, KEY_SEARCH },		
	{ 0x0e, KEY_CAMERA },		
	{ 0x2d, KEY_SETUP },
	{ 0x0f, KEY_SCREEN },		
	{ 0x14, KEY_RADIO },		
	{ 0x25, KEY_POWER },		
};

struct ir_scancode_table ir_codes_powercolor_real_angel_table = {
	.scan = ir_codes_powercolor_real_angel,
	.size = ARRAY_SIZE(ir_codes_powercolor_real_angel),
};
EXPORT_SYMBOL_GPL(ir_codes_powercolor_real_angel_table);


static struct ir_scancode ir_codes_kworld_plus_tv_analog[] = {
	{ 0x0c, KEY_PROG1 },		
	{ 0x16, KEY_CLOSECD },		
	{ 0x1d, KEY_POWER2 },

	{ 0x00, KEY_1 },
	{ 0x01, KEY_2 },
	{ 0x02, KEY_3 },		
	{ 0x03, KEY_4 },		
	{ 0x04, KEY_5 },
	{ 0x05, KEY_6 },
	{ 0x06, KEY_7 },
	{ 0x07, KEY_8 },
	{ 0x08, KEY_9 },
	{ 0x0a, KEY_0 },

	{ 0x09, KEY_AGAIN },
	{ 0x14, KEY_MUTE },

	{ 0x20, KEY_UP },
	{ 0x21, KEY_DOWN },
	{ 0x0b, KEY_ENTER },

	{ 0x10, KEY_CHANNELUP },
	{ 0x11, KEY_CHANNELDOWN },

	

	{ 0x13, KEY_VOLUMEUP },
	{ 0x12, KEY_VOLUMEDOWN },

	
	{ 0x19, KEY_TIME},		
	{ 0x1a, KEY_STOP},
	{ 0x1b, KEY_RECORD},

	{ 0x22, KEY_TEXT},

	{ 0x15, KEY_AUDIO},		
	{ 0x0f, KEY_ZOOM},
	{ 0x1c, KEY_CAMERA},		

	{ 0x18, KEY_RED},		
	{ 0x23, KEY_GREEN},		
};
struct ir_scancode_table ir_codes_kworld_plus_tv_analog_table = {
	.scan = ir_codes_kworld_plus_tv_analog,
	.size = ARRAY_SIZE(ir_codes_kworld_plus_tv_analog),
};
EXPORT_SYMBOL_GPL(ir_codes_kworld_plus_tv_analog_table);


static struct ir_scancode ir_codes_kaiomy[] = {
	{ 0x43, KEY_POWER2},
	{ 0x01, KEY_LIST},
	{ 0x0b, KEY_ZOOM},
	{ 0x03, KEY_POWER},

	{ 0x04, KEY_1},
	{ 0x08, KEY_2},
	{ 0x02, KEY_3},

	{ 0x0f, KEY_4},
	{ 0x05, KEY_5},
	{ 0x06, KEY_6},

	{ 0x0c, KEY_7},
	{ 0x0d, KEY_8},
	{ 0x0a, KEY_9},

	{ 0x11, KEY_0},

	{ 0x09, KEY_CHANNELUP},
	{ 0x07, KEY_CHANNELDOWN},

	{ 0x0e, KEY_VOLUMEUP},
	{ 0x13, KEY_VOLUMEDOWN},

	{ 0x10, KEY_HOME},
	{ 0x12, KEY_ENTER},

	{ 0x14, KEY_RECORD},
	{ 0x15, KEY_STOP},
	{ 0x16, KEY_PLAY},
	{ 0x17, KEY_MUTE},

	{ 0x18, KEY_UP},
	{ 0x19, KEY_DOWN},
	{ 0x1a, KEY_LEFT},
	{ 0x1b, KEY_RIGHT},

	{ 0x1c, KEY_RED},
	{ 0x1d, KEY_GREEN},
	{ 0x1e, KEY_YELLOW},
	{ 0x1f, KEY_BLUE},
};
struct ir_scancode_table ir_codes_kaiomy_table = {
	.scan = ir_codes_kaiomy,
	.size = ARRAY_SIZE(ir_codes_kaiomy),
};
EXPORT_SYMBOL_GPL(ir_codes_kaiomy_table);

static struct ir_scancode ir_codes_avermedia_a16d[] = {
	{ 0x20, KEY_LIST},
	{ 0x00, KEY_POWER},
	{ 0x28, KEY_1},
	{ 0x18, KEY_2},
	{ 0x38, KEY_3},
	{ 0x24, KEY_4},
	{ 0x14, KEY_5},
	{ 0x34, KEY_6},
	{ 0x2c, KEY_7},
	{ 0x1c, KEY_8},
	{ 0x3c, KEY_9},
	{ 0x12, KEY_SUBTITLE},
	{ 0x22, KEY_0},
	{ 0x32, KEY_REWIND},
	{ 0x3a, KEY_SHUFFLE},
	{ 0x02, KEY_PRINT},
	{ 0x11, KEY_CHANNELDOWN},
	{ 0x31, KEY_CHANNELUP},
	{ 0x0c, KEY_ZOOM},
	{ 0x1e, KEY_VOLUMEDOWN},
	{ 0x3e, KEY_VOLUMEUP},
	{ 0x0a, KEY_MUTE},
	{ 0x04, KEY_AUDIO},
	{ 0x26, KEY_RECORD},
	{ 0x06, KEY_PLAY},
	{ 0x36, KEY_STOP},
	{ 0x16, KEY_PAUSE},
	{ 0x2e, KEY_REWIND},
	{ 0x0e, KEY_FASTFORWARD},
	{ 0x30, KEY_TEXT},
	{ 0x21, KEY_GREEN},
	{ 0x01, KEY_BLUE},
	{ 0x08, KEY_EPG},
	{ 0x2a, KEY_MENU},
};
struct ir_scancode_table ir_codes_avermedia_a16d_table = {
	.scan = ir_codes_avermedia_a16d,
	.size = ARRAY_SIZE(ir_codes_avermedia_a16d),
};
EXPORT_SYMBOL_GPL(ir_codes_avermedia_a16d_table);


static struct ir_scancode ir_codes_encore_enltv_fm53[] = {
	{ 0x10, KEY_POWER2},
	{ 0x06, KEY_MUTE},

	{ 0x09, KEY_1},
	{ 0x1d, KEY_2},
	{ 0x1f, KEY_3},
	{ 0x19, KEY_4},
	{ 0x1b, KEY_5},
	{ 0x11, KEY_6},
	{ 0x17, KEY_7},
	{ 0x12, KEY_8},
	{ 0x16, KEY_9},
	{ 0x48, KEY_0},

	{ 0x04, KEY_LIST},		
	{ 0x40, KEY_LAST},		

	{ 0x02, KEY_MODE},		
	{ 0x05, KEY_CAMERA},		

	{ 0x4c, KEY_CHANNELUP},		
	{ 0x00, KEY_CHANNELDOWN},	
	{ 0x0d, KEY_VOLUMEUP},		
	{ 0x15, KEY_VOLUMEDOWN},	
	{ 0x49, KEY_ENTER},		

	{ 0x54, KEY_RECORD},
	{ 0x4d, KEY_PLAY},		

	{ 0x1e, KEY_MENU},		
	{ 0x0e, KEY_RIGHT},		
	{ 0x1a, KEY_LEFT},		

	{ 0x0a, KEY_CLEAR},		
	{ 0x0c, KEY_ZOOM},		
	{ 0x47, KEY_SLEEP},		
};
struct ir_scancode_table ir_codes_encore_enltv_fm53_table = {
	.scan = ir_codes_encore_enltv_fm53,
	.size = ARRAY_SIZE(ir_codes_encore_enltv_fm53),
};
EXPORT_SYMBOL_GPL(ir_codes_encore_enltv_fm53_table);


static struct ir_scancode ir_codes_real_audio_220_32_keys[] = {
	{ 0x1c, KEY_RADIO},
	{ 0x12, KEY_POWER2},

	{ 0x01, KEY_1},
	{ 0x02, KEY_2},
	{ 0x03, KEY_3},
	{ 0x04, KEY_4},
	{ 0x05, KEY_5},
	{ 0x06, KEY_6},
	{ 0x07, KEY_7},
	{ 0x08, KEY_8},
	{ 0x09, KEY_9},
	{ 0x00, KEY_0},

	{ 0x0c, KEY_VOLUMEUP},
	{ 0x18, KEY_VOLUMEDOWN},
	{ 0x0b, KEY_CHANNELUP},
	{ 0x15, KEY_CHANNELDOWN},
	{ 0x16, KEY_ENTER},

	{ 0x11, KEY_LIST},		
	{ 0x0d, KEY_AUDIO},		

	{ 0x0f, KEY_PREVIOUS},		
	{ 0x1b, KEY_TIME},		
	{ 0x1a, KEY_NEXT},		

	{ 0x0e, KEY_STOP},
	{ 0x1f, KEY_PLAY},
	{ 0x1e, KEY_PLAYPAUSE},		

	{ 0x1d, KEY_RECORD},
	{ 0x13, KEY_MUTE},
	{ 0x19, KEY_CAMERA},		

};
struct ir_scancode_table ir_codes_real_audio_220_32_keys_table = {
	.scan = ir_codes_real_audio_220_32_keys,
	.size = ARRAY_SIZE(ir_codes_real_audio_220_32_keys),
};
EXPORT_SYMBOL_GPL(ir_codes_real_audio_220_32_keys_table);


static struct ir_scancode ir_codes_ati_tv_wonder_hd_600[] = {
	{ 0x00, KEY_RECORD},		
	{ 0x01, KEY_PLAYPAUSE},
	{ 0x02, KEY_STOP},
	{ 0x03, KEY_POWER},
	{ 0x04, KEY_PREVIOUS},	
	{ 0x05, KEY_REWIND},
	{ 0x06, KEY_FORWARD},
	{ 0x07, KEY_NEXT},
	{ 0x08, KEY_EPG},		
	{ 0x09, KEY_HOME},
	{ 0x0a, KEY_MENU},
	{ 0x0b, KEY_CHANNELUP},
	{ 0x0c, KEY_BACK},		
	{ 0x0d, KEY_UP},
	{ 0x0e, KEY_INFO},
	{ 0x0f, KEY_CHANNELDOWN},
	{ 0x10, KEY_LEFT},		
	{ 0x11, KEY_SELECT},
	{ 0x12, KEY_RIGHT},
	{ 0x13, KEY_VOLUMEUP},
	{ 0x14, KEY_LAST},		
	{ 0x15, KEY_DOWN},
	{ 0x16, KEY_MUTE},
	{ 0x17, KEY_VOLUMEDOWN},
};
struct ir_scancode_table ir_codes_ati_tv_wonder_hd_600_table = {
	.scan = ir_codes_ati_tv_wonder_hd_600,
	.size = ARRAY_SIZE(ir_codes_ati_tv_wonder_hd_600),
};
EXPORT_SYMBOL_GPL(ir_codes_ati_tv_wonder_hd_600_table);


static struct ir_scancode ir_codes_dm1105_nec[] = {
	{ 0x0a, KEY_POWER2},		
	{ 0x0c, KEY_MUTE},		
	{ 0x11, KEY_1},
	{ 0x12, KEY_2},
	{ 0x13, KEY_3},
	{ 0x14, KEY_4},
	{ 0x15, KEY_5},
	{ 0x16, KEY_6},
	{ 0x17, KEY_7},
	{ 0x18, KEY_8},
	{ 0x19, KEY_9},
	{ 0x10, KEY_0},
	{ 0x1c, KEY_CHANNELUP},		
	{ 0x0f, KEY_CHANNELDOWN},	
	{ 0x1a, KEY_VOLUMEUP},		
	{ 0x0e, KEY_VOLUMEDOWN},	
	{ 0x04, KEY_RECORD},		
	{ 0x09, KEY_CHANNEL},		
	{ 0x08, KEY_BACKSPACE},		
	{ 0x07, KEY_FASTFORWARD},	
	{ 0x0b, KEY_PAUSE},		
	{ 0x02, KEY_ESC},		
	{ 0x03, KEY_TAB},		
	{ 0x00, KEY_UP},		
	{ 0x1f, KEY_ENTER},		
	{ 0x01, KEY_DOWN},		
	{ 0x05, KEY_RECORD},		
	{ 0x06, KEY_STOP},		
	{ 0x40, KEY_ZOOM},		
	{ 0x1e, KEY_TV},		
	{ 0x1b, KEY_B},			
};
struct ir_scancode_table ir_codes_dm1105_nec_table = {
	.scan = ir_codes_dm1105_nec,
	.size = ARRAY_SIZE(ir_codes_dm1105_nec),
};
EXPORT_SYMBOL_GPL(ir_codes_dm1105_nec_table);


static struct ir_scancode ir_codes_terratec_cinergy_xs[] = {
	{ 0x41, KEY_HOME},
	{ 0x01, KEY_POWER},
	{ 0x42, KEY_MENU},
	{ 0x02, KEY_1},
	{ 0x03, KEY_2},
	{ 0x04, KEY_3},
	{ 0x43, KEY_SUBTITLE},
	{ 0x05, KEY_4},
	{ 0x06, KEY_5},
	{ 0x07, KEY_6},
	{ 0x44, KEY_TEXT},
	{ 0x08, KEY_7},
	{ 0x09, KEY_8},
	{ 0x0a, KEY_9},
	{ 0x45, KEY_DELETE},
	{ 0x0b, KEY_TUNER},
	{ 0x0c, KEY_0},
	{ 0x0d, KEY_MODE},
	{ 0x46, KEY_TV},
	{ 0x47, KEY_DVD},
	{ 0x49, KEY_VIDEO},
	{ 0x4b, KEY_AUX},
	{ 0x10, KEY_UP},
	{ 0x11, KEY_LEFT},
	{ 0x12, KEY_OK},
	{ 0x13, KEY_RIGHT},
	{ 0x14, KEY_DOWN},
	{ 0x0f, KEY_EPG},
	{ 0x16, KEY_INFO},
	{ 0x4d, KEY_BACKSPACE},
	{ 0x1c, KEY_VOLUMEUP},
	{ 0x4c, KEY_PLAY},
	{ 0x1b, KEY_CHANNELUP},
	{ 0x1e, KEY_VOLUMEDOWN},
	{ 0x1d, KEY_MUTE},
	{ 0x1f, KEY_CHANNELDOWN},
	{ 0x17, KEY_RED},
	{ 0x18, KEY_GREEN},
	{ 0x19, KEY_YELLOW},
	{ 0x1a, KEY_BLUE},
	{ 0x58, KEY_RECORD},
	{ 0x48, KEY_STOP},
	{ 0x40, KEY_PAUSE},
	{ 0x54, KEY_LAST},
	{ 0x4e, KEY_REWIND},
	{ 0x4f, KEY_FASTFORWARD},
	{ 0x5c, KEY_NEXT},
};
struct ir_scancode_table ir_codes_terratec_cinergy_xs_table = {
	.scan = ir_codes_terratec_cinergy_xs,
	.size = ARRAY_SIZE(ir_codes_terratec_cinergy_xs),
};
EXPORT_SYMBOL_GPL(ir_codes_terratec_cinergy_xs_table);


static struct ir_scancode ir_codes_evga_indtube[] = {
	{ 0x12, KEY_POWER},
	{ 0x02, KEY_MODE},	
	{ 0x14, KEY_MUTE},
	{ 0x1a, KEY_CHANNELUP},
	{ 0x16, KEY_TV2},	
	{ 0x1d, KEY_VOLUMEUP},
	{ 0x05, KEY_CHANNELDOWN},
	{ 0x0f, KEY_PLAYPAUSE},
	{ 0x19, KEY_VOLUMEDOWN},
	{ 0x1c, KEY_REWIND},
	{ 0x0d, KEY_RECORD},
	{ 0x18, KEY_FORWARD},
	{ 0x1e, KEY_PREVIOUS},
	{ 0x1b, KEY_STOP},
	{ 0x1f, KEY_NEXT},
	{ 0x13, KEY_CAMERA},
};
struct ir_scancode_table ir_codes_evga_indtube_table = {
	.scan = ir_codes_evga_indtube,
	.size = ARRAY_SIZE(ir_codes_evga_indtube),
};
EXPORT_SYMBOL_GPL(ir_codes_evga_indtube_table);

static struct ir_scancode ir_codes_videomate_s350[] = {
	{ 0x00, KEY_TV},
	{ 0x01, KEY_DVD},
	{ 0x04, KEY_RECORD},
	{ 0x05, KEY_VIDEO},	
	{ 0x07, KEY_STOP},
	{ 0x08, KEY_PLAYPAUSE},
	{ 0x0a, KEY_REWIND},
	{ 0x0f, KEY_FASTFORWARD},
	{ 0x10, KEY_CHANNELUP},
	{ 0x12, KEY_VOLUMEUP},
	{ 0x13, KEY_CHANNELDOWN},
	{ 0x14, KEY_MUTE},
	{ 0x15, KEY_VOLUMEDOWN},
	{ 0x16, KEY_1},
	{ 0x17, KEY_2},
	{ 0x18, KEY_3},
	{ 0x19, KEY_4},
	{ 0x1a, KEY_5},
	{ 0x1b, KEY_6},
	{ 0x1c, KEY_7},
	{ 0x1d, KEY_8},
	{ 0x1e, KEY_9},
	{ 0x1f, KEY_0},
	{ 0x21, KEY_SLEEP},
	{ 0x24, KEY_ZOOM},
	{ 0x25, KEY_LAST},	
	{ 0x26, KEY_SUBTITLE},	
	{ 0x27, KEY_LANGUAGE},	
	{ 0x29, KEY_CHANNEL},	
	{ 0x2b, KEY_A},
	{ 0x2c, KEY_B},
	{ 0x2f, KEY_CAMERA},	
	{ 0x23, KEY_RADIO},
	{ 0x02, KEY_PREVIOUSSONG},
	{ 0x06, KEY_NEXTSONG},
	{ 0x03, KEY_EPG},
	{ 0x09, KEY_SETUP},
	{ 0x22, KEY_BACKSPACE},
	{ 0x0c, KEY_UP},
	{ 0x0e, KEY_DOWN},
	{ 0x0b, KEY_LEFT},
	{ 0x0d, KEY_RIGHT},
	{ 0x11, KEY_ENTER},
	{ 0x20, KEY_TEXT},
};
struct ir_scancode_table ir_codes_videomate_s350_table = {
	.scan = ir_codes_videomate_s350,
	.size = ARRAY_SIZE(ir_codes_videomate_s350),
};
EXPORT_SYMBOL_GPL(ir_codes_videomate_s350_table);


static struct ir_scancode ir_codes_gadmei_rm008z[] = {
	{ 0x14, KEY_POWER2},		
	{ 0x0c, KEY_MUTE},		

	{ 0x18, KEY_TV},		
	{ 0x0e, KEY_VIDEO},		
	{ 0x0b, KEY_AUDIO},		
	{ 0x0f, KEY_RADIO},		

	{ 0x00, KEY_1},
	{ 0x01, KEY_2},
	{ 0x02, KEY_3},
	{ 0x03, KEY_4},
	{ 0x04, KEY_5},
	{ 0x05, KEY_6},
	{ 0x06, KEY_7},
	{ 0x07, KEY_8},
	{ 0x08, KEY_9},
	{ 0x09, KEY_0},
	{ 0x0a, KEY_INFO},		
	{ 0x1c, KEY_BACKSPACE},		

	{ 0x0d, KEY_PLAY},		
	{ 0x1e, KEY_CAMERA},		
	{ 0x1a, KEY_RECORD},		
	{ 0x17, KEY_STOP},		

	{ 0x1f, KEY_UP},		
	{ 0x44, KEY_DOWN},		
	{ 0x46, KEY_TAB},		
	{ 0x4a, KEY_ZOOM},		

	{ 0x10, KEY_VOLUMEUP},		
	{ 0x11, KEY_VOLUMEDOWN},	
	{ 0x12, KEY_CHANNELUP},		
	{ 0x13, KEY_CHANNELDOWN},	
	{ 0x15, KEY_ENTER},		
};
struct ir_scancode_table ir_codes_gadmei_rm008z_table = {
	.scan = ir_codes_gadmei_rm008z,
	.size = ARRAY_SIZE(ir_codes_gadmei_rm008z),
};
EXPORT_SYMBOL_GPL(ir_codes_gadmei_rm008z_table);
