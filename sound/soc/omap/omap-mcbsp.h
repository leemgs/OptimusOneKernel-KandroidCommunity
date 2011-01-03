

#ifndef __OMAP_I2S_H__
#define __OMAP_I2S_H__


enum omap_mcbsp_clksrg_clk {
	OMAP_MCBSP_SYSCLK_CLKS_FCLK,	
	OMAP_MCBSP_SYSCLK_CLKS_EXT,	
	OMAP_MCBSP_SYSCLK_CLK,		
	OMAP_MCBSP_SYSCLK_CLKX_EXT,	
	OMAP_MCBSP_SYSCLK_CLKR_EXT,	
	OMAP_MCBSP_CLKR_SRC_CLKR,	
	OMAP_MCBSP_CLKR_SRC_CLKX,	
	OMAP_MCBSP_FSR_SRC_FSR,		
	OMAP_MCBSP_FSR_SRC_FSX,		
};


enum omap_mcbsp_div {
	OMAP_MCBSP_CLKGDV,		
};

#if defined(CONFIG_ARCH_OMAP2420)
#define NUM_LINKS	2
#endif
#if defined(CONFIG_ARCH_OMAP15XX) || defined(CONFIG_ARCH_OMAP16XX)
#undef  NUM_LINKS
#define NUM_LINKS	3
#endif
#if defined(CONFIG_ARCH_OMAP2430) || defined(CONFIG_ARCH_OMAP34XX)
#undef  NUM_LINKS
#define NUM_LINKS	5
#endif

extern struct snd_soc_dai omap_mcbsp_dai[NUM_LINKS];

#endif
