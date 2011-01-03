

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include "bttv.h"
#include "bttvp.h"


static int ir_debug;
module_param(ir_debug, int, 0644);
static int repeat_delay = 500;
module_param(repeat_delay, int, 0644);
static int repeat_period = 33;
module_param(repeat_period, int, 0644);

static int ir_rc5_remote_gap = 885;
module_param(ir_rc5_remote_gap, int, 0644);
static int ir_rc5_key_timeout = 200;
module_param(ir_rc5_key_timeout, int, 0644);

#undef dprintk
#define dprintk(arg...) do {	\
	if (ir_debug >= 1)	\
		printk(arg);	\
} while (0)

#define DEVNAME "bttv-input"



static void ir_handle_key(struct bttv *btv)
{
	struct card_ir *ir = btv->remote;
	u32 gpio,data;

	
	gpio = bttv_gpio_read(&btv->c);
	if (ir->polling) {
		if (ir->last_gpio == gpio)
			return;
		ir->last_gpio = gpio;
	}

	
	data = ir_extract_bits(gpio, ir->mask_keycode);
	dprintk(KERN_INFO DEVNAME ": irq gpio=0x%x code=%d | %s%s%s\n",
		gpio, data,
		ir->polling               ? "poll"  : "irq",
		(gpio & ir->mask_keydown) ? " down" : "",
		(gpio & ir->mask_keyup)   ? " up"   : "");

	if ((ir->mask_keydown  &&  (0 != (gpio & ir->mask_keydown))) ||
	    (ir->mask_keyup    &&  (0 == (gpio & ir->mask_keyup)))) {
		ir_input_keydown(ir->dev,&ir->ir,data,data);
	} else {
		
		if (btv->c.type == BTTV_BOARD_WINFAST2000)
			ir_input_keydown(ir->dev, &ir->ir, data, data);

		ir_input_nokey(ir->dev,&ir->ir);
	}

}

static void ir_enltv_handle_key(struct bttv *btv)
{
	struct card_ir *ir = btv->remote;
	u32 gpio, data, keyup;

	
	gpio = bttv_gpio_read(&btv->c);

	
	data = ir_extract_bits(gpio, ir->mask_keycode);

	
	keyup = (gpio & ir->mask_keyup) ? 1 << 31 : 0;

	if ((ir->last_gpio & 0x7f) != data) {
		dprintk(KERN_INFO DEVNAME ": gpio=0x%x code=%d | %s\n",
			gpio, data,
			(gpio & ir->mask_keyup) ? " up" : "up/down");

		ir_input_keydown(ir->dev, &ir->ir, data, data);
		if (keyup)
			ir_input_nokey(ir->dev, &ir->ir);
	} else {
		if ((ir->last_gpio & 1 << 31) == keyup)
			return;

		dprintk(KERN_INFO DEVNAME ":(cnt) gpio=0x%x code=%d | %s\n",
			gpio, data,
			(gpio & ir->mask_keyup) ? " up" : "down");

		if (keyup)
			ir_input_nokey(ir->dev, &ir->ir);
		else
			ir_input_keydown(ir->dev, &ir->ir, data, data);
	}

	ir->last_gpio = data | keyup;
}

void bttv_input_irq(struct bttv *btv)
{
	struct card_ir *ir = btv->remote;

	if (!ir->polling)
		ir_handle_key(btv);
}

static void bttv_input_timer(unsigned long data)
{
	struct bttv *btv = (struct bttv*)data;
	struct card_ir *ir = btv->remote;

	if (btv->c.type == BTTV_BOARD_ENLTV_FM_2)
		ir_enltv_handle_key(btv);
	else
		ir_handle_key(btv);
	mod_timer(&ir->timer, jiffies + msecs_to_jiffies(ir->polling));
}



static int bttv_rc5_irq(struct bttv *btv)
{
	struct card_ir *ir = btv->remote;
	struct timeval tv;
	u32 gpio;
	u32 gap;
	unsigned long current_jiffies;

	
	gpio = bttv_gpio_read(&btv->c);

	
	if (!(gpio & 0x20))
		return 0;

	
	current_jiffies = jiffies;
	do_gettimeofday(&tv);

	
	if (tv.tv_sec - ir->base_time.tv_sec > 1) {
		gap = 200000;
	} else {
		gap = 1000000 * (tv.tv_sec - ir->base_time.tv_sec) +
		    tv.tv_usec - ir->base_time.tv_usec;
	}

	
	if (ir->active) {
		
		if (ir->last_bit < 28) {
			ir->last_bit = (gap - ir_rc5_remote_gap / 2) /
			    ir_rc5_remote_gap;
			ir->code |= 1 << ir->last_bit;
		}
		
	} else {
		ir->active = 1;
		ir->code = 0;
		ir->base_time = tv;
		ir->last_bit = 0;

		mod_timer(&ir->timer_end,
			  current_jiffies + msecs_to_jiffies(30));
	}

	
	bttv_gpio_write(&btv->c, gpio & ~(1 << 4));
	bttv_gpio_write(&btv->c, gpio | (1 << 4));
	return 1;
}



static void bttv_ir_start(struct bttv *btv, struct card_ir *ir)
{
	if (ir->polling) {
		setup_timer(&ir->timer, bttv_input_timer, (unsigned long)btv);
		ir->timer.expires  = jiffies + msecs_to_jiffies(1000);
		add_timer(&ir->timer);
	} else if (ir->rc5_gpio) {
		
		init_timer(&ir->timer_end);
		ir->timer_end.function = ir_rc5_timer_end;
		ir->timer_end.data = (unsigned long)ir;

		init_timer(&ir->timer_keyup);
		ir->timer_keyup.function = ir_rc5_timer_keyup;
		ir->timer_keyup.data = (unsigned long)ir;
		ir->shift_by = 1;
		ir->start = 3;
		ir->addr = 0x0;
		ir->rc5_key_timeout = ir_rc5_key_timeout;
		ir->rc5_remote_gap = ir_rc5_remote_gap;
	}
}

static void bttv_ir_stop(struct bttv *btv)
{
	if (btv->remote->polling) {
		del_timer_sync(&btv->remote->timer);
		flush_scheduled_work();
	}

	if (btv->remote->rc5_gpio) {
		u32 gpio;

		del_timer_sync(&btv->remote->timer_end);
		flush_scheduled_work();

		gpio = bttv_gpio_read(&btv->c);
		bttv_gpio_write(&btv->c, gpio & ~(1 << 4));
	}
}

int bttv_input_init(struct bttv *btv)
{
	struct card_ir *ir;
	struct ir_scancode_table *ir_codes = NULL;
	struct input_dev *input_dev;
	int ir_type = IR_TYPE_OTHER;
	int err = -ENOMEM;

	if (!btv->has_remote)
		return -ENODEV;

	ir = kzalloc(sizeof(*ir),GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ir || !input_dev)
		goto err_out_free;

	
	switch (btv->c.type) {
	case BTTV_BOARD_AVERMEDIA:
	case BTTV_BOARD_AVPHONE98:
	case BTTV_BOARD_AVERMEDIA98:
		ir_codes         = &ir_codes_avermedia_table;
		ir->mask_keycode = 0xf88000;
		ir->mask_keydown = 0x010000;
		ir->polling      = 50; 
		break;

	case BTTV_BOARD_AVDVBT_761:
	case BTTV_BOARD_AVDVBT_771:
		ir_codes         = &ir_codes_avermedia_dvbt_table;
		ir->mask_keycode = 0x0f00c0;
		ir->mask_keydown = 0x000020;
		ir->polling      = 50; 
		break;

	case BTTV_BOARD_PXELVWPLTVPAK:
		ir_codes         = &ir_codes_pixelview_table;
		ir->mask_keycode = 0x003e00;
		ir->mask_keyup   = 0x010000;
		ir->polling      = 50; 
		break;
	case BTTV_BOARD_PV_M4900:
	case BTTV_BOARD_PV_BT878P_9B:
	case BTTV_BOARD_PV_BT878P_PLUS:
		ir_codes         = &ir_codes_pixelview_table;
		ir->mask_keycode = 0x001f00;
		ir->mask_keyup   = 0x008000;
		ir->polling      = 50; 
		break;

	case BTTV_BOARD_WINFAST2000:
		ir_codes         = &ir_codes_winfast_table;
		ir->mask_keycode = 0x1f8;
		break;
	case BTTV_BOARD_MAGICTVIEW061:
	case BTTV_BOARD_MAGICTVIEW063:
		ir_codes         = &ir_codes_winfast_table;
		ir->mask_keycode = 0x0008e000;
		ir->mask_keydown = 0x00200000;
		break;
	case BTTV_BOARD_APAC_VIEWCOMP:
		ir_codes         = &ir_codes_apac_viewcomp_table;
		ir->mask_keycode = 0x001f00;
		ir->mask_keyup   = 0x008000;
		ir->polling      = 50; 
		break;
	case BTTV_BOARD_ASKEY_CPH03X:
	case BTTV_BOARD_CONCEPTRONIC_CTVFMI2:
	case BTTV_BOARD_CONTVFMI:
		ir_codes         = &ir_codes_pixelview_table;
		ir->mask_keycode = 0x001F00;
		ir->mask_keyup   = 0x006000;
		ir->polling      = 50; 
		break;
	case BTTV_BOARD_NEBULA_DIGITV:
		ir_codes = &ir_codes_nebula_table;
		btv->custom_irq = bttv_rc5_irq;
		ir->rc5_gpio = 1;
		break;
	case BTTV_BOARD_MACHTV_MAGICTV:
		ir_codes         = &ir_codes_apac_viewcomp_table;
		ir->mask_keycode = 0x001F00;
		ir->mask_keyup   = 0x004000;
		ir->polling      = 50; 
		break;
	case BTTV_BOARD_KOZUMI_KTV_01C:
		ir_codes         = &ir_codes_pctv_sedna_table;
		ir->mask_keycode = 0x001f00;
		ir->mask_keyup   = 0x006000;
		ir->polling      = 50; 
		break;
	case BTTV_BOARD_ENLTV_FM_2:
		ir_codes         = &ir_codes_encore_enltv2_table;
		ir->mask_keycode = 0x00fd00;
		ir->mask_keyup   = 0x000080;
		ir->polling      = 1; 
		ir->last_gpio    = ir_extract_bits(bttv_gpio_read(&btv->c),
						   ir->mask_keycode);
		break;
	}
	if (NULL == ir_codes) {
		dprintk(KERN_INFO "Ooops: IR config error [card=%d]\n", btv->c.type);
		err = -ENODEV;
		goto err_out_free;
	}

	if (ir->rc5_gpio) {
		u32 gpio;
		
		bttv_gpio_inout(&btv->c, (1 << 4), 1 << 4);
		gpio = bttv_gpio_read(&btv->c);
		bttv_gpio_write(&btv->c, gpio & ~(1 << 4));
		bttv_gpio_write(&btv->c, gpio | (1 << 4));
	} else {
		
		bttv_gpio_inout(&btv->c, ir->mask_keycode | ir->mask_keydown, 0);
	}

	
	ir->dev = input_dev;

	snprintf(ir->name, sizeof(ir->name), "bttv IR (card=%d)",
		 btv->c.type);
	snprintf(ir->phys, sizeof(ir->phys), "pci-%s/ir0",
		 pci_name(btv->c.pci));

	ir_input_init(input_dev, &ir->ir, ir_type, ir_codes);
	input_dev->name = ir->name;
	input_dev->phys = ir->phys;
	input_dev->id.bustype = BUS_PCI;
	input_dev->id.version = 1;
	if (btv->c.pci->subsystem_vendor) {
		input_dev->id.vendor  = btv->c.pci->subsystem_vendor;
		input_dev->id.product = btv->c.pci->subsystem_device;
	} else {
		input_dev->id.vendor  = btv->c.pci->vendor;
		input_dev->id.product = btv->c.pci->device;
	}
	input_dev->dev.parent = &btv->c.pci->dev;

	btv->remote = ir;
	bttv_ir_start(btv, ir);

	
	err = input_register_device(btv->remote->dev);
	if (err)
		goto err_out_stop;

	
	ir->dev->rep[REP_DELAY] = repeat_delay;
	ir->dev->rep[REP_PERIOD] = repeat_period;

	return 0;

 err_out_stop:
	bttv_ir_stop(btv);
	btv->remote = NULL;
 err_out_free:
	input_free_device(input_dev);
	kfree(ir);
	return err;
}

void bttv_input_fini(struct bttv *btv)
{
	if (btv->remote == NULL)
		return;

	bttv_ir_stop(btv);
	input_unregister_device(btv->remote->dev);
	kfree(btv->remote);
	btv->remote = NULL;
}



