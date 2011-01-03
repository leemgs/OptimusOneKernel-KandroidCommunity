#ifndef _LINUX_VIRTIO_BALLOON_H
#define _LINUX_VIRTIO_BALLOON_H

#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>


#define VIRTIO_BALLOON_F_MUST_TELL_HOST	0 


#define VIRTIO_BALLOON_PFN_SHIFT 12

struct virtio_balloon_config
{
	
	__le32 num_pages;
	
	__le32 actual;
};
#endif 
