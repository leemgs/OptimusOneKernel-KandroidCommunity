
#ifndef _VIDEO_720P_RESOURCE_TRACKER_API_H_
#define _VIDEO_720P_RESOURCE_TRACKER_API_H_

#include "vcd_core.h"

void res_trk_init(struct device *device, u32 irq);
u32 res_trk_power_up(void);
u32 res_trk_power_down(void);
u32 res_trk_enable_clocks(void);
u32 res_trk_disable_clocks(void);
u32 res_trk_get_max_perf_level(u32 *pn_max_perf_lvl);
u32 res_trk_set_perf_level(u32 n_req_perf_lvl, u32 *pn_set_perf_lvl,
	struct vcd_clnt_ctxt_type_t *p_cctxt);
u32 res_trk_get_curr_perf_level(u32 *pn_perf_lvl);
u32 res_trk_download_firmware(void);

#endif
