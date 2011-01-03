

#ifndef __LINUX_KEYCHORD_H_
#define __LINUX_KEYCHORD_H_

#include <linux/input.h>

#define KEYCHORD_VERSION		1


struct input_keychord {
	
	__u16 version;
	
	__u16 id;

	
	__u16 count;

	
	__u16 keycodes[];
};

#endif	
