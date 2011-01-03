

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/freezer.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/clk.h>
#include <linux/wakelock.h>
#include <net/sock.h>
#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <mach/msm_iomap.h>
#include <mach/msm_rpcrouter.h>
#include <mach/vreg.h>
#include <mach/board.h>
#include <mach/clk.h>
#include <linux/mt9t013.h> 


#define ALLOW_USPACE_RW		0

static const uint32_t fps_divider = 1;

#define AF_I2C_ID	0x18  

static struct i2c_client *pclient;


static struct clk *vfe_clk;


static struct clk *vfe_mdc_clk;
static struct clk *mdc_clk;

static int mdc_clk_enabled;
static int vfe_mdc_clk_enabled;
static int vfe_clk_enabled;
static int opened;
static int pclk_set;

static const struct mt9t013_reg_pat mt9t013_reg_pattern = { .reg = {
	{ 
		10,	
			
		1,	
			
		3,	
			
		80,	
			
		10,	
		1,	
		16,	
		0x0111,	
		8,	
		2053,	
		8,	
		1541,	
		0x046C,	
		1024,	
		768,	
		3540,	
		861,	
		16,	
		1461	
	},
	{ 
		10,	
			
		1,	
			
		3,	
			
		80,	
			
		10,	
		1,	
		16,	
		0x0111,	
		8,	
		2063,	
		8,	
		1551,	
		0x0024,	
		2063,	
		1544,	
		4800,	
		1629,	
		16,	
		733	
	}
}};

#define MT9T013_MU3M0VC_REG_MODEL_ID		0x0000
#define MT9T013_MU3M0VC_MODEL_ID		0x2600
#define REG_GROUPED_PARAMETER_HOLD		0x0104
#define GROUPED_PARAMETER_HOLD			0x0100
#define GROUPED_PARAMETER_UPDATE		0x0000
#define REG_COARSE_INTEGRATION_TIME		0x3012
#define REG_VT_PIX_CLK_DIV			0x0300
#define REG_VT_SYS_CLK_DIV			0x0302
#define REG_PRE_PLL_CLK_DIV			0x0304
#define REG_PLL_MULTIPLIER			0x0306
#define REG_OP_PIX_CLK_DIV			0x0308
#define REG_OP_SYS_CLK_DIV			0x030A
#define REG_SCALE_M				0x0404
#define REG_FRAME_LENGTH_LINES			0x300A
#define REG_LINE_LENGTH_PCK			0x300C
#define REG_X_ADDR_START			0x3004
#define REG_Y_ADDR_START			0x3002
#define REG_X_ADDR_END				0x3008
#define REG_Y_ADDR_END				0x3006
#define REG_X_OUTPUT_SIZE			0x034C
#define REG_Y_OUTPUT_SIZE			0x034E
#define REG_FINE_INTEGRATION_TIME		0x3014
#define REG_ROW_SPEED				0x3016
#define MT9T013_REG_RESET_REGISTER		0x301A
#define MT9T013_RESET_REGISTER_PWON		0x10CC   
#define MT9T013_RESET_REGISTER_PWOFF		0x1008 
#define REG_READ_MODE				0x3040
#define REG_GLOBAL_GAIN				0x305E
#define REG_TEST_PATTERN_MODE			0x3070

static struct wake_lock mt9t013_wake_lock;

static inline void init_suspend(void)
{
	wake_lock_init(&mt9t013_wake_lock, WAKE_LOCK_IDLE, "mt9t013");
}

static inline void deinit_suspend(void)
{
	wake_lock_destroy(&mt9t013_wake_lock);
}

static inline void prevent_suspend(void)
{
	wake_lock(&mt9t013_wake_lock);
}

static inline void allow_suspend(void)
{
	wake_unlock(&mt9t013_wake_lock);
}

#define CLK_GET(clk) do {						\
	if (!clk) {							\
		clk = clk_get(NULL, #clk);				\
		printk(KERN_INFO 					\
			"mt9t013: clk_get(%s): %p\n", #clk, clk);	\
	}								\
} while(0)

DECLARE_MUTEX(sem);

static struct msm_camera_legacy_device_platform_data  *cam;

#define out_dword(addr, val) \
	(*((volatile unsigned long  *)(addr)) = ((unsigned long)(val)))

#define out_dword_masked_ns(io, mask, val, current_reg_content)	    \
  (void) out_dword(io, ((current_reg_content & (uint32_t)(~(mask))) | \
			 ((uint32_t)((val) & (mask)))))

#define __inpdw(port) (*((volatile uint32_t *) (port)))
#define in_dword_masked(addr, mask) (__inpdw(addr) & (uint32_t)mask )

#define HWIO_MDDI_CAMIF_CFG_ADDR MSM_MDC_BASE
#define HWIO_MDDI_CAMIF_CFG_RMSK 0x1fffff
#define HWIO_MDDI_CAMIF_CFG_IN \
  in_dword_masked(HWIO_MDDI_CAMIF_CFG_ADDR, HWIO_MDDI_CAMIF_CFG_RMSK)

#define HWIO_MDDI_CAMIF_CFG_OUTM(m,v) \
  out_dword_masked_ns(HWIO_MDDI_CAMIF_CFG_ADDR,m,v,HWIO_MDDI_CAMIF_CFG_IN);
#define __msmhwio_outm(hwiosym, mask, val) HWIO_##hwiosym##_OUTM(mask, val)
#define HWIO_OUTM(hwiosym, mask, val) __msmhwio_outm(hwiosym, mask, val)

#define HWIO_MDDI_CAMIF_CFG_CAM_SEL_BMSK 0x2
#define HWIO_MDDI_CAMIF_CFG_CAM_PCLK_SRC_SEL_BMSK 0x60000
#define HWIO_MDDI_CAMIF_CFG_CAM_PCLK_INVERT_BMSK 0x80000
#define HWIO_MDDI_CAMIF_CFG_CAM_PAD_REG_SW_RESET_BMSK 0x100000

#define HWIO_MDDI_CAMIF_CFG_CAM_SEL_SHFT 0x1
#define HWIO_MDDI_CAMIF_CFG_CAM_PCLK_SRC_SEL_SHFT 0x11
#define HWIO_MDDI_CAMIF_CFG_CAM_PCLK_INVERT_SHFT 0x13
#define HWIO_MDDI_CAMIF_CFG_CAM_PAD_REG_SW_RESET_SHFT 0x14

#define __msmhwio_shft(hwio_regsym, hwio_fldsym) HWIO_##hwio_regsym##_##hwio_fldsym##_SHFT
#define HWIO_SHFT(hwio_regsym, hwio_fldsym) __msmhwio_shft(hwio_regsym, hwio_fldsym)

#define __msmhwio_fmsk(hwio_regsym, hwio_fldsym) HWIO_##hwio_regsym##_##hwio_fldsym##_BMSK
#define HWIO_FMSK(hwio_regsym, hwio_fldsym) __msmhwio_fmsk(hwio_regsym, hwio_fldsym)

#define HWIO_APPS_RESET_ADDR (MSM_CLK_CTL_BASE + 0x00000210)
#define HWIO_APPS_RESET_RMSK 0x1fff
#define HWIO_APPS_RESET_VFE_BMSK 1
#define HWIO_APPS_RESET_VFE_SHFT 0 
#define HWIO_APPS_RESET_IN in_dword_masked(HWIO_APPS_RESET_ADDR, HWIO_APPS_RESET_RMSK)
#define HWIO_APPS_RESET_OUTM(m,v) out_dword_masked_ns(HWIO_APPS_RESET_ADDR,m,v,HWIO_APPS_RESET_IN)

struct mt9t013_data {
	struct work_struct work;
};

static DECLARE_WAIT_QUEUE_HEAD(g_data_ready_wait_queue);

static int mt9t013_i2c_sensor_init(struct mt9t013_init *init);
static int mt9t013_i2c_sensor_setting(unsigned long arg);
static int mt9t013_i2c_exposure_gain(uint32_t mode, uint16_t line,
					uint16_t gain);
static int mt9t013_i2c_move_focus(uint16_t position);
static int mt9t013_i2c_set_default_focus(uint8_t step);
static int mt9t013_i2c_power_up(void);
static int mt9t013_i2c_power_down(void);
static int mt9t013_camif_pad_reg_reset(void);
static int mt9t013_lens_power(int on);

int mt_i2c_lens_tx_data(unsigned char slave_addr, char* txData, int length)
{
	int rc;
	struct i2c_msg msg[] = {
		{
			.addr = slave_addr,
			.flags = 0,
			.len = length,
			.buf = txData,		
		},
	};

#if 0
	{
		int i;
		
		for (i = 0; i < length - 2; i++)
			printk(KERN_INFO "\tdata[%d]: 0x%02x\n", i, txData[i+2]);
	}
#endif
    
	rc = i2c_transfer(pclient->adapter, msg, 1);
	if (rc < 0) {
		printk(KERN_ERR "mt_i2c_lens_tx_data: i2c_transfer error %d\n", rc);
		return rc;
	}
	return 0;
}

static int mt9t013_i2c_lens_write(unsigned char slave_addr, unsigned char u_addr, unsigned char u_data)
{
	unsigned char buf[2] = { u_addr, u_data };
	return mt_i2c_lens_tx_data(slave_addr, buf, sizeof(buf));
}

static int mt_i2c_rx_data(char* rxData, int length)
{
	int rc;
	struct i2c_msg msgs[] = {
		{
			.addr = pclient->addr,
			.flags = 0,      
			.len = 2,
			.buf = rxData,
		},
		{
			.addr = pclient->addr,
			.flags = I2C_M_RD,
			.len = length,
			.buf = rxData,
		},
	};

	rc = i2c_transfer(pclient->adapter, msgs, 2);
	if (rc < 0) {
		printk(KERN_ERR "mt9t013: mt_i2c_rx_data error %d\n", rc);
		return rc;
	}
#if 0
	else {
		int i;
		for (i = 0; i < length; i++)
			printk(KERN_INFO "\tdata[%d]: 0x%02x\n", i, rxData[i]);
	}
#endif

	return 0;
}

int mt_i2c_tx_data(char* txData, int length)
{
	int rc; 

	struct i2c_msg msg[] = {
		{
			.addr = pclient->addr,
			.flags = 0,
			.len = length,
			.buf = txData,		
		},
	};
    
	rc = i2c_transfer(pclient->adapter, msg, 1);
	if (rc < 0) {
		printk(KERN_ERR "mt9t013: mt_i2c_tx_data error %d\n", rc);
		return rc;
	}
	return 0;
}

static int mt9t013_i2c_write(unsigned short u_addr, unsigned short u_data)
{
	int rc;
	unsigned char buf[4];

	buf[0] = (u_addr & 0xFF00) >> 8;
	buf[1] = u_addr & 0x00FF;
	buf[2] = (u_data & 0xFF00) >> 8;
	buf[3] = u_data & 0x00FF;

	rc = mt_i2c_tx_data(buf, sizeof(buf));
	if(rc < 0)
		printk(KERN_ERR "mt9t013: txdata error %d add:0x%02x data:0x%02x\n",
			rc, u_addr, u_data);
	return rc;	
}

static int mt9t013_i2c_read(unsigned short u_addr, unsigned short *pu_data)
{
	int rc;
	unsigned char buf[2];

	buf[0] = (u_addr & 0xFF00)>>8;
	buf[1] = (u_addr & 0x00FF);
	rc = mt_i2c_rx_data(buf, 2);
	if (!rc)
		*pu_data = buf[0]<<8 | buf[1];
	else printk(KERN_ERR "mt9t013: i2c read failed\n");
	return rc;	
}

static int msm_camio_clk_enable (int clk_type)
{
	struct clk *clk = NULL;
	int *enabled = NULL;

	switch (clk_type) {
	case CAMIO_VFE_MDC_CLK:
		CLK_GET(vfe_mdc_clk);
		clk = vfe_mdc_clk;
		enabled = &vfe_mdc_clk_enabled;
		break;
	case CAMIO_MDC_CLK:
		CLK_GET(mdc_clk);
		clk = mdc_clk;
		enabled = &mdc_clk_enabled;
		break;
	default:
		break;
	}

	if (clk != NULL && !*enabled) {
		int rc = clk_enable(clk);
		*enabled = !rc;	
		return rc;
	}

	return -EINVAL; 
}

static int msm_camio_clk_disable(int clk_type)
{
	int rc = 0;
	struct clk *clk = NULL;
	int *enabled = NULL;

	switch (clk_type) {
	case CAMIO_VFE_MDC_CLK:
		clk = vfe_mdc_clk;
		enabled = &vfe_mdc_clk_enabled;
		break;
	case CAMIO_MDC_CLK:
		clk = mdc_clk;
		enabled = &mdc_clk_enabled;
		break;
	default:
		rc = -1;
		break;
	}

	if (clk != NULL && *enabled) {
		clk_disable(clk);
		*enabled = 0;
		return 0;
	}

	return -EINVAL;
}

static int msm_camio_vfe_clk_enable(void)
{
	CLK_GET(vfe_clk);
	if (vfe_clk && !vfe_clk_enabled) {
		vfe_clk_enabled = !clk_enable(vfe_clk);
		printk(KERN_INFO "mt9t013: enable vfe_clk\n");
	}
	return vfe_clk_enabled ? 0 : -EIO;
}

static int msm_camio_clk_rate_set(int rate)
{
	int rc = msm_camio_vfe_clk_enable();
	if (!rc && vfe_clk_enabled)
		rc = clk_set_rate(vfe_clk, rate);
	return rc;
}

static int clk_select(int internal)
{
	int rc = -EIO;
	printk(KERN_INFO "mt9t013: clk select %d\n", internal);
	CLK_GET(vfe_clk);
	if (vfe_clk != NULL) {
		rc = clk_set_flags(vfe_clk, 0x00000100 << internal);
		if (!rc && internal) rc = msm_camio_vfe_clk_enable();
	}
	return rc;
}

static void mt9t013_sensor_init(void)
{
	int ret;
	printk(KERN_INFO "mt9t013: init\n");
	if (!pclient)
		return;

	
	printk(KERN_INFO "mt9t013: mt9t013_register_init\n");
	ret = gpio_request(cam->sensor_reset, "mt9t013");
	if (!ret) {
		gpio_direction_output(cam->sensor_reset, 1);
		printk(KERN_INFO "mt9t013: camera sensor_reset set as 1\n");
	} else
		printk(KERN_ERR "mt9t013 error: request gpio %d failed: "
				"%d\n", cam->sensor_reset, ret);
	mdelay(2);

	
	ret = gpio_request(cam->sensor_pwd, "mt9t013");
	if (!ret || ret == -EBUSY)
		gpio_direction_output(cam->sensor_pwd, 0);
	else printk(KERN_ERR "mt913t013 error: request gpio %d failed: "
			"%d\n", cam->sensor_pwd, ret);
	gpio_free(cam->sensor_pwd);

	
	msm_camio_clk_enable(CAMIO_VFE_MDC_CLK);
	msm_camio_clk_enable(CAMIO_MDC_CLK);

	
	mt9t013_camif_pad_reg_reset();

	
	ret = msm_camio_clk_rate_set(24000000);
	if(ret < 0)
		printk(KERN_ERR "camio clk rate select error\n");
	mdelay(2);

	
	cam->config_gpio_on();
	
	mdelay(2);

	
	gpio_direction_output(cam->sensor_reset, 0);
	mdelay(2);
	gpio_direction_output(cam->sensor_reset, 1);
	gpio_free(cam->sensor_reset);
	mdelay(2);

	printk(KERN_INFO "mt9t013: camera sensor init sequence done\n");
}

#define CLK_DISABLE_AND_PUT(clk) do {					\
	if (clk) {							\
		if (clk##_enabled) {					\
			printk(KERN_INFO "mt9t013: disabling "#clk"\n");\
			clk_disable(clk);				\
			clk##_enabled = 0;				\
		}							\
		printk(KERN_INFO 					\
			"mt9t013: clk_put(%s): %p\n", #clk, clk);	\
		clk_put(clk);						\
		clk = NULL; 						\
	}								\
} while(0)

static void mt9t013_sensor_suspend(void)
{
	printk(KERN_INFO "mt9t013: camera sensor suspend sequence\n");
	if (!pclient) {
		return;
	}
	
	msm_camio_clk_disable(CAMIO_VFE_MDC_CLK);
	msm_camio_clk_disable(CAMIO_MDC_CLK);
	CLK_DISABLE_AND_PUT(vfe_clk); 
	
	cam->config_gpio_off();
	printk(KERN_INFO "mt9t013: camera sensor suspend sequence done\n");
}

static int mt9t013_open(struct inode *ip, struct file *fp)
{
	int rc = -EBUSY;
	down(&sem);
	printk(KERN_INFO "mt9t013: open\n");
	if (!opened) {
		printk(KERN_INFO "mt9t013: prevent collapse on idle\n");
		prevent_suspend();
		cam->config_gpio_on();
		opened = 1;
		rc = 0;
	}
	up(&sem);
	return rc;
}

static int mt9t013_release(struct inode *ip, struct file *fp)
{
	int rc = -EBADF;
	printk(KERN_INFO "mt9t013: release\n");
	down(&sem);
	if (opened) {
		printk(KERN_INFO "mt9t013: release clocks\n");


		
		
		mt9t013_i2c_power_down();

		CLK_DISABLE_AND_PUT(mdc_clk);
		CLK_DISABLE_AND_PUT(vfe_mdc_clk);
		CLK_DISABLE_AND_PUT(vfe_clk);
		mt9t013_lens_power(0);

		cam->config_gpio_off();

		printk(KERN_INFO "mt9t013: allow collapse on idle\n");
		allow_suspend();
		rc = pclk_set = opened = 0;
	}
	up(&sem);
	return rc;
}

#undef CLK_DISABLE_AND_PUT

#define CHECK() ({ 							\
	if (!mdc_clk_enabled || !vfe_mdc_clk_enabled) { 		\
		printk(KERN_ERR "mt9t013 error: one or more clocks"	\
			" are NULL.\n"); 				\
		rc = -EIO; 						\
	} 								\
	!rc; })

static int mt9t013_camif_pad_reg_reset(void)
{
	int rc = clk_select(1);
	if(rc < 0) {
		printk(KERN_ERR "mt9t013 error switching to internal clock\n");
		return rc;
	}
	HWIO_OUTM (MDDI_CAMIF_CFG,
		HWIO_FMSK (MDDI_CAMIF_CFG, CAM_SEL) |
		HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PCLK_SRC_SEL) |
		HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PCLK_INVERT),
		1 << HWIO_SHFT (MDDI_CAMIF_CFG, CAM_SEL) |
		3 << HWIO_SHFT (MDDI_CAMIF_CFG, CAM_PCLK_SRC_SEL) |
		0 << HWIO_SHFT (MDDI_CAMIF_CFG, CAM_PCLK_INVERT));
	msleep(10);
	HWIO_OUTM (MDDI_CAMIF_CFG,
		HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PAD_REG_SW_RESET),
		1 << HWIO_SHFT (MDDI_CAMIF_CFG,
		CAM_PAD_REG_SW_RESET));
	msleep(10);
	HWIO_OUTM (MDDI_CAMIF_CFG,
		HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PAD_REG_SW_RESET),
		0 << HWIO_SHFT (MDDI_CAMIF_CFG,
		CAM_PAD_REG_SW_RESET));
	msleep(10);
	rc = clk_select(0); 
	if(rc < 0) {
		printk(KERN_ERR "mt9t013 error switching to external clock\n");
		return rc;
	}

	return rc;
}

#if ALLOW_USPACE_RW
#define COPY_FROM_USER(size) ({                                         \
        if (copy_from_user(rwbuf, argp, size)) rc = -EFAULT;            \
        !rc; })
#endif

static long mt9t013_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int rc = 0;
	
#if ALLOW_USPACE_RW
	unsigned short addr = 0;
	unsigned short data = 0;
	char rwbuf[4];
#endif

	down(&sem);

	switch(cmd) {
#if ALLOW_USPACE_RW
	case MT9T013_I2C_IOCTL_W:
		if ( COPY_FROM_USER(4)) {
			addr = *((unsigned short *)rwbuf);
			data = *((unsigned short *)(rwbuf+2));
			rc = mt9t013_i2c_write(addr, data);
		} else
			printk(KERN_ERR "mt9t013: write: err %d\n", rc);
		break;

	case MT9T013_I2C_IOCTL_R:
		if ( COPY_FROM_USER(4)) {
			addr = *((unsigned short*) rwbuf);
			rc = mt9t013_i2c_read(addr, (unsigned short *)(rwbuf+2));
			if (!rc) {
				if (copy_to_user(argp, rwbuf, 4)) {
					printk(KERN_ERR "mt9t013: read: err " \
							"writeback -EFAULT\n");
					rc = -EFAULT;
				}
			}
		} else
			printk(KERN_ERR "mt9t013: read: err %d\n", rc);
		break;

	case MT9T013_I2C_IOCTL_AF_W:
		if ( COPY_FROM_USER(3))
			rc = mt9t013_i2c_lens_write(*rwbuf, *(rwbuf + 1), *(rwbuf + 2));
		else
			printk(KERN_ERR "mt9t013: af write: err %d\n", rc);
		break;
#endif 

	case MT9T013_I2C_IOCTL_CAMIF_PAD_REG_RESET:
		printk(KERN_INFO "mt9t013: CAMIF_PAD_REG_RESET\n"); 
		if (CHECK())
			rc = mt9t013_camif_pad_reg_reset();
		break;

	case MT9T013_I2C_IOCTL_CAMIF_PAD_REG_RESET_2:
		printk(KERN_INFO "mt9t013: CAMIF_PAD_REG_RESET_2 (pclk_set %d)\n",
				pclk_set);
		if (!pclk_set)
			rc = -EIO;
		else if (CHECK()) {
			HWIO_OUTM (MDDI_CAMIF_CFG,
				HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PAD_REG_SW_RESET),
					1 << HWIO_SHFT (MDDI_CAMIF_CFG,
					CAM_PAD_REG_SW_RESET));
			msleep(10);
			HWIO_OUTM (MDDI_CAMIF_CFG,
				HWIO_FMSK (MDDI_CAMIF_CFG, CAM_PAD_REG_SW_RESET),
				0 << HWIO_SHFT (MDDI_CAMIF_CFG,
				CAM_PAD_REG_SW_RESET));
			msleep(10);
		}
		break;

	case MT9T013_I2C_IOCTL_CAMIF_APPS_RESET:
		printk(KERN_INFO "mt9t013: CAMIF_APPS_RESET\n"); 
		if (CHECK()) {
			rc = clk_select(1);
			if(rc < 0) {
				printk(KERN_ERR "mt9t013 error switching to internal clock\n");
				break;	
			}
			HWIO_OUTM (APPS_RESET,
				HWIO_FMSK(APPS_RESET,VFE),
				1 << HWIO_SHFT(APPS_RESET,VFE));
			udelay(10);
			HWIO_OUTM (APPS_RESET,
				HWIO_FMSK(APPS_RESET,VFE),
				0 << HWIO_SHFT(APPS_RESET,VFE));
			udelay(10);
			rc = clk_select(0); 
			if(rc < 0) {
				printk(KERN_ERR "mt9t013 error switching to external clock\n");
				break;
			}
		}
		break;

	case CAMERA_LENS_POWER_ON:
		rc = mt9t013_lens_power(1);
		break;

	case CAMERA_LENS_POWER_OFF:
		rc = mt9t013_lens_power(0);
		break;

	case MT9T013_I2C_IOCTL_CLK_ENABLE:
		printk(KERN_INFO "mt9t013: clk enable %ld\n", arg);
		rc = msm_camio_clk_enable(arg);
		break;

	case MT9T013_I2C_IOCTL_CLK_DISABLE:
		printk(KERN_INFO "mt9t013: clk disable %ld\n", arg);
		rc = msm_camio_clk_disable(arg);
		break;

	case MT9T013_I2C_IOCTL_CLK_SELECT:
		printk(KERN_INFO "mt9t013: clk select %ld\n", arg);
		rc = clk_select(!!arg);
		break;
  
	case MT9T013_I2C_IOCTL_CLK_FREQ_PROG:
		printk(KERN_INFO "mt9t013: clk rate select %ld\n", arg);
		rc = msm_camio_clk_rate_set(arg);
		break;

	case MT9T013_I2C_IOCTL_GET_REGISTERS:
		printk(KERN_INFO "mt9t013: get registers\n");
		if (copy_to_user(argp, &mt9t013_reg_pattern.reg, sizeof(mt9t013_reg_pattern.reg)))
			rc = -EFAULT;
		break;

	case MT9T013_I2C_IOCTL_SENSOR_SETTING:
		printk(KERN_INFO "mt9t013: sensor setting 0x%lx\n", arg);
		rc = mt9t013_i2c_sensor_setting(arg);
		break;

	case MT9T013_I2C_IOCTL_EXPOSURE_GAIN: {
		struct mt9t013_exposure_gain exp;
		if (copy_from_user(&exp, argp, sizeof(exp))) {
			printk(KERN_ERR "mt9t013: (exposure gain) invalid user pointer\n");
			rc = -EFAULT;
			break;
		}
		rc = mt9t013_i2c_exposure_gain(exp.mode, exp.line, exp.gain); 
	}
		break;

	case MT9T013_I2C_IOCTL_MOVE_FOCUS:
		printk(KERN_INFO "mt9t013: move focus %ld\n", arg);
		rc = mt9t013_i2c_move_focus((uint16_t)arg);
		break;

	case MT9T013_I2C_IOCTL_SET_DEFAULT_FOCUS:
		printk(KERN_INFO "mt9t013: set default focus %ld\n", arg);
		rc = mt9t013_i2c_set_default_focus((uint8_t)arg);
		break;

	case MT9T013_I2C_IOCTL_POWER_DOWN:
		rc = mt9t013_i2c_power_down();
		break;

	case MT9T013_I2C_IOCTL_INIT: {
		struct mt9t013_init init;
		printk(KERN_INFO "mt9t013: init\n");
		if (copy_from_user(&init, argp, sizeof(init))) {
			printk(KERN_ERR "mt9t013: (init) invalid user pointer\n");
			rc = -EFAULT;
			break;
		}
		rc = mt9t013_i2c_sensor_init(&init);
		if (copy_to_user(argp, &init, sizeof(init)))
			rc = -EFAULT;
	}
		break;

	case CAMERA_CONFIGURE_GPIOS:
	case CAMERA_UNCONFIGURE_GPIOS: 
		break;

	default:
		printk(KERN_INFO "mt9t013: unknown ioctl %d\n", cmd);
		break;
	}

	up(&sem);

	return rc;
}

#undef CHECK

static int mt9t013_lens_power(int on)
{
	int rc;
	printk(KERN_INFO "mt9t013: lens power %d\n", on);
	rc = gpio_request(cam->vcm_pwd, "mt9t013");
	if (!rc)
		gpio_direction_output(cam->vcm_pwd, !on);
	else printk(KERN_ERR "mt9t013 error: request gpio %d failed:"
		" %d\n", cam->vcm_pwd, rc);
	gpio_free(cam->vcm_pwd);
	return rc;
}

#define I2C_WRITE(reg,data) if (!mt9t013_i2c_write(reg, data) < 0) return -EIO
#define MT9T013_MU3M0VC_RESET_DELAY_MSECS    66

static int mt9t013_i2c_sensor_init(struct mt9t013_init *init)
{
	int rc;
	
	
	I2C_WRITE(MT9T013_REG_RESET_REGISTER, 0x10cc & 0xfffe);
	msleep(MT9T013_MU3M0VC_RESET_DELAY_MSECS);

	if ((rc = mt9t013_i2c_read(MT9T013_MU3M0VC_REG_MODEL_ID, &init->chipid)) < 0) {
		printk(KERN_ERR "mt9t013: could not read chip id: %d\n", rc);
		return rc;
	}
	printk(KERN_INFO "mt9t013: chip id: %d\n", init->chipid);

	if (init->chipid != MT9T013_MU3M0VC_MODEL_ID) {
		printk(KERN_INFO "mt9t013: chip id %d is invalid\n",
			init->chipid);
		return -EINVAL;
	}

	I2C_WRITE(0x306E, 0x9080);
	I2C_WRITE(0x301A, 0x10CC);
	I2C_WRITE(0x3064, 0x0805);
	msleep(MT9T013_MU3M0VC_RESET_DELAY_MSECS);

	if ((rc = mt9t013_i2c_sensor_setting(CAMSENSOR_REG_INIT |
					((init->preview ? 0 : 1) << 1))) < 0) {
		printk(KERN_INFO "mt9t013: failed to configure the sensor\n");
		return rc;
	}

	mt9t013_i2c_power_up();

	return 0;
}

static int mt9t013_mu3m0vc_set_lc(void)
{
	
	I2C_WRITE(0x360A, 0x0290); 
	I2C_WRITE(0x360C, 0xC92D); 
	I2C_WRITE(0x360E, 0x0771); 
	I2C_WRITE(0x3610, 0xE38C); 
	I2C_WRITE(0x3612, 0xD74F); 
	I2C_WRITE(0x364A, 0x168C); 
	I2C_WRITE(0x364C, 0xCACB); 
	I2C_WRITE(0x364E, 0x8C4C); 
	I2C_WRITE(0x3650, 0x0BEA); 
	I2C_WRITE(0x3652, 0xDC0F); 
	I2C_WRITE(0x368A, 0x70B0); 
	I2C_WRITE(0x368C, 0x200B); 
	I2C_WRITE(0x368E, 0x30B2); 
	I2C_WRITE(0x3690, 0xD04F); 
	I2C_WRITE(0x3692, 0xACF5); 
	I2C_WRITE(0x36CA, 0xF7C9); 
	I2C_WRITE(0x36CC, 0x2AED); 
	I2C_WRITE(0x36CE, 0xA652); 
	I2C_WRITE(0x36D0, 0x8192); 
	I2C_WRITE(0x36D2, 0x3A15); 
	I2C_WRITE(0x370A, 0xDA30); 
	I2C_WRITE(0x370C, 0x2E2F); 
	I2C_WRITE(0x370E, 0xBB56); 
	I2C_WRITE(0x3710, 0x8195); 
	I2C_WRITE(0x3712, 0x02F9); 
	I2C_WRITE(0x3600, 0x0230); 
	I2C_WRITE(0x3602, 0x58AD); 
	I2C_WRITE(0x3604, 0x18D1); 
	I2C_WRITE(0x3606, 0x260D); 
	I2C_WRITE(0x3608, 0xF530); 
	I2C_WRITE(0x3640, 0x17EB); 
	I2C_WRITE(0x3642, 0x3CAB); 
	I2C_WRITE(0x3644, 0x87CE); 
	I2C_WRITE(0x3646, 0xC02E); 
	I2C_WRITE(0x3648, 0xF48F); 
	I2C_WRITE(0x3680, 0x5350); 
	I2C_WRITE(0x3682, 0x7EAF); 
	I2C_WRITE(0x3684, 0x4312); 
	I2C_WRITE(0x3686, 0xC652); 
	I2C_WRITE(0x3688, 0xBC15); 
	I2C_WRITE(0x36C0, 0xB8AD); 
	I2C_WRITE(0x36C2, 0xBDCD); 
	I2C_WRITE(0x36C4, 0xE4B2); 
	I2C_WRITE(0x36C6, 0xB50F); 
	I2C_WRITE(0x36C8, 0x5B95); 
	I2C_WRITE(0x3700, 0xFC90); 
	I2C_WRITE(0x3702, 0x8C51); 
	I2C_WRITE(0x3704, 0xCED6); 
	I2C_WRITE(0x3706, 0xB594); 
	I2C_WRITE(0x3708, 0x0A39); 
	I2C_WRITE(0x3614, 0x0230); 
	I2C_WRITE(0x3616, 0x160D); 
	I2C_WRITE(0x3618, 0x08D1); 
	I2C_WRITE(0x361A, 0x98AB); 
	I2C_WRITE(0x361C, 0xEA50); 
	I2C_WRITE(0x3654, 0xB4EA); 
	I2C_WRITE(0x3656, 0xEA6C); 
	I2C_WRITE(0x3658, 0xFE08); 
	I2C_WRITE(0x365A, 0x2C6E); 
	I2C_WRITE(0x365C, 0xEB0E); 
	I2C_WRITE(0x3694, 0x6DF0); 
	I2C_WRITE(0x3696, 0x3ACF); 
	I2C_WRITE(0x3698, 0x3E0F); 
	I2C_WRITE(0x369A, 0xB2B1); 
	I2C_WRITE(0x369C, 0xC374); 
	I2C_WRITE(0x36D4, 0xF2AA); 
	I2C_WRITE(0x36D6, 0x8CCC); 
	I2C_WRITE(0x36D8, 0xDEF2); 
	I2C_WRITE(0x36DA, 0xFA11); 
	I2C_WRITE(0x36DC, 0x42F5); 
	I2C_WRITE(0x3714, 0xF4F1); 
	I2C_WRITE(0x3716, 0xF6F0); 
	I2C_WRITE(0x3718, 0x8FD6); 
	I2C_WRITE(0x371A, 0xEA14); 
	I2C_WRITE(0x371C, 0x6338); 
	I2C_WRITE(0x361E, 0x0350); 
	I2C_WRITE(0x3620, 0x91AE); 
	I2C_WRITE(0x3622, 0x0571); 
	I2C_WRITE(0x3624, 0x100D); 
	I2C_WRITE(0x3626, 0xCA70); 
	I2C_WRITE(0x365E, 0xE6CB); 
	I2C_WRITE(0x3660, 0x50ED); 
	I2C_WRITE(0x3662, 0x3DAE); 
	I2C_WRITE(0x3664, 0xAA4F); 
	I2C_WRITE(0x3666, 0xDC50); 
	I2C_WRITE(0x369E, 0x5470); 
	I2C_WRITE(0x36A0, 0x1F6E); 
	I2C_WRITE(0x36A2, 0x6671); 
	I2C_WRITE(0x36A4, 0xC010); 
	I2C_WRITE(0x36A6, 0x8DF5); 
	I2C_WRITE(0x36DE, 0x0B0C); 
	I2C_WRITE(0x36E0, 0x84CE); 
	I2C_WRITE(0x36E2, 0x8493); 
	I2C_WRITE(0x36E4, 0xA610); 
	I2C_WRITE(0x36E6, 0x50B5); 
	I2C_WRITE(0x371E, 0x9651); 
	I2C_WRITE(0x3720, 0x1EAB); 
	I2C_WRITE(0x3722, 0xAF76); 
	I2C_WRITE(0x3724, 0xE4F4); 
	I2C_WRITE(0x3726, 0x79F8); 
	I2C_WRITE(0x3782, 0x0410); 
	I2C_WRITE(0x3784, 0x0320); 
	I2C_WRITE(0x3780, 0x8000); 

	return 0;
}

static int mt9t013_set_pclk(int rt, int div_adj)
{
	int rc;
	if ((rc = mt9t013_i2c_power_down()) < 0) return rc; 
	I2C_WRITE(REG_VT_PIX_CLK_DIV, mt9t013_reg_pattern.reg[rt].vt_pix_clk_div);
	I2C_WRITE(REG_VT_SYS_CLK_DIV, mt9t013_reg_pattern.reg[rt].vt_sys_clk_div);
	I2C_WRITE(REG_PRE_PLL_CLK_DIV, mt9t013_reg_pattern.reg[rt].pre_pll_clk_div * div_adj);
	I2C_WRITE(REG_PLL_MULTIPLIER, mt9t013_reg_pattern.reg[rt].pll_multiplier);
	I2C_WRITE(REG_OP_PIX_CLK_DIV, mt9t013_reg_pattern.reg[rt].op_pix_clk_div);
	I2C_WRITE(REG_OP_SYS_CLK_DIV, mt9t013_reg_pattern.reg[rt].op_sys_clk_div);
	if ((rc = mt9t013_i2c_power_up()) < 0) return rc; 
	pclk_set = 1;
	return 0;
}

static int mt9t013_i2c_sensor_setting(unsigned long arg) 
{
	uint32_t update = arg & 1;
	uint32_t rt = (arg & 2) >> 1;

	if (rt > 1 || update > 1) {
		printk(KERN_ERR "mt9t013: invalid values %d of rt or %d of update\n",
			rt, update);
		return -EINVAL;
	}

	switch (update) {
	case CAMSENSOR_REG_UPDATE_PERIODIC: {
		uint16_t pclk_div_adj = arg >> 16;

		printk(KERN_INFO "CAMSENSOR_REG_UPDATE_PERIODIC (rt %d)\n", rt);
		
		if (!pclk_div_adj || pclk_div_adj > 2) {
			printk(KERN_ERR "mt9t013: invalid value %d of pclk_div_adj\n",
					pclk_div_adj); 
			return -EINVAL;
		}
		
		if (mt9t013_set_pclk(rt, pclk_div_adj) < 0)
			return -EIO;

		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_HOLD);
		I2C_WRITE(REG_ROW_SPEED, mt9t013_reg_pattern.reg[rt].row_speed);
		I2C_WRITE(REG_X_ADDR_START, mt9t013_reg_pattern.reg[rt].x_addr_start);
		I2C_WRITE(REG_X_ADDR_END, mt9t013_reg_pattern.reg[rt].x_addr_end);
		I2C_WRITE(REG_Y_ADDR_START, mt9t013_reg_pattern.reg[rt].y_addr_start);
		I2C_WRITE(REG_Y_ADDR_END, mt9t013_reg_pattern.reg[rt].y_addr_end);

		if (machine_is_sapphire()) {
			if (rt == 0) {
				I2C_WRITE(REG_READ_MODE, 0x046F);
			} else {
				I2C_WRITE(REG_READ_MODE, 0x0027);
			}
		} else {
			I2C_WRITE(REG_READ_MODE,
				mt9t013_reg_pattern.reg[rt].read_mode);
		}

		I2C_WRITE(REG_SCALE_M, mt9t013_reg_pattern.reg[rt].scale_m);
		I2C_WRITE(REG_X_OUTPUT_SIZE, mt9t013_reg_pattern.reg[rt].x_output_size);
		I2C_WRITE(REG_Y_OUTPUT_SIZE, mt9t013_reg_pattern.reg[rt].y_output_size);
		I2C_WRITE(REG_LINE_LENGTH_PCK, mt9t013_reg_pattern.reg[rt].line_length_pck);
		I2C_WRITE(REG_FRAME_LENGTH_LINES, (uint16_t) (mt9t013_reg_pattern.reg[rt].frame_length_lines * fps_divider));
		I2C_WRITE(REG_COARSE_INTEGRATION_TIME, mt9t013_reg_pattern.reg[rt].coarse_integration_time);
		I2C_WRITE(REG_FINE_INTEGRATION_TIME, mt9t013_reg_pattern.reg[rt].fine_integration_time);
		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_UPDATE);
	}
		break;
		
	case CAMSENSOR_REG_INIT:
		printk(KERN_INFO "CAMSENSOR_REG_INIT (rt %d)\n", rt);

		if (mt9t013_set_pclk(rt, 1) < 0) return -EIO;

		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_HOLD);
		
		
		I2C_WRITE(0x3084, 0x2409);
		I2C_WRITE(0x3092, 0x0A49);
		I2C_WRITE(0x3094, 0x4949);
		I2C_WRITE(0x3096, 0x4949);

		
		I2C_WRITE(REG_ROW_SPEED,
			mt9t013_reg_pattern.reg[rt].row_speed);
		I2C_WRITE(REG_X_ADDR_START,
			mt9t013_reg_pattern.reg[rt].x_addr_start);
		I2C_WRITE(REG_X_ADDR_END,
			mt9t013_reg_pattern.reg[rt].x_addr_end);
		I2C_WRITE(REG_Y_ADDR_START,
			mt9t013_reg_pattern.reg[rt].y_addr_start);
		I2C_WRITE(REG_Y_ADDR_END,
			mt9t013_reg_pattern.reg[rt].y_addr_end);

		if (machine_is_sapphire()) {
			if (rt == 0) {
				I2C_WRITE(REG_READ_MODE, 0x046F);
			} else {
				I2C_WRITE(REG_READ_MODE, 0x0027);
			}
		} else {
			I2C_WRITE(REG_READ_MODE,
				mt9t013_reg_pattern.reg[rt].read_mode);
		}

		I2C_WRITE(REG_SCALE_M, mt9t013_reg_pattern.reg[rt].scale_m);
		I2C_WRITE(REG_X_OUTPUT_SIZE, mt9t013_reg_pattern.reg[rt].x_output_size);
		I2C_WRITE(REG_Y_OUTPUT_SIZE, mt9t013_reg_pattern.reg[rt].y_output_size);
		I2C_WRITE(REG_LINE_LENGTH_PCK, mt9t013_reg_pattern.reg[rt].line_length_pck);
		I2C_WRITE(REG_FRAME_LENGTH_LINES, mt9t013_reg_pattern.reg[rt].frame_length_lines);
		I2C_WRITE(REG_COARSE_INTEGRATION_TIME, mt9t013_reg_pattern.reg[rt].coarse_integration_time);
		I2C_WRITE(REG_FINE_INTEGRATION_TIME, mt9t013_reg_pattern.reg[rt].fine_integration_time);

		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_UPDATE);

		
		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_HOLD);
		if(mt9t013_mu3m0vc_set_lc() < 0) return -EIO;
		I2C_WRITE(REG_GROUPED_PARAMETER_HOLD, GROUPED_PARAMETER_UPDATE);
		break;
		
	default:
		return -EINVAL;
	}

	return 0;
}

static int mt9t013_i2c_exposure_gain(uint32_t mode, uint16_t line,
					uint16_t gain)
{
	static const uint16_t max_legal_gain  = 0x01FF;
	
	if (gain > max_legal_gain) gain = max_legal_gain;

	gain |= 0x200; 

	
	I2C_WRITE(REG_GLOBAL_GAIN, gain);
	I2C_WRITE(REG_COARSE_INTEGRATION_TIME, line);
	
	if (mode == 1) {
		
		I2C_WRITE(MT9T013_REG_RESET_REGISTER, 0x10cc|0x0002);
	}
	return 0;
}

#define I2C_AF_WRITE(command, data) if (mt9t013_i2c_lens_write(AF_I2C_ID >> 1, command, data) < 0) return -EIO;

static int mt9t013_i2c_move_focus(uint16_t position)
{
	uint8_t code_val_msb = (position >> 2) | ((position << 4) >> 6);
	uint8_t code_val_lsb = (position & 0x03) << 6;

	I2C_AF_WRITE(code_val_msb, code_val_lsb);
	return 0;
}

static int mt9t013_i2c_set_default_focus(uint8_t step)
{
	I2C_AF_WRITE(0x01, step);
    	return 0; 
}

static int powered;

static int mt9t013_i2c_power_up(void)
{
	printk(KERN_INFO "mt9t013: power up\n");
	if (powered) {
		printk(KERN_INFO "mt9t013: already powered up\n");
		return 0;
	}
	I2C_WRITE(MT9T013_REG_RESET_REGISTER, MT9T013_RESET_REGISTER_PWON);
	mdelay(5);
	powered = 1;
	return 0;
}

static int mt9t013_i2c_power_down(void)
{
	int i = 0, try_more = 100;

	printk(KERN_INFO "mt9t013: power down\n");
	if (!powered) {
		printk(KERN_INFO "mt9t013: already powered down\n");
		return 0;
	}

	
	
	
	while(mt9t013_i2c_write(MT9T013_REG_RESET_REGISTER, MT9T013_RESET_REGISTER_PWOFF) < 0)
	{
		if (i >= try_more)
			return -EIO;
		else {
			i++;
			printk(KERN_INFO "mt9p012: in mt9p012_i2c_power_down() call mt9p012_i2c_write() failed !!!  (try %d times)\n", i);
			mdelay(i+5);
		}
	}
	
	mdelay(5);
	powered = pclk_set = 0;
	return 0;
}

#undef I2C_WRITE
#undef I2C_AF_WRITE

static int mt9t013_init_client(struct i2c_client *client)
{
	
	init_waitqueue_head(&g_data_ready_wait_queue);
	return 0;
}   

static struct file_operations mt9t013_fops = {
        .owner 	= THIS_MODULE,
        .open 	= mt9t013_open,
        .release = mt9t013_release,
        .unlocked_ioctl = mt9t013_ioctl,
};

static struct miscdevice mt9t013_device = {
        .minor 	= MISC_DYNAMIC_MINOR,
        .name 	= "mt9t013",
        .fops 	= &mt9t013_fops,
};

static const char *MT9T013Vendor = "micron";
static const char *MT9T013NAME = "mt9t013";
static const char *MT9T013Size = "3M";



static ssize_t sensor_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	sprintf(buf, "%s %s %s\n", MT9T013Vendor, MT9T013NAME, MT9T013Size);
	ret = strlen(buf) + 1;

	return ret;
}

static DEVICE_ATTR(sensor, 0444, sensor_vendor_show, NULL);


static struct kobject *android_mt9t013 = NULL;

static int mt9t013_sysfs_init(void)
{
	int ret ;
	printk(KERN_INFO "mt9t013:kobject creat and add\n");
	android_mt9t013 = kobject_create_and_add("android_camera", NULL);
	if (android_mt9t013 == NULL) {
		printk(KERN_INFO "mt9t013_sysfs_init: subsystem_register " \
		"failed\n");
		ret = -ENOMEM;
		return ret ;
	}
	printk(KERN_INFO "mt9t013:sysfs_create_file\n");
	ret = sysfs_create_file(android_mt9t013, &dev_attr_sensor.attr);
	if (ret) {
		printk(KERN_INFO "mt9t013_sysfs_init: sysfs_create_file " \
		"failed\n");
		kobject_del(android_mt9t013);
	}
	return 0 ;
}



static int mt9t013_probe(
	struct i2c_client *client, const struct i2c_device_id *id)
{
	struct mt9t013_data *mt;
	int err = 0;
	printk(KERN_INFO "mt9t013: probe\n");

	if(!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		goto exit_check_functionality_failed;		

	if(!(mt = kzalloc( sizeof(struct mt9t013_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, mt);
	mt9t013_init_client(client);
	pclient = client;
	mt9t013_sensor_init();
	mt9t013_sensor_suspend();
	
	
	err = misc_register(&mt9t013_device);
	if(err) {
		printk(KERN_ERR "mt9t013_probe: misc_register failed \n");
		goto exit_misc_device_register_failed;
	}
	init_suspend();
	mt9t013_sysfs_init();
	return 0;
	
exit_misc_device_register_failed:
exit_alloc_data_failed:
exit_check_functionality_failed:
	
	return err;
}

	
static int mt9t013_remove(struct i2c_client *client)
{
	struct mt9t013_data *mt = i2c_get_clientdata(client);
	free_irq(client->irq, mt);
	deinit_suspend();
	pclient = NULL;
	misc_deregister(&mt9t013_device);
	kfree(mt);
	return 0;
}

static const struct i2c_device_id mt9t013_id[] = {
	{ "mt9t013", 0 },
	{ }
};

static struct i2c_driver mt9t013_driver = {
	.probe = mt9t013_probe,
	.remove = mt9t013_remove,
	.id_table = mt9t013_id,
	.driver = {		
		.name   = "mt9t013",
	},
};

static int mt9t013_plat_probe(struct platform_device *pdev __attribute__((unused)))
{
       int rc = -EFAULT;

       if(pdev->dev.platform_data)
       {
               printk(KERN_INFO "pdev->dev.platform_data is not NULL\n");
               cam = pdev->dev.platform_data;
               rc = i2c_add_driver(&mt9t013_driver);
       }
       return rc;
}

static struct platform_driver mt9t013_plat_driver = {
        .probe = mt9t013_plat_probe,
        .driver = {
                .name = "camera",
                .owner = THIS_MODULE,
        },
};

static int __init mt9t013_init(void)
{
       return platform_driver_register(&mt9t013_plat_driver);
}

module_init(mt9t013_init);

MODULE_AUTHOR("Kidd Chen");
MODULE_DESCRIPTION("MT9T013 Driver");
MODULE_LICENSE("GPL");
