
#include <linux/string.h>
#include <linux/types.h>
#include <linux/msm_kgsl.h>

#include "kgsl_g12_drawctxt.h"
#include "kgsl_sharedmem.h"
#include "kgsl.h"
#include "kgsl_g12.h"
#include "kgsl_log.h"
#include "kgsl_g12_cmdwindow.h"
#include "kgsl_g12_vgv3types.h"
#include "g12_reg.h"

struct kgsl_g12_z1xx g_z1xx = {0};

int
kgsl_g12_drawctxt_create(struct kgsl_device_private *dev_priv,
			uint32_t unused,
			unsigned int *drawctxt_id)
{
	int cmd;
	int result;
	unsigned int ctx_id;
	struct kgsl_device *device = dev_priv->device;

	if (g_z1xx.numcontext == 0) {
		if (kgsl_sharedmem_alloc(0, KGSL_G12_RB_SIZE,
					&g_z1xx.cmdbufdesc) !=  0)
			return -ENOMEM;

		cmd = (int)(((VGV3_NEXTCMD_JUMP) &
			VGV3_NEXTCMD_NEXTCMD_FMASK)
			<< VGV3_NEXTCMD_NEXTCMD_FSHIFT);

		result = kgsl_g12_cmdwindow_write(device, KGSL_CMDWINDOW_2D,
					 ADDR_VGV3_MODE, 4);
		if (result != 0)
			return result;
		result = kgsl_g12_cmdwindow_write(device, KGSL_CMDWINDOW_2D,
					 ADDR_VGV3_NEXTADDR,
					 g_z1xx.cmdbufdesc.physaddr);
		if (result != 0)
			return result;
		result = kgsl_g12_cmdwindow_write(device, KGSL_CMDWINDOW_2D,
					 ADDR_VGV3_NEXTCMD, cmd | 5);
		if (result != 0)
			return result;

		result = kgsl_g12_cmdwindow_write(device, KGSL_CMDWINDOW_2D,
					 ADDR_VGV3_CONTROL, 0);
		if (result != 0)
			return result;
	}
	ctx_id = ffz(dev_priv->ctxt_id_mask);

	g_z1xx.numcontext++;
	if (g_z1xx.numcontext > KGSL_G12_CONTEXT_MAX) {
		*drawctxt_id = 0;
		return KGSL_FAILURE;

	}
	*drawctxt_id = ctx_id;

	return KGSL_SUCCESS;
}

int
kgsl_g12_drawctxt_destroy(struct kgsl_device *device,
			unsigned int drawctxt_id)
{
	struct kgsl_g12_device *g12_device = (struct kgsl_g12_device *) device;

	if (drawctxt_id >= KGSL_G12_CONTEXT_MAX)
		return KGSL_FAILURE;

	if (g_z1xx.numcontext == 0)
		return KGSL_FAILURE;

	g_z1xx.numcontext--;
	if (g_z1xx.numcontext == 0) {
		kgsl_sharedmem_free(&g_z1xx.cmdbufdesc);
		memset(&g_z1xx, 0, sizeof(struct kgsl_g12_z1xx));
		g12_device->timestamp = 0;
		g12_device->current_timestamp = 0;
	}

	return KGSL_SUCCESS;
}
