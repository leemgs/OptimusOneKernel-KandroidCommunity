

#ifndef POD_H
#define POD_H


#include "driver.h"

#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include <sound/core.h>

#include "dumprequest.h"



#define PODXTLIVE_INTERFACE_POD    0
#define PODXTLIVE_INTERFACE_VARIAX 1


#define	POD_NAME_OFFSET 0
#define	POD_NAME_LENGTH 16


#define POD_CONTROL_SIZE 0x80
#define POD_BUFSIZE_DUMPREQ 7
#define POD_STARTUP_DELAY 3



struct ValueWait {
	unsigned short value;
	wait_queue_head_t wait;
};


struct pod_program {
	
	unsigned char header[0x20];

	
	unsigned char control[POD_CONTROL_SIZE];
};

struct usb_line6_pod {
	
	struct usb_line6 line6;

	
	struct line6_dump_request dumpreq;

	
	unsigned char channel_num;

	
	struct pod_program prog_data;

	
	struct pod_program prog_data_buf;

	
	unsigned char *buffer_versionreq;

	
	struct ValueWait tuner_mute;

	
	struct ValueWait tuner_freq;

	
	struct ValueWait tuner_note;

	
	struct ValueWait tuner_pitch;

	
	struct ValueWait monitor_level;

	
	struct ValueWait routing;

	
	struct ValueWait clipping;

	
	struct work_struct create_files_work;

	
	unsigned long param_dirty[POD_CONTROL_SIZE / sizeof(unsigned long)];

	
	unsigned long atomic_flags;

	
	int startup_count;

	
	int serial_number;

	
	int firmware_version;

	
	int device_id;

	
	char dirty;

	
	char versionreq_ok;

	
	char midi_postprocess;
};


extern void pod_disconnect(struct usb_interface *interface);
extern int pod_init(struct usb_interface *interface, struct usb_line6_pod *pod);
extern void pod_midi_postprocess(struct usb_line6_pod *pod,
				 unsigned char *data, int length);
extern void pod_process_message(struct usb_line6_pod *pod);
extern void pod_receive_parameter(struct usb_line6_pod *pod, int param);
extern void pod_transmit_parameter(struct usb_line6_pod *pod, int param,
				   int value);


#endif
