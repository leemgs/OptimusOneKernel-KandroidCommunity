#ifndef _LINUX_VIRTIO_BLK_H
#define _LINUX_VIRTIO_BLK_H

#include <linux/types.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>


#define VIRTIO_BLK_F_BARRIER	0	
#define VIRTIO_BLK_F_SIZE_MAX	1	
#define VIRTIO_BLK_F_SEG_MAX	2	
#define VIRTIO_BLK_F_GEOMETRY	4	
#define VIRTIO_BLK_F_RO		5	
#define VIRTIO_BLK_F_BLK_SIZE	6	
#define VIRTIO_BLK_F_SCSI	7	
#define VIRTIO_BLK_F_FLUSH	9	

struct virtio_blk_config {
	
	__u64 capacity;
	
	__u32 size_max;
	
	__u32 seg_max;
	
	struct virtio_blk_geometry {
		__u16 cylinders;
		__u8 heads;
		__u8 sectors;
	} geometry;
	
	__u32 blk_size;
} __attribute__((packed));




#define VIRTIO_BLK_T_IN		0
#define VIRTIO_BLK_T_OUT	1


#define VIRTIO_BLK_T_SCSI_CMD	2


#define VIRTIO_BLK_T_FLUSH	4


#define VIRTIO_BLK_T_BARRIER	0x80000000


struct virtio_blk_outhdr {
	
	__u32 type;
	
	__u32 ioprio;
	
	__u64 sector;
};

struct virtio_scsi_inhdr {
	__u32 errors;
	__u32 data_len;
	__u32 sense_len;
	__u32 residual;
};


#define VIRTIO_BLK_S_OK		0
#define VIRTIO_BLK_S_IOERR	1
#define VIRTIO_BLK_S_UNSUPP	2
#endif 
