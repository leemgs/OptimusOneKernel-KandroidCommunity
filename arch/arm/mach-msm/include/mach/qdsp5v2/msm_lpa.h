
#ifndef _MACH_QDSP5_V2_MSM_LPA_H
#define _MACH_QDSP5_V2_MSM_LPA_H

struct lpa_mem_config {
	u32 llb_min_addr;
	u32 llb_max_addr;
	u32 sb_min_addr;
	u32 sb_max_addr;
};

struct msm_lpa_platform_data {
	u32 obuf_hlb_size;
	u32 dsp_proc_id;
	u32 app_proc_id;
	struct lpa_mem_config nosb_config; 
	struct lpa_mem_config sb_config; 
};

#endif
