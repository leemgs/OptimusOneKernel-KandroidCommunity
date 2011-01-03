

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/i2c.h>

#include <linux/gpio.h>
#include <linux/delay.h>


#include <linux/broadcast/broadcast_lg2102.h>
#include <linux/broadcast/broadcast_tdmb.h>
#include <linux/broadcast/broadcast_lg2102_ioctrl.h>
#include <linux/broadcast/broadcast_lg2102_includes.h>

#define	MS_DIVIDE_UNIT		((int)100)





#define DMB_POWER_GPIO    124
#define DMB_RESET_GPIO    100
#define DMB_INT_GPIO       99

static uint32 user_stop_flg = 0;
static uint32 mdelay_in_flg = 0;


struct TDMB_LG2102_CTRL
{
	struct i2c_client*		pClient;

};

static broadcast_pwr_func pwr_func;

static struct TDMB_LG2102_CTRL TdmbCtrlInfo;

struct i2c_client* INC_GET_I2C_DRIVER(void)
{
	return TdmbCtrlInfo.pClient;
}

void LGD_RW_TEST(void);


void tdmb_lg2102_set_userstop(void)
{
	user_stop_flg = ((mdelay_in_flg == 1)? 1: 0 );
}


int tdmb_lg2102_mdelay(int32 ms)
{
	long 		wait_loop =0;
	long 		wait_ms = ms;
	int 		rc = 1;

	
	if(ms > MS_DIVIDE_UNIT)
	{
		wait_loop = (ms /MS_DIVIDE_UNIT);
		wait_ms = MS_DIVIDE_UNIT;
	}

	mdelay_in_flg = 1;
	do
	{
		msleep(wait_ms);
		if(user_stop_flg == 1)
		{
			printk("~~~~~~~~ Ustop flag is set so return false ~~~~~~~~\n");
			rc = 0;
			break;
		}
	}while((--wait_loop) > 0);
	mdelay_in_flg = 0;
	user_stop_flg = 0;

	return rc;
}

int tdmb_lg2102_power_on(void)
{
	int rc = TRUE;

	gpio_set_value(DMB_RESET_GPIO, 0);
	gpio_set_value(DMB_POWER_GPIO, 1);
	
	mdelay(2); 
	gpio_set_value(DMB_RESET_GPIO, 1);
	
	LGD_CMD_WRITE(TDMB_RFBB_DEV_ADDR, APB_SPI_BASE+ 0x00, 0x0011);  

	udelay(100);
	gpio_set_value(DMB_RESET_GPIO, 0);
	
	mdelay(1);
	gpio_set_value(DMB_RESET_GPIO, 1);
	
	LGD_CMD_WRITE(TDMB_RFBB_DEV_ADDR, APB_SPI_BASE+ 0x00, 0x0011);	
	udelay(10);


	return rc;
}

int tdmb_lg2102_power_off(void)
{
	int rc = TRUE;
	gpio_set_value(DMB_POWER_GPIO, 0);
	gpio_set_value(DMB_RESET_GPIO, 0);
	udelay(10);
	
	return rc;
}
 static int tdmb_lg2102_i2c_write(uint8* txdata, int length)
{
	struct i2c_msg msg = 
	{	
		TdmbCtrlInfo.pClient->addr,
		0,
		length,
		txdata 
	};


	if (i2c_transfer( TdmbCtrlInfo.pClient->adapter, &msg, 1) < 0) 
	{
		printk("tdmb lg2102 i2c write failed\n");
		return FALSE;
	}

	
	
	return TRUE;
}
 
int tdmb_lg2102_i2c_write_burst(uint16 waddr, uint8* wdata, int length)
{
 	uint8 *buf;
	int	wlen;

	int rc;

	wlen = length + 2;

	buf = (uint8*)kmalloc( wlen, GFP_KERNEL);

	if((buf == NULL) || ( length <= 0 ))
	{
		printk("tdmb_lg2102_i2c_write_burst buf alloc failed\n");
		return FALSE;
	}

	buf[0] = (waddr>>8)&0xFF;
	buf[1] = (waddr&0xFF);

	memcpy(&buf[2], wdata, length);
 
	rc = tdmb_lg2102_i2c_write(buf, wlen);

	kfree(buf);
		
	return rc;
}

static int tdmb_lg2102_i2c_read( uint16 raddr,	uint8 *rxdata, int length)
{
	uint8	r_addr[2] = {raddr>>8, raddr&0xff};
	
	
	struct i2c_msg msgs[] = 
	{
		{
			.addr   = TdmbCtrlInfo.pClient->addr,
			.flags = 0,
			.len   = 2,
			.buf   = &r_addr[0],
		},
		{
			.addr   = TdmbCtrlInfo.pClient->addr,
			.flags = I2C_M_RD,
			.len   = length,
			.buf   = rxdata,
		},
	};
	
	

	if (i2c_transfer(TdmbCtrlInfo.pClient->adapter, msgs, 2) < 0) 
	{
		printk("tdmb_lg2102_i2c_read failed! %x \n",TdmbCtrlInfo.pClient->addr);
		return FALSE;
	}
	
	

	
	
	
	return TRUE;
}

int tdmb_lg2102_i2c_read_burst(uint16 raddr, uint8* rdata, int length)
{
	int rc;
 
	rc = tdmb_lg2102_i2c_read(raddr, rdata, length);

	return rc;

}


int tdmb_lg2102_i2c_write16(unsigned short reg, unsigned short val)
{
	unsigned int err;
	unsigned char buf[4] = { reg>>8, reg&0xff, val>>8, val&0xff };
	struct i2c_msg	msg = 
	{	
		TdmbCtrlInfo.pClient->addr,
		0,
		4,
		buf 
	};
	
	if ((err = i2c_transfer( TdmbCtrlInfo.pClient->adapter, &msg, 1)) < 0) 
	{
		dev_err(&TdmbCtrlInfo.pClient->dev, "i2c write error\n");
		err = FALSE;
	}
	else
	{
		
		err = TRUE;
	}

	return err;
}


int tdmb_lg2102_i2c_read16(uint16 reg, uint16 *ret)
{

	uint32 err;
	uint8 w_buf[2] = {reg>>8, reg&0xff};	
	uint8 r_buf[2] = {0,0};

	struct i2c_msg msgs[2] = 
	{
		{ TdmbCtrlInfo.pClient->addr, 0, 2, &w_buf[0] },
		{ TdmbCtrlInfo.pClient->addr, I2C_M_RD, 2, &r_buf[0]}
	};

	if ((err = i2c_transfer(TdmbCtrlInfo.pClient->adapter, msgs, 2)) < 0) 
	{
		dev_err(&TdmbCtrlInfo.pClient->dev, "i2c read error\n");
		err = FALSE;
	}
	else
	{
		
		*ret = r_buf[0]<<8 | r_buf[1];
		
		err = TRUE;
	}

	return err;
}


void LGD_RW_TEST(void)
{
	unsigned short i = 0;
	unsigned short w_val = 0;
	unsigned short r_val = 0;
	unsigned short err_cnt = 0;

	err_cnt = 0;
	for(i=1;i<11;i++)
	{
		w_val = (i%0xFF);
		tdmb_lg2102_i2c_write16( 0x0a00+ 0x05, w_val);
		tdmb_lg2102_i2c_read16(0x0a00+ 0x05, &r_val );
		if(r_val != w_val)
		{
			err_cnt++;
			printk("w_val:%x, r_val:%x\n", w_val,r_val);
		}
	}
}



static int tdmb_lg2102_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int nRet;
	struct broadcast_tdmb_data *pdata;

	memset(&TdmbCtrlInfo, 0x00, sizeof(struct TDMB_LG2102_CTRL));

	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_ERR "tdmb_lg2102_i2c_probe: need I2C_FUNC_I2C\n");
		nRet = -ENODEV;
		return nRet;
	}

	TdmbCtrlInfo.pClient 	= client;
	
	
	i2c_set_clientdata(client, (void*)&TdmbCtrlInfo);

	pdata = client->dev.platform_data;

	pwr_func.tdmb_pwr_on = pdata->pwr_on;
	pwr_func.tdmb_pwr_off = pdata->pwr_off;

	printk("broadcast_tdmb_lg2102_i2c_probe start \n");
	
	return TRUE;
}
#if 0
static int broadcast_tdmb_lg2102_probe(struct platform_device *pdev)
{
	int rc;

	printk("broadcast_tdmb_lg2102_probe start \n");

	return rc;

}


static struct platform_driver broadcast_tdmb_driver = {
	.probe = broadcast_tdmb_lg2102_probe,
	
	.driver = {
		.name = TDMB_C_NAME,
		.owner = THIS_MODULE,
	},
};



	
}


#endif
