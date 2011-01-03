

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/usb.h>

#include "em28xx.h"

#define EM28XX_SNAPSHOT_KEY KEY_CAMERA
#define EM28XX_SBUTTON_QUERY_INTERVAL 500
#define EM28XX_R0C_USBSUSP_SNAPSHOT 0x20

static unsigned int ir_debug;
module_param(ir_debug, int, 0644);
MODULE_PARM_DESC(ir_debug, "enable debug messages [IR]");

#define i2cdprintk(fmt, arg...) \
	if (ir_debug) { \
		printk(KERN_DEBUG "%s/ir: " fmt, ir->name , ## arg); \
	}

#define dprintk(fmt, arg...) \
	if (ir_debug) { \
		printk(KERN_DEBUG "%s/ir: " fmt, ir->name , ## arg); \
	}



struct em28xx_ir_poll_result {
	unsigned int toggle_bit:1;
	unsigned int read_count:7;
	u8 rc_address;
	u8 rc_data[4]; 
};

struct em28xx_IR {
	struct em28xx *dev;
	struct input_dev *input;
	struct ir_input_state ir;
	char name[32];
	char phys[32];

	
	int polling;
	struct delayed_work work;
	unsigned int last_toggle:1;
	unsigned int last_readcount;
	unsigned int repeat_interval;

	int  (*get_key)(struct em28xx_IR *, struct em28xx_ir_poll_result *);
};



int em28xx_get_key_terratec(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char b;

	
	if (1 != i2c_master_recv(ir->c, &b, 1)) {
		i2cdprintk("read error\n");
		return -EIO;
	}

	

	i2cdprintk("key %02x\n", b);

	if (b == 0xff)
		return 0;

	if (b == 0xfe)
		
		return 1;

	*ir_key = b;
	*ir_raw = b;
	return 1;
}

int em28xx_get_key_em_haup(struct IR_i2c *ir, u32 *ir_key, u32 *ir_raw)
{
	unsigned char buf[2];
	unsigned char code;

	
	if (2 != i2c_master_recv(ir->c, buf, 2))
		return -EIO;

	
	if (buf[1] == 0xff)
		return 0;

	ir->old = buf[1];

	
	code =   ((buf[0]&0x01)<<5) | 
		 ((buf[0]&0x02)<<3) | 
		 ((buf[0]&0x04)<<1) | 
		 ((buf[0]&0x08)>>1) | 
		 ((buf[0]&0x10)>>3) | 
		 ((buf[0]&0x20)>>5);  

	i2cdprintk("ir hauppauge (em2840): code=0x%02x (rcv=0x%02x)\n",
			code, buf[0]);

	
	*ir_key = code;
	*ir_raw = code;
	return 1;
}

int em28xx_get_key_pinnacle_usb_grey(struct IR_i2c *ir, u32 *ir_key,
				     u32 *ir_raw)
{
	unsigned char buf[3];

	

	if (3 != i2c_master_recv(ir->c, buf, 3)) {
		i2cdprintk("read error\n");
		return -EIO;
	}

	i2cdprintk("key %02x\n", buf[2]&0x3f);
	if (buf[0] != 0x00)
		return 0;

	*ir_key = buf[2]&0x3f;
	*ir_raw = buf[2]&0x3f;

	return 1;
}




static int default_polling_getkey(struct em28xx_IR *ir,
				  struct em28xx_ir_poll_result *poll_result)
{
	struct em28xx *dev = ir->dev;
	int rc;
	u8 msg[3] = { 0, 0, 0 };

	
	rc = dev->em28xx_read_reg_req_len(dev, 0, EM28XX_R45_IR,
					  msg, sizeof(msg));
	if (rc < 0)
		return rc;

	
	poll_result->toggle_bit = (msg[0] >> 7);

	
	poll_result->read_count = (msg[0] & 0x7f);

	
	poll_result->rc_address = msg[1];

	
	poll_result->rc_data[0] = msg[2];

	return 0;
}

static int em2874_polling_getkey(struct em28xx_IR *ir,
				 struct em28xx_ir_poll_result *poll_result)
{
	struct em28xx *dev = ir->dev;
	int rc;
	u8 msg[5] = { 0, 0, 0, 0, 0 };

	
	rc = dev->em28xx_read_reg_req_len(dev, 0, EM2874_R51_IR,
					  msg, sizeof(msg));
	if (rc < 0)
		return rc;

	
	poll_result->toggle_bit = (msg[0] >> 7);

	
	poll_result->read_count = (msg[0] & 0x7f);

	
	poll_result->rc_address = msg[1];

	
	poll_result->rc_data[0] = msg[2];
	poll_result->rc_data[1] = msg[3];
	poll_result->rc_data[2] = msg[4];

	return 0;
}



static void em28xx_ir_handle_key(struct em28xx_IR *ir)
{
	int result;
	int do_sendkey = 0;
	struct em28xx_ir_poll_result poll_result;

	
	result = ir->get_key(ir, &poll_result);
	if (result < 0) {
		dprintk("ir->get_key() failed %d\n", result);
		return;
	}

	dprintk("ir->get_key result tb=%02x rc=%02x lr=%02x data=%02x\n",
		poll_result.toggle_bit, poll_result.read_count,
		ir->last_readcount, poll_result.rc_data[0]);

	if (ir->dev->chip_id == CHIP_ID_EM2874) {
		
		ir->last_readcount = 0;
	}

	if (poll_result.read_count == 0) {
		
	} else if (ir->last_toggle != poll_result.toggle_bit) {
		
		dprintk("button has been pressed\n");
		ir->last_toggle = poll_result.toggle_bit;
		ir->repeat_interval = 0;
		do_sendkey = 1;
	} else if (poll_result.toggle_bit == ir->last_toggle &&
		   poll_result.read_count > 0 &&
		   poll_result.read_count != ir->last_readcount) {
		
		dprintk("button being held down\n");

		
		if (ir->repeat_interval++ > 9) {
			
			do_sendkey = 1;
		}
	}

	if (do_sendkey) {
		dprintk("sending keypress\n");
		ir_input_keydown(ir->input, &ir->ir, poll_result.rc_data[0],
				 poll_result.rc_data[0]);
		ir_input_nokey(ir->input, &ir->ir);
	}

	ir->last_readcount = poll_result.read_count;
	return;
}

static void em28xx_ir_work(struct work_struct *work)
{
	struct em28xx_IR *ir = container_of(work, struct em28xx_IR, work.work);

	em28xx_ir_handle_key(ir);
	schedule_delayed_work(&ir->work, msecs_to_jiffies(ir->polling));
}

static void em28xx_ir_start(struct em28xx_IR *ir)
{
	INIT_DELAYED_WORK(&ir->work, em28xx_ir_work);
	schedule_delayed_work(&ir->work, 0);
}

static void em28xx_ir_stop(struct em28xx_IR *ir)
{
	cancel_delayed_work_sync(&ir->work);
}

int em28xx_ir_init(struct em28xx *dev)
{
	struct em28xx_IR *ir;
	struct input_dev *input_dev;
	u8 ir_config;
	int err = -ENOMEM;

	if (dev->board.ir_codes == NULL) {
		
		return 0;
	}

	ir = kzalloc(sizeof(*ir), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!ir || !input_dev)
		goto err_out_free;

	ir->input = input_dev;

	
	switch (dev->chip_id) {
	case CHIP_ID_EM2860:
	case CHIP_ID_EM2883:
		ir->get_key = default_polling_getkey;
		break;
	case CHIP_ID_EM2874:
		ir->get_key = em2874_polling_getkey;
		
		ir_config = EM2874_IR_RC5;
		em28xx_write_regs(dev, EM2874_R50_IR_CONFIG, &ir_config, 1);
		break;
	default:
		printk("Unrecognized em28xx chip id: IR not supported\n");
		goto err_out_free;
	}

	
	ir->polling = 100; 

	
	snprintf(ir->name, sizeof(ir->name), "em28xx IR (%s)",
						dev->name);

	usb_make_path(dev->udev, ir->phys, sizeof(ir->phys));
	strlcat(ir->phys, "/input0", sizeof(ir->phys));

	ir_input_init(input_dev, &ir->ir, IR_TYPE_OTHER, dev->board.ir_codes);
	input_dev->name = ir->name;
	input_dev->phys = ir->phys;
	input_dev->id.bustype = BUS_USB;
	input_dev->id.version = 1;
	input_dev->id.vendor = le16_to_cpu(dev->udev->descriptor.idVendor);
	input_dev->id.product = le16_to_cpu(dev->udev->descriptor.idProduct);

	input_dev->dev.parent = &dev->udev->dev;
	
	ir->dev = dev;
	dev->ir = ir;

	em28xx_ir_start(ir);

	
	err = input_register_device(ir->input);
	if (err)
		goto err_out_stop;

	return 0;
 err_out_stop:
	em28xx_ir_stop(ir);
	dev->ir = NULL;
 err_out_free:
	input_free_device(input_dev);
	kfree(ir);
	return err;
}

int em28xx_ir_fini(struct em28xx *dev)
{
	struct em28xx_IR *ir = dev->ir;

	
	if (!ir)
		return 0;

	em28xx_ir_stop(ir);
	input_unregister_device(ir->input);
	kfree(ir);

	
	dev->ir = NULL;
	return 0;
}



static void em28xx_query_sbutton(struct work_struct *work)
{
	
	struct em28xx *dev =
		container_of(work, struct em28xx, sbutton_query_work.work);
	int ret;

	ret = em28xx_read_reg(dev, EM28XX_R0C_USBSUSP);

	if (ret & EM28XX_R0C_USBSUSP_SNAPSHOT) {
		u8 cleared;
		
		cleared = ((u8) ret) & ~EM28XX_R0C_USBSUSP_SNAPSHOT;
		em28xx_write_regs(dev, EM28XX_R0C_USBSUSP, &cleared, 1);

		
		input_report_key(dev->sbutton_input_dev, EM28XX_SNAPSHOT_KEY,
				 1);
		
		input_report_key(dev->sbutton_input_dev, EM28XX_SNAPSHOT_KEY,
				 0);
	}

	
	schedule_delayed_work(&dev->sbutton_query_work,
			      msecs_to_jiffies(EM28XX_SBUTTON_QUERY_INTERVAL));
}

void em28xx_register_snapshot_button(struct em28xx *dev)
{
	struct input_dev *input_dev;
	int err;

	em28xx_info("Registering snapshot button...\n");
	input_dev = input_allocate_device();
	if (!input_dev) {
		em28xx_errdev("input_allocate_device failed\n");
		return;
	}

	usb_make_path(dev->udev, dev->snapshot_button_path,
		      sizeof(dev->snapshot_button_path));
	strlcat(dev->snapshot_button_path, "/sbutton",
		sizeof(dev->snapshot_button_path));
	INIT_DELAYED_WORK(&dev->sbutton_query_work, em28xx_query_sbutton);

	input_dev->name = "em28xx snapshot button";
	input_dev->phys = dev->snapshot_button_path;
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
	set_bit(EM28XX_SNAPSHOT_KEY, input_dev->keybit);
	input_dev->keycodesize = 0;
	input_dev->keycodemax = 0;
	input_dev->id.bustype = BUS_USB;
	input_dev->id.vendor = le16_to_cpu(dev->udev->descriptor.idVendor);
	input_dev->id.product = le16_to_cpu(dev->udev->descriptor.idProduct);
	input_dev->id.version = 1;
	input_dev->dev.parent = &dev->udev->dev;

	err = input_register_device(input_dev);
	if (err) {
		em28xx_errdev("input_register_device failed\n");
		input_free_device(input_dev);
		return;
	}

	dev->sbutton_input_dev = input_dev;
	schedule_delayed_work(&dev->sbutton_query_work,
			      msecs_to_jiffies(EM28XX_SBUTTON_QUERY_INTERVAL));
	return;

}

void em28xx_deregister_snapshot_button(struct em28xx *dev)
{
	if (dev->sbutton_input_dev != NULL) {
		em28xx_info("Deregistering snapshot button\n");
		cancel_rearming_delayed_work(&dev->sbutton_query_work);
		input_unregister_device(dev->sbutton_input_dev);
		dev->sbutton_input_dev = NULL;
	}
	return;
}
