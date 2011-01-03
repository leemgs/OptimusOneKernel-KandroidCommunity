

#ifndef __XEN_PUBLIC_IO_BLKIF_H__
#define __XEN_PUBLIC_IO_BLKIF_H__

#include "ring.h"
#include "../grant_table.h"



typedef uint16_t blkif_vdev_t;
typedef uint64_t blkif_sector_t;


#define BLKIF_OP_READ              0
#define BLKIF_OP_WRITE             1

#define BLKIF_OP_WRITE_BARRIER     2


#define BLKIF_MAX_SEGMENTS_PER_REQUEST 11

struct blkif_request {
	uint8_t        operation;    
	uint8_t        nr_segments;  
	blkif_vdev_t   handle;       
	uint64_t       id;           
	blkif_sector_t sector_number;
	struct blkif_request_segment {
		grant_ref_t gref;        
		
		
		uint8_t     first_sect, last_sect;
	} seg[BLKIF_MAX_SEGMENTS_PER_REQUEST];
};

struct blkif_response {
	uint64_t        id;              
	uint8_t         operation;       
	int16_t         status;          
};


 
#define BLKIF_RSP_EOPNOTSUPP  -2
 
#define BLKIF_RSP_ERROR       -1
 
#define BLKIF_RSP_OKAY         0



DEFINE_RING_TYPES(blkif, struct blkif_request, struct blkif_response);

#define VDISK_CDROM        0x1
#define VDISK_REMOVABLE    0x2
#define VDISK_READONLY     0x4

#endif 
