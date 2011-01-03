
#ifndef _DVB_USB_FRIIO_H_
#define _DVB_USB_FRIIO_H_



#define DVB_USB_LOG_PREFIX "friio"
#include "dvb-usb.h"

extern int dvb_usb_friio_debug;
#define deb_info(args...) dprintk(dvb_usb_friio_debug, 0x01, args)
#define deb_xfer(args...) dprintk(dvb_usb_friio_debug, 0x02, args)
#define deb_rc(args...)   dprintk(dvb_usb_friio_debug, 0x04, args)
#define deb_fe(args...)   dprintk(dvb_usb_friio_debug, 0x08, args)


#define GL861_WRITE		0x40
#define GL861_READ		0xc0


#define GL861_REQ_I2C_WRITE	0x01
#define GL861_REQ_I2C_READ	0x02


#define GL861_REQ_I2C_DATA_CTRL_WRITE	0x03

#define GL861_ALTSETTING_COUNT	2
#define FRIIO_BULK_ALTSETTING	0
#define FRIIO_ISOC_ALTSETTING	1





#define FRIIO_CTL_LNB (1 << 0)
#define FRIIO_CTL_STROBE (1 << 1)
#define FRIIO_CTL_CLK (1 << 2)
#define FRIIO_CTL_LED (1 << 3)



#define FRIIO_DEMOD_ADDR  (0x30 >> 1)
#define FRIIO_PLL_ADDR  (0xC0 >> 1)

#define JDVBT90502_PLL_CLK	4000000
#define JDVBT90502_PLL_DIVIDER	28

#define JDVBT90502_2ND_I2C_REG 0xFE



#define DEMOD_REDIRECT_REG 0
#define ADDRESS_BYTE       1
#define DIVIDER_BYTE1      2
#define DIVIDER_BYTE2      3
#define CONTROL_BYTE       4
#define BANDSWITCH_BYTE    5
#define AGC_CTRL_BYTE      5
#define PLL_CMD_LEN        6


#define PLL_STATUS_POR_MODE   0x80 
#define PLL_STATUS_LOCKED     0x40 
#define PLL_STATUS_AGC_ACTIVE 0x08 
#define PLL_STATUS_TESTMODE   0x07 
  


struct jdvbt90502_config {
	u8 demod_address; 
	u8 pll_address;   
};
extern struct jdvbt90502_config friio_fe_config;

extern struct dvb_frontend *jdvbt90502_attach(struct dvb_usb_device *d);
#endif
