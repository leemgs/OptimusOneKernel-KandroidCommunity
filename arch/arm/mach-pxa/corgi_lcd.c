

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/string.h>
#include <mach/corgi.h>
#include <mach/hardware.h>
#include <mach/sharpsl.h>
#include <mach/spitz.h>
#include <asm/hardware/scoop.h>
#include <asm/mach/sharpsl_param.h>
#include "generic.h"


#define RESCTL_ADRS     0x00
#define PHACTRL_ADRS    0x01
#define DUTYCTRL_ADRS   0x02
#define POWERREG0_ADRS  0x03
#define POWERREG1_ADRS  0x04
#define GPOR3_ADRS      0x05
#define PICTRL_ADRS     0x06
#define POLCTRL_ADRS    0x07


#define RESCTL_QVGA     0x01
#define RESCTL_VGA      0x00

#define POWER1_VW_ON    0x01  
#define POWER1_GVSS_ON  0x02  
#define POWER1_VDD_ON   0x04  

#define POWER1_VW_OFF   0x00  
#define POWER1_GVSS_OFF 0x00  
#define POWER1_VDD_OFF  0x00  

#define POWER0_COM_DCLK 0x01  
#define POWER0_COM_DOUT 0x02  
#define POWER0_DAC_ON   0x04  
#define POWER0_COM_ON   0x08  
#define POWER0_VCC5_ON  0x10  

#define POWER0_DAC_OFF  0x00  
#define POWER0_COM_OFF  0x00  
#define POWER0_VCC5_OFF 0x00  

#define PICTRL_INIT_STATE      0x01
#define PICTRL_INIOFF          0x02
#define PICTRL_POWER_DOWN      0x04
#define PICTRL_COM_SIGNAL_OFF  0x08
#define PICTRL_DAC_SIGNAL_OFF  0x10

#define POLCTRL_SYNC_POL_FALL  0x01
#define POLCTRL_EN_POL_FALL    0x02
#define POLCTRL_DATA_POL_FALL  0x04
#define POLCTRL_SYNC_ACT_H     0x08
#define POLCTRL_EN_ACT_L       0x10

#define POLCTRL_SYNC_POL_RISE  0x00
#define POLCTRL_EN_POL_RISE    0x00
#define POLCTRL_DATA_POL_RISE  0x00
#define POLCTRL_SYNC_ACT_L     0x00
#define POLCTRL_EN_ACT_H       0x00

#define PHACTRL_PHASE_MANUAL   0x01
#define DEFAULT_PHAD_QVGA     (9)
#define DEFAULT_COMADJ        (125)


static void lcdtg_ssp_i2c_send(u8 data)
{
	corgi_ssp_lcdtg_send(POWERREG0_ADRS, data);
	udelay(10);
}

static void lcdtg_i2c_send_bit(u8 data)
{
	lcdtg_ssp_i2c_send(data);
	lcdtg_ssp_i2c_send(data | POWER0_COM_DCLK);
	lcdtg_ssp_i2c_send(data);
}

static void lcdtg_i2c_send_start(u8 base)
{
	lcdtg_ssp_i2c_send(base | POWER0_COM_DCLK | POWER0_COM_DOUT);
	lcdtg_ssp_i2c_send(base | POWER0_COM_DCLK);
	lcdtg_ssp_i2c_send(base);
}

static void lcdtg_i2c_send_stop(u8 base)
{
	lcdtg_ssp_i2c_send(base);
	lcdtg_ssp_i2c_send(base | POWER0_COM_DCLK);
	lcdtg_ssp_i2c_send(base | POWER0_COM_DCLK | POWER0_COM_DOUT);
}

static void lcdtg_i2c_send_byte(u8 base, u8 data)
{
	int i;
	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			lcdtg_i2c_send_bit(base | POWER0_COM_DOUT);
		else
			lcdtg_i2c_send_bit(base);
		data <<= 1;
	}
}

static void lcdtg_i2c_wait_ack(u8 base)
{
	lcdtg_i2c_send_bit(base);
}

static void lcdtg_set_common_voltage(u8 base_data, u8 data)
{
	
	lcdtg_i2c_send_start(base_data);
	lcdtg_i2c_send_byte(base_data, 0x9c);
	lcdtg_i2c_wait_ack(base_data);
	lcdtg_i2c_send_byte(base_data, 0x00);
	lcdtg_i2c_wait_ack(base_data);
	lcdtg_i2c_send_byte(base_data, data);
	lcdtg_i2c_wait_ack(base_data);
	lcdtg_i2c_send_stop(base_data);
}


static void lcdtg_set_phadadj(int mode)
{
	int adj;
	switch(mode) {
		case 480:
		case 640:
			
			adj = sharpsl_param.phadadj;
			if (adj < 0) {
				adj = PHACTRL_PHASE_MANUAL;
			} else {
				adj = ((adj & 0x0f) << 1) | PHACTRL_PHASE_MANUAL;
			}
			break;
		case 240:
		case 320:
		default:
			
			adj = (DEFAULT_PHAD_QVGA << 1) | PHACTRL_PHASE_MANUAL;
			break;
	}

	corgi_ssp_lcdtg_send(PHACTRL_ADRS, adj);
}

static int lcd_inited;

void corgi_lcdtg_hw_init(int mode)
{
	if (!lcd_inited) {
		int comadj;

		
		corgi_ssp_lcdtg_send(PICTRL_ADRS, PICTRL_POWER_DOWN | PICTRL_INIOFF | PICTRL_INIT_STATE
	  			| PICTRL_COM_SIGNAL_OFF | PICTRL_DAC_SIGNAL_OFF);

		corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_COM_DCLK | POWER0_COM_DOUT | POWER0_DAC_OFF
				| POWER0_COM_OFF | POWER0_VCC5_OFF);

		corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_OFF | POWER1_VDD_OFF);

		
		corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_OFF | POWER1_VDD_ON);
		mdelay(3);

		
		corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_COM_DCLK | POWER0_COM_DOUT | POWER0_DAC_ON
				| POWER0_COM_OFF | POWER0_VCC5_OFF);

		
		
		corgi_ssp_lcdtg_send(PICTRL_ADRS, PICTRL_INIT_STATE | PICTRL_COM_SIGNAL_OFF);

		
		comadj = sharpsl_param.comadj;
		if (comadj < 0)
			comadj = DEFAULT_COMADJ;
		lcdtg_set_common_voltage((POWER0_DAC_ON | POWER0_COM_OFF | POWER0_VCC5_OFF), comadj);

		
		corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_COM_DCLK | POWER0_COM_DOUT | POWER0_DAC_ON |
				POWER0_COM_OFF | POWER0_VCC5_ON);

		
		corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_ON | POWER1_VDD_ON);
		mdelay(2);

		
		corgi_ssp_lcdtg_send(PICTRL_ADRS, PICTRL_INIT_STATE);

		
		corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_COM_DCLK | POWER0_COM_DOUT | POWER0_DAC_ON
				| POWER0_COM_ON | POWER0_VCC5_ON);

		
		corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_ON | POWER1_GVSS_ON | POWER1_VDD_ON);

		
		corgi_ssp_lcdtg_send(PICTRL_ADRS, 0);

		
		lcdtg_set_phadadj(mode);

		
		corgi_ssp_lcdtg_send(POLCTRL_ADRS, POLCTRL_SYNC_POL_RISE | POLCTRL_EN_POL_RISE
				| POLCTRL_DATA_POL_RISE | POLCTRL_SYNC_ACT_L | POLCTRL_EN_ACT_H);
		udelay(1000);

		lcd_inited=1;
	} else {
		lcdtg_set_phadadj(mode);
	}

	switch(mode) {
		case 480:
		case 640:
			
			corgi_ssp_lcdtg_send(RESCTL_ADRS, RESCTL_VGA);
			break;
		case 240:
		case 320:
		default:
			
			corgi_ssp_lcdtg_send(RESCTL_ADRS, RESCTL_QVGA);
			break;
	}
}

void corgi_lcdtg_suspend(void)
{
	
	mdelay(34);

	
	corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_ON | POWER1_VDD_ON);

	
	corgi_ssp_lcdtg_send(PICTRL_ADRS, PICTRL_COM_SIGNAL_OFF);
	corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_DAC_ON | POWER0_COM_OFF | POWER0_VCC5_ON);

	
	lcdtg_set_common_voltage(POWER0_DAC_ON | POWER0_COM_OFF | POWER0_VCC5_ON, 0);

	
	corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_OFF | POWER1_VDD_ON);

	
	corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_DAC_ON | POWER0_COM_OFF | POWER0_VCC5_OFF);

	
	corgi_ssp_lcdtg_send(PICTRL_ADRS, PICTRL_INIOFF | PICTRL_DAC_SIGNAL_OFF |
			PICTRL_POWER_DOWN | PICTRL_COM_SIGNAL_OFF);

	
	corgi_ssp_lcdtg_send(POWERREG0_ADRS, POWER0_DAC_OFF | POWER0_COM_OFF | POWER0_VCC5_OFF);

	
	corgi_ssp_lcdtg_send(POWERREG1_ADRS, POWER1_VW_OFF | POWER1_GVSS_OFF | POWER1_VDD_OFF);

	lcd_inited = 0;
}

