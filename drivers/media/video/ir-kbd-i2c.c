

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/workqueue.h>

#include <media/ir-common.h>
#include <media/ir-kbd-i2c.h>




static int debug;
module_param(debug, int, 0644);    

static int hauppauge;
module_param(hauppauge, int, 0644);    
MODULE_PARM_DESC(hauppauge, "Specify Hauppauge remote: 0=black, 1=grey (defaults to 0)");


#define DEVNAME "ir-kbd-i2c"
#define dprintk(level, fmt, arg...)	if (debug >= level) \
	printk(KERN_DEBUG DEVNAME ": " fmt , ## arg)



static int get_key_haup_common(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw,
			       int size, int offset)
{
	unsigned char buf[6];
	int start, range, toggle, dev, code, ircode;

	
	if (size != i2c_master_recv(ir->c, buf, size))
		return -EIO;

	
	start  = (buf[offset] >> 7) &    1;
	range  = (buf[offset] >> 6) &    1;
	toggle = (buf[offset] >> 5) &    1;
	dev    =  buf[offset]       & 0x1f;
	code   = (buf[offset+1] >> 2) & 0x3f;

	
	if (!start)
		
		return 0;
	
	ircode= (start << 12) | (toggle << 11) | (dev << 6) | code;
	if ((ircode & 0x1fff)==0x1fff)
		
		return 0;

	if (dev!=0x1e && dev!=0x1f)
		
		return 0;

	if (!range)
		code += 64;

	dprintk(1,"ir hauppauge (rc5): s%d r%d t%d dev=%d code=%d\n",
		start, range, toggle, dev, code);

	
	*ir_key = code;
	*ir_raw = ircode;
	return 1;
}

static int get_key_haup(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	return get_key_haup_common (ir, ir_key, ir_raw, 3, 0);
}

static int get_key_haup_xvr(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	return get_key_haup_common (ir, ir_key, ir_raw, 6, 3);
}

static int get_key_pixelview(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char b;

	
	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		dprintk(1,"read error\n");
		return -EIO;
	}
	*ir_key = b;
	*ir_raw = b;
	return 1;
}

static int get_key_pv951(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char b;

	
	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		dprintk(1,"read error\n");
		return -EIO;
	}

	
	if (b==0xaa)
		return 0;
	dprintk(2,"key %02x\n", b);

	*ir_key = b;
	*ir_raw = b;
	return 1;
}

static int get_key_fusionhdtv(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char buf[4];

	
	if (4 != i2c_master_recv(ir->c, buf, 4)) {
		dprintk(1,"read error\n");
		return -EIO;
	}

	if(buf[0] !=0 || buf[1] !=0 || buf[2] !=0 || buf[3] != 0)
		dprintk(2, "%s: 0x%2x 0x%2x 0x%2x 0x%2x\n", __func__,
			buf[0], buf[1], buf[2], buf[3]);

	
	if(buf[0] != 0x1 ||  buf[1] != 0xfe)
		return 0;

	*ir_key = buf[2];
	*ir_raw = (buf[2] << 8) | buf[3];

	return 1;
}

static int get_key_knc1(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char b;

	
	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		dprintk(1,"read error\n");
		return -EIO;
	}

	

	dprintk(2,"key %02x\n", b);

	if (b == 0xff)
		return 0;

	if (b == 0xfe)
		
		return 1;

	*ir_key = b;
	*ir_raw = b;
	return 1;
}

static int get_key_avermedia_cardbus(struct IR_i2c *ir,
				     u32 *ir_key, u32 *ir_raw)
{
	unsigned char subaddr, key, keygroup;
	struct i2c_msg msg[] = { { .addr = ir->c->addr, .flags = 0,
				   .buf = &subaddr, .len = 1},
				 { .addr = ir->c->addr, .flags = I2C_M_RD,
				  .buf = &key, .len = 1} };
	subaddr = 0x0d;
	if (2 != i2c_transfer(ir->c->adapter, msg, 2)) {
		dprintk(1, "read error\n");
		return -EIO;
	}

	if (key == 0xff)
		return 0;

	subaddr = 0x0b;
	msg[1].buf = &keygroup;
	if (2 != i2c_transfer(ir->c->adapter, msg, 2)) {
		dprintk(1, "read error\n");
		return -EIO;
	}

	if (keygroup == 0xff)
		return 0;

	dprintk(1, "read key 0x%02x/0x%02x\n", key, keygroup);
	if (keygroup < 2 || keygroup > 3) {
		
		dprintk(1, "warning: invalid key group 0x%02x for key 0x%02x\n",
								keygroup, key);
	}
	key |= (keygroup & 1) << 6;

	*ir_key = key;
	*ir_raw = key;
	return 1;
}



static void ir_key_poll(struct IR_i2c *ir)
{
	static u32 ir_key, ir_raw;
	int rc;

	dprintk(2,"ir_poll_key\n");
	rc = ir->get_key(ir, &ir_key, &ir_raw);
	if (rc < 0) {
		dprintk(2,"error\n");
		return;
	}

	if (0 == rc) {
		ir_input_nokey(ir->input, &ir->ir);
	} else {
		ir_input_keydown(ir->input, &ir->ir, ir_key, ir_raw);
	}
}

static void ir_work(struct work_struct *work)
{
	struct IR_i2c *ir = container_of(work, struct IR_i2c, work.work);
	int polling_interval = 100;

	
	if (ir->c->adapter->id == I2C_HW_SAA7134 && ir->c->addr == 0x30)
		polling_interval = 50;

	ir_key_poll(ir);
	schedule_delayed_work(&ir->work, msecs_to_jiffies(polling_interval));
}



static int ir_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ir_scancode_table *ir_codes = NULL;
	const char *name = NULL;
	int ir_type;
	struct IR_i2c *ir;
	struct input_dev *input_dev;
	struct i2c_adapter *adap = client->adapter;
	unsigned short addr = client->addr;
	int err;

	ir = kzalloc(sizeof(struct IR_i2c),GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ir || !input_dev) {
		err = -ENOMEM;
		goto err_out_free;
	}

	ir->c = client;
	ir->input = input_dev;
	i2c_set_clientdata(client, ir);

	switch(addr) {
	case 0x64:
		name        = "Pixelview";
		ir->get_key = get_key_pixelview;
		ir_type     = IR_TYPE_OTHER;
		ir_codes    = &ir_codes_empty_table;
		break;
	case 0x4b:
		name        = "PV951";
		ir->get_key = get_key_pv951;
		ir_type     = IR_TYPE_OTHER;
		ir_codes    = &ir_codes_pv951_table;
		break;
	case 0x18:
	case 0x1a:
		name        = "Hauppauge";
		ir->get_key = get_key_haup;
		ir_type     = IR_TYPE_RC5;
		if (hauppauge == 1) {
			ir_codes    = &ir_codes_hauppauge_new_table;
		} else {
			ir_codes    = &ir_codes_rc5_tv_table;
		}
		break;
	case 0x30:
		name        = "KNC One";
		ir->get_key = get_key_knc1;
		ir_type     = IR_TYPE_OTHER;
		ir_codes    = &ir_codes_empty_table;
		break;
	case 0x6b:
		name        = "FusionHDTV";
		ir->get_key = get_key_fusionhdtv;
		ir_type     = IR_TYPE_RC5;
		ir_codes    = &ir_codes_fusionhdtv_mce_table;
		break;
	case 0x7a:
	case 0x47:
	case 0x71:
	case 0x2d:
		if (adap->id == I2C_HW_B_CX2388x ||
		    adap->id == I2C_HW_B_CX2341X) {
			
			name = adap->id == I2C_HW_B_CX2341X ? "CX2341x remote"
							    : "CX2388x remote";
			ir_type     = IR_TYPE_RC5;
			ir->get_key = get_key_haup_xvr;
			if (hauppauge == 1) {
				ir_codes    = &ir_codes_hauppauge_new_table;
			} else {
				ir_codes    = &ir_codes_rc5_tv_table;
			}
		} else {
			
			name        = "SAA713x remote";
			ir_type     = IR_TYPE_OTHER;
		}
		break;
	case 0x40:
		name        = "AVerMedia Cardbus remote";
		ir->get_key = get_key_avermedia_cardbus;
		ir_type     = IR_TYPE_OTHER;
		ir_codes    = &ir_codes_avermedia_cardbus_table;
		break;
	default:
		dprintk(1, DEVNAME ": Unsupported i2c address 0x%02x\n", addr);
		err = -ENODEV;
		goto err_out_free;
	}

	
	if (client->dev.platform_data) {
		const struct IR_i2c_init_data *init_data =
						client->dev.platform_data;

		ir_codes = init_data->ir_codes;
		name = init_data->name;
		if (init_data->type)
			ir_type = init_data->type;

		switch (init_data->internal_get_key_func) {
		case IR_KBD_GET_KEY_CUSTOM:
			
			ir->get_key = init_data->get_key;
			break;
		case IR_KBD_GET_KEY_PIXELVIEW:
			ir->get_key = get_key_pixelview;
			break;
		case IR_KBD_GET_KEY_PV951:
			ir->get_key = get_key_pv951;
			break;
		case IR_KBD_GET_KEY_HAUP:
			ir->get_key = get_key_haup;
			break;
		case IR_KBD_GET_KEY_KNC1:
			ir->get_key = get_key_knc1;
			break;
		case IR_KBD_GET_KEY_FUSIONHDTV:
			ir->get_key = get_key_fusionhdtv;
			break;
		case IR_KBD_GET_KEY_HAUP_XVR:
			ir->get_key = get_key_haup_xvr;
			break;
		case IR_KBD_GET_KEY_AVERMEDIA_CARDBUS:
			ir->get_key = get_key_avermedia_cardbus;
			break;
		}
	}

	
	if (!name || !ir->get_key || !ir_codes) {
		dprintk(1, DEVNAME ": Unsupported device at address 0x%02x\n",
			addr);
		err = -ENODEV;
		goto err_out_free;
	}

	
	snprintf(ir->name, sizeof(ir->name), "i2c IR (%s)", name);
	ir->ir_codes = ir_codes;

	snprintf(ir->phys, sizeof(ir->phys), "%s/%s/ir0",
		 dev_name(&adap->dev),
		 dev_name(&client->dev));

	
	ir_input_init(input_dev, &ir->ir, ir_type, ir->ir_codes);
	input_dev->id.bustype = BUS_I2C;
	input_dev->name       = ir->name;
	input_dev->phys       = ir->phys;

	err = input_register_device(ir->input);
	if (err)
		goto err_out_free;

	printk(DEVNAME ": %s detected at %s [%s]\n",
	       ir->input->name, ir->input->phys, adap->name);

	
	INIT_DELAYED_WORK(&ir->work, ir_work);
	schedule_delayed_work(&ir->work, 0);

	return 0;

 err_out_free:
	input_free_device(input_dev);
	kfree(ir);
	return err;
}

static int ir_remove(struct i2c_client *client)
{
	struct IR_i2c *ir = i2c_get_clientdata(client);

	
	cancel_delayed_work_sync(&ir->work);

	
	input_unregister_device(ir->input);

	
	kfree(ir);
	return 0;
}

static const struct i2c_device_id ir_kbd_id[] = {
	
	{ "ir_video", 0 },
	
	{ "ir_rx_z8f0811_haup", 0 },
	{ }
};

static struct i2c_driver driver = {
	.driver = {
		.name   = "ir-kbd-i2c",
	},
	.probe          = ir_probe,
	.remove         = ir_remove,
	.id_table       = ir_kbd_id,
};



MODULE_AUTHOR("Gerd Knorr, Michal Kochanowicz, Christoph Bartelmus, Ulrich Mueller");
MODULE_DESCRIPTION("input driver for i2c IR remote controls");
MODULE_LICENSE("GPL");

static int __init ir_init(void)
{
	return i2c_add_driver(&driver);
}

static void __exit ir_fini(void)
{
	i2c_del_driver(&driver);
}

module_init(ir_init);
module_exit(ir_fini);


