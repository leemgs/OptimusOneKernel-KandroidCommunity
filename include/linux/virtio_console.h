#ifndef _LINUX_VIRTIO_CONSOLE_H
#define _LINUX_VIRTIO_CONSOLE_H
#include <linux/types.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>



#define VIRTIO_CONSOLE_F_SIZE	0	

struct virtio_console_config {
	
	__u16 cols;
	
	__u16 rows;
} __attribute__((packed));


#ifdef __KERNEL__
int __init virtio_cons_early_init(int (*put_chars)(u32, const char *, int));
#endif 

#endif 
