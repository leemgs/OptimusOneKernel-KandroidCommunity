
#ifndef _KGSL_DEVICE_H
#define _KGSL_DEVICE_H

#include <linux/types.h>
#include <linux/irqreturn.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <linux/msm_kgsl.h>

#include <asm/atomic.h>

#include "kgsl_mmu.h"
#include "kgsl_ringbuffer.h"

#define KGSL_CONTEXT_MAX        8

#define KGSL_TIMEOUT_NONE       0
#define KGSL_TIMEOUT_DEFAULT    0xFFFFFFFF

#define FIRST_TIMEOUT (HZ / 2)

#define KGSL_DEV_FLAGS_INITIALIZED0	0x00000001
#define KGSL_DEV_FLAGS_INITIALIZED	0x00000002
#define KGSL_DEV_FLAGS_STARTED		0x00000004
#define KGSL_DEV_FLAGS_ACTIVE		0x00000008


#define KGSL_PWRFLAGS_YAMATO_POWER_OFF		0x00000001
#define KGSL_PWRFLAGS_YAMATO_POWER_ON		0x00000002
#define KGSL_PWRFLAGS_YAMATO_CLK_ON		0x00000004
#define KGSL_PWRFLAGS_YAMATO_CLK_OFF		0x00000008
#define KGSL_PWRFLAGS_OVERRIDE_ON		0x00000010
#define KGSL_PWRFLAGS_OVERRIDE_OFF		0x00000020
#define KGSL_PWRFLAGS_YAMATO_IRQ_ON		0x00000040
#define KGSL_PWRFLAGS_YAMATO_IRQ_OFF		0x00000080
#define KGSL_PWRFLAGS_G12_CLK_ON		0x00000100
#define KGSL_PWRFLAGS_G12_CLK_OFF		0x00000200
#define KGSL_PWRFLAGS_G12_IRQ_ON		0x00000400
#define KGSL_PWRFLAGS_G12_IRQ_OFF		0x00000800
#define KGSL_PWRFLAGS_G12_POWER_OFF		0x00001000
#define KGSL_PWRFLAGS_G12_POWER_ON		0x00002000

#define KGSL_CHIPID_YAMATODX_REV21  0x20100
#define KGSL_CHIPID_YAMATODX_REV211 0x20101

int kgsl_yamato_setup_pt(struct kgsl_device *device,
			 struct kgsl_pagetable *pagetable);
int kgsl_yamato_cleanup_pt(struct kgsl_device *device,
			   struct kgsl_pagetable *pagetable);

#define KGSL_GRAPHICS_MEMORY_LOW_WATERMARK  0x1000000

#define KGSL_IS_PAGE_ALIGNED(addr) (!((addr) & (~PAGE_MASK)))

struct kgsl_device;
struct platform_device;
struct kgsl_device_private;

struct kgsl_functable {
	int (*device_regread) (struct kgsl_device *device,
					unsigned int offsetwords,
					unsigned int *value);
	int (*device_regwrite) (struct kgsl_device *device,
					unsigned int offsetwords,
					unsigned int value);
	int (*device_setstate) (struct kgsl_device *device, uint32_t flags);
	int (*device_idle) (struct kgsl_device *device, unsigned int timeout);
	int (*device_suspend) (struct kgsl_device *device);
	int (*device_wake) (struct kgsl_device *device);
	int (*device_last_release_locked) (struct kgsl_device *device);
	int (*device_first_open_locked) (struct kgsl_device *device);
	int (*device_getproperty) (struct kgsl_device *device,
					enum kgsl_property_type type,
					void *value,
					unsigned int sizebytes);
	int (*device_waittimestamp) (struct kgsl_device *device,
					unsigned int timestamp,
					unsigned int msecs);
	unsigned int (*device_cmdstream_readtimestamp) (
					struct kgsl_device *device,
					enum kgsl_timestamp_type type);
	int (*device_issueibcmds) (struct kgsl_device_private *dev_priv,
				int drawctxt_index,
				uint32_t ibaddr, int sizedwords,
				uint32_t *timestamp,
				unsigned int flags);
	int (*device_drawctxt_create) (struct kgsl_device_private *dev_priv,
					uint32_t flags,
					unsigned int *drawctxt_id);
	int (*device_drawctxt_destroy) (struct kgsl_device *device,
					unsigned int drawctxt_id);
	long (*device_ioctl) (struct kgsl_device_private *dev_priv,
					unsigned int cmd,
					unsigned long arg);

};

struct kgsl_memregion {
	unsigned char  *mmio_virt_base;
	unsigned int   mmio_phys_base;
	uint32_t      gpu_base;
	unsigned int   sizebytes;
};

struct kgsl_device {
	uint32_t       flags;
	enum kgsl_deviceid    id;
	unsigned int      chip_id;
	struct kgsl_memregion regspace;
	struct kgsl_memdesc memstore;

	struct kgsl_mmu 	  mmu;
	struct kgsl_ringbuffer ringbuffer;
	unsigned int hwaccess_blocked;
	struct completion hwaccess_gate;
	struct kgsl_functable ftbl;
	struct work_struct idle_check_ws;
	struct timer_list idle_timer;
	atomic_t open_count;

	struct atomic_notifier_head ts_notifier_list;
};

struct kgsl_file_private {
	unsigned int refcnt;
	struct list_head mem_list;
	struct kgsl_pagetable *pagetable;
	unsigned long vmalloc_size;
	struct list_head preserve_entry_list;
	int preserve_list_size;
};

struct kgsl_device_private {
	struct list_head list;
	uint32_t ctxt_id_mask;
	unsigned long pid;
	struct kgsl_device *device;
	struct kgsl_file_private *process_priv;
};

struct kgsl_devconfig {
	struct kgsl_memregion regspace;

	unsigned int     mmu_config;
	uint32_t        mpu_base;
	int              mpu_range;
	uint32_t        va_base;
	unsigned int     va_range;

	struct kgsl_memregion gmemspace;
};

static inline struct kgsl_mmu *
kgsl_get_mmu(struct kgsl_device *device)
{
	return (struct kgsl_mmu *) (device ? &device->mmu : NULL);
}

#endif  
