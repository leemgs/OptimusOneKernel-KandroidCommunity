
#ifndef _MSM_TSIF_H_
#define _MSM_TSIF_H_

struct msm_tsif_platform_data {
	int num_gpios;
	const struct msm_gpio *gpios;
	const char *tsif_clk;
	const char *tsif_pclk;
	const char *tsif_ref_clk;
};

#endif 

