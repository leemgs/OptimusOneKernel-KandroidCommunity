

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include "saa7134-reg.h"
#include "saa7134.h"

static unsigned int disable_ir;
module_param(disable_ir, int, 0444);
MODULE_PARM_DESC(disable_ir,"disable infrared remote support");

static unsigned int ir_debug;
module_param(ir_debug, int, 0644);
MODULE_PARM_DESC(ir_debug,"enable debug messages [IR]");

static int pinnacle_remote;
module_param(pinnacle_remote, int, 0644);    
MODULE_PARM_DESC(pinnacle_remote, "Specify Pinnacle PCTV remote: 0=coloured, 1=grey (defaults to 0)");

static int ir_rc5_remote_gap = 885;
module_param(ir_rc5_remote_gap, int, 0644);
static int ir_rc5_key_timeout = 115;
module_param(ir_rc5_key_timeout, int, 0644);

static int repeat_delay = 500;
module_param(repeat_delay, int, 0644);
MODULE_PARM_DESC(repeat_delay, "delay before key repeat started");
static int repeat_period = 33;
module_param(repeat_period, int, 0644);
MODULE_PARM_DESC(repeat_period, "repeat period between "
    "keypresses when key is down");

static unsigned int disable_other_ir;
module_param(disable_other_ir, int, 0644);
MODULE_PARM_DESC(disable_other_ir, "disable full codes of "
    "alternative remotes from other manufacturers");

#define dprintk(fmt, arg...)	if (ir_debug) \
	printk(KERN_DEBUG "%s/ir: " fmt, dev->name , ## arg)
#define i2cdprintk(fmt, arg...)    if (ir_debug) \
	printk(KERN_DEBUG "%s/ir: " fmt, ir->name , ## arg)


static int saa7134_rc5_irq(struct saa7134_dev *dev);
static int saa7134_nec_irq(struct saa7134_dev *dev);
static void nec_task(unsigned long data);
static void saa7134_nec_timer(unsigned long data);



static int build_key(struct saa7134_dev *dev)
{
	struct card_ir *ir = dev->remote;
	u32 gpio, data;

	
	switch (dev->board) {
	case SAA7134_BOARD_GOTVIEW_7135:
		saa_setb(SAA7134_GPIO_GPSTATUS1, 0x80);
		saa_clearb(SAA7134_GPIO_GPSTATUS1, 0x80);
		break;
	}
	
	saa_clearb(SAA7134_GPIO_GPMODE3,SAA7134_GPIO_GPRESCAN);
	saa_setb(SAA7134_GPIO_GPMODE3,SAA7134_GPIO_GPRESCAN);

	gpio = saa_readl(SAA7134_GPIO_GPSTATUS0 >> 2);
	if (ir->polling) {
		if (ir->last_gpio == gpio)
			return 0;
		ir->last_gpio = gpio;
	}

	data = ir_extract_bits(gpio, ir->mask_keycode);
	dprintk("build_key gpio=0x%x mask=0x%x data=%d\n",
		gpio, ir->mask_keycode, data);

	switch (dev->board) {
	case SAA7134_BOARD_KWORLD_PLUS_TV_ANALOG:
		if (data == ir->mask_keycode)
			ir_input_nokey(ir->dev, &ir->ir);
		else
			ir_input_keydown(ir->dev, &ir->ir, data, data);
		return 0;
	}

	if (ir->polling) {
		if ((ir->mask_keydown  &&  (0 != (gpio & ir->mask_keydown))) ||
		    (ir->mask_keyup    &&  (0 == (gpio & ir->mask_keyup)))) {
			ir_input_keydown(ir->dev, &ir->ir, data, data);
		} else {
			ir_input_nokey(ir->dev, &ir->ir);
		}
	}
	else {	
		if ((ir->mask_keydown  &&  (0 != (gpio & ir->mask_keydown))) ||
		    (ir->mask_keyup    &&  (0 == (gpio & ir->mask_keyup)))) {
			ir_input_keydown(ir->dev, &ir->ir, data, data);
			ir_input_nokey(ir->dev, &ir->ir);
		}
	}

	return 0;
}



static int get_key_msi_tvanywhere_plus(struct IR_i2c *ir, u32 *ir_key,
				       u32 *ir_raw)
{
	unsigned char b;
	int gpio;

	
	struct saa7134_dev *dev = ir->c->adapter->algo_data;
	if (dev == NULL) {
		dprintk("get_key_msi_tvanywhere_plus: "
			"gir->c->adapter->algo_data is NULL!\n");
		return -EIO;
	}

	

	saa_clearb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);
	saa_setb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);

	gpio = saa_readl(SAA7134_GPIO_GPSTATUS0 >> 2);

	

	if (gpio & 0x40)
		return 0;       

	

	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		i2cdprintk("read error\n");
		return -EIO;
	}

	

	if (b == 0xff)
		return 0;

	

	dprintk("get_key_msi_tvanywhere_plus: Key = 0x%02X\n", b);
	*ir_key = b;
	*ir_raw = b;
	return 1;
}

static int get_key_purpletv(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char b;

	
	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		i2cdprintk("read error\n");
		return -EIO;
	}

	
	if (b==0)
		return 0;

	
	if (b & 0x80)
		return 1;

	*ir_key = b;
	*ir_raw = b;
	return 1;
}

static int get_key_hvr1110(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char buf[5], cod4, code3, code4;

	
	if (5 != i2c_master_recv(ir->c, buf, 5))
		return -EIO;

	cod4	= buf[4];
	code4	= (cod4 >> 2);
	code3	= buf[3];
	if (code3 == 0)
		
		return 0;

	
	*ir_key = code4;
	*ir_raw = code4;
	return 1;
}


static int get_key_beholdm6xx(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char data[12];
	u32 gpio;

	struct saa7134_dev *dev = ir->c->adapter->algo_data;

	
	saa_clearb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);
	saa_setb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);

	gpio = saa_readl(SAA7134_GPIO_GPSTATUS0 >> 2);

	if (0x400000 & ~gpio)
		return 0; 

	ir->c->addr = 0x5a >> 1;

	if (12 != i2c_master_recv(ir->c, data, 12)) {
		i2cdprintk("read error\n");
		return -EIO;
	}
	
	if (data[10] != 0x6b && data[11] != 0x86 && disable_other_ir)
		return 0;

	
	if (data[9] != (unsigned char)(~data[8]))
		return 0;

	*ir_key = data[9];
	*ir_raw = data[9];

	return 1;
}


static int get_key_pinnacle(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw,
			    int parity_offset, int marker, int code_modulo)
{
	unsigned char b[4];
	unsigned int start = 0,parity = 0,code = 0;

	
	if (4 != i2c_master_recv(ir->c, b, 4)) {
		i2cdprintk("read error\n");
		return -EIO;
	}

	for (start = 0; start < ARRAY_SIZE(b); start++) {
		if (b[start] == marker) {
			code=b[(start+parity_offset + 1) % 4];
			parity=b[(start+parity_offset) % 4];
		}
	}

	
	if (parity == 0)
		return 0;

	
	if (ir->old == parity)
		return 0;

	ir->old = parity;

	
	if (marker == 0xfe && (code & 0x40))
		return 0;

	code %= code_modulo;

	*ir_raw = code;
	*ir_key = code;

	i2cdprintk("Pinnacle PCTV key %02x\n", code);

	return 1;
}


static int get_key_pinnacle_grey(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{

	return get_key_pinnacle(ir, ir_key, ir_raw, 1, 0xfe, 0xff);
}



static int get_key_pinnacle_color(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	

	return get_key_pinnacle(ir, ir_key, ir_raw, 2, 0x80, 0x88);
}

void saa7134_input_irq(struct saa7134_dev *dev)
{
	struct card_ir *ir = dev->remote;

	if (ir->nec_gpio) {
		saa7134_nec_irq(dev);
	} else if (!ir->polling && !ir->rc5_gpio) {
		build_key(dev);
	} else if (ir->rc5_gpio) {
		saa7134_rc5_irq(dev);
	}
}

static void saa7134_input_timer(unsigned long data)
{
	struct saa7134_dev *dev = (struct saa7134_dev *)data;
	struct card_ir *ir = dev->remote;

	build_key(dev);
	mod_timer(&ir->timer, jiffies + msecs_to_jiffies(ir->polling));
}

void saa7134_ir_start(struct saa7134_dev *dev, struct card_ir *ir)
{
	if (ir->polling) {
		setup_timer(&ir->timer, saa7134_input_timer,
			    (unsigned long)dev);
		ir->timer.expires  = jiffies + HZ;
		add_timer(&ir->timer);
	} else if (ir->rc5_gpio) {
		
		init_timer(&ir->timer_end);
		ir->timer_end.function = ir_rc5_timer_end;
		ir->timer_end.data = (unsigned long)ir;
		init_timer(&ir->timer_keyup);
		ir->timer_keyup.function = ir_rc5_timer_keyup;
		ir->timer_keyup.data = (unsigned long)ir;
		ir->shift_by = 2;
		ir->start = 0x2;
		ir->addr = 0x17;
		ir->rc5_key_timeout = ir_rc5_key_timeout;
		ir->rc5_remote_gap = ir_rc5_remote_gap;
	} else if (ir->nec_gpio) {
		setup_timer(&ir->timer_keyup, saa7134_nec_timer,
			    (unsigned long)dev);
		tasklet_init(&ir->tlet, nec_task, (unsigned long)dev);
	}
}

void saa7134_ir_stop(struct saa7134_dev *dev)
{
	if (dev->remote->polling)
		del_timer_sync(&dev->remote->timer);
}

int saa7134_input_init1(struct saa7134_dev *dev)
{
	struct card_ir *ir;
	struct input_dev *input_dev;
	struct ir_scancode_table *ir_codes = NULL;
	u32 mask_keycode = 0;
	u32 mask_keydown = 0;
	u32 mask_keyup   = 0;
	int polling      = 0;
	int rc5_gpio	 = 0;
	int nec_gpio	 = 0;
	int ir_type      = IR_TYPE_OTHER;
	int err;

	if (dev->has_remote != SAA7134_REMOTE_GPIO)
		return -ENODEV;
	if (disable_ir)
		return -ENODEV;

	
	switch (dev->board) {
	case SAA7134_BOARD_FLYVIDEO2000:
	case SAA7134_BOARD_FLYVIDEO3000:
	case SAA7134_BOARD_FLYTVPLATINUM_FM:
	case SAA7134_BOARD_FLYTVPLATINUM_MINI2:
	case SAA7134_BOARD_ROVERMEDIA_LINK_PRO_FM:
		ir_codes     = &ir_codes_flyvideo_table;
		mask_keycode = 0xEC00000;
		mask_keydown = 0x0040000;
		break;
	case SAA7134_BOARD_CINERGY400:
	case SAA7134_BOARD_CINERGY600:
	case SAA7134_BOARD_CINERGY600_MK3:
		ir_codes     = &ir_codes_cinergy_table;
		mask_keycode = 0x00003f;
		mask_keyup   = 0x040000;
		break;
	case SAA7134_BOARD_ECS_TVP3XP:
	case SAA7134_BOARD_ECS_TVP3XP_4CB5:
		ir_codes     = &ir_codes_eztv_table;
		mask_keycode = 0x00017c;
		mask_keyup   = 0x000002;
		polling      = 50; 
		break;
	case SAA7134_BOARD_KWORLD_XPERT:
	case SAA7134_BOARD_AVACSSMARTTV:
		ir_codes     = &ir_codes_pixelview_table;
		mask_keycode = 0x00001F;
		mask_keyup   = 0x000020;
		polling      = 50; 
		break;
	case SAA7134_BOARD_MD2819:
	case SAA7134_BOARD_KWORLD_VSTREAM_XPERT:
	case SAA7134_BOARD_AVERMEDIA_305:
	case SAA7134_BOARD_AVERMEDIA_307:
	case SAA7134_BOARD_AVERMEDIA_STUDIO_305:
	case SAA7134_BOARD_AVERMEDIA_STUDIO_505:
	case SAA7134_BOARD_AVERMEDIA_STUDIO_307:
	case SAA7134_BOARD_AVERMEDIA_STUDIO_507:
	case SAA7134_BOARD_AVERMEDIA_STUDIO_507UA:
	case SAA7134_BOARD_AVERMEDIA_GO_007_FM:
	case SAA7134_BOARD_AVERMEDIA_M102:
	case SAA7134_BOARD_AVERMEDIA_GO_007_FM_PLUS:
		ir_codes     = &ir_codes_avermedia_table;
		mask_keycode = 0x0007C8;
		mask_keydown = 0x000010;
		polling      = 50; 
		
		saa_setb(SAA7134_GPIO_GPMODE0, 0x4);
		saa_setb(SAA7134_GPIO_GPSTATUS0, 0x4);
		break;
	case SAA7134_BOARD_AVERMEDIA_M135A:
		ir_codes     = &ir_codes_avermedia_m135a_table;
		mask_keydown = 0x0040000;
		mask_keycode = 0x00013f;
		nec_gpio     = 1;
		break;
	case SAA7134_BOARD_AVERMEDIA_777:
	case SAA7134_BOARD_AVERMEDIA_A16AR:
		ir_codes     = &ir_codes_avermedia_table;
		mask_keycode = 0x02F200;
		mask_keydown = 0x000400;
		polling      = 50; 
		
		saa_setb(SAA7134_GPIO_GPMODE1, 0x1);
		saa_setb(SAA7134_GPIO_GPSTATUS1, 0x1);
		break;
	case SAA7134_BOARD_AVERMEDIA_A16D:
		ir_codes     = &ir_codes_avermedia_a16d_table;
		mask_keycode = 0x02F200;
		mask_keydown = 0x000400;
		polling      = 50; 
		
		saa_setb(SAA7134_GPIO_GPMODE1, 0x1);
		saa_setb(SAA7134_GPIO_GPSTATUS1, 0x1);
		break;
	case SAA7134_BOARD_KWORLD_TERMINATOR:
		ir_codes     = &ir_codes_pixelview_table;
		mask_keycode = 0x00001f;
		mask_keyup   = 0x000060;
		polling      = 50; 
		break;
	case SAA7134_BOARD_MANLI_MTV001:
	case SAA7134_BOARD_MANLI_MTV002:
		ir_codes     = &ir_codes_manli_table;
		mask_keycode = 0x001f00;
		mask_keyup   = 0x004000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_BEHOLD_409FM:
	case SAA7134_BOARD_BEHOLD_401:
	case SAA7134_BOARD_BEHOLD_403:
	case SAA7134_BOARD_BEHOLD_403FM:
	case SAA7134_BOARD_BEHOLD_405:
	case SAA7134_BOARD_BEHOLD_405FM:
	case SAA7134_BOARD_BEHOLD_407:
	case SAA7134_BOARD_BEHOLD_407FM:
	case SAA7134_BOARD_BEHOLD_409:
	case SAA7134_BOARD_BEHOLD_505FM:
	case SAA7134_BOARD_BEHOLD_505RDS:
	case SAA7134_BOARD_BEHOLD_507_9FM:
	case SAA7134_BOARD_BEHOLD_507RDS_MK3:
	case SAA7134_BOARD_BEHOLD_507RDS_MK5:
		ir_codes     = &ir_codes_manli_table;
		mask_keycode = 0x003f00;
		mask_keyup   = 0x004000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_BEHOLD_COLUMBUS_TVFM:
		ir_codes     = &ir_codes_behold_columbus_table;
		mask_keycode = 0x003f00;
		mask_keyup   = 0x004000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_SEDNA_PC_TV_CARDBUS:
		ir_codes     = &ir_codes_pctv_sedna_table;
		mask_keycode = 0x001f00;
		mask_keyup   = 0x004000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_GOTVIEW_7135:
		ir_codes     = &ir_codes_gotview7135_table;
		mask_keycode = 0x0003CC;
		mask_keydown = 0x000010;
		polling	     = 5; 
		saa_setb(SAA7134_GPIO_GPMODE1, 0x80);
		break;
	case SAA7134_BOARD_VIDEOMATE_TV_PVR:
	case SAA7134_BOARD_VIDEOMATE_GOLD_PLUS:
	case SAA7134_BOARD_VIDEOMATE_TV_GOLD_PLUSII:
		ir_codes     = &ir_codes_videomate_tv_pvr_table;
		mask_keycode = 0x00003F;
		mask_keyup   = 0x400000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_PROTEUS_2309:
		ir_codes     = &ir_codes_proteus_2309_table;
		mask_keycode = 0x00007F;
		mask_keyup   = 0x000080;
		polling      = 50; 
		break;
	case SAA7134_BOARD_VIDEOMATE_DVBT_300:
	case SAA7134_BOARD_VIDEOMATE_DVBT_200:
		ir_codes     = &ir_codes_videomate_tv_pvr_table;
		mask_keycode = 0x003F00;
		mask_keyup   = 0x040000;
		break;
	case SAA7134_BOARD_FLYDVBS_LR300:
	case SAA7134_BOARD_FLYDVBT_LR301:
	case SAA7134_BOARD_FLYDVBTDUO:
		ir_codes     = &ir_codes_flydvb_table;
		mask_keycode = 0x0001F00;
		mask_keydown = 0x0040000;
		break;
	case SAA7134_BOARD_ASUSTeK_P7131_DUAL:
	case SAA7134_BOARD_ASUSTeK_P7131_HYBRID_LNA:
	case SAA7134_BOARD_ASUSTeK_P7131_ANALOG:
		ir_codes     = &ir_codes_asus_pc39_table;
		mask_keydown = 0x0040000;
		rc5_gpio = 1;
		break;
	case SAA7134_BOARD_ENCORE_ENLTV:
	case SAA7134_BOARD_ENCORE_ENLTV_FM:
		ir_codes     = &ir_codes_encore_enltv_table;
		mask_keycode = 0x00007f;
		mask_keyup   = 0x040000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_ENCORE_ENLTV_FM53:
		ir_codes     = &ir_codes_encore_enltv_fm53_table;
		mask_keydown = 0x0040000;
		mask_keycode = 0x00007f;
		nec_gpio = 1;
		break;
	case SAA7134_BOARD_10MOONSTVMASTER3:
		ir_codes     = &ir_codes_encore_enltv_table;
		mask_keycode = 0x5f80000;
		mask_keyup   = 0x8000000;
		polling      = 50; 
		break;
	case SAA7134_BOARD_GENIUS_TVGO_A11MCE:
		ir_codes     = &ir_codes_genius_tvgo_a11mce_table;
		mask_keycode = 0xff;
		mask_keydown = 0xf00000;
		polling = 50; 
		break;
	case SAA7134_BOARD_REAL_ANGEL_220:
		ir_codes     = &ir_codes_real_audio_220_32_keys_table;
		mask_keycode = 0x3f00;
		mask_keyup   = 0x4000;
		polling = 50; 
		break;
	case SAA7134_BOARD_KWORLD_PLUS_TV_ANALOG:
		ir_codes     = &ir_codes_kworld_plus_tv_analog_table;
		mask_keycode = 0x7f;
		polling = 40; 
		break;
	case SAA7134_BOARD_VIDEOMATE_S350:
		ir_codes     = &ir_codes_videomate_s350_table;
		mask_keycode = 0x003f00;
		mask_keydown = 0x040000;
		break;
	}
	if (NULL == ir_codes) {
		printk("%s: Oops: IR config error [card=%d]\n",
		       dev->name, dev->board);
		return -ENODEV;
	}

	ir = kzalloc(sizeof(*ir), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ir || !input_dev) {
		err = -ENOMEM;
		goto err_out_free;
	}

	ir->dev = input_dev;

	
	ir->mask_keycode = mask_keycode;
	ir->mask_keydown = mask_keydown;
	ir->mask_keyup   = mask_keyup;
	ir->polling      = polling;
	ir->rc5_gpio	 = rc5_gpio;
	ir->nec_gpio	 = nec_gpio;

	
	snprintf(ir->name, sizeof(ir->name), "saa7134 IR (%s)",
		 saa7134_boards[dev->board].name);
	snprintf(ir->phys, sizeof(ir->phys), "pci-%s/ir0",
		 pci_name(dev->pci));

	ir_input_init(input_dev, &ir->ir, ir_type, ir_codes);
	input_dev->name = ir->name;
	input_dev->phys = ir->phys;
	input_dev->id.bustype = BUS_PCI;
	input_dev->id.version = 1;
	if (dev->pci->subsystem_vendor) {
		input_dev->id.vendor  = dev->pci->subsystem_vendor;
		input_dev->id.product = dev->pci->subsystem_device;
	} else {
		input_dev->id.vendor  = dev->pci->vendor;
		input_dev->id.product = dev->pci->device;
	}
	input_dev->dev.parent = &dev->pci->dev;

	dev->remote = ir;
	saa7134_ir_start(dev, ir);

	err = input_register_device(ir->dev);
	if (err)
		goto err_out_stop;

	
	ir->dev->rep[REP_DELAY] = repeat_delay;
	ir->dev->rep[REP_PERIOD] = repeat_period;

	return 0;

 err_out_stop:
	saa7134_ir_stop(dev);
	dev->remote = NULL;
 err_out_free:
	input_free_device(input_dev);
	kfree(ir);
	return err;
}

void saa7134_input_fini(struct saa7134_dev *dev)
{
	if (NULL == dev->remote)
		return;

	saa7134_ir_stop(dev);
	input_unregister_device(dev->remote->dev);
	kfree(dev->remote);
	dev->remote = NULL;
}

void saa7134_probe_i2c_ir(struct saa7134_dev *dev)
{
	const unsigned short addr_list[] = {
		0x7a, 0x47, 0x71, 0x2d,
		I2C_CLIENT_END
	};

	struct i2c_msg msg_msi = {
		.addr = 0x50,
		.flags = I2C_M_RD,
		.len = 0,
		.buf = NULL,
	};

	int rc;

	if (disable_ir) {
		dprintk("IR has been disabled, not probing for i2c remote\n");
		return;
	}

	memset(&dev->info, 0, sizeof(dev->info));
	memset(&dev->init_data, 0, sizeof(dev->init_data));
	strlcpy(dev->info.type, "ir_video", I2C_NAME_SIZE);

	switch (dev->board) {
	case SAA7134_BOARD_PINNACLE_PCTV_110i:
	case SAA7134_BOARD_PINNACLE_PCTV_310i:
		dev->init_data.name = "Pinnacle PCTV";
		if (pinnacle_remote == 0) {
			dev->init_data.get_key = get_key_pinnacle_color;
			dev->init_data.ir_codes = &ir_codes_pinnacle_color_table;
			dev->info.addr = 0x47;
		} else {
			dev->init_data.get_key = get_key_pinnacle_grey;
			dev->init_data.ir_codes = &ir_codes_pinnacle_grey_table;
			dev->info.addr = 0x47;
		}
		break;
	case SAA7134_BOARD_UPMOST_PURPLE_TV:
		dev->init_data.name = "Purple TV";
		dev->init_data.get_key = get_key_purpletv;
		dev->init_data.ir_codes = &ir_codes_purpletv_table;
		break;
	case SAA7134_BOARD_MSI_TVATANYWHERE_PLUS:
		dev->init_data.name = "MSI TV@nywhere Plus";
		dev->init_data.get_key = get_key_msi_tvanywhere_plus;
		dev->init_data.ir_codes = &ir_codes_msi_tvanywhere_plus_table;
		dev->info.addr = 0x30;
		
		rc = i2c_transfer(&dev->i2c_adap, &msg_msi, 1);
		dprintk(KERN_DEBUG "probe 0x%02x @ %s: %s\n",
			msg_msi.addr, dev->i2c_adap.name,
			(1 == rc) ? "yes" : "no");
		break;
	case SAA7134_BOARD_HAUPPAUGE_HVR1110:
		dev->init_data.name = "HVR 1110";
		dev->init_data.get_key = get_key_hvr1110;
		dev->init_data.ir_codes = &ir_codes_hauppauge_new_table;
		break;
	case SAA7134_BOARD_BEHOLD_607FM_MK3:
	case SAA7134_BOARD_BEHOLD_607FM_MK5:
	case SAA7134_BOARD_BEHOLD_609FM_MK3:
	case SAA7134_BOARD_BEHOLD_609FM_MK5:
	case SAA7134_BOARD_BEHOLD_607RDS_MK3:
	case SAA7134_BOARD_BEHOLD_607RDS_MK5:
	case SAA7134_BOARD_BEHOLD_609RDS_MK3:
	case SAA7134_BOARD_BEHOLD_609RDS_MK5:
	case SAA7134_BOARD_BEHOLD_M6:
	case SAA7134_BOARD_BEHOLD_M63:
	case SAA7134_BOARD_BEHOLD_M6_EXTRA:
	case SAA7134_BOARD_BEHOLD_H6:
	case SAA7134_BOARD_BEHOLD_X7:
		dev->init_data.name = "BeholdTV";
		dev->init_data.get_key = get_key_beholdm6xx;
		dev->init_data.ir_codes = &ir_codes_behold_table;
		break;
	case SAA7134_BOARD_AVERMEDIA_CARDBUS_501:
	case SAA7134_BOARD_AVERMEDIA_CARDBUS_506:
		dev->info.addr = 0x40;
		break;
	}

	if (dev->init_data.name)
		dev->info.platform_data = &dev->init_data;
	
	if (dev->info.addr) {
		i2c_new_device(&dev->i2c_adap, &dev->info);
		return;
	}

	
	i2c_new_probed_device(&dev->i2c_adap, &dev->info, addr_list);
}

static int saa7134_rc5_irq(struct saa7134_dev *dev)
{
	struct card_ir *ir = dev->remote;
	struct timeval tv;
	u32 gap;
	unsigned long current_jiffies, timeout;

	
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

		timeout = current_jiffies + (500 + 30 * HZ) / 1000;
		mod_timer(&ir->timer_end, timeout);
	}

	return 1;
}




static void saa7134_nec_timer(unsigned long data)
{
	struct saa7134_dev *dev = (struct saa7134_dev *) data;
	struct card_ir *ir = dev->remote;

	dprintk("Cancel key repeat\n");

	ir_input_nokey(ir->dev, &ir->ir);
}

static void nec_task(unsigned long data)
{
	struct saa7134_dev *dev = (struct saa7134_dev *) data;
	struct card_ir *ir;
	struct timeval tv;
	int count, pulse, oldpulse, gap;
	u32 ircode = 0, not_code = 0;
	int ngap = 0;

	if (!data) {
		printk(KERN_ERR "saa713x/ir: Can't recover dev struct\n");
		
		return;
	}

	ir = dev->remote;

	
	saa_clearb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);
	saa_setb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);

	oldpulse = saa_readl(SAA7134_GPIO_GPSTATUS0 >> 2) & ir->mask_keydown;
	pulse = oldpulse;

	do_gettimeofday(&tv);
	ir->base_time = tv;

	
	do {
		
		for (count = 0; count < 500; count++)  {
			udelay(10);
			
			saa_clearb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);
			saa_setb(SAA7134_GPIO_GPMODE3, SAA7134_GPIO_GPRESCAN);
			pulse = saa_readl(SAA7134_GPIO_GPSTATUS0 >> 2)
				& ir->mask_keydown;
			if (pulse != oldpulse)
				break;
		}

		do_gettimeofday(&tv);
		gap = 1000000 * (tv.tv_sec - ir->base_time.tv_sec) +
				tv.tv_usec - ir->base_time.tv_usec;

		if (!pulse) {
			
			if (ngap && (gap > 560 + 280)) {
				unsigned int shift = ngap - 1;

				
				if (shift < 8) {
					shift += 8;
					ircode |= 1 << shift;
				} else if (shift < 16) {
					not_code |= 1 << shift;
				} else if (shift < 24) {
					shift -= 16;
					ircode |= 1 << shift;
				} else {
					shift -= 24;
					not_code |= 1 << shift;
				}
			}
			ngap++;
		}


		ir->base_time = tv;

		
		if (gap >= 5000)
			break;
		oldpulse = pulse;
	} while (ngap < 32);

	if (ngap == 32) {
		
		ir->code = ir_extract_bits(ircode, ir->mask_keycode);

		dprintk("scancode = 0x%02x (code = 0x%02x, notcode= 0x%02x)\n",
			 ir->code, ircode, not_code);

		ir_input_keydown(ir->dev, &ir->ir, ir->code, ir->code);
	} else
		dprintk("Repeat last key\n");

	
	mod_timer(&ir->timer_keyup, jiffies + msecs_to_jiffies(150));

	saa_setl(SAA7134_IRQ2, SAA7134_IRQ2_INTE_GPIO18);
}

static int saa7134_nec_irq(struct saa7134_dev *dev)
{
	struct card_ir *ir = dev->remote;

	saa_clearl(SAA7134_IRQ2, SAA7134_IRQ2_INTE_GPIO18);
	tasklet_schedule(&ir->tlet);

	return 1;
}
