
#ifndef __GSL_DRAWCTXT_G12_H
#define __GSL_DRAWCTXT_G12_H

#include "kgsl_sharedmem.h"

struct kgsl_device;
struct kgsl_device_private;

#define KGSL_G12_PACKET_SIZE 10
#define KGSL_G12_PACKET_COUNT 8
#define KGSL_G12_RB_SIZE (KGSL_G12_PACKET_SIZE*KGSL_G12_PACKET_COUNT \
			  *sizeof(uint32_t))

#define ALIGN_IN_BYTES(dim, alignment) (((dim) + (alignment - 1)) & \
		~(alignment - 1))


#define NUMTEXUNITS             4
#define TEXUNITREGCOUNT         25
#define VG_REGCOUNT             0x39

#define PACKETSIZE_BEGIN        3
#define PACKETSIZE_G2DCOLOR     2
#define PACKETSIZE_TEXUNIT      (TEXUNITREGCOUNT * 2)
#define PACKETSIZE_REG          (VG_REGCOUNT * 2)
#define PACKETSIZE_STATE        (PACKETSIZE_TEXUNIT * NUMTEXUNITS + \
				 PACKETSIZE_REG + PACKETSIZE_BEGIN + \
				 PACKETSIZE_G2DCOLOR)
#define PACKETSIZE_STATESTREAM  (ALIGN_IN_BYTES((PACKETSIZE_STATE * \
				 sizeof(unsigned int)), 32) / \
				 sizeof(unsigned int))
#define KGSL_G12_CONTEXT_MAX 16

int
kgsl_g12_drawctxt_create(struct kgsl_device_private *dev_priv,
			uint32_t unused,
			unsigned int *drawctxt_id);

int
kgsl_g12_drawctxt_destroy(struct kgsl_device *device,
			unsigned int drawctxt_id);

#endif  
