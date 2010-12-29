

#include <mach/dal_axi.h>


#define DALDEVICEID_AXI   0x02000053
#define DALRPC_PORT_NAME  "DAL00"

enum {
	DALRPC_AXI_CONFIGURE_BRIDGE = DALDEVICE_FIRST_DEVICE_API_IDX + 11
};

enum {
	DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_SYNC_MODE = 14,
	DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_ASYNC_MODE,
	DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_ISOSYNC_MODE,
	DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_DEBUG_EN,
	DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_DEBUG_DIS,
	DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_SYNC_MODE,
	DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_ASYNC_MODE,
	DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_ISOSYNC_MODE,
	DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_DEBUG_EN,
	DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_DEBUG_DIS,
};

static int axi_configure_bridge_grfx_sync_mode(int bridge_mode)
{
	int rc;
	void *dev_handle;

	
	rc = daldevice_attach(
		DALDEVICEID_AXI, DALRPC_PORT_NAME,
		DALRPC_DEST_MODEM, &dev_handle
	);
	if (rc) {
		printk(KERN_ERR "%s: failed to attach AXI bus device (%d)\n",
			__func__, rc);
		goto fail_dal_attach_detach;
	}

	
	rc = dalrpc_fcn_0(
		DALRPC_AXI_CONFIGURE_BRIDGE, dev_handle,
		bridge_mode
	);
	if (rc) {
		printk(KERN_ERR "%s: AXI bus device (%d) failed to be configured\n",
			__func__, rc);
		goto fail_dal_fcn_0;
	}

	
	rc = daldevice_detach(dev_handle);
	if (rc) {
		printk(KERN_ERR "%s: failed to detach AXI bus device (%d)\n",
			__func__, rc);
		goto fail_dal_attach_detach;
	}

	return 0;

fail_dal_fcn_0:
	(void)daldevice_detach(dev_handle);
fail_dal_attach_detach:

	return rc;
}



int set_grp2d_async(void)
{
	return axi_configure_bridge_grfx_sync_mode(
		DAL_AXI_BRIDGE_CFG_CGR_SS_2DGRP_ASYNC_MODE);
}

int set_grp3d_async(void)
{
	return axi_configure_bridge_grfx_sync_mode(
		DAL_AXI_BRIDGE_CFG_CGR_SS_3DGRP_ASYNC_MODE);
}
