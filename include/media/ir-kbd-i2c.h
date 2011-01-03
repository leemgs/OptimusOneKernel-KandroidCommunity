#ifndef _IR_I2C
#define _IR_I2C

#include <media/ir-common.h>

struct IR_i2c;

struct IR_i2c {
	struct ir_scancode_table *ir_codes;

	struct i2c_client      *c;
	struct input_dev       *input;
	struct ir_input_state  ir;

	
	unsigned char          old;

	struct delayed_work    work;
	char                   name[32];
	char                   phys[32];
	int                    (*get_key)(struct IR_i2c*, u32*, u32*);
};

enum ir_kbd_get_key_fn {
	IR_KBD_GET_KEY_CUSTOM = 0,
	IR_KBD_GET_KEY_PIXELVIEW,
	IR_KBD_GET_KEY_PV951,
	IR_KBD_GET_KEY_HAUP,
	IR_KBD_GET_KEY_KNC1,
	IR_KBD_GET_KEY_FUSIONHDTV,
	IR_KBD_GET_KEY_HAUP_XVR,
	IR_KBD_GET_KEY_AVERMEDIA_CARDBUS,
};


struct IR_i2c_init_data {
	struct ir_scancode_table *ir_codes;
	const char             *name;
	int                    type; 
	
	int                    (*get_key)(struct IR_i2c*, u32*, u32*);
	enum ir_kbd_get_key_fn internal_get_key_func;
};
#endif
