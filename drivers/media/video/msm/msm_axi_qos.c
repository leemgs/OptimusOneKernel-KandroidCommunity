

#include <linux/pm_qos_params.h>
#include <mach/camera.h>
#define MSM_AXI_QOS_NAME "msm_camera"


int add_axi_qos(void)
{
	int rc = 0;

	rc = pm_qos_add_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_AXI_QOS_NAME, PM_QOS_DEFAULT_VALUE);
	if (rc < 0)
		CDBG("request AXI bus QOS fails. rc = %d\n", rc);
	return rc;
}

int update_axi_qos(uint32_t rate)
{
	int rc = 0;
	rc = pm_qos_update_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_AXI_QOS_NAME, rate);
	if (rc < 0)
		CDBG("update AXI bus QOS fails. rc = %d\n", rc);
	return rc;
}

void release_axi_qos(void)
{
	pm_qos_remove_requirement(PM_QOS_SYSTEM_BUS_FREQ,
		MSM_AXI_QOS_NAME);
}
