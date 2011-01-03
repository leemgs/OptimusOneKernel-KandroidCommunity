

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-i2c-drv.h>
#include <media/i2c-addr.h>

static int debug; 
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_LICENSE("GPL");



struct tda9875 {
	struct v4l2_subdev sd;
	int rvol, lvol;
	int bass, treble;
};

static inline struct tda9875 *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct tda9875, sd);
}

#define dprintk  if (debug) printk



		
#define TDA9875_MUT         0x12  
#define TDA9875_CFG         0x01  
#define TDA9875_DACOS       0x13  
#define TDA9875_LOSR        0x16  

#define TDA9875_CH1V        0x0c  
#define TDA9875_CH2V        0x0d  
#define TDA9875_SC1         0x14  
#define TDA9875_SC2         0x15  

#define TDA9875_ADCIS       0x17  
#define TDA9875_AER         0x19  
#define TDA9875_MCS         0x18  
#define TDA9875_MVL         0x1a  
#define TDA9875_MVR         0x1b  
#define TDA9875_MBA         0x1d  
#define TDA9875_MTR         0x1e  
#define TDA9875_ACS         0x1f  
#define TDA9875_AVL         0x20  
#define TDA9875_AVR         0x21  
#define TDA9875_ABA         0x22  
#define TDA9875_ATR         0x23  

#define TDA9875_MSR         0x02  
#define TDA9875_C1MSB       0x03  
#define TDA9875_C1MIB       0x04  
#define TDA9875_C1LSB       0x05  
#define TDA9875_C2MSB       0x06  
#define TDA9875_C2MIB       0x07  
#define TDA9875_C2LSB       0x08  
#define TDA9875_DCR         0x09  
#define TDA9875_DEEM        0x0a  
#define TDA9875_FMAT        0x0b  


#define TDA9875_MUTE_ON	    0xff 
#define TDA9875_MUTE_OFF    0xcc 





static int tda9875_write(struct v4l2_subdev *sd, int subaddr, unsigned char val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buffer[2];

	v4l2_dbg(1, debug, sd, "Writing %d 0x%x\n", subaddr, val);
	buffer[0] = subaddr;
	buffer[1] = val;
	if (2 != i2c_master_send(client, buffer, 2)) {
		v4l2_warn(sd, "I/O error, trying (write %d 0x%x)\n",
		       subaddr, val);
		return -1;
	}
	return 0;
}


static int i2c_read_register(struct i2c_client *client, int addr, int reg)
{
	unsigned char write[1];
	unsigned char read[1];
	struct i2c_msg msgs[2] = {
		{ addr, 0,        1, write },
		{ addr, I2C_M_RD, 1, read  }
	};

	write[0] = reg;

	if (2 != i2c_transfer(client->adapter, msgs, 2)) {
		v4l_warn(client, "I/O error (read2)\n");
		return -1;
	}
	v4l_dbg(1, debug, client, "chip_read2: reg%d=0x%x\n", reg, read[0]);
	return read[0];
}

static void tda9875_set(struct v4l2_subdev *sd)
{
	struct tda9875 *tda = to_state(sd);
	unsigned char a;

	v4l2_dbg(1, debug, sd, "tda9875_set(%04x,%04x,%04x,%04x)\n",
		tda->lvol, tda->rvol, tda->bass, tda->treble);

	a = tda->lvol & 0xff;
	tda9875_write(sd, TDA9875_MVL, a);
	a =tda->rvol & 0xff;
	tda9875_write(sd, TDA9875_MVR, a);
	a =tda->bass & 0xff;
	tda9875_write(sd, TDA9875_MBA, a);
	a =tda->treble  & 0xff;
	tda9875_write(sd, TDA9875_MTR, a);
}

static void do_tda9875_init(struct v4l2_subdev *sd)
{
	struct tda9875 *t = to_state(sd);

	v4l2_dbg(1, debug, sd, "In tda9875_init\n");
	tda9875_write(sd, TDA9875_CFG, 0xd0); 
	tda9875_write(sd, TDA9875_MSR, 0x03);    
	tda9875_write(sd, TDA9875_C1MSB, 0x00);  
	tda9875_write(sd, TDA9875_C1MIB, 0x00);  
	tda9875_write(sd, TDA9875_C1LSB, 0x00);  
	tda9875_write(sd, TDA9875_C2MSB, 0x00);  
	tda9875_write(sd, TDA9875_C2MIB, 0x00);  
	tda9875_write(sd, TDA9875_C2LSB, 0x00);  
	tda9875_write(sd, TDA9875_DCR, 0x00);    
	tda9875_write(sd, TDA9875_DEEM, 0x44);   
	tda9875_write(sd, TDA9875_FMAT, 0x00);   
	tda9875_write(sd, TDA9875_SC1, 0x00);    
	tda9875_write(sd, TDA9875_SC2, 0x01);    

	tda9875_write(sd, TDA9875_CH1V, 0x10);  
	tda9875_write(sd, TDA9875_CH2V, 0x10);  
	tda9875_write(sd, TDA9875_DACOS, 0x02); 
	tda9875_write(sd, TDA9875_ADCIS, 0x6f); 
	tda9875_write(sd, TDA9875_LOSR, 0x00);  
	tda9875_write(sd, TDA9875_AER, 0x00);   
	tda9875_write(sd, TDA9875_MCS, 0x44);   
	tda9875_write(sd, TDA9875_MVL, 0x03);   
	tda9875_write(sd, TDA9875_MVR, 0x03);   
	tda9875_write(sd, TDA9875_MBA, 0x00);   
	tda9875_write(sd, TDA9875_MTR, 0x00);   
	tda9875_write(sd, TDA9875_ACS, 0x44);   
	tda9875_write(sd, TDA9875_AVL, 0x00);   
	tda9875_write(sd, TDA9875_AVR, 0x00);   
	tda9875_write(sd, TDA9875_ABA, 0x00);   
	tda9875_write(sd, TDA9875_ATR, 0x00);   

	tda9875_write(sd, TDA9875_MUT, 0xcc);   

	t->lvol = t->rvol = 0;  	
	t->bass = 0; 			
	t->treble = 0;  		
	tda9875_set(sd);
}


static int tda9875_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct tda9875 *t = to_state(sd);

	switch (ctrl->id) {
	case V4L2_CID_AUDIO_VOLUME:
	{
		int left = (t->lvol+84)*606;
		int right = (t->rvol+84)*606;

		ctrl->value=max(left,right);
		return 0;
	}
	case V4L2_CID_AUDIO_BALANCE:
	{
		int left = (t->lvol+84)*606;
		int right = (t->rvol+84)*606;
		int volume = max(left,right);
		int balance = (32768*min(left,right))/
			      (volume ? volume : 1);
		ctrl->value=(left<right)?
			(65535-balance) : balance;
		return 0;
	}
	case V4L2_CID_AUDIO_BASS:
		ctrl->value = (t->bass+12)*2427;    
		return 0;
	case V4L2_CID_AUDIO_TREBLE:
		ctrl->value = (t->treble+12)*2730;
		return 0;
	}
	return -EINVAL;
}

static int tda9875_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct tda9875 *t = to_state(sd);
	int chvol = 0, volume = 0, balance = 0, left, right;

	switch (ctrl->id) {
	case V4L2_CID_AUDIO_VOLUME:
		left = (t->lvol+84)*606;
		right = (t->rvol+84)*606;

		volume = max(left,right);
		balance = (32768*min(left,right))/
			      (volume ? volume : 1);
		balance =(left<right)?
			(65535-balance) : balance;

		volume = ctrl->value;

		chvol=1;
		break;
	case V4L2_CID_AUDIO_BALANCE:
		left = (t->lvol+84)*606;
		right = (t->rvol+84)*606;

		volume=max(left,right);

		balance = ctrl->value;

		chvol=1;
		break;
	case V4L2_CID_AUDIO_BASS:
		t->bass = ((ctrl->value/2400)-12) & 0xff;
		if (t->bass > 15)
			t->bass = 15;
		if (t->bass < -12)
			t->bass = -12 & 0xff;
		break;
	case V4L2_CID_AUDIO_TREBLE:
		t->treble = ((ctrl->value/2700)-12) & 0xff;
		if (t->treble > 12)
			t->treble = 12;
		if (t->treble < -12)
			t->treble = -12 & 0xff;
		break;
	default:
		return -EINVAL;
	}

	if (chvol) {
		left = (min(65536 - balance,32768) *
			volume) / 32768;
		right = (min(balance,32768) *
				volume) / 32768;
		t->lvol = ((left/606)-84) & 0xff;
		if (t->lvol > 24)
			t->lvol = 24;
		if (t->lvol < -84)
			t->lvol = -84 & 0xff;

		t->rvol = ((right/606)-84) & 0xff;
		if (t->rvol > 24)
			t->rvol = 24;
		if (t->rvol < -84)
			t->rvol = -84 & 0xff;
	}

	tda9875_set(sd);
	return 0;
}

static int tda9875_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	switch (qc->id) {
	case V4L2_CID_AUDIO_VOLUME:
		return v4l2_ctrl_query_fill(qc, 0, 65535, 65535 / 100, 58880);
	case V4L2_CID_AUDIO_BASS:
	case V4L2_CID_AUDIO_TREBLE:
		return v4l2_ctrl_query_fill(qc, 0, 65535, 65535 / 100, 32768);
	}
	return -EINVAL;
}



static const struct v4l2_subdev_core_ops tda9875_core_ops = {
	.queryctrl = tda9875_queryctrl,
	.g_ctrl = tda9875_g_ctrl,
	.s_ctrl = tda9875_s_ctrl,
};

static const struct v4l2_subdev_ops tda9875_ops = {
	.core = &tda9875_core_ops,
};






static int tda9875_checkit(struct i2c_client *client, int addr)
{
	int dic, rev;

	dic = i2c_read_register(client, addr, 254);
	rev = i2c_read_register(client, addr, 255);

	if (dic == 0 || dic == 2) { 
		v4l_info(client, "tda9875%s rev. %d detected at 0x%02x\n",
			dic == 0 ? "" : "A", rev, addr << 1);
		return 1;
	}
	v4l_info(client, "no such chip at 0x%02x (dic=0x%x rev=0x%x)\n",
			addr << 1, dic, rev);
	return 0;
}

static int tda9875_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct tda9875 *t;
	struct v4l2_subdev *sd;

	v4l_info(client, "chip found @ 0x%02x (%s)\n",
			client->addr << 1, client->adapter->name);

	if (!tda9875_checkit(client, client->addr))
		return -ENODEV;

	t = kzalloc(sizeof(*t), GFP_KERNEL);
	if (!t)
		return -ENOMEM;
	sd = &t->sd;
	v4l2_i2c_subdev_init(sd, client, &tda9875_ops);

	do_tda9875_init(sd);
	return 0;
}

static int tda9875_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);

	do_tda9875_init(sd);
	v4l2_device_unregister_subdev(sd);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id tda9875_id[] = {
	{ "tda9875", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, tda9875_id);

static struct v4l2_i2c_driver_data v4l2_i2c_data = {
	.name = "tda9875",
	.probe = tda9875_probe,
	.remove = tda9875_remove,
	.id_table = tda9875_id,
};
