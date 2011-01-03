

#include "cx18-driver.h"
#include "cx18-io.h"
#include "cx18-cards.h"
#include "cx18-gpio.h"
#include "cx18-i2c.h"
#include "cx18-irq.h"
#include <media/ir-kbd-i2c.h>

#define CX18_REG_I2C_1_WR   0xf15000
#define CX18_REG_I2C_1_RD   0xf15008
#define CX18_REG_I2C_2_WR   0xf25100
#define CX18_REG_I2C_2_RD   0xf25108

#define SETSCL_BIT      0x0001
#define SETSDL_BIT      0x0002
#define GETSCL_BIT      0x0004
#define GETSDL_BIT      0x0008

#define CX18_CS5345_I2C_ADDR		0x4c
#define CX18_Z8F0811_IR_TX_I2C_ADDR	0x70
#define CX18_Z8F0811_IR_RX_I2C_ADDR	0x71


static const u8 hw_addrs[] = {
	0,				
	0,				
	CX18_CS5345_I2C_ADDR,		
	0,				
	0,				
	0,				
	0,				
	CX18_Z8F0811_IR_TX_I2C_ADDR,	
	CX18_Z8F0811_IR_RX_I2C_ADDR,	
};



static const u8 hw_bus[] = {
	1,	
	0,	
	0,	
	0,	
	0,	
	0,	
	0,	
	0,	
	0,	
};


static const char * const hw_modules[] = {
	"tuner",	
	NULL,		
	"cs5345",	
	NULL,		
	NULL,		
	NULL,		
	NULL,		
	NULL,		
	NULL,		
};


static const char * const hw_devicenames[] = {
	"tuner",
	"tveeprom",
	"cs5345",
	"cx23418_DTV",
	"cx23418_AV",
	"gpio_mux",
	"gpio_reset_ctrl",
	"ir_tx_z8f0811_haup",
	"ir_rx_z8f0811_haup",
};

static const struct IR_i2c_init_data z8f0811_ir_init_data = {
	.ir_codes = &ir_codes_hauppauge_new_table,
	.internal_get_key_func = IR_KBD_GET_KEY_HAUP_XVR,
	.type = IR_TYPE_RC5,
	.name = "CX23418 Z8F0811 Hauppauge",
};

static int cx18_i2c_new_ir(struct i2c_adapter *adap, u32 hw, const char *type,
			   u8 addr)
{
	struct i2c_board_info info;
	unsigned short addr_list[2] = { addr, I2C_CLIENT_END };

	memset(&info, 0, sizeof(struct i2c_board_info));
	strlcpy(info.type, type, I2C_NAME_SIZE);

	
	switch (hw) {
	case CX18_HW_Z8F0811_IR_RX_HAUP:
		info.platform_data = (void *) &z8f0811_ir_init_data;
		break;
	default:
		break;
	}

	return i2c_new_probed_device(adap, &info, addr_list) == NULL ? -1 : 0;
}

int cx18_i2c_register(struct cx18 *cx, unsigned idx)
{
	struct v4l2_subdev *sd;
	int bus = hw_bus[idx];
	struct i2c_adapter *adap = &cx->i2c_adap[bus];
	const char *mod = hw_modules[idx];
	const char *type = hw_devicenames[idx];
	u32 hw = 1 << idx;

	if (idx >= ARRAY_SIZE(hw_addrs))
		return -1;

	if (hw == CX18_HW_TUNER) {
		
		sd = v4l2_i2c_new_subdev(&cx->v4l2_dev,
				adap, mod, type, 0, cx->card_i2c->radio);
		if (sd != NULL)
			sd->grp_id = hw;
		sd = v4l2_i2c_new_subdev(&cx->v4l2_dev,
				adap, mod, type, 0, cx->card_i2c->demod);
		if (sd != NULL)
			sd->grp_id = hw;
		sd = v4l2_i2c_new_subdev(&cx->v4l2_dev,
				adap, mod, type, 0, cx->card_i2c->tv);
		if (sd != NULL)
			sd->grp_id = hw;
		return sd != NULL ? 0 : -1;
	}

	if (hw & CX18_HW_Z8F0811_IR_HAUP)
		return cx18_i2c_new_ir(adap, hw, type, hw_addrs[idx]);

	
	if (!hw_addrs[idx])
		return -1;

	
	sd = v4l2_i2c_new_subdev(&cx->v4l2_dev, adap, mod, type, hw_addrs[idx], NULL);
	if (sd != NULL)
		sd->grp_id = hw;
	return sd != NULL ? 0 : -1;
}


struct v4l2_subdev *cx18_find_hw(struct cx18 *cx, u32 hw)
{
	struct v4l2_subdev *result = NULL;
	struct v4l2_subdev *sd;

	spin_lock(&cx->v4l2_dev.lock);
	v4l2_device_for_each_subdev(sd, &cx->v4l2_dev) {
		if (sd->grp_id == hw) {
			result = sd;
			break;
		}
	}
	spin_unlock(&cx->v4l2_dev.lock);
	return result;
}

static void cx18_setscl(void *data, int state)
{
	struct cx18 *cx = ((struct cx18_i2c_algo_callback_data *)data)->cx;
	int bus_index = ((struct cx18_i2c_algo_callback_data *)data)->bus_index;
	u32 addr = bus_index ? CX18_REG_I2C_2_WR : CX18_REG_I2C_1_WR;
	u32 r = cx18_read_reg(cx, addr);

	if (state)
		cx18_write_reg(cx, r | SETSCL_BIT, addr);
	else
		cx18_write_reg(cx, r & ~SETSCL_BIT, addr);
}

static void cx18_setsda(void *data, int state)
{
	struct cx18 *cx = ((struct cx18_i2c_algo_callback_data *)data)->cx;
	int bus_index = ((struct cx18_i2c_algo_callback_data *)data)->bus_index;
	u32 addr = bus_index ? CX18_REG_I2C_2_WR : CX18_REG_I2C_1_WR;
	u32 r = cx18_read_reg(cx, addr);

	if (state)
		cx18_write_reg(cx, r | SETSDL_BIT, addr);
	else
		cx18_write_reg(cx, r & ~SETSDL_BIT, addr);
}

static int cx18_getscl(void *data)
{
	struct cx18 *cx = ((struct cx18_i2c_algo_callback_data *)data)->cx;
	int bus_index = ((struct cx18_i2c_algo_callback_data *)data)->bus_index;
	u32 addr = bus_index ? CX18_REG_I2C_2_RD : CX18_REG_I2C_1_RD;

	return cx18_read_reg(cx, addr) & GETSCL_BIT;
}

static int cx18_getsda(void *data)
{
	struct cx18 *cx = ((struct cx18_i2c_algo_callback_data *)data)->cx;
	int bus_index = ((struct cx18_i2c_algo_callback_data *)data)->bus_index;
	u32 addr = bus_index ? CX18_REG_I2C_2_RD : CX18_REG_I2C_1_RD;

	return cx18_read_reg(cx, addr) & GETSDL_BIT;
}


static struct i2c_adapter cx18_i2c_adap_template = {
	.name = "cx18 i2c driver",
	.algo = NULL,                   
	.algo_data = NULL,              
	.owner = THIS_MODULE,
};

#define CX18_SCL_PERIOD (10) 
#define CX18_ALGO_BIT_TIMEOUT (2) 

static struct i2c_algo_bit_data cx18_i2c_algo_template = {
	.setsda		= cx18_setsda,
	.setscl		= cx18_setscl,
	.getsda		= cx18_getsda,
	.getscl		= cx18_getscl,
	.udelay		= CX18_SCL_PERIOD/2,       
	.timeout	= CX18_ALGO_BIT_TIMEOUT*HZ 
};


int init_cx18_i2c(struct cx18 *cx)
{
	int i, err;
	CX18_DEBUG_I2C("i2c init\n");

	for (i = 0; i < 2; i++) {
		
		memcpy(&cx->i2c_algo[i], &cx18_i2c_algo_template,
			sizeof(struct i2c_algo_bit_data));
		cx->i2c_algo_cb_data[i].cx = cx;
		cx->i2c_algo_cb_data[i].bus_index = i;
		cx->i2c_algo[i].data = &cx->i2c_algo_cb_data[i];

		
		memcpy(&cx->i2c_adap[i], &cx18_i2c_adap_template,
			sizeof(struct i2c_adapter));
		cx->i2c_adap[i].algo_data = &cx->i2c_algo[i];
		sprintf(cx->i2c_adap[i].name + strlen(cx->i2c_adap[i].name),
				" #%d-%d", cx->instance, i);
		i2c_set_adapdata(&cx->i2c_adap[i], &cx->v4l2_dev);
		cx->i2c_adap[i].dev.parent = &cx->pci_dev->dev;
	}

	if (cx18_read_reg(cx, CX18_REG_I2C_2_WR) != 0x0003c02f) {
		
		
		cx18_write_reg_expect(cx, 0x10000000, 0xc71004,
					  0x00000000, 0x10001000);
		
		cx18_write_reg_expect(cx, 0x10001000, 0xc71024,
					  0x00001000, 0x10001000);
	}
	
	cx18_write_reg_expect(cx, 0x00c00000, 0xc7001c, 0x00000000, 0x00c000c0);
	mdelay(10);
	cx18_write_reg_expect(cx, 0x00c000c0, 0xc7001c, 0x000000c0, 0x00c000c0);
	mdelay(10);
	cx18_write_reg_expect(cx, 0x00c00000, 0xc7001c, 0x00000000, 0x00c000c0);
	mdelay(10);

	
	cx18_write_reg(cx, 0x00c00000, 0xc730c8);
	
	cx18_write_reg_expect(cx, HW2_I2C1_INT|HW2_I2C2_INT, HW2_INT_CLR_STATUS,
		       ~(HW2_I2C1_INT|HW2_I2C2_INT), HW2_I2C1_INT|HW2_I2C2_INT);

	
	cx18_write_reg(cx, 0x00021c0f & ~4, CX18_REG_I2C_1_WR);
	cx18_setscl(&cx->i2c_algo_cb_data[0], 1);
	cx18_setsda(&cx->i2c_algo_cb_data[0], 1);

	
	cx18_write_reg(cx, 0x00021c0f & ~4, CX18_REG_I2C_2_WR);
	cx18_setscl(&cx->i2c_algo_cb_data[1], 1);
	cx18_setsda(&cx->i2c_algo_cb_data[1], 1);

	cx18_call_hw(cx, CX18_HW_GPIO_RESET_CTRL,
		     core, reset, (u32) CX18_GPIO_RESET_I2C);

	err = i2c_bit_add_bus(&cx->i2c_adap[0]);
	if (err)
		goto err;
	err = i2c_bit_add_bus(&cx->i2c_adap[1]);
	if (err)
		goto err_del_bus_0;
	return 0;

 err_del_bus_0:
	i2c_del_adapter(&cx->i2c_adap[0]);
 err:
	return err;
}

void exit_cx18_i2c(struct cx18 *cx)
{
	int i;
	CX18_DEBUG_I2C("i2c exit\n");
	cx18_write_reg(cx, cx18_read_reg(cx, CX18_REG_I2C_1_WR) | 4,
							CX18_REG_I2C_1_WR);
	cx18_write_reg(cx, cx18_read_reg(cx, CX18_REG_I2C_2_WR) | 4,
							CX18_REG_I2C_2_WR);

	for (i = 0; i < 2; i++) {
		i2c_del_adapter(&cx->i2c_adap[i]);
	}
}


