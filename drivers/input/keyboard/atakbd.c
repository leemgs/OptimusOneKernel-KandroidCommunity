





#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#include <asm/atariints.h>
#include <asm/atarihw.h>
#include <asm/atarikb.h>
#include <asm/irq.h>

MODULE_AUTHOR("Michael Schmitz <schmitz@biophys.uni-duesseldorf.de>");
MODULE_DESCRIPTION("Atari keyboard driver");
MODULE_LICENSE("GPL");




static unsigned char atakbd_keycode[0x72] = {	
	[0]	 = KEY_GRAVE,
	[1]	 = KEY_ESC,
	[2]	 = KEY_1,
	[3]	 = KEY_2,
	[4]	 = KEY_3,
	[5]	 = KEY_4,
	[6]	 = KEY_5,
	[7]	 = KEY_6,
	[8]	 = KEY_7,
	[9]	 = KEY_8,
	[10]	 = KEY_9,
	[11]	 = KEY_0,
	[12]	 = KEY_MINUS,
	[13]	 = KEY_EQUAL,
	[14]	 = KEY_BACKSPACE,
	[15]	 = KEY_TAB,
	[16]	 = KEY_Q,
	[17]	 = KEY_W,
	[18]	 = KEY_E,
	[19]	 = KEY_R,
	[20]	 = KEY_T,
	[21]	 = KEY_Y,
	[22]	 = KEY_U,
	[23]	 = KEY_I,
	[24]	 = KEY_O,
	[25]	 = KEY_P,
	[26]	 = KEY_LEFTBRACE,
	[27]	 = KEY_RIGHTBRACE,
	[28]	 = KEY_ENTER,
	[29]	 = KEY_LEFTCTRL,
	[30]	 = KEY_A,
	[31]	 = KEY_S,
	[32]	 = KEY_D,
	[33]	 = KEY_F,
	[34]	 = KEY_G,
	[35]	 = KEY_H,
	[36]	 = KEY_J,
	[37]	 = KEY_K,
	[38]	 = KEY_L,
	[39]	 = KEY_SEMICOLON,
	[40]	 = KEY_APOSTROPHE,
	[41]	 = KEY_BACKSLASH,	
	[42]	 = KEY_LEFTSHIFT,
	[43]	 = KEY_GRAVE,		
	[44]	 = KEY_Z,
	[45]	 = KEY_X,
	[46]	 = KEY_C,
	[47]	 = KEY_V,
	[48]	 = KEY_B,
	[49]	 = KEY_N,
	[50]	 = KEY_M,
	[51]	 = KEY_COMMA,
	[52]	 = KEY_DOT,
	[53]	 = KEY_SLASH,
	[54]	 = KEY_RIGHTSHIFT,
	[55]	 = KEY_KPASTERISK,
	[56]	 = KEY_LEFTALT,
	[57]	 = KEY_SPACE,
	[58]	 = KEY_CAPSLOCK,
	[59]	 = KEY_F1,
	[60]	 = KEY_F2,
	[61]	 = KEY_F3,
	[62]	 = KEY_F4,
	[63]	 = KEY_F5,
	[64]	 = KEY_F6,
	[65]	 = KEY_F7,
	[66]	 = KEY_F8,
	[67]	 = KEY_F9,
	[68]	 = KEY_F10,
	[69]	 = KEY_ESC,
	[70]	 = KEY_DELETE,
	[71]	 = KEY_KP7,
	[72]	 = KEY_KP8,
	[73]	 = KEY_KP9,
	[74]	 = KEY_KPMINUS,
	[75]	 = KEY_KP4,
	[76]	 = KEY_KP5,
	[77]	 = KEY_KP6,
	[78]	 = KEY_KPPLUS,
	[79]	 = KEY_KP1,
	[80]	 = KEY_KP2,
	[81]	 = KEY_KP3,
	[82]	 = KEY_KP0,
	[83]	 = KEY_KPDOT,
	[90]	 = KEY_KPLEFTPAREN,
	[91]	 = KEY_KPRIGHTPAREN,
	[92]	 = KEY_KPASTERISK,	
	[93]	 = KEY_KPASTERISK,
	[94]	 = KEY_KPPLUS,
	[95]	 = KEY_HELP,
	[96]	 = KEY_BACKSLASH,	
	[97]	 = KEY_KPASTERISK,	
	[98]	 = KEY_KPSLASH,
	[99]	 = KEY_KPLEFTPAREN,
	[100]	 = KEY_KPRIGHTPAREN,
	[101]	 = KEY_KPSLASH,
	[102]	 = KEY_KPASTERISK,
	[103]	 = KEY_UP,
	[104]	 = KEY_KPASTERISK,	
	[105]	 = KEY_LEFT,
	[106]	 = KEY_RIGHT,
	[107]	 = KEY_KPASTERISK,	
	[108]	 = KEY_DOWN,
	[109]	 = KEY_KPASTERISK,	
	[110]	 = KEY_KPASTERISK,	
	[111]	 = KEY_KPASTERISK,	
	[112]	 = KEY_KPASTERISK,	
	[113]	 = KEY_KPASTERISK	
};

static struct input_dev *atakbd_dev;

static void atakbd_interrupt(unsigned char scancode, char down)
{

	if (scancode < 0x72) {		

		

		scancode = atakbd_keycode[scancode];

		if (scancode == KEY_CAPSLOCK) {	
			input_report_key(atakbd_dev, scancode, 1);
			input_report_key(atakbd_dev, scancode, 0);
			input_sync(atakbd_dev);
		} else {
			input_report_key(atakbd_dev, scancode, down);
			input_sync(atakbd_dev);
		}
	} else				
		printk(KERN_INFO "atakbd: unhandled scancode %x\n", scancode);

	return;
}

static int __init atakbd_init(void)
{
	int i, error;

	if (!MACH_IS_ATARI || !ATARIHW_PRESENT(ST_MFP))
		return -ENODEV;

	
	if (atari_keyb_init())
		return -ENODEV;

	atakbd_dev = input_allocate_device();
	if (!atakbd_dev)
		return -ENOMEM;

	atakbd_dev->name = "Atari Keyboard";
	atakbd_dev->phys = "atakbd/input0";
	atakbd_dev->id.bustype = BUS_HOST;
	atakbd_dev->id.vendor = 0x0001;
	atakbd_dev->id.product = 0x0001;
	atakbd_dev->id.version = 0x0100;

	atakbd_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	atakbd_dev->keycode = atakbd_keycode;
	atakbd_dev->keycodesize = sizeof(unsigned char);
	atakbd_dev->keycodemax = ARRAY_SIZE(atakbd_keycode);

	for (i = 1; i < 0x72; i++) {
		set_bit(atakbd_keycode[i], atakbd_dev->keybit);
	}

	
	error = input_register_device(atakbd_dev);
	if (error) {
		input_free_device(atakbd_dev);
		return error;
	}

	atari_input_keyboard_interrupt_hook = atakbd_interrupt;

	return 0;
}

static void __exit atakbd_exit(void)
{
	atari_input_keyboard_interrupt_hook = NULL;
	input_unregister_device(atakbd_dev);
}

module_init(atakbd_init);
module_exit(atakbd_exit);
