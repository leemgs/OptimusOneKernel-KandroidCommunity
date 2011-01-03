
#ifndef _BROADCAST_LG2102_H_
#define _BROADCAST_LG2102_H_
#include <linux/broadcast/broadcast_tdmb_typedef.h>

typedef struct
{
	void		(*tdmb_pwr_on)(void);
	void		(*tdmb_pwr_off)(void);
}broadcast_pwr_func;

struct broadcast_tdmb_data
{
	void (*pwr_on)(void);
	void (*pwr_off)(void);
};


int tdmb_lg2102_power_on(void);
int tdmb_lg2102_power_off(void);
int tdmb_lg2102_i2c_write_burst(uint16 waddr, uint8* wdata, int length);
int tdmb_lg2102_i2c_read_burst(uint16 raddr, uint8* rdata, int length);
int tdmb_lg2102_mdelay(int32 ms);
void tdmb_lg2102_set_userstop(void);
#endif

