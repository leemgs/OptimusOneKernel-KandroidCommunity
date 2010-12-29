

#ifndef __MSM_PM_QOS_H__
#define __MSM_PM_QOS_H__

#include <linux/pm_qos_params.h>

int msm_pm_qos_add(struct pm_qos_object *class, char *request_name,
	s32 value, void **request_data);
int msm_pm_qos_update(struct pm_qos_object *class, char *request_name,
	s32 value, void **request_data);
int msm_pm_qos_remove(struct pm_qos_object *class, char *request_name,
	s32 value, void **request_data);

#endif 

