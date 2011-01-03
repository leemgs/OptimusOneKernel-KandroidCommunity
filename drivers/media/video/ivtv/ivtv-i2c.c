



#include "ivtv-driver.h"
#include "ivtv-cards.h"
#include "ivtv-gpio.h"
#include "ivtv-i2c.h"



#define IVTV_REG_I2C_SETSCL_OFFSET 0x7000
#define IVTV_REG_I2C_SETSDA_OFFSET 0x7004
#define IVTV_REG_I2C_GETSCL_OFFSET 0x7008
#define IVTV_REG_I2C_GETSDA_OFFSET 0x700c

#define IVTV_CS53L32A_I2C_ADDR		0x11
#define IVTV_M52790_I2C_ADDR		0x48
#define IVTV_CX25840_I2C_ADDR 		0x44
#define IVTV_SAA7115_I2C_ADDR 		0x21
#define IVTV_SAA7127_I2C_ADDR 		0x44
#define IVTV_SAA717x_I2C_ADDR 		0x21
#define IVTV_MSP3400_I2C_ADDR 		0x40
#define IVTV_HAUPPAUGE_I2C_ADDR 	0x50
#define IVTV_WM8739_I2C_ADDR 		0x1a
#define IVTV_WM8775_I2C_ADDR		0x1b
#define IVTV_TEA5767_I2C_ADDR		0x60
#define IVTV_UPD64031A_I2C_ADDR 	0x12
#define IVTV_UPD64083_I2C_ADDR 		0x5c
#define IVTV_VP27SMPX_I2C_ADDR      	0x5b
#define IVTV_M52790_I2C_ADDR      	0x48


static const u8 hw_addrs[] = {
	IVTV_CX25840_I2C_ADDR,
	IVTV_SAA7115_I2C_ADDR,
	IVTV_SAA7127_I2C_ADDR,
	IVTV_MSP3400_I2C_ADDR,
	0,
	IVTV_WM8775_I2C_ADDR,
	IVTV_CS53L32A_I2C_ADDR,
	0,
	IVTV_SAA7115_I2C_ADDR,
	IVTV_UPD64031A_I2C_ADDR,
	IVTV_UPD64083_I2C_ADDR,
	IVTV_SAA717x_I2C_ADDR,
	IVTV_WM8739_I2C_ADDR,
	IVTV_VP27SMPX_I2C_ADDR,
	IVTV_M52790_I2C_ADDR,
	0 		
};


static const char *hw_modules[] = {
	"cx25840",
	"saa7115",
	"saa7127",
	"msp3400",
	"tuner",
	"wm8775",
	"cs53l32a",
	NULL,
	"saa7115",
	"upd64031a",
	"upd64083",
	"saa717x",
	"wm8739",
	"vp27smpx",
	"m52790",
	NULL
};


static const char * const hw_devicenames[] = {
	"cx25840",
	"saa7115",
	"saa7127_auto",	
	"msp3400",
	"tuner",
	"wm8775",
	"cs53l32a",
	"tveeprom",
	"saa7114",
	"upd64031a",
	"upd64083",
	"saa717x",
	"wm8739",
	"vp27smpx",
	"m52790",
	"gpio",
};

int ivtv_i2c_register(struct ivtv *itv, unsigned idx)
{
	struct v4l2_subdev *sd;
	struct i2c_adapter *adap = &itv->i2c_adap;
	const char *mod = hw_modules[idx];
	const char *type = hw_devicenames[idx];
	u32 hw = 1 << idx;

	if (idx >= ARRAY_SIZE(hw_addrs))
		return -1;
	if (hw == IVTV_HW_TUNER) {
		
		sd = v4l2_i2c_new_subdev(&itv->v4l2_dev,
				adap, mod, type,
				0, itv->card_i2c->radio);
		if (sd)
			sd->grp_id = 1 << idx;
		sd = v4l2_i2c_new_subdev(&itv->v4l2_dev,
				adap, mod, type,
				0, itv->card_i2c->demod);
		if (sd)
			sd->grp_id = 1 << idx;
		sd = v4l2_i2c_new_subdev(&itv->v4l2_dev,
				adap, mod, type,
				0, itv->card_i2c->tv);
		if (sd)
			sd->grp_id = 1 << idx;
		return sd ? 0 : -1;
	}
	if (!hw_addrs[idx])
		return -1;
	if (hw == IVTV_HW_UPD64031A || hw == IVTV_HW_UPD6408X) {
		sd = v4l2_i2c_new_subdev(&itv->v4l2_dev,
				adap, mod, type, 0, I2C_ADDRS(hw_addrs[idx]));
	} else {
		sd = v4l2_i2c_new_subdev(&itv->v4l2_dev,
				adap, mod, type, hw_addrs[idx], NULL);
	}
	if (sd)
		sd->grp_id = 1 << idx;
	return sd ? 0 : -1;
}

struct v4l2_subdev *ivtv_find_hw(struct ivtv *itv, u32 hw)
{
	struct v4l2_subdev *result = NULL;
	struct v4l2_subdev *sd;

	spin_lock(&itv->v4l2_dev.lock);
	v4l2_device_for_each_subdev(sd, &itv->v4l2_dev) {
		if (sd->grp_id == hw) {
			result = sd;
			break;
		}
	}
	spin_unlock(&itv->v4l2_dev.lock);
	return result;
}


static void ivtv_setscl(struct ivtv *itv, int state)
{
	
	
	write_reg(~state, IVTV_REG_I2C_SETSCL_OFFSET);
}


static void ivtv_setsda(struct ivtv *itv, int state)
{
	
	
	write_reg(~state & 1, IVTV_REG_I2C_SETSDA_OFFSET);
}


static int ivtv_getscl(struct ivtv *itv)
{
	return read_reg(IVTV_REG_I2C_GETSCL_OFFSET) & 1;
}


static int ivtv_getsda(struct ivtv *itv)
{
	return read_reg(IVTV_REG_I2C_GETSDA_OFFSET) & 1;
}


static void ivtv_scldelay(struct ivtv *itv)
{
	int i;

	for (i = 0; i < 5; ++i)
		ivtv_getscl(itv);
}


static int ivtv_waitscl(struct ivtv *itv, int val)
{
	int i;

	ivtv_scldelay(itv);
	for (i = 0; i < 1000; ++i) {
		if (ivtv_getscl(itv) == val)
			return 1;
	}
	return 0;
}


static int ivtv_waitsda(struct ivtv *itv, int val)
{
	int i;

	ivtv_scldelay(itv);
	for (i = 0; i < 1000; ++i) {
		if (ivtv_getsda(itv) == val)
			return 1;
	}
	return 0;
}


static int ivtv_ack(struct ivtv *itv)
{
	int ret = 0;

	if (ivtv_getscl(itv) == 1) {
		IVTV_DEBUG_HI_I2C("SCL was high starting an ack\n");
		ivtv_setscl(itv, 0);
		if (!ivtv_waitscl(itv, 0)) {
			IVTV_DEBUG_I2C("Could not set SCL low starting an ack\n");
			return -EREMOTEIO;
		}
	}
	ivtv_setsda(itv, 1);
	ivtv_scldelay(itv);
	ivtv_setscl(itv, 1);
	if (!ivtv_waitsda(itv, 0)) {
		IVTV_DEBUG_I2C("Slave did not ack\n");
		ret = -EREMOTEIO;
	}
	ivtv_setscl(itv, 0);
	if (!ivtv_waitscl(itv, 0)) {
		IVTV_DEBUG_I2C("Failed to set SCL low after ACK\n");
		ret = -EREMOTEIO;
	}
	return ret;
}


static int ivtv_sendbyte(struct ivtv *itv, unsigned char byte)
{
	int i, bit;

	IVTV_DEBUG_HI_I2C("write %x\n",byte);
	for (i = 0; i < 8; ++i, byte<<=1) {
		ivtv_setscl(itv, 0);
		if (!ivtv_waitscl(itv, 0)) {
			IVTV_DEBUG_I2C("Error setting SCL low\n");
			return -EREMOTEIO;
		}
		bit = (byte>>7)&1;
		ivtv_setsda(itv, bit);
		if (!ivtv_waitsda(itv, bit)) {
			IVTV_DEBUG_I2C("Error setting SDA\n");
			return -EREMOTEIO;
		}
		ivtv_setscl(itv, 1);
		if (!ivtv_waitscl(itv, 1)) {
			IVTV_DEBUG_I2C("Slave not ready for bit\n");
			return -EREMOTEIO;
		}
	}
	ivtv_setscl(itv, 0);
	if (!ivtv_waitscl(itv, 0)) {
		IVTV_DEBUG_I2C("Error setting SCL low\n");
		return -EREMOTEIO;
	}
	return ivtv_ack(itv);
}


static int ivtv_readbyte(struct ivtv *itv, unsigned char *byte, int nack)
{
	int i;

	*byte = 0;

	ivtv_setsda(itv, 1);
	ivtv_scldelay(itv);
	for (i = 0; i < 8; ++i) {
		ivtv_setscl(itv, 0);
		ivtv_scldelay(itv);
		ivtv_setscl(itv, 1);
		if (!ivtv_waitscl(itv, 1)) {
			IVTV_DEBUG_I2C("Error setting SCL high\n");
			return -EREMOTEIO;
		}
		*byte = ((*byte)<<1)|ivtv_getsda(itv);
	}
	ivtv_setscl(itv, 0);
	ivtv_scldelay(itv);
	ivtv_setsda(itv, nack);
	ivtv_scldelay(itv);
	ivtv_setscl(itv, 1);
	ivtv_scldelay(itv);
	ivtv_setscl(itv, 0);
	ivtv_scldelay(itv);
	IVTV_DEBUG_HI_I2C("read %x\n",*byte);
	return 0;
}


static int ivtv_start(struct ivtv *itv)
{
	int sda;

	sda = ivtv_getsda(itv);
	if (sda != 1) {
		IVTV_DEBUG_HI_I2C("SDA was low at start\n");
		ivtv_setsda(itv, 1);
		if (!ivtv_waitsda(itv, 1)) {
			IVTV_DEBUG_I2C("SDA stuck low\n");
			return -EREMOTEIO;
		}
	}
	if (ivtv_getscl(itv) != 1) {
		ivtv_setscl(itv, 1);
		if (!ivtv_waitscl(itv, 1)) {
			IVTV_DEBUG_I2C("SCL stuck low at start\n");
			return -EREMOTEIO;
		}
	}
	ivtv_setsda(itv, 0);
	ivtv_scldelay(itv);
	return 0;
}


static int ivtv_stop(struct ivtv *itv)
{
	int i;

	if (ivtv_getscl(itv) != 0) {
		IVTV_DEBUG_HI_I2C("SCL not low when stopping\n");
		ivtv_setscl(itv, 0);
		if (!ivtv_waitscl(itv, 0)) {
			IVTV_DEBUG_I2C("SCL could not be set low\n");
		}
	}
	ivtv_setsda(itv, 0);
	ivtv_scldelay(itv);
	ivtv_setscl(itv, 1);
	if (!ivtv_waitscl(itv, 1)) {
		IVTV_DEBUG_I2C("SCL could not be set high\n");
		return -EREMOTEIO;
	}
	ivtv_scldelay(itv);
	ivtv_setsda(itv, 1);
	if (!ivtv_waitsda(itv, 1)) {
		IVTV_DEBUG_I2C("resetting I2C\n");
		for (i = 0; i < 16; ++i) {
			ivtv_setscl(itv, 0);
			ivtv_scldelay(itv);
			ivtv_setscl(itv, 1);
			ivtv_scldelay(itv);
			ivtv_setsda(itv, 1);
		}
		ivtv_waitsda(itv, 1);
		return -EREMOTEIO;
	}
	return 0;
}


static int ivtv_write(struct ivtv *itv, unsigned char addr, unsigned char *data, u32 len, int do_stop)
{
	int retry, ret = -EREMOTEIO;
	u32 i;

	for (retry = 0; ret != 0 && retry < 8; ++retry) {
		ret = ivtv_start(itv);

		if (ret == 0) {
			ret = ivtv_sendbyte(itv, addr<<1);
			for (i = 0; ret == 0 && i < len; ++i)
				ret = ivtv_sendbyte(itv, data[i]);
		}
		if (ret != 0 || do_stop) {
			ivtv_stop(itv);
		}
	}
	if (ret)
		IVTV_DEBUG_I2C("i2c write to %x failed\n", addr);
	return ret;
}


static int ivtv_read(struct ivtv *itv, unsigned char addr, unsigned char *data, u32 len)
{
	int retry, ret = -EREMOTEIO;
	u32 i;

	for (retry = 0; ret != 0 && retry < 8; ++retry) {
		ret = ivtv_start(itv);
		if (ret == 0)
			ret = ivtv_sendbyte(itv, (addr << 1) | 1);
		for (i = 0; ret == 0 && i < len; ++i) {
			ret = ivtv_readbyte(itv, &data[i], i == len - 1);
		}
		ivtv_stop(itv);
	}
	if (ret)
		IVTV_DEBUG_I2C("i2c read from %x failed\n", addr);
	return ret;
}


static int ivtv_xfer(struct i2c_adapter *i2c_adap, struct i2c_msg *msgs, int num)
{
	struct v4l2_device *v4l2_dev = i2c_get_adapdata(i2c_adap);
	struct ivtv *itv = to_ivtv(v4l2_dev);
	int retval;
	int i;

	mutex_lock(&itv->i2c_bus_lock);
	for (i = retval = 0; retval == 0 && i < num; i++) {
		if (msgs[i].flags & I2C_M_RD)
			retval = ivtv_read(itv, msgs[i].addr, msgs[i].buf, msgs[i].len);
		else {
			
			int stop = !(i + 1 < num && msgs[i + 1].flags == I2C_M_RD);

			retval = ivtv_write(itv, msgs[i].addr, msgs[i].buf, msgs[i].len, stop);
		}
	}
	mutex_unlock(&itv->i2c_bus_lock);
	return retval ? retval : num;
}


static u32 ivtv_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm ivtv_algo = {
	.master_xfer   = ivtv_xfer,
	.functionality = ivtv_functionality,
};


static struct i2c_adapter ivtv_i2c_adap_hw_template = {
	.name = "ivtv i2c driver",
	.algo = &ivtv_algo,
	.algo_data = NULL,			
	.owner = THIS_MODULE,
};

static void ivtv_setscl_old(void *data, int state)
{
	struct ivtv *itv = (struct ivtv *)data;

	if (state)
		itv->i2c_state |= 0x01;
	else
		itv->i2c_state &= ~0x01;

	
	
	write_reg(~itv->i2c_state, IVTV_REG_I2C_SETSCL_OFFSET);
}

static void ivtv_setsda_old(void *data, int state)
{
	struct ivtv *itv = (struct ivtv *)data;

	if (state)
		itv->i2c_state |= 0x01;
	else
		itv->i2c_state &= ~0x01;

	
	
	write_reg(~itv->i2c_state, IVTV_REG_I2C_SETSDA_OFFSET);
}

static int ivtv_getscl_old(void *data)
{
	struct ivtv *itv = (struct ivtv *)data;

	return read_reg(IVTV_REG_I2C_GETSCL_OFFSET) & 1;
}

static int ivtv_getsda_old(void *data)
{
	struct ivtv *itv = (struct ivtv *)data;

	return read_reg(IVTV_REG_I2C_GETSDA_OFFSET) & 1;
}


static struct i2c_adapter ivtv_i2c_adap_template = {
	.name = "ivtv i2c driver",
	.algo = NULL,                   
	.algo_data = NULL,              
	.owner = THIS_MODULE,
};

static const struct i2c_algo_bit_data ivtv_i2c_algo_template = {
	.setsda		= ivtv_setsda_old,
	.setscl		= ivtv_setscl_old,
	.getsda		= ivtv_getsda_old,
	.getscl		= ivtv_getscl_old,
	.udelay		= 10,
	.timeout	= 200,
};

static struct i2c_client ivtv_i2c_client_template = {
	.name = "ivtv internal",
};


int init_ivtv_i2c(struct ivtv *itv)
{
	int retval;

	IVTV_DEBUG_I2C("i2c init\n");

	
	if (ARRAY_SIZE(hw_devicenames) != ARRAY_SIZE(hw_addrs) ||
	    ARRAY_SIZE(hw_devicenames) != ARRAY_SIZE(hw_modules) ||
	    IVTV_HW_GPIO != (1 << (ARRAY_SIZE(hw_addrs) - 1))) {
		IVTV_ERR("Mismatched I2C hardware arrays\n");
		return -ENODEV;
	}
	if (itv->options.newi2c > 0) {
		memcpy(&itv->i2c_adap, &ivtv_i2c_adap_hw_template,
		       sizeof(struct i2c_adapter));
	} else {
		memcpy(&itv->i2c_adap, &ivtv_i2c_adap_template,
		       sizeof(struct i2c_adapter));
		memcpy(&itv->i2c_algo, &ivtv_i2c_algo_template,
		       sizeof(struct i2c_algo_bit_data));
	}
	itv->i2c_algo.data = itv;
	itv->i2c_adap.algo_data = &itv->i2c_algo;

	sprintf(itv->i2c_adap.name + strlen(itv->i2c_adap.name), " #%d",
		itv->instance);
	i2c_set_adapdata(&itv->i2c_adap, &itv->v4l2_dev);

	memcpy(&itv->i2c_client, &ivtv_i2c_client_template,
	       sizeof(struct i2c_client));
	itv->i2c_client.adapter = &itv->i2c_adap;
	itv->i2c_adap.dev.parent = &itv->pdev->dev;

	IVTV_DEBUG_I2C("setting scl and sda to 1\n");
	ivtv_setscl(itv, 1);
	ivtv_setsda(itv, 1);

	if (itv->options.newi2c > 0)
		retval = i2c_add_adapter(&itv->i2c_adap);
	else
		retval = i2c_bit_add_bus(&itv->i2c_adap);

	
	if (retval == 0) {
		struct i2c_board_info info;
		
		const unsigned short addr_list[] = {
			0x1a,	
			0x18,	
			0x71,	
			0x64,	
			0x30,	
			0x6b,	
			I2C_CLIENT_END
		};

		memset(&info, 0, sizeof(struct i2c_board_info));
		strlcpy(info.type, "ir_video", I2C_NAME_SIZE);
		i2c_new_probed_device(&itv->i2c_adap, &info, addr_list);
	}

	return retval;
}

void exit_ivtv_i2c(struct ivtv *itv)
{
	IVTV_DEBUG_I2C("i2c exit\n");

	i2c_del_adapter(&itv->i2c_adap);
}
