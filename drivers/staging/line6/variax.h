

#ifndef VARIAX_H
#define VARIAX_H


#include "driver.h"

#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/wait.h>

#include <sound/core.h>

#include "dumprequest.h"


#define VARIAX_ACTIVATE_DELAY 10
#define VARIAX_STARTUP_DELAY 3


enum {
	VARIAX_DUMP_PASS1 = LINE6_DUMP_CURRENT,
	VARIAX_DUMP_PASS2,
	VARIAX_DUMP_PASS3
};



struct variax_model {
	
	unsigned char name[18];

	
	unsigned char control[78 * 2];
};

struct usb_line6_variax {
	
	struct usb_line6 line6;

	
	struct line6_dump_request dumpreq; struct line6_dump_reqbuf extrabuf[2];

	
	unsigned char *buffer_activate;

	
	int model;

	
	struct variax_model model_data;

	
	unsigned char bank[18];

	
	int volume;

	
	int tone;

	
	struct timer_list activate_timer;
};


extern void variax_disconnect(struct usb_interface *interface);
extern int variax_init(struct usb_interface *interface,
		       struct usb_line6_variax *variax);
extern void variax_process_message(struct usb_line6_variax *variax);


#endif
