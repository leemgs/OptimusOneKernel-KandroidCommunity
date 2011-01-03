
#ifndef _BOARD_BROADCAST_H_
#define _BOARD_BROADCAST_H_

struct broadcast_tdmb_data
{
	void (*pwr_on)(void);
	void (*pwr_off)(void);
};

#endif

