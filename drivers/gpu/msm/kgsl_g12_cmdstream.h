
#ifndef _GSL_CMDSTREAM_H
#define _GSL_CMDSTREAM_H

#include <linux/types.h>
#include <linux/msm_kgsl.h>
#include "kgsl_device.h"
#include "kgsl_g12.h"
#include <linux/mutex.h>
#include <linux/msm_kgsl.h>
#include "kgsl_sharedmem.h"

struct kgsl_g12_device;

int kgsl_g12_cmdstream_check_timestamp(struct kgsl_g12_device *g12_device,
					unsigned int timestamp);
unsigned int kgsl_g12_cmdstream_readtimestamp(struct kgsl_device *device,
					enum kgsl_timestamp_type unused);
void kgsl_g12_cmdstream_memqueue_drain(struct kgsl_g12_device *g12_device);
int kgsl_g12_cmdstream_issueibcmds(struct kgsl_device_private *dev_priv,
			int drawctxt_index,
			uint32_t ibaddr,
			int sizedwords,
			uint32_t *timestamp,
			unsigned int ctrl);
#endif  
