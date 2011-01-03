

#include <linux/module.h>
#include <linux/string.h>
#include <linux/jiffies.h>
#include <media/ir-common.h>



MODULE_AUTHOR("Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]");
MODULE_LICENSE("GPL");

static int repeat = 1;
module_param(repeat, int, 0444);
MODULE_PARM_DESC(repeat,"auto-repeat for IR keys (default: on)");

static int debug;    
module_param(debug, int, 0644);

#define dprintk(level, fmt, arg...)	if (debug >= level) \
	printk(KERN_DEBUG fmt , ## arg)



static void ir_input_key_event(struct input_dev *dev, struct ir_input_state *ir)
{
	if (KEY_RESERVED == ir->keycode) {
		printk(KERN_INFO "%s: unknown key: key=0x%02x raw=0x%02x down=%d\n",
		       dev->name,ir->ir_key,ir->ir_raw,ir->keypressed);
		return;
	}
	dprintk(1,"%s: key event code=%d down=%d\n",
		dev->name,ir->keycode,ir->keypressed);
	input_report_key(dev,ir->keycode,ir->keypressed);
	input_sync(dev);
}



void ir_input_init(struct input_dev *dev, struct ir_input_state *ir,
		   int ir_type, struct ir_scancode_table *ir_codes)
{
	int i;

	ir->ir_type = ir_type;

	memset(ir->ir_codes, 0, sizeof(ir->ir_codes));

	

	if (ir_codes)
		for (i = 0; i < ir_codes->size; i++)
			if (ir_codes->scan[i].scancode < IR_KEYTAB_SIZE)
				ir->ir_codes[ir_codes->scan[i].scancode] = ir_codes->scan[i].keycode;

	dev->keycode     = ir->ir_codes;
	dev->keycodesize = sizeof(IR_KEYTAB_TYPE);
	dev->keycodemax  = IR_KEYTAB_SIZE;
	for (i = 0; i < IR_KEYTAB_SIZE; i++)
		set_bit(ir->ir_codes[i], dev->keybit);
	clear_bit(0, dev->keybit);

	set_bit(EV_KEY, dev->evbit);
	if (repeat)
		set_bit(EV_REP, dev->evbit);
}
EXPORT_SYMBOL_GPL(ir_input_init);

void ir_input_nokey(struct input_dev *dev, struct ir_input_state *ir)
{
	if (ir->keypressed) {
		ir->keypressed = 0;
		ir_input_key_event(dev,ir);
	}
}
EXPORT_SYMBOL_GPL(ir_input_nokey);

void ir_input_keydown(struct input_dev *dev, struct ir_input_state *ir,
		      u32 ir_key, u32 ir_raw)
{
	u32 keycode = IR_KEYCODE(ir->ir_codes, ir_key);

	if (ir->keypressed && ir->keycode != keycode) {
		ir->keypressed = 0;
		ir_input_key_event(dev,ir);
	}
	if (!ir->keypressed) {
		ir->ir_key  = ir_key;
		ir->ir_raw  = ir_raw;
		ir->keycode = keycode;
		ir->keypressed = 1;
		ir_input_key_event(dev,ir);
	}
}
EXPORT_SYMBOL_GPL(ir_input_keydown);



u32 ir_extract_bits(u32 data, u32 mask)
{
	u32 vbit = 1, value = 0;

	do {
	    if (mask&1) {
		if (data&1)
			value |= vbit;
		vbit<<=1;
	    }
	    data>>=1;
	} while (mask>>=1);

	return value;
}
EXPORT_SYMBOL_GPL(ir_extract_bits);

static int inline getbit(u32 *samples, int bit)
{
	return (samples[bit/32] & (1 << (31-(bit%32)))) ? 1 : 0;
}


int ir_dump_samples(u32 *samples, int count)
{
	int i, bit, start;

	printk(KERN_DEBUG "ir samples: ");
	start = 0;
	for (i = 0; i < count * 32; i++) {
		bit = getbit(samples,i);
		if (bit)
			start = 1;
		if (0 == start)
			continue;
		printk("%s", bit ? "#" : "_");
	}
	printk("\n");
	return 0;
}
EXPORT_SYMBOL_GPL(ir_dump_samples);


int ir_decode_pulsedistance(u32 *samples, int count, int low, int high)
{
	int i,last,bit,len;
	u32 curBit;
	u32 value;

	
	for (i = len = 0; i < count * 32; i++) {
		bit = getbit(samples,i);
		if (bit) {
			len++;
		} else {
			if (len >= 29)
				break;
			len = 0;
		}
	}

	
	if (len < 29)
		return 0xffffffff;

	
	for (len = 0; i < count * 32; i++) {
		bit = getbit(samples,i);
		if (bit) {
			break;
		} else {
			len++;
		}
	}

	
	if (len < 7)
		return 0xffffffff;

	
	len   = 0;
	last = 1;
	value = 0; curBit = 1;
	for (; i < count * 32; i++) {
		bit  = getbit(samples,i);
		if (last) {
			if(bit) {
				continue;
			} else {
				len = 1;
			}
		} else {
			if (bit) {
				if (len > (low + high) /2)
					value |= curBit;
				curBit <<= 1;
				if (curBit == 1)
					break;
			} else {
				len++;
			}
		}
		last = bit;
	}

	return value;
}
EXPORT_SYMBOL_GPL(ir_decode_pulsedistance);


int ir_decode_biphase(u32 *samples, int count, int low, int high)
{
	int i,last,bit,len,flips;
	u32 value;

	
	for (i = 0; i < 32; i++) {
		bit = getbit(samples,i);
		if (bit)
			break;
	}

	
	len   = 0;
	flips = 0;
	value = 1;
	for (; i < count * 32; i++) {
		if (len > high)
			break;
		if (flips > 1)
			break;
		last = bit;
		bit  = getbit(samples,i);
		if (last == bit) {
			len++;
			continue;
		}
		if (len < low) {
			len++;
			flips++;
			continue;
		}
		value <<= 1;
		value |= bit;
		flips = 0;
		len   = 1;
	}
	return value;
}
EXPORT_SYMBOL_GPL(ir_decode_biphase);




static u32 ir_rc5_decode(unsigned int code)
{
	unsigned int org_code = code;
	unsigned int pair;
	unsigned int rc5 = 0;
	int i;

	for (i = 0; i < 14; ++i) {
		pair = code & 0x3;
		code >>= 2;

		rc5 <<= 1;
		switch (pair) {
		case 0:
		case 2:
			break;
		case 1:
			rc5 |= 1;
			break;
		case 3:
			dprintk(1, "ir-common: ir_rc5_decode(%x) bad code\n", org_code);
			return 0;
		}
	}
	dprintk(1, "ir-common: code=%x, rc5=%x, start=%x, toggle=%x, address=%x, "
		"instr=%x\n", rc5, org_code, RC5_START(rc5),
		RC5_TOGGLE(rc5), RC5_ADDR(rc5), RC5_INSTR(rc5));
	return rc5;
}

void ir_rc5_timer_end(unsigned long data)
{
	struct card_ir *ir = (struct card_ir *)data;
	struct timeval tv;
	unsigned long current_jiffies, timeout;
	u32 gap;
	u32 rc5 = 0;

	
	current_jiffies = jiffies;
	do_gettimeofday(&tv);

	
	if (tv.tv_sec - ir->base_time.tv_sec > 1) {
		gap = 200000;
	} else {
		gap = 1000000 * (tv.tv_sec - ir->base_time.tv_sec) +
		    tv.tv_usec - ir->base_time.tv_usec;
	}

	
	ir->active = 0;

	
	if (gap < 28000) {
		dprintk(1, "ir-common: spurious timer_end\n");
		return;
	}

	if (ir->last_bit < 20) {
		
		dprintk(1, "ir-common: short code: %x\n", ir->code);
	} else {
		ir->code = (ir->code << ir->shift_by) | 1;
		rc5 = ir_rc5_decode(ir->code);

		
		if (RC5_START(rc5) != ir->start) {
			dprintk(1, "ir-common: rc5 start bits invalid: %u\n", RC5_START(rc5));

			
		} else if (RC5_ADDR(rc5) == ir->addr) {
			u32 toggle = RC5_TOGGLE(rc5);
			u32 instr = RC5_INSTR(rc5);

			
			if (toggle != RC5_TOGGLE(ir->last_rc5) ||
			    instr != RC5_INSTR(ir->last_rc5)) {
				dprintk(1, "ir-common: instruction %x, toggle %x\n", instr,
					toggle);
				ir_input_nokey(ir->dev, &ir->ir);
				ir_input_keydown(ir->dev, &ir->ir, instr,
						 instr);
			}

			
			timeout = current_jiffies +
				  msecs_to_jiffies(ir->rc5_key_timeout);
			mod_timer(&ir->timer_keyup, timeout);

			
			ir->last_rc5 = rc5;
		}
	}
}
EXPORT_SYMBOL_GPL(ir_rc5_timer_end);

void ir_rc5_timer_keyup(unsigned long data)
{
	struct card_ir *ir = (struct card_ir *)data;

	dprintk(1, "ir-common: key released\n");
	ir_input_nokey(ir->dev, &ir->ir);
}
EXPORT_SYMBOL_GPL(ir_rc5_timer_keyup);
