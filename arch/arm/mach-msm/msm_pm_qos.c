

#include <linux/pm_qos_params.h>
#include <linux/err.h>
#include <mach/msm_reqs.h>

int msm_pm_qos_add(struct pm_qos_object *class, char *request_name,
		   s32 value, void **request_data)
{
	char *resource_name = class->plugin->data;

	
	BUG_ON(value != class->default_value);

	
	*request_data = msm_req_add(resource_name, request_name);
	if (IS_ERR(*request_data))
		return PTR_ERR(*request_data);

	return 0;
}

int msm_pm_qos_update(struct pm_qos_object *class, char *request_name,
		      s32 value, void **request_data)
{
	return msm_req_update(*request_data, value);
}

int msm_pm_qos_remove(struct pm_qos_object *class, char *request_name,
		      s32 value, void **request_data)
{
	return msm_req_remove(*request_data);
}

