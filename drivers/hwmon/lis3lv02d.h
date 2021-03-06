
#include <linux/platform_device.h>
#include <linux/input-polldev.h>



#include <linux/lis3lv02d.h>


#define LIS_DOUBLE_ID	0x3A 

#define LIS_SINGLE_ID	0x3B 

enum lis3_reg {
	WHO_AM_I	= 0x0F,
	OFFSET_X	= 0x16,
	OFFSET_Y	= 0x17,
	OFFSET_Z	= 0x18,
	GAIN_X		= 0x19,
	GAIN_Y		= 0x1A,
	GAIN_Z		= 0x1B,
	CTRL_REG1	= 0x20,
	CTRL_REG2	= 0x21,
	CTRL_REG3	= 0x22,
	HP_FILTER_RESET	= 0x23,
	STATUS_REG	= 0x27,
	OUTX_L		= 0x28,
	OUTX_H		= 0x29,
	OUTX		= 0x29,
	OUTY_L		= 0x2A,
	OUTY_H		= 0x2B,
	OUTY		= 0x2B,
	OUTZ_L		= 0x2C,
	OUTZ_H		= 0x2D,
	OUTZ		= 0x2D,
};

enum lis302d_reg {
	FF_WU_CFG_1	= 0x30,
	FF_WU_SRC_1	= 0x31,
	FF_WU_THS_1	= 0x32,
	FF_WU_DURATION_1 = 0x33,
	FF_WU_CFG_2	= 0x34,
	FF_WU_SRC_2	= 0x35,
	FF_WU_THS_2	= 0x36,
	FF_WU_DURATION_2 = 0x37,
	CLICK_CFG	= 0x38,
	CLICK_SRC	= 0x39,
	CLICK_THSY_X	= 0x3B,
	CLICK_THSZ	= 0x3C,
	CLICK_TIMELIMIT	= 0x3D,
	CLICK_LATENCY	= 0x3E,
	CLICK_WINDOW	= 0x3F,
};

enum lis3lv02d_reg {
	FF_WU_CFG	= 0x30,
	FF_WU_SRC	= 0x31,
	FF_WU_ACK	= 0x32,
	FF_WU_THS_L	= 0x34,
	FF_WU_THS_H	= 0x35,
	FF_WU_DURATION	= 0x36,
	DD_CFG		= 0x38,
	DD_SRC		= 0x39,
	DD_ACK		= 0x3A,
	DD_THSI_L	= 0x3C,
	DD_THSI_H	= 0x3D,
	DD_THSE_L	= 0x3E,
	DD_THSE_H	= 0x3F,
};

enum lis3lv02d_ctrl1 {
	CTRL1_Xen	= 0x01,
	CTRL1_Yen	= 0x02,
	CTRL1_Zen	= 0x04,
	CTRL1_ST	= 0x08,
	CTRL1_DF0	= 0x10,
	CTRL1_DF1	= 0x20,
	CTRL1_PD0	= 0x40,
	CTRL1_PD1	= 0x80,
};
enum lis3lv02d_ctrl2 {
	CTRL2_DAS	= 0x01,
	CTRL2_SIM	= 0x02,
	CTRL2_DRDY	= 0x04,
	CTRL2_IEN	= 0x08,
	CTRL2_BOOT	= 0x10,
	CTRL2_BLE	= 0x20,
	CTRL2_BDU	= 0x40, 
	CTRL2_FS	= 0x80, 
};

enum lis302d_ctrl2 {
	HP_FF_WU2	= 0x08,
	HP_FF_WU1	= 0x04,
};

enum lis3lv02d_ctrl3 {
	CTRL3_CFS0	= 0x01,
	CTRL3_CFS1	= 0x02,
	CTRL3_FDS	= 0x10,
	CTRL3_HPFF	= 0x20,
	CTRL3_HPDD	= 0x40,
	CTRL3_ECK	= 0x80,
};

enum lis3lv02d_status_reg {
	STATUS_XDA	= 0x01,
	STATUS_YDA	= 0x02,
	STATUS_ZDA	= 0x04,
	STATUS_XYZDA	= 0x08,
	STATUS_XOR	= 0x10,
	STATUS_YOR	= 0x20,
	STATUS_ZOR	= 0x40,
	STATUS_XYZOR	= 0x80,
};

enum lis3lv02d_ff_wu_cfg {
	FF_WU_CFG_XLIE	= 0x01,
	FF_WU_CFG_XHIE	= 0x02,
	FF_WU_CFG_YLIE	= 0x04,
	FF_WU_CFG_YHIE	= 0x08,
	FF_WU_CFG_ZLIE	= 0x10,
	FF_WU_CFG_ZHIE	= 0x20,
	FF_WU_CFG_LIR	= 0x40,
	FF_WU_CFG_AOI	= 0x80,
};

enum lis3lv02d_ff_wu_src {
	FF_WU_SRC_XL	= 0x01,
	FF_WU_SRC_XH	= 0x02,
	FF_WU_SRC_YL	= 0x04,
	FF_WU_SRC_YH	= 0x08,
	FF_WU_SRC_ZL	= 0x10,
	FF_WU_SRC_ZH	= 0x20,
	FF_WU_SRC_IA	= 0x40,
};

enum lis3lv02d_dd_cfg {
	DD_CFG_XLIE	= 0x01,
	DD_CFG_XHIE	= 0x02,
	DD_CFG_YLIE	= 0x04,
	DD_CFG_YHIE	= 0x08,
	DD_CFG_ZLIE	= 0x10,
	DD_CFG_ZHIE	= 0x20,
	DD_CFG_LIR	= 0x40,
	DD_CFG_IEND	= 0x80,
};

enum lis3lv02d_dd_src {
	DD_SRC_XL	= 0x01,
	DD_SRC_XH	= 0x02,
	DD_SRC_YL	= 0x04,
	DD_SRC_YH	= 0x08,
	DD_SRC_ZL	= 0x10,
	DD_SRC_ZH	= 0x20,
	DD_SRC_IA	= 0x40,
};

struct axis_conversion {
	s8	x;
	s8	y;
	s8	z;
};

struct lis3lv02d {
	void			*bus_priv; 
	int (*init) (struct lis3lv02d *lis3);
	int (*write) (struct lis3lv02d *lis3, int reg, u8 val);
	int (*read) (struct lis3lv02d *lis3, int reg, u8 *ret);

	u8			whoami;    
	s16 (*read_data) (struct lis3lv02d *lis3, int reg);
	int			mdps_max_val;

	struct input_polled_dev	*idev;     
	struct platform_device	*pdev;     
	atomic_t		count;     
	int			xcalib;    
	int			ycalib;    
	int			zcalib;    
	struct axis_conversion	ac;        

	u32			irq;       
	struct fasync_struct	*async_queue; 
	wait_queue_head_t	misc_wait; 
	unsigned long		misc_opened; 

	struct lis3lv02d_platform_data *pdata;	
};

int lis3lv02d_init_device(struct lis3lv02d *lis3);
int lis3lv02d_joystick_enable(void);
void lis3lv02d_joystick_disable(void);
void lis3lv02d_poweroff(struct lis3lv02d *lis3);
void lis3lv02d_poweron(struct lis3lv02d *lis3);
int lis3lv02d_remove_fs(struct lis3lv02d *lis3);

extern struct lis3lv02d lis3_dev;
