

#include <linux/module.h>
#include <linux/pnp.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/pci_ids.h>
#include <linux/io.h>
#include <linux/bitrev.h>
#include <linux/bitops.h>

#define DRVNAME "winbond-cir"


#define WBCIR_REG_WCEIR_CTL	0x03 
#define WBCIR_REG_WCEIR_STS	0x04 
#define WBCIR_REG_WCEIR_EV_EN	0x05 
#define WBCIR_REG_WCEIR_CNTL	0x06 
#define WBCIR_REG_WCEIR_CNTH	0x07 
#define WBCIR_REG_WCEIR_INDEX	0x08 
#define WBCIR_REG_WCEIR_DATA	0x09 
#define WBCIR_REG_WCEIR_CSL	0x0A 
#define WBCIR_REG_WCEIR_CFG1	0x0B 
#define WBCIR_REG_WCEIR_CFG2	0x0C 


#define WBCIR_REG_ECEIR_CTS	0x00 
#define WBCIR_REG_ECEIR_CCTL	0x01 
#define WBCIR_REG_ECEIR_CNT_LO	0x02 
#define WBCIR_REG_ECEIR_CNT_HI	0x03 
#define WBCIR_REG_ECEIR_IREM	0x04 


#define WBCIR_REG_SP3_BSR	0x03 
				      
#define WBCIR_REG_SP3_RXDATA	0x00 
#define WBCIR_REG_SP3_TXDATA	0x00 
#define WBCIR_REG_SP3_IER	0x01 
#define WBCIR_REG_SP3_EIR	0x02 
#define WBCIR_REG_SP3_FCR	0x02 
#define WBCIR_REG_SP3_MCR	0x04 
#define WBCIR_REG_SP3_LSR	0x05 
#define WBCIR_REG_SP3_MSR	0x06 
#define WBCIR_REG_SP3_ASCR	0x07 
				      
#define WBCIR_REG_SP3_BGDL	0x00 
#define WBCIR_REG_SP3_BGDH	0x01 
#define WBCIR_REG_SP3_EXCR1	0x02 
#define WBCIR_REG_SP3_EXCR2	0x04 
#define WBCIR_REG_SP3_TXFLV	0x06 
#define WBCIR_REG_SP3_RXFLV	0x07 
				      
#define WBCIR_REG_SP3_MRID	0x00 
#define WBCIR_REG_SP3_SH_LCR	0x01 
#define WBCIR_REG_SP3_SH_FCR	0x02 
				      
#define WBCIR_REG_SP3_IRCR1	0x02 
				      
#define WBCIR_REG_SP3_IRCR2	0x04 
				      
#define WBCIR_REG_SP3_IRCR3	0x00 
#define WBCIR_REG_SP3_SIR_PW	0x02 
				      
#define WBCIR_REG_SP3_IRRXDC	0x00 
#define WBCIR_REG_SP3_IRTXMC	0x01 
#define WBCIR_REG_SP3_RCCFG	0x02 
#define WBCIR_REG_SP3_IRCFG1	0x04 
#define WBCIR_REG_SP3_IRCFG4	0x07 




#define WBCIR_IRQ_NONE		0x00

#define WBCIR_IRQ_RX		0x01

#define WBCIR_IRQ_ERR		0x04

#define WBCIR_LED_ENABLE	0x80

#define WBCIR_RX_AVAIL		0x01

#define WBCIR_RX_DISABLE	0x20

#define WBCIR_EXT_ENABLE	0x01

#define WBCIR_REGSEL_COMPARE	0x10

#define WBCIR_REGSEL_MASK	0x20

#define WBCIR_REG_ADDR0		0x00


enum wbcir_bank {
	WBCIR_BANK_0          = 0x00,
	WBCIR_BANK_1          = 0x80,
	WBCIR_BANK_2          = 0xE0,
	WBCIR_BANK_3          = 0xE4,
	WBCIR_BANK_4          = 0xE8,
	WBCIR_BANK_5          = 0xEC,
	WBCIR_BANK_6          = 0xF0,
	WBCIR_BANK_7          = 0xF4,
};


enum wbcir_protocol {
	IR_PROTOCOL_RC5          = 0x0,
	IR_PROTOCOL_NEC          = 0x1,
	IR_PROTOCOL_RC6          = 0x2,
};


#define WBCIR_NAME	"Winbond CIR"
#define WBCIR_ID_FAMILY          0xF1 
#define	WBCIR_ID_CHIP            0x04 
#define IR_KEYPRESS_TIMEOUT       250 
#define INVALID_SCANCODE   0x7FFFFFFF 
#define WAKEUP_IOMEM_LEN         0x10 
#define EHFUNC_IOMEM_LEN         0x10 
#define SP_IOMEM_LEN             0x08 
#define WBCIR_MAX_IDLE_BYTES       10

static DEFINE_SPINLOCK(wbcir_lock);
static DEFINE_RWLOCK(keytable_lock);

struct wbcir_key {
	u32 scancode;
	unsigned int keycode;
};

struct wbcir_keyentry {
	struct wbcir_key key;
	struct list_head list;
};

static struct wbcir_key rc6_def_keymap[] = {
	{ 0x800F0400, KEY_NUMERIC_0		},
	{ 0x800F0401, KEY_NUMERIC_1		},
	{ 0x800F0402, KEY_NUMERIC_2		},
	{ 0x800F0403, KEY_NUMERIC_3		},
	{ 0x800F0404, KEY_NUMERIC_4		},
	{ 0x800F0405, KEY_NUMERIC_5		},
	{ 0x800F0406, KEY_NUMERIC_6		},
	{ 0x800F0407, KEY_NUMERIC_7		},
	{ 0x800F0408, KEY_NUMERIC_8		},
	{ 0x800F0409, KEY_NUMERIC_9		},
	{ 0x800F041D, KEY_NUMERIC_STAR		},
	{ 0x800F041C, KEY_NUMERIC_POUND		},
	{ 0x800F0410, KEY_VOLUMEUP		},
	{ 0x800F0411, KEY_VOLUMEDOWN		},
	{ 0x800F0412, KEY_CHANNELUP		},
	{ 0x800F0413, KEY_CHANNELDOWN		},
	{ 0x800F040E, KEY_MUTE			},
	{ 0x800F040D, KEY_VENDOR		}, 
	{ 0x800F041E, KEY_UP			},
	{ 0x800F041F, KEY_DOWN			},
	{ 0x800F0420, KEY_LEFT			},
	{ 0x800F0421, KEY_RIGHT			},
	{ 0x800F0422, KEY_OK			},
	{ 0x800F0423, KEY_ESC			},
	{ 0x800F040F, KEY_INFO			},
	{ 0x800F040A, KEY_CLEAR			},
	{ 0x800F040B, KEY_ENTER			},
	{ 0x800F045B, KEY_RED			},
	{ 0x800F045C, KEY_GREEN			},
	{ 0x800F045D, KEY_YELLOW		},
	{ 0x800F045E, KEY_BLUE			},
	{ 0x800F045A, KEY_TEXT			},
	{ 0x800F0427, KEY_SWITCHVIDEOMODE	},
	{ 0x800F040C, KEY_POWER			},
	{ 0x800F0450, KEY_RADIO			},
	{ 0x800F0448, KEY_PVR			},
	{ 0x800F0447, KEY_AUDIO			},
	{ 0x800F0426, KEY_EPG			},
	{ 0x800F0449, KEY_CAMERA		},
	{ 0x800F0425, KEY_TV			},
	{ 0x800F044A, KEY_VIDEO			},
	{ 0x800F0424, KEY_DVD			},
	{ 0x800F0416, KEY_PLAY			},
	{ 0x800F0418, KEY_PAUSE			},
	{ 0x800F0419, KEY_STOP			},
	{ 0x800F0414, KEY_FASTFORWARD		},
	{ 0x800F041A, KEY_NEXT			},
	{ 0x800F041B, KEY_PREVIOUS		},
	{ 0x800F0415, KEY_REWIND		},
	{ 0x800F0417, KEY_RECORD		},
};


struct wbcir_data {
	unsigned long wbase;        
	unsigned long ebase;        
	unsigned long sbase;        
	unsigned int  irq;          

	struct input_dev *input_dev;
	struct timer_list timer_keyup;
	struct led_trigger *rxtrigger;
	struct led_trigger *txtrigger;
	struct led_classdev led;

	u32 last_scancode;
	unsigned int last_keycode;
	u8 last_toggle;
	u8 keypressed;
	unsigned long keyup_jiffies;
	unsigned int idle_count;

	
	unsigned long irdata[30];
	unsigned int irdata_count;
	unsigned int irdata_idle;
	unsigned int irdata_off;
	unsigned int irdata_error;

	
	struct list_head keytable;
};

static enum wbcir_protocol protocol = IR_PROTOCOL_RC6;
module_param(protocol, uint, 0444);
MODULE_PARM_DESC(protocol, "IR protocol to use "
		 "(0 = RC5, 1 = NEC, 2 = RC6A, default)");

static int invert; 
module_param(invert, bool, 0444);
MODULE_PARM_DESC(invert, "Invert the signal from the IR receiver");

static unsigned int wake_sc = 0x800F040C;
module_param(wake_sc, uint, 0644);
MODULE_PARM_DESC(wake_sc, "Scancode of the power-on IR command");

static unsigned int wake_rc6mode = 6;
module_param(wake_rc6mode, uint, 0644);
MODULE_PARM_DESC(wake_rc6mode, "RC6 mode for the power-on command "
		 "(0 = 0, 6 = 6A, default)");






static void
wbcir_set_bits(unsigned long addr, u8 bits, u8 mask)
{
	u8 val;

	val = inb(addr);
	val = ((val & ~mask) | (bits & mask));
	outb(val, addr);
}


static inline void
wbcir_select_bank(struct wbcir_data *data, enum wbcir_bank bank)
{
	outb(bank, data->sbase + WBCIR_REG_SP3_BSR);
}

static enum led_brightness
wbcir_led_brightness_get(struct led_classdev *led_cdev)
{
	struct wbcir_data *data = container_of(led_cdev,
					       struct wbcir_data,
					       led);

	if (inb(data->ebase + WBCIR_REG_ECEIR_CTS) & WBCIR_LED_ENABLE)
		return LED_FULL;
	else
		return LED_OFF;
}

static void
wbcir_led_brightness_set(struct led_classdev *led_cdev,
			    enum led_brightness brightness)
{
	struct wbcir_data *data = container_of(led_cdev,
					       struct wbcir_data,
					       led);

	wbcir_set_bits(data->ebase + WBCIR_REG_ECEIR_CTS,
		       brightness == LED_OFF ? 0x00 : WBCIR_LED_ENABLE,
		       WBCIR_LED_ENABLE);
}


static u8
wbcir_to_rc6cells(u8 val)
{
	u8 coded = 0x00;
	int i;

	val &= 0x0F;
	for (i = 0; i < 4; i++) {
		if (val & 0x01)
			coded |= 0x02 << (i * 2);
		else
			coded |= 0x01 << (i * 2);
		val >>= 1;
	}

	return coded;
}





static unsigned int
wbcir_do_getkeycode(struct wbcir_data *data, u32 scancode)
{
	struct wbcir_keyentry *keyentry;
	unsigned int keycode = KEY_RESERVED;
	unsigned long flags;

	read_lock_irqsave(&keytable_lock, flags);

	list_for_each_entry(keyentry, &data->keytable, list) {
		if (keyentry->key.scancode == scancode) {
			keycode = keyentry->key.keycode;
			break;
		}
	}

	read_unlock_irqrestore(&keytable_lock, flags);
	return keycode;
}

static int
wbcir_getkeycode(struct input_dev *dev, int scancode, int *keycode)
{
	struct wbcir_data *data = input_get_drvdata(dev);

	*keycode = (int)wbcir_do_getkeycode(data, (u32)scancode);
	return 0;
}

static int
wbcir_setkeycode(struct input_dev *dev, int sscancode, int keycode)
{
	struct wbcir_data *data = input_get_drvdata(dev);
	struct wbcir_keyentry *keyentry;
	struct wbcir_keyentry *new_keyentry;
	unsigned long flags;
	unsigned int old_keycode = KEY_RESERVED;
	u32 scancode = (u32)sscancode;

	if (keycode < 0 || keycode > KEY_MAX)
		return -EINVAL;

	new_keyentry = kmalloc(sizeof(*new_keyentry), GFP_KERNEL);
	if (!new_keyentry)
		return -ENOMEM;

	write_lock_irqsave(&keytable_lock, flags);

	list_for_each_entry(keyentry, &data->keytable, list) {
		if (keyentry->key.scancode != scancode)
			continue;

		old_keycode = keyentry->key.keycode;
		keyentry->key.keycode = keycode;

		if (keyentry->key.keycode == KEY_RESERVED) {
			list_del(&keyentry->list);
			kfree(keyentry);
		}

		break;
	}

	set_bit(keycode, dev->keybit);

	if (old_keycode == KEY_RESERVED) {
		new_keyentry->key.scancode = scancode;
		new_keyentry->key.keycode = keycode;
		list_add(&new_keyentry->list, &data->keytable);
	} else {
		kfree(new_keyentry);
		clear_bit(old_keycode, dev->keybit);
		list_for_each_entry(keyentry, &data->keytable, list) {
			if (keyentry->key.keycode == old_keycode) {
				set_bit(old_keycode, dev->keybit);
				break;
			}
		}
	}

	write_unlock_irqrestore(&keytable_lock, flags);
	return 0;
}


static void
wbcir_keyup(unsigned long cookie)
{
	struct wbcir_data *data = (struct wbcir_data *)cookie;
	unsigned long flags;

	

	spin_lock_irqsave(&wbcir_lock, flags);

	if (time_is_after_eq_jiffies(data->keyup_jiffies) && data->keypressed) {
		data->keypressed = 0;
		led_trigger_event(data->rxtrigger, LED_OFF);
		input_report_key(data->input_dev, data->last_keycode, 0);
		input_sync(data->input_dev);
	}

	spin_unlock_irqrestore(&wbcir_lock, flags);
}

static void
wbcir_keydown(struct wbcir_data *data, u32 scancode, u8 toggle)
{
	unsigned int keycode;

	
	if (data->last_scancode == scancode &&
	    data->last_toggle == toggle &&
	    data->keypressed)
		goto set_timer;
	data->last_scancode = scancode;

	
	if (data->keypressed) {
		input_report_key(data->input_dev, data->last_keycode, 0);
		input_sync(data->input_dev);
		data->keypressed = 0;
	}

	
	input_event(data->input_dev, EV_MSC, MSC_SCAN, (int)scancode);

	
	keycode = wbcir_do_getkeycode(data, scancode);
	if (keycode == KEY_RESERVED)
		goto set_timer;

	
	input_report_key(data->input_dev, keycode, 1);
	data->keypressed = 1;
	data->last_keycode = keycode;
	data->last_toggle = toggle;

set_timer:
	input_sync(data->input_dev);
	led_trigger_event(data->rxtrigger,
			  data->keypressed ? LED_FULL : LED_OFF);
	data->keyup_jiffies = jiffies + msecs_to_jiffies(IR_KEYPRESS_TIMEOUT);
	mod_timer(&data->timer_keyup, data->keyup_jiffies);
}






static void
wbcir_reset_irdata(struct wbcir_data *data)
{
	memset(data->irdata, 0, sizeof(data->irdata));
	data->irdata_count = 0;
	data->irdata_off = 0;
	data->irdata_error = 0;
}


static void
add_irdata_bit(struct wbcir_data *data, int set)
{
	if (data->irdata_count >= sizeof(data->irdata) * 8) {
		data->irdata_error = 1;
		return;
	}

	if (set)
		__set_bit(data->irdata_count, data->irdata);
	data->irdata_count++;
}


static u16
get_bits(struct wbcir_data *data, int count)
{
	u16 val = 0x0;

	if (data->irdata_count - data->irdata_off < count) {
		data->irdata_error = 1;
		return 0x0;
	}

	while (count > 0) {
		val <<= 1;
		if (test_bit(data->irdata_off, data->irdata))
			val |= 0x1;
		count--;
		data->irdata_off++;
	}

	return val;
}


static u8
wbcir_rc6cells_to_byte(struct wbcir_data *data)
{
	u16 raw = get_bits(data, 16);
	u8 val = 0x00;
	int bit;

	for (bit = 0; bit < 8; bit++) {
		switch (raw & 0x03) {
		case 0x01:
			break;
		case 0x02:
			val |= (0x01 << bit);
			break;
		default:
			data->irdata_error = 1;
			break;
		}
		raw >>= 2;
	}

	return val;
}


static u8
wbcir_get_rc5bits(struct wbcir_data *data, unsigned int count)
{
	u16 raw = get_bits(data, count * 2);
	u8 val = 0x00;
	int bit;

	for (bit = 0; bit < count; bit++) {
		switch (raw & 0x03) {
		case 0x01:
			val |= (0x01 << bit);
			break;
		case 0x02:
			break;
		default:
			data->irdata_error = 1;
			break;
		}
		raw >>= 2;
	}

	return val;
}

static void
wbcir_parse_rc6(struct device *dev, struct wbcir_data *data)
{
	
	u8 mode;
	u8 toggle;
	u16 customer = 0x0;
	u8 address;
	u8 command;
	u32 scancode;

	
	while (get_bits(data, 1) && !data->irdata_error)
		;

	
	if (get_bits(data, 1)) {
		dev_dbg(dev, "RC6 - Invalid leader space\n");
		return;
	}

	
	if (get_bits(data, 2) != 0x02) {
		dev_dbg(dev, "RC6 - Invalid start bit\n");
		return;
	}

	
	mode = get_bits(data, 6);
	switch (mode) {
	case 0x15: 
		mode = 0;
		break;
	case 0x29: 
		mode = 6;
		break;
	default:
		dev_dbg(dev, "RC6 - Invalid mode\n");
		return;
	}

	
	toggle = get_bits(data, 4);
	switch (toggle) {
	case 0x03:
		toggle = 0;
		break;
	case 0x0C:
		toggle = 1;
		break;
	default:
		dev_dbg(dev, "RC6 - Toggle bit error\n");
		break;
	}

	
	if (mode == 6) {
		if (toggle != 0) {
			dev_dbg(dev, "RC6B - Not Supported\n");
			return;
		}

		customer = wbcir_rc6cells_to_byte(data);

		if (customer & 0x80) {
			
			customer <<= 8;
			customer |= wbcir_rc6cells_to_byte(data);
		}
	}

	
	address = wbcir_rc6cells_to_byte(data);
	if (mode == 6) {
		toggle = address >> 7;
		address &= 0x7F;
	}

	
	command = wbcir_rc6cells_to_byte(data);

	
	scancode =  command;
	scancode |= address << 8;
	scancode |= customer << 16;

	
	if (data->irdata_error) {
		dev_dbg(dev, "RC6 - Cell error(s)\n");
		return;
	}

	dev_dbg(dev, "IR-RC6 ad 0x%02X cm 0x%02X cu 0x%04X "
		"toggle %u mode %u scan 0x%08X\n",
		address,
		command,
		customer,
		(unsigned int)toggle,
		(unsigned int)mode,
		scancode);

	wbcir_keydown(data, scancode, toggle);
}

static void
wbcir_parse_rc5(struct device *dev, struct wbcir_data *data)
{
	
	u8 toggle;
	u8 address;
	u8 command;
	u32 scancode;

	
	if (!get_bits(data, 1)) {
		dev_dbg(dev, "RC5 - Invalid start bit\n");
		return;
	}

	
	if (!wbcir_get_rc5bits(data, 1))
		command = 0x40;
	else
		command = 0x00;

	toggle   = wbcir_get_rc5bits(data, 1);
	address  = wbcir_get_rc5bits(data, 5);
	command |= wbcir_get_rc5bits(data, 6);
	scancode = address << 7 | command;

	
	if (data->irdata_error) {
		dev_dbg(dev, "RC5 - Invalid message\n");
		return;
	}

	dev_dbg(dev, "IR-RC5 ad %u cm %u t %u s %u\n",
		(unsigned int)address,
		(unsigned int)command,
		(unsigned int)toggle,
		(unsigned int)scancode);

	wbcir_keydown(data, scancode, toggle);
}

static void
wbcir_parse_nec(struct device *dev, struct wbcir_data *data)
{
	
	u8 address1;
	u8 address2;
	u8 command1;
	u8 command2;
	u16 address;
	u32 scancode;

	
	while (get_bits(data, 1) && !data->irdata_error)
		;

	
	if (get_bits(data, 4)) {
		dev_dbg(dev, "NEC - Invalid leader space\n");
		return;
	}

	
	if (get_bits(data, 1)) {
		if (!data->keypressed) {
			dev_dbg(dev, "NEC - Stray repeat message\n");
			return;
		}

		dev_dbg(dev, "IR-NEC repeat s %u\n",
			(unsigned int)data->last_scancode);

		wbcir_keydown(data, data->last_scancode, data->last_toggle);
		return;
	}

	
	if (get_bits(data, 3)) {
		dev_dbg(dev, "NEC - Invalid leader space\n");
		return;
	}

	address1  = bitrev8(get_bits(data, 8));
	address2  = bitrev8(get_bits(data, 8));
	command1  = bitrev8(get_bits(data, 8));
	command2  = bitrev8(get_bits(data, 8));

	
	if (data->irdata_error) {
		dev_dbg(dev, "NEC - Invalid message\n");
		return;
	}

	
	if (command1 != ~command2) {
		dev_dbg(dev, "NEC - Command bytes mismatch\n");
		return;
	}

	
	address = address1;
	if (address1 != ~address2)
		address |= address2 << 8;

	scancode = address << 8 | command1;

	dev_dbg(dev, "IR-NEC ad %u cm %u s %u\n",
		(unsigned int)address,
		(unsigned int)command1,
		(unsigned int)scancode);

	wbcir_keydown(data, scancode, !data->last_toggle);
}





static irqreturn_t
wbcir_irq_handler(int irqno, void *cookie)
{
	struct pnp_dev *device = cookie;
	struct wbcir_data *data = pnp_get_drvdata(device);
	struct device *dev = &device->dev;
	u8 status;
	unsigned long flags;
	u8 irdata[8];
	int i;
	unsigned int hw;

	spin_lock_irqsave(&wbcir_lock, flags);

	wbcir_select_bank(data, WBCIR_BANK_0);

	status = inb(data->sbase + WBCIR_REG_SP3_EIR);

	if (!(status & (WBCIR_IRQ_RX | WBCIR_IRQ_ERR))) {
		spin_unlock_irqrestore(&wbcir_lock, flags);
		return IRQ_NONE;
	}

	if (status & WBCIR_IRQ_ERR)
		data->irdata_error = 1;

	if (!(status & WBCIR_IRQ_RX))
		goto out;

	
	insb(data->sbase + WBCIR_REG_SP3_RXDATA, &irdata[0], 8);

	for (i = 0; i < sizeof(irdata); i++) {
		hw = hweight8(irdata[i]);
		if (hw > 4)
			add_irdata_bit(data, 0);
		else
			add_irdata_bit(data, 1);

		if (hw == 8)
			data->idle_count++;
		else
			data->idle_count = 0;
	}

	if (data->idle_count > WBCIR_MAX_IDLE_BYTES) {
		
		outb(WBCIR_RX_DISABLE, data->sbase + WBCIR_REG_SP3_ASCR);

		
		while (inb(data->sbase + WBCIR_REG_SP3_LSR) & WBCIR_RX_AVAIL)
			inb(data->sbase + WBCIR_REG_SP3_RXDATA);

		dev_dbg(dev, "IRDATA:\n");
		for (i = 0; i < data->irdata_count; i += BITS_PER_LONG)
			dev_dbg(dev, "0x%08lX\n", data->irdata[i/BITS_PER_LONG]);

		switch (protocol) {
		case IR_PROTOCOL_RC5:
			wbcir_parse_rc5(dev, data);
			break;
		case IR_PROTOCOL_RC6:
			wbcir_parse_rc6(dev, data);
			break;
		case IR_PROTOCOL_NEC:
			wbcir_parse_nec(dev, data);
			break;
		}

		wbcir_reset_irdata(data);
		data->idle_count = 0;
	}

out:
	spin_unlock_irqrestore(&wbcir_lock, flags);
	return IRQ_HANDLED;
}





static void
wbcir_shutdown(struct pnp_dev *device)
{
	struct device *dev = &device->dev;
	struct wbcir_data *data = pnp_get_drvdata(device);
	int do_wake = 1;
	u8 match[11];
	u8 mask[11];
	u8 rc6_csl = 0;
	int i;

	memset(match, 0, sizeof(match));
	memset(mask, 0, sizeof(mask));

	if (wake_sc == INVALID_SCANCODE || !device_may_wakeup(dev)) {
		do_wake = 0;
		goto finish;
	}

	switch (protocol) {
	case IR_PROTOCOL_RC5:
		if (wake_sc > 0xFFF) {
			do_wake = 0;
			dev_err(dev, "RC5 - Invalid wake scancode\n");
			break;
		}

		
		mask[0] = 0xFF;
		mask[1] = 0x17;

		match[0]  = (wake_sc & 0x003F);      
		match[0] |= (wake_sc & 0x0180) >> 1; 
		match[1]  = (wake_sc & 0x0E00) >> 9; 
		if (!(wake_sc & 0x0040))             
			match[1] |= 0x10;

		break;

	case IR_PROTOCOL_NEC:
		if (wake_sc > 0xFFFFFF) {
			do_wake = 0;
			dev_err(dev, "NEC - Invalid wake scancode\n");
			break;
		}

		mask[0] = mask[1] = mask[2] = mask[3] = 0xFF;

		match[1] = bitrev8((wake_sc & 0xFF));
		match[0] = ~match[1];

		match[3] = bitrev8((wake_sc & 0xFF00) >> 8);
		if (wake_sc > 0xFFFF)
			match[2] = bitrev8((wake_sc & 0xFF0000) >> 16);
		else
			match[2] = ~match[3];

		break;

	case IR_PROTOCOL_RC6:

		if (wake_rc6mode == 0) {
			if (wake_sc > 0xFFFF) {
				do_wake = 0;
				dev_err(dev, "RC6 - Invalid wake scancode\n");
				break;
			}

			
			match[0] = wbcir_to_rc6cells(wake_sc >>  0);
			mask[0]  = 0xFF;
			match[1] = wbcir_to_rc6cells(wake_sc >>  4);
			mask[1]  = 0xFF;

			
			match[2] = wbcir_to_rc6cells(wake_sc >>  8);
			mask[2]  = 0xFF;
			match[3] = wbcir_to_rc6cells(wake_sc >> 12);
			mask[3]  = 0xFF;

			
			match[4] = 0x50; 
			mask[4]  = 0xF0;
			match[5] = 0x09; 
			mask[5]  = 0x0F;

			rc6_csl = 44;

		} else if (wake_rc6mode == 6) {
			i = 0;

			
			match[i]  = wbcir_to_rc6cells(wake_sc >>  0);
			mask[i++] = 0xFF;
			match[i]  = wbcir_to_rc6cells(wake_sc >>  4);
			mask[i++] = 0xFF;

			
			match[i]  = wbcir_to_rc6cells(wake_sc >>  8);
			mask[i++] = 0xFF;
			match[i]  = wbcir_to_rc6cells(wake_sc >> 12);
			mask[i++] = 0x3F;

			
			match[i]  = wbcir_to_rc6cells(wake_sc >> 16);
			mask[i++] = 0xFF;
			match[i]  = wbcir_to_rc6cells(wake_sc >> 20);
			mask[i++] = 0xFF;

			if (wake_sc & 0x80000000) {
				
				match[i]  = wbcir_to_rc6cells(wake_sc >> 24);
				mask[i++] = 0xFF;
				match[i]  = wbcir_to_rc6cells(wake_sc >> 28);
				mask[i++] = 0xFF;
				rc6_csl = 76;
			} else if (wake_sc <= 0x007FFFFF) {
				rc6_csl = 60;
			} else {
				do_wake = 0;
				dev_err(dev, "RC6 - Invalid wake scancode\n");
				break;
			}

			
			match[i]  = 0x93; 
			mask[i++] = 0xFF;
			match[i]  = 0x0A; 
			mask[i++] = 0x0F;

		} else {
			do_wake = 0;
			dev_err(dev, "RC6 - Invalid wake mode\n");
		}

		break;

	default:
		do_wake = 0;
		break;
	}

finish:
	if (do_wake) {
		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_INDEX,
			       WBCIR_REGSEL_COMPARE | WBCIR_REG_ADDR0,
			       0x3F);
		outsb(data->wbase + WBCIR_REG_WCEIR_DATA, match, 11);
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_INDEX,
			       WBCIR_REGSEL_MASK | WBCIR_REG_ADDR0,
			       0x3F);
		outsb(data->wbase + WBCIR_REG_WCEIR_DATA, mask, 11);

		
		outb(rc6_csl, data->wbase + WBCIR_REG_WCEIR_CSL);

		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_STS, 0x17, 0x17);

		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_EV_EN, 0x01, 0x07);

		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_CTL, 0x01, 0x01);

	} else {
		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_EV_EN, 0x00, 0x07);

		
		wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_CTL, 0x00, 0x01);
	}

	
	outb(WBCIR_IRQ_NONE, data->sbase + WBCIR_REG_SP3_IER);
}

static int
wbcir_suspend(struct pnp_dev *device, pm_message_t state)
{
	wbcir_shutdown(device);
	return 0;
}

static int
wbcir_resume(struct pnp_dev *device)
{
	struct wbcir_data *data = pnp_get_drvdata(device);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_EV_EN, 0x00, 0x07);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_CTL, 0x00, 0x01);

	
	wbcir_reset_irdata(data);
	outb(WBCIR_IRQ_RX | WBCIR_IRQ_ERR, data->sbase + WBCIR_REG_SP3_IER);

	return 0;
}





static void
wbcir_cfg_ceir(struct wbcir_data *data)
{
	u8 tmp;

	
	tmp = protocol << 4;
	if (invert)
		tmp |= 0x08;
	outb(tmp, data->wbase + WBCIR_REG_WCEIR_CTL);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_STS, 0x17, 0x17);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_EV_EN, 0x00, 0x07);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_CFG1, 0x4A, 0x7F);

	
	if (invert)
		outb(0x04, data->ebase + WBCIR_REG_ECEIR_CCTL);
	else
		outb(0x00, data->ebase + WBCIR_REG_ECEIR_CCTL);

	
	outb(0x10, data->ebase + WBCIR_REG_ECEIR_CTS);
}

static int __devinit
wbcir_probe(struct pnp_dev *device, const struct pnp_device_id *dev_id)
{
	struct device *dev = &device->dev;
	struct wbcir_data *data;
	int err;

	if (!(pnp_port_len(device, 0) == EHFUNC_IOMEM_LEN &&
	      pnp_port_len(device, 1) == WAKEUP_IOMEM_LEN &&
	      pnp_port_len(device, 2) == SP_IOMEM_LEN)) {
		dev_err(dev, "Invalid resources\n");
		return -ENODEV;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}

	pnp_set_drvdata(device, data);

	data->ebase = pnp_port_start(device, 0);
	data->wbase = pnp_port_start(device, 1);
	data->sbase = pnp_port_start(device, 2);
	data->irq = pnp_irq(device, 0);

	if (data->wbase == 0 || data->ebase == 0 ||
	    data->sbase == 0 || data->irq == 0) {
		err = -ENODEV;
		dev_err(dev, "Invalid resources\n");
		goto exit_free_data;
	}

	dev_dbg(&device->dev, "Found device "
		"(w: 0x%lX, e: 0x%lX, s: 0x%lX, i: %u)\n",
		data->wbase, data->ebase, data->sbase, data->irq);

	if (!request_region(data->wbase, WAKEUP_IOMEM_LEN, DRVNAME)) {
		dev_err(dev, "Region 0x%lx-0x%lx already in use!\n",
			data->wbase, data->wbase + WAKEUP_IOMEM_LEN - 1);
		err = -EBUSY;
		goto exit_free_data;
	}

	if (!request_region(data->ebase, EHFUNC_IOMEM_LEN, DRVNAME)) {
		dev_err(dev, "Region 0x%lx-0x%lx already in use!\n",
			data->ebase, data->ebase + EHFUNC_IOMEM_LEN - 1);
		err = -EBUSY;
		goto exit_release_wbase;
	}

	if (!request_region(data->sbase, SP_IOMEM_LEN, DRVNAME)) {
		dev_err(dev, "Region 0x%lx-0x%lx already in use!\n",
			data->sbase, data->sbase + SP_IOMEM_LEN - 1);
		err = -EBUSY;
		goto exit_release_ebase;
	}

	err = request_irq(data->irq, wbcir_irq_handler,
			  IRQF_DISABLED, DRVNAME, device);
	if (err) {
		dev_err(dev, "Failed to claim IRQ %u\n", data->irq);
		err = -EBUSY;
		goto exit_release_sbase;
	}

	led_trigger_register_simple("cir-tx", &data->txtrigger);
	if (!data->txtrigger) {
		err = -ENOMEM;
		goto exit_free_irq;
	}

	led_trigger_register_simple("cir-rx", &data->rxtrigger);
	if (!data->rxtrigger) {
		err = -ENOMEM;
		goto exit_unregister_txtrigger;
	}

	data->led.name = "cir::activity";
	data->led.default_trigger = "cir-rx";
	data->led.brightness_set = wbcir_led_brightness_set;
	data->led.brightness_get = wbcir_led_brightness_get;
	err = led_classdev_register(&device->dev, &data->led);
	if (err)
		goto exit_unregister_rxtrigger;

	data->input_dev = input_allocate_device();
	if (!data->input_dev) {
		err = -ENOMEM;
		goto exit_unregister_led;
	}

	data->input_dev->evbit[0] = BIT(EV_KEY);
	data->input_dev->name = WBCIR_NAME;
	data->input_dev->phys = "wbcir/cir0";
	data->input_dev->id.bustype = BUS_HOST;
	data->input_dev->id.vendor  = PCI_VENDOR_ID_WINBOND;
	data->input_dev->id.product = WBCIR_ID_FAMILY;
	data->input_dev->id.version = WBCIR_ID_CHIP;
	data->input_dev->getkeycode = wbcir_getkeycode;
	data->input_dev->setkeycode = wbcir_setkeycode;
	input_set_capability(data->input_dev, EV_MSC, MSC_SCAN);
	input_set_drvdata(data->input_dev, data);

	err = input_register_device(data->input_dev);
	if (err)
		goto exit_free_input;

	data->last_scancode = INVALID_SCANCODE;
	INIT_LIST_HEAD(&data->keytable);
	setup_timer(&data->timer_keyup, wbcir_keyup, (unsigned long)data);

	
	if (protocol == IR_PROTOCOL_RC6) {
		int i;
		for (i = 0; i < ARRAY_SIZE(rc6_def_keymap); i++) {
			err = wbcir_setkeycode(data->input_dev,
					       (int)rc6_def_keymap[i].scancode,
					       (int)rc6_def_keymap[i].keycode);
			if (err)
				goto exit_unregister_keys;
		}
	}

	device_init_wakeup(&device->dev, 1);

	wbcir_cfg_ceir(data);

	
	wbcir_select_bank(data, WBCIR_BANK_0);
	outb(WBCIR_IRQ_NONE, data->sbase + WBCIR_REG_SP3_IER);

	
	wbcir_select_bank(data, WBCIR_BANK_2);
	outb(WBCIR_EXT_ENABLE, data->sbase + WBCIR_REG_SP3_EXCR1);

	

	
	outb(0x30, data->sbase + WBCIR_REG_SP3_EXCR2);

	
	switch (protocol) {
	case IR_PROTOCOL_RC5:
		outb(0xA7, data->sbase + WBCIR_REG_SP3_BGDL);
		break;
	case IR_PROTOCOL_RC6:
		outb(0x53, data->sbase + WBCIR_REG_SP3_BGDL);
		break;
	case IR_PROTOCOL_NEC:
		outb(0x69, data->sbase + WBCIR_REG_SP3_BGDL);
		break;
	}
	outb(0x00, data->sbase + WBCIR_REG_SP3_BGDH);

	
	wbcir_select_bank(data, WBCIR_BANK_0);
	outb(0xC0, data->sbase + WBCIR_REG_SP3_MCR);
	inb(data->sbase + WBCIR_REG_SP3_LSR); 
	inb(data->sbase + WBCIR_REG_SP3_MSR); 

	
	wbcir_select_bank(data, WBCIR_BANK_7);
	outb(0x10, data->sbase + WBCIR_REG_SP3_RCCFG);

	
	wbcir_select_bank(data, WBCIR_BANK_4);
	outb(0x00, data->sbase + WBCIR_REG_SP3_IRCR1);

	
	wbcir_select_bank(data, WBCIR_BANK_5);
	outb(0x00, data->sbase + WBCIR_REG_SP3_IRCR2);

	
	wbcir_select_bank(data, WBCIR_BANK_6);
	outb(0x20, data->sbase + WBCIR_REG_SP3_IRCR3);

	
	wbcir_select_bank(data, WBCIR_BANK_7);
	outb(0xF2, data->sbase + WBCIR_REG_SP3_IRRXDC);
	outb(0x69, data->sbase + WBCIR_REG_SP3_IRTXMC);

	
	if (invert)
		outb(0x10, data->sbase + WBCIR_REG_SP3_IRCFG4);
	else
		outb(0x00, data->sbase + WBCIR_REG_SP3_IRCFG4);

	
	wbcir_select_bank(data, WBCIR_BANK_0);
	outb(0x97, data->sbase + WBCIR_REG_SP3_FCR);

	
	outb(0xE0, data->sbase + WBCIR_REG_SP3_ASCR);

	
	outb(WBCIR_IRQ_RX | WBCIR_IRQ_ERR, data->sbase + WBCIR_REG_SP3_IER);

	return 0;

exit_unregister_keys:
	if (!list_empty(&data->keytable)) {
		struct wbcir_keyentry *key;
		struct wbcir_keyentry *keytmp;

		list_for_each_entry_safe(key, keytmp, &data->keytable, list) {
			list_del(&key->list);
			kfree(key);
		}
	}
	input_unregister_device(data->input_dev);
	
	data->input_dev = NULL;
exit_free_input:
	input_free_device(data->input_dev);
exit_unregister_led:
	led_classdev_unregister(&data->led);
exit_unregister_rxtrigger:
	led_trigger_unregister_simple(data->rxtrigger);
exit_unregister_txtrigger:
	led_trigger_unregister_simple(data->txtrigger);
exit_free_irq:
	free_irq(data->irq, device);
exit_release_sbase:
	release_region(data->sbase, SP_IOMEM_LEN);
exit_release_ebase:
	release_region(data->ebase, EHFUNC_IOMEM_LEN);
exit_release_wbase:
	release_region(data->wbase, WAKEUP_IOMEM_LEN);
exit_free_data:
	kfree(data);
	pnp_set_drvdata(device, NULL);
exit:
	return err;
}

static void __devexit
wbcir_remove(struct pnp_dev *device)
{
	struct wbcir_data *data = pnp_get_drvdata(device);
	struct wbcir_keyentry *key;
	struct wbcir_keyentry *keytmp;

	
	wbcir_select_bank(data, WBCIR_BANK_0);
	outb(WBCIR_IRQ_NONE, data->sbase + WBCIR_REG_SP3_IER);

	del_timer_sync(&data->timer_keyup);

	free_irq(data->irq, device);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_STS, 0x17, 0x17);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_CTL, 0x00, 0x01);

	
	wbcir_set_bits(data->wbase + WBCIR_REG_WCEIR_EV_EN, 0x00, 0x07);

	
	input_unregister_device(data->input_dev);

	led_trigger_unregister_simple(data->rxtrigger);
	led_trigger_unregister_simple(data->txtrigger);
	led_classdev_unregister(&data->led);

	
	wbcir_led_brightness_set(&data->led, LED_OFF);

	release_region(data->wbase, WAKEUP_IOMEM_LEN);
	release_region(data->ebase, EHFUNC_IOMEM_LEN);
	release_region(data->sbase, SP_IOMEM_LEN);

	list_for_each_entry_safe(key, keytmp, &data->keytable, list) {
		list_del(&key->list);
		kfree(key);
	}

	kfree(data);

	pnp_set_drvdata(device, NULL);
}

static const struct pnp_device_id wbcir_ids[] = {
	{ "WEC1022", 0 },
	{ "", 0 }
};
MODULE_DEVICE_TABLE(pnp, wbcir_ids);

static struct pnp_driver wbcir_driver = {
	.name     = WBCIR_NAME,
	.id_table = wbcir_ids,
	.probe    = wbcir_probe,
	.remove   = __devexit_p(wbcir_remove),
	.suspend  = wbcir_suspend,
	.resume   = wbcir_resume,
	.shutdown = wbcir_shutdown
};

static int __init
wbcir_init(void)
{
	int ret;

	switch (protocol) {
	case IR_PROTOCOL_RC5:
	case IR_PROTOCOL_NEC:
	case IR_PROTOCOL_RC6:
		break;
	default:
		printk(KERN_ERR DRVNAME ": Invalid protocol argument\n");
		return -EINVAL;
	}

	ret = pnp_register_driver(&wbcir_driver);
	if (ret)
		printk(KERN_ERR DRVNAME ": Unable to register driver\n");

	return ret;
}

static void __exit
wbcir_exit(void)
{
	pnp_unregister_driver(&wbcir_driver);
}

MODULE_AUTHOR("David H�rdeman <david@hardeman.nu>");
MODULE_DESCRIPTION("Winbond SuperI/O Consumer IR Driver");
MODULE_LICENSE("GPL");

module_init(wbcir_init);
module_exit(wbcir_exit);


