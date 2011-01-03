
#include "kgsl.h"
#include "kgsl_device.h"
#include "kgsl_log.h"
#include "kgsl_g12.h"

#include "g12_reg.h"
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/msm_kgsl.h>

#define KGSL_G12_CMDWINDOW_TARGET_MASK		0x000000FF
#define KGSL_G12_CMDWINDOW_ADDR_MASK		0x00FFFF00
#define KGSL_G12_CMDWINDOW_TARGET_SHIFT		0
#define KGSL_G12_CMDWINDOW_ADDR_SHIFT		8

int kgsl_g12_cmdwindow_init(struct kgsl_device *device)
{
	return 0;
}

int kgsl_g12_cmdwindow_close(struct kgsl_device *device)
{
	return 0;
}

int kgsl_g12_cmdwindow_write(struct kgsl_device *device,
		enum kgsl_cmdwindow_type target, unsigned int addr,
		unsigned int data)
{
	unsigned int cmdwinaddr;
	unsigned int cmdstream;

	KGSL_DRV_INFO("enter (device=%p,addr=%08x,data=0x%x)\n", device, addr,
			data);

	if (target < KGSL_CMDWINDOW_MIN ||
		target > KGSL_CMDWINDOW_MAX) {
		KGSL_DRV_ERR("dev %p invalid target\n", device);
		return -EINVAL;
	}

	if (!(device->flags & KGSL_FLAGS_INITIALIZED)) {
		KGSL_DRV_ERR("Trying to write uninitialized device.\n");
		return -EINVAL;
	}

	if (target == KGSL_CMDWINDOW_MMU)
		cmdstream = ADDR_VGC_MMUCOMMANDSTREAM;
	else
		cmdstream = ADDR_VGC_COMMANDSTREAM;

	cmdwinaddr = ((target << KGSL_G12_CMDWINDOW_TARGET_SHIFT) &
			KGSL_G12_CMDWINDOW_TARGET_MASK);
	cmdwinaddr |= ((addr << KGSL_G12_CMDWINDOW_ADDR_SHIFT) &
			KGSL_G12_CMDWINDOW_ADDR_MASK);

	kgsl_g12_regwrite(device, cmdstream >> 2, cmdwinaddr);
	kgsl_g12_regwrite(device, cmdstream >> 2, data);

	return 0;
}
