
#undef DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/limits.h>
#include <linux/bitops.h>

#include <mach/cpu.h>
#include <mach/clock.h>
#include <mach/sram.h>
#include <asm/div64.h>
#include <asm/clkdev.h>

#include <mach/sdrc.h>
#include "clock.h"
#include "prm.h"
#include "prm-regbits-34xx.h"
#include "cm.h"
#include "cm-regbits-34xx.h"

static const struct clkops clkops_noncore_dpll_ops;

static void omap3430es2_clk_ssi_find_idlest(struct clk *clk,
					    void __iomem **idlest_reg,
					    u8 *idlest_bit);
static void omap3430es2_clk_hsotgusb_find_idlest(struct clk *clk,
					    void __iomem **idlest_reg,
					    u8 *idlest_bit);
static void omap3430es2_clk_dss_usbhost_find_idlest(struct clk *clk,
						    void __iomem **idlest_reg,
						    u8 *idlest_bit);

static const struct clkops clkops_omap3430es2_ssi_wait = {
	.enable		= omap2_dflt_clk_enable,
	.disable	= omap2_dflt_clk_disable,
	.find_idlest	= omap3430es2_clk_ssi_find_idlest,
	.find_companion = omap2_clk_dflt_find_companion,
};

static const struct clkops clkops_omap3430es2_hsotgusb_wait = {
	.enable		= omap2_dflt_clk_enable,
	.disable	= omap2_dflt_clk_disable,
	.find_idlest	= omap3430es2_clk_hsotgusb_find_idlest,
	.find_companion = omap2_clk_dflt_find_companion,
};

static const struct clkops clkops_omap3430es2_dss_usbhost_wait = {
	.enable		= omap2_dflt_clk_enable,
	.disable	= omap2_dflt_clk_disable,
	.find_idlest	= omap3430es2_clk_dss_usbhost_find_idlest,
	.find_companion = omap2_clk_dflt_find_companion,
};

#include "clock34xx.h"

struct omap_clk {
	u32		cpu;
	struct clk_lookup lk;
};

#define CLK(dev, con, ck, cp) 		\
	{				\
		 .cpu = cp,		\
		.lk = {			\
			.dev_id = dev,	\
			.con_id = con,	\
			.clk = ck,	\
		},			\
	}

#define CK_343X		(1 << 0)
#define CK_3430ES1	(1 << 1)
#define CK_3430ES2	(1 << 2)

static struct omap_clk omap34xx_clks[] = {
	CLK(NULL,	"omap_32k_fck",	&omap_32k_fck,	CK_343X),
	CLK(NULL,	"virt_12m_ck",	&virt_12m_ck,	CK_343X),
	CLK(NULL,	"virt_13m_ck",	&virt_13m_ck,	CK_343X),
	CLK(NULL,	"virt_16_8m_ck", &virt_16_8m_ck, CK_3430ES2),
	CLK(NULL,	"virt_19_2m_ck", &virt_19_2m_ck, CK_343X),
	CLK(NULL,	"virt_26m_ck",	&virt_26m_ck,	CK_343X),
	CLK(NULL,	"virt_38_4m_ck", &virt_38_4m_ck, CK_343X),
	CLK(NULL,	"osc_sys_ck",	&osc_sys_ck,	CK_343X),
	CLK(NULL,	"sys_ck",	&sys_ck,	CK_343X),
	CLK(NULL,	"sys_altclk",	&sys_altclk,	CK_343X),
	CLK(NULL,	"mcbsp_clks",	&mcbsp_clks,	CK_343X),
	CLK(NULL,	"sys_clkout1",	&sys_clkout1,	CK_343X),
	CLK(NULL,	"dpll1_ck",	&dpll1_ck,	CK_343X),
	CLK(NULL,	"dpll1_x2_ck",	&dpll1_x2_ck,	CK_343X),
	CLK(NULL,	"dpll1_x2m2_ck", &dpll1_x2m2_ck, CK_343X),
	CLK(NULL,	"dpll2_ck",	&dpll2_ck,	CK_343X),
	CLK(NULL,	"dpll2_m2_ck",	&dpll2_m2_ck,	CK_343X),
	CLK(NULL,	"dpll3_ck",	&dpll3_ck,	CK_343X),
	CLK(NULL,	"core_ck",	&core_ck,	CK_343X),
	CLK(NULL,	"dpll3_x2_ck",	&dpll3_x2_ck,	CK_343X),
	CLK(NULL,	"dpll3_m2_ck",	&dpll3_m2_ck,	CK_343X),
	CLK(NULL,	"dpll3_m2x2_ck", &dpll3_m2x2_ck, CK_343X),
	CLK(NULL,	"dpll3_m3_ck",	&dpll3_m3_ck,	CK_343X),
	CLK(NULL,	"dpll3_m3x2_ck", &dpll3_m3x2_ck, CK_343X),
	CLK(NULL,	"emu_core_alwon_ck", &emu_core_alwon_ck, CK_343X),
	CLK(NULL,	"dpll4_ck",	&dpll4_ck,	CK_343X),
	CLK(NULL,	"dpll4_x2_ck",	&dpll4_x2_ck,	CK_343X),
	CLK(NULL,	"omap_96m_alwon_fck", &omap_96m_alwon_fck, CK_343X),
	CLK(NULL,	"omap_96m_fck",	&omap_96m_fck,	CK_343X),
	CLK(NULL,	"cm_96m_fck",	&cm_96m_fck,	CK_343X),
	CLK(NULL,	"omap_54m_fck",	&omap_54m_fck,	CK_343X),
	CLK(NULL,	"omap_48m_fck",	&omap_48m_fck,	CK_343X),
	CLK(NULL,	"omap_12m_fck",	&omap_12m_fck,	CK_343X),
	CLK(NULL,	"dpll4_m2_ck",	&dpll4_m2_ck,	CK_343X),
	CLK(NULL,	"dpll4_m2x2_ck", &dpll4_m2x2_ck, CK_343X),
	CLK(NULL,	"dpll4_m3_ck",	&dpll4_m3_ck,	CK_343X),
	CLK(NULL,	"dpll4_m3x2_ck", &dpll4_m3x2_ck, CK_343X),
	CLK(NULL,	"dpll4_m4_ck",	&dpll4_m4_ck,	CK_343X),
	CLK(NULL,	"dpll4_m4x2_ck", &dpll4_m4x2_ck, CK_343X),
	CLK(NULL,	"dpll4_m5_ck",	&dpll4_m5_ck,	CK_343X),
	CLK(NULL,	"dpll4_m5x2_ck", &dpll4_m5x2_ck, CK_343X),
	CLK(NULL,	"dpll4_m6_ck",	&dpll4_m6_ck,	CK_343X),
	CLK(NULL,	"dpll4_m6x2_ck", &dpll4_m6x2_ck, CK_343X),
	CLK(NULL,	"emu_per_alwon_ck", &emu_per_alwon_ck, CK_343X),
	CLK(NULL,	"dpll5_ck",	&dpll5_ck,	CK_3430ES2),
	CLK(NULL,	"dpll5_m2_ck",	&dpll5_m2_ck,	CK_3430ES2),
	CLK(NULL,	"clkout2_src_ck", &clkout2_src_ck, CK_343X),
	CLK(NULL,	"sys_clkout2",	&sys_clkout2,	CK_343X),
	CLK(NULL,	"corex2_fck",	&corex2_fck,	CK_343X),
	CLK(NULL,	"dpll1_fck",	&dpll1_fck,	CK_343X),
	CLK(NULL,	"mpu_ck",	&mpu_ck,	CK_343X),
	CLK(NULL,	"arm_fck",	&arm_fck,	CK_343X),
	CLK(NULL,	"emu_mpu_alwon_ck", &emu_mpu_alwon_ck, CK_343X),
	CLK(NULL,	"dpll2_fck",	&dpll2_fck,	CK_343X),
	CLK(NULL,	"iva2_ck",	&iva2_ck,	CK_343X),
	CLK(NULL,	"l3_ick",	&l3_ick,	CK_343X),
	CLK(NULL,	"l4_ick",	&l4_ick,	CK_343X),
	CLK(NULL,	"rm_ick",	&rm_ick,	CK_343X),
	CLK(NULL,	"gfx_l3_ck",	&gfx_l3_ck,	CK_3430ES1),
	CLK(NULL,	"gfx_l3_fck",	&gfx_l3_fck,	CK_3430ES1),
	CLK(NULL,	"gfx_l3_ick",	&gfx_l3_ick,	CK_3430ES1),
	CLK(NULL,	"gfx_cg1_ck",	&gfx_cg1_ck,	CK_3430ES1),
	CLK(NULL,	"gfx_cg2_ck",	&gfx_cg2_ck,	CK_3430ES1),
	CLK(NULL,	"sgx_fck",	&sgx_fck,	CK_3430ES2),
	CLK(NULL,	"sgx_ick",	&sgx_ick,	CK_3430ES2),
	CLK(NULL,	"d2d_26m_fck",	&d2d_26m_fck,	CK_3430ES1),
	CLK(NULL,	"modem_fck",	&modem_fck,	CK_343X),
	CLK(NULL,	"sad2d_ick",	&sad2d_ick,	CK_343X),
	CLK(NULL,	"mad2d_ick",	&mad2d_ick,	CK_343X),
	CLK(NULL,	"gpt10_fck",	&gpt10_fck,	CK_343X),
	CLK(NULL,	"gpt11_fck",	&gpt11_fck,	CK_343X),
	CLK(NULL,	"cpefuse_fck",	&cpefuse_fck,	CK_3430ES2),
	CLK(NULL,	"ts_fck",	&ts_fck,	CK_3430ES2),
	CLK(NULL,	"usbtll_fck",	&usbtll_fck,	CK_3430ES2),
	CLK(NULL,	"core_96m_fck",	&core_96m_fck,	CK_343X),
	CLK("mmci-omap-hs.2",	"fck",	&mmchs3_fck,	CK_3430ES2),
	CLK("mmci-omap-hs.1",	"fck",	&mmchs2_fck,	CK_343X),
	CLK(NULL,	"mspro_fck",	&mspro_fck,	CK_343X),
	CLK("mmci-omap-hs.0",	"fck",	&mmchs1_fck,	CK_343X),
	CLK("i2c_omap.3", "fck",	&i2c3_fck,	CK_343X),
	CLK("i2c_omap.2", "fck",	&i2c2_fck,	CK_343X),
	CLK("i2c_omap.1", "fck",	&i2c1_fck,	CK_343X),
	CLK("omap-mcbsp.5", "fck",	&mcbsp5_fck,	CK_343X),
	CLK("omap-mcbsp.1", "fck",	&mcbsp1_fck,	CK_343X),
	CLK(NULL,	"core_48m_fck",	&core_48m_fck,	CK_343X),
	CLK("omap2_mcspi.4", "fck",	&mcspi4_fck,	CK_343X),
	CLK("omap2_mcspi.3", "fck",	&mcspi3_fck,	CK_343X),
	CLK("omap2_mcspi.2", "fck",	&mcspi2_fck,	CK_343X),
	CLK("omap2_mcspi.1", "fck",	&mcspi1_fck,	CK_343X),
	CLK(NULL,	"uart2_fck",	&uart2_fck,	CK_343X),
	CLK(NULL,	"uart1_fck",	&uart1_fck,	CK_343X),
	CLK(NULL,	"fshostusb_fck", &fshostusb_fck, CK_3430ES1),
	CLK(NULL,	"core_12m_fck",	&core_12m_fck,	CK_343X),
	CLK("omap_hdq.0", "fck",	&hdq_fck,	CK_343X),
	CLK(NULL,	"ssi_ssr_fck",	&ssi_ssr_fck_3430es1,	CK_3430ES1),
	CLK(NULL,	"ssi_ssr_fck",	&ssi_ssr_fck_3430es2,	CK_3430ES2),
	CLK(NULL,	"ssi_sst_fck",	&ssi_sst_fck_3430es1,	CK_3430ES1),
	CLK(NULL,	"ssi_sst_fck",	&ssi_sst_fck_3430es2,	CK_3430ES2),
	CLK(NULL,	"core_l3_ick",	&core_l3_ick,	CK_343X),
	CLK("musb_hdrc",	"ick",	&hsotgusb_ick_3430es1,	CK_3430ES1),
	CLK("musb_hdrc",	"ick",	&hsotgusb_ick_3430es2,	CK_3430ES2),
	CLK(NULL,	"sdrc_ick",	&sdrc_ick,	CK_343X),
	CLK(NULL,	"gpmc_fck",	&gpmc_fck,	CK_343X),
	CLK(NULL,	"security_l3_ick", &security_l3_ick, CK_343X),
	CLK(NULL,	"pka_ick",	&pka_ick,	CK_343X),
	CLK(NULL,	"core_l4_ick",	&core_l4_ick,	CK_343X),
	CLK(NULL,	"usbtll_ick",	&usbtll_ick,	CK_3430ES2),
	CLK("mmci-omap-hs.2",	"ick",	&mmchs3_ick,	CK_3430ES2),
	CLK(NULL,	"icr_ick",	&icr_ick,	CK_343X),
	CLK(NULL,	"aes2_ick",	&aes2_ick,	CK_343X),
	CLK(NULL,	"sha12_ick",	&sha12_ick,	CK_343X),
	CLK(NULL,	"des2_ick",	&des2_ick,	CK_343X),
	CLK("mmci-omap-hs.1",	"ick",	&mmchs2_ick,	CK_343X),
	CLK("mmci-omap-hs.0",	"ick",	&mmchs1_ick,	CK_343X),
	CLK(NULL,	"mspro_ick",	&mspro_ick,	CK_343X),
	CLK("omap_hdq.0", "ick",	&hdq_ick,	CK_343X),
	CLK("omap2_mcspi.4", "ick",	&mcspi4_ick,	CK_343X),
	CLK("omap2_mcspi.3", "ick",	&mcspi3_ick,	CK_343X),
	CLK("omap2_mcspi.2", "ick",	&mcspi2_ick,	CK_343X),
	CLK("omap2_mcspi.1", "ick",	&mcspi1_ick,	CK_343X),
	CLK("i2c_omap.3", "ick",	&i2c3_ick,	CK_343X),
	CLK("i2c_omap.2", "ick",	&i2c2_ick,	CK_343X),
	CLK("i2c_omap.1", "ick",	&i2c1_ick,	CK_343X),
	CLK(NULL,	"uart2_ick",	&uart2_ick,	CK_343X),
	CLK(NULL,	"uart1_ick",	&uart1_ick,	CK_343X),
	CLK(NULL,	"gpt11_ick",	&gpt11_ick,	CK_343X),
	CLK(NULL,	"gpt10_ick",	&gpt10_ick,	CK_343X),
	CLK("omap-mcbsp.5", "ick",	&mcbsp5_ick,	CK_343X),
	CLK("omap-mcbsp.1", "ick",	&mcbsp1_ick,	CK_343X),
	CLK(NULL,	"fac_ick",	&fac_ick,	CK_3430ES1),
	CLK(NULL,	"mailboxes_ick", &mailboxes_ick, CK_343X),
	CLK(NULL,	"omapctrl_ick",	&omapctrl_ick,	CK_343X),
	CLK(NULL,	"ssi_l4_ick",	&ssi_l4_ick,	CK_343X),
	CLK(NULL,	"ssi_ick",	&ssi_ick_3430es1,	CK_3430ES1),
	CLK(NULL,	"ssi_ick",	&ssi_ick_3430es2,	CK_3430ES2),
	CLK(NULL,	"usb_l4_ick",	&usb_l4_ick,	CK_3430ES1),
	CLK(NULL,	"security_l4_ick2", &security_l4_ick2, CK_343X),
	CLK(NULL,	"aes1_ick",	&aes1_ick,	CK_343X),
	CLK("omap_rng",	"ick",		&rng_ick,	CK_343X),
	CLK(NULL,	"sha11_ick",	&sha11_ick,	CK_343X),
	CLK(NULL,	"des1_ick",	&des1_ick,	CK_343X),
	CLK("omapfb",	"dss1_fck",	&dss1_alwon_fck_3430es1, CK_3430ES1),
	CLK("omapfb",	"dss1_fck",	&dss1_alwon_fck_3430es2, CK_3430ES2),
	CLK("omapfb",	"tv_fck",	&dss_tv_fck,	CK_343X),
	CLK("omapfb",	"video_fck",	&dss_96m_fck,	CK_343X),
	CLK("omapfb",	"dss2_fck",	&dss2_alwon_fck, CK_343X),
	CLK("omapfb",	"ick",		&dss_ick_3430es1,	CK_3430ES1),
	CLK("omapfb",	"ick",		&dss_ick_3430es2,	CK_3430ES2),
	CLK(NULL,	"cam_mclk",	&cam_mclk,	CK_343X),
	CLK(NULL,	"cam_ick",	&cam_ick,	CK_343X),
	CLK(NULL,	"csi2_96m_fck",	&csi2_96m_fck,	CK_343X),
	CLK(NULL,	"usbhost_120m_fck", &usbhost_120m_fck, CK_3430ES2),
	CLK(NULL,	"usbhost_48m_fck", &usbhost_48m_fck, CK_3430ES2),
	CLK(NULL,	"usbhost_ick",	&usbhost_ick,	CK_3430ES2),
	CLK(NULL,	"usim_fck",	&usim_fck,	CK_3430ES2),
	CLK(NULL,	"gpt1_fck",	&gpt1_fck,	CK_343X),
	CLK(NULL,	"wkup_32k_fck",	&wkup_32k_fck,	CK_343X),
	CLK(NULL,	"gpio1_dbck",	&gpio1_dbck,	CK_343X),
	CLK("omap_wdt",	"fck",		&wdt2_fck,	CK_343X),
	CLK(NULL,	"wkup_l4_ick",	&wkup_l4_ick,	CK_343X),
	CLK(NULL,	"usim_ick",	&usim_ick,	CK_3430ES2),
	CLK("omap_wdt",	"ick",		&wdt2_ick,	CK_343X),
	CLK(NULL,	"wdt1_ick",	&wdt1_ick,	CK_343X),
	CLK(NULL,	"gpio1_ick",	&gpio1_ick,	CK_343X),
	CLK(NULL,	"omap_32ksync_ick", &omap_32ksync_ick, CK_343X),
	CLK(NULL,	"gpt12_ick",	&gpt12_ick,	CK_343X),
	CLK(NULL,	"gpt1_ick",	&gpt1_ick,	CK_343X),
	CLK(NULL,	"per_96m_fck",	&per_96m_fck,	CK_343X),
	CLK(NULL,	"per_48m_fck",	&per_48m_fck,	CK_343X),
	CLK(NULL,	"uart3_fck",	&uart3_fck,	CK_343X),
	CLK(NULL,	"gpt2_fck",	&gpt2_fck,	CK_343X),
	CLK(NULL,	"gpt3_fck",	&gpt3_fck,	CK_343X),
	CLK(NULL,	"gpt4_fck",	&gpt4_fck,	CK_343X),
	CLK(NULL,	"gpt5_fck",	&gpt5_fck,	CK_343X),
	CLK(NULL,	"gpt6_fck",	&gpt6_fck,	CK_343X),
	CLK(NULL,	"gpt7_fck",	&gpt7_fck,	CK_343X),
	CLK(NULL,	"gpt8_fck",	&gpt8_fck,	CK_343X),
	CLK(NULL,	"gpt9_fck",	&gpt9_fck,	CK_343X),
	CLK(NULL,	"per_32k_alwon_fck", &per_32k_alwon_fck, CK_343X),
	CLK(NULL,	"gpio6_dbck",	&gpio6_dbck,	CK_343X),
	CLK(NULL,	"gpio5_dbck",	&gpio5_dbck,	CK_343X),
	CLK(NULL,	"gpio4_dbck",	&gpio4_dbck,	CK_343X),
	CLK(NULL,	"gpio3_dbck",	&gpio3_dbck,	CK_343X),
	CLK(NULL,	"gpio2_dbck",	&gpio2_dbck,	CK_343X),
	CLK(NULL,	"wdt3_fck",	&wdt3_fck,	CK_343X),
	CLK(NULL,	"per_l4_ick",	&per_l4_ick,	CK_343X),
	CLK(NULL,	"gpio6_ick",	&gpio6_ick,	CK_343X),
	CLK(NULL,	"gpio5_ick",	&gpio5_ick,	CK_343X),
	CLK(NULL,	"gpio4_ick",	&gpio4_ick,	CK_343X),
	CLK(NULL,	"gpio3_ick",	&gpio3_ick,	CK_343X),
	CLK(NULL,	"gpio2_ick",	&gpio2_ick,	CK_343X),
	CLK(NULL,	"wdt3_ick",	&wdt3_ick,	CK_343X),
	CLK(NULL,	"uart3_ick",	&uart3_ick,	CK_343X),
	CLK(NULL,	"gpt9_ick",	&gpt9_ick,	CK_343X),
	CLK(NULL,	"gpt8_ick",	&gpt8_ick,	CK_343X),
	CLK(NULL,	"gpt7_ick",	&gpt7_ick,	CK_343X),
	CLK(NULL,	"gpt6_ick",	&gpt6_ick,	CK_343X),
	CLK(NULL,	"gpt5_ick",	&gpt5_ick,	CK_343X),
	CLK(NULL,	"gpt4_ick",	&gpt4_ick,	CK_343X),
	CLK(NULL,	"gpt3_ick",	&gpt3_ick,	CK_343X),
	CLK(NULL,	"gpt2_ick",	&gpt2_ick,	CK_343X),
	CLK("omap-mcbsp.2", "ick",	&mcbsp2_ick,	CK_343X),
	CLK("omap-mcbsp.3", "ick",	&mcbsp3_ick,	CK_343X),
	CLK("omap-mcbsp.4", "ick",	&mcbsp4_ick,	CK_343X),
	CLK("omap-mcbsp.2", "fck",	&mcbsp2_fck,	CK_343X),
	CLK("omap-mcbsp.3", "fck",	&mcbsp3_fck,	CK_343X),
	CLK("omap-mcbsp.4", "fck",	&mcbsp4_fck,	CK_343X),
	CLK(NULL,	"emu_src_ck",	&emu_src_ck,	CK_343X),
	CLK(NULL,	"pclk_fck",	&pclk_fck,	CK_343X),
	CLK(NULL,	"pclkx2_fck",	&pclkx2_fck,	CK_343X),
	CLK(NULL,	"atclk_fck",	&atclk_fck,	CK_343X),
	CLK(NULL,	"traceclk_src_fck", &traceclk_src_fck, CK_343X),
	CLK(NULL,	"traceclk_fck",	&traceclk_fck,	CK_343X),
	CLK(NULL,	"sr1_fck",	&sr1_fck,	CK_343X),
	CLK(NULL,	"sr2_fck",	&sr2_fck,	CK_343X),
	CLK(NULL,	"sr_l4_ick",	&sr_l4_ick,	CK_343X),
	CLK(NULL,	"secure_32k_fck", &secure_32k_fck, CK_343X),
	CLK(NULL,	"gpt12_fck",	&gpt12_fck,	CK_343X),
	CLK(NULL,	"wdt1_fck",	&wdt1_fck,	CK_343X),
};


#define DPLL_AUTOIDLE_DISABLE			0x0
#define DPLL_AUTOIDLE_LOW_POWER_STOP		0x1

#define MAX_DPLL_WAIT_TRIES		1000000

#define MIN_SDRC_DLL_LOCK_FREQ		83000000

#define CYCLES_PER_MHZ			1000000


#define SDRC_MPURATE_SCALE		8


#define SDRC_MPURATE_BASE_SHIFT		9


#define SDRC_MPURATE_LOOPS		96


#define DPLL5_FREQ_FOR_USBHOST		120000000


static void omap3430es2_clk_ssi_find_idlest(struct clk *clk,
					    void __iomem **idlest_reg,
					    u8 *idlest_bit)
{
	u32 r;

	r = (((__force u32)clk->enable_reg & ~0xf0) | 0x20);
	*idlest_reg = (__force void __iomem *)r;
	*idlest_bit = OMAP3430ES2_ST_SSI_IDLE_SHIFT;
}


static void omap3430es2_clk_dss_usbhost_find_idlest(struct clk *clk,
						    void __iomem **idlest_reg,
						    u8 *idlest_bit)
{
	u32 r;

	r = (((__force u32)clk->enable_reg & ~0xf0) | 0x20);
	*idlest_reg = (__force void __iomem *)r;
	
	*idlest_bit = OMAP3430ES2_ST_DSS_IDLE_SHIFT;
}


static void omap3430es2_clk_hsotgusb_find_idlest(struct clk *clk,
						 void __iomem **idlest_reg,
						 u8 *idlest_bit)
{
	u32 r;

	r = (((__force u32)clk->enable_reg & ~0xf0) | 0x20);
	*idlest_reg = (__force void __iomem *)r;
	*idlest_bit = OMAP3430ES2_ST_HSOTGUSB_IDLE_SHIFT;
}


static unsigned long omap3_dpll_recalc(struct clk *clk)
{
	return omap2_get_dpll_rate(clk);
}


static void _omap3_dpll_write_clken(struct clk *clk, u8 clken_bits)
{
	const struct dpll_data *dd;
	u32 v;

	dd = clk->dpll_data;

	v = __raw_readl(dd->control_reg);
	v &= ~dd->enable_mask;
	v |= clken_bits << __ffs(dd->enable_mask);
	__raw_writel(v, dd->control_reg);
}


static int _omap3_wait_dpll_status(struct clk *clk, u8 state)
{
	const struct dpll_data *dd;
	int i = 0;
	int ret = -EINVAL;

	dd = clk->dpll_data;

	state <<= __ffs(dd->idlest_mask);

	while (((__raw_readl(dd->idlest_reg) & dd->idlest_mask) != state) &&
	       i < MAX_DPLL_WAIT_TRIES) {
		i++;
		udelay(1);
	}

	if (i == MAX_DPLL_WAIT_TRIES) {
		printk(KERN_ERR "clock: %s failed transition to '%s'\n",
		       clk->name, (state) ? "locked" : "bypassed");
	} else {
		pr_debug("clock: %s transition to '%s' in %d loops\n",
			 clk->name, (state) ? "locked" : "bypassed", i);

		ret = 0;
	}

	return ret;
}


static u16 _omap3_dpll_compute_freqsel(struct clk *clk, u8 n)
{
	unsigned long fint;
	u16 f = 0;

	fint = clk->dpll_data->clk_ref->rate / n;

	pr_debug("clock: fint is %lu\n", fint);

	if (fint >= 750000 && fint <= 1000000)
		f = 0x3;
	else if (fint > 1000000 && fint <= 1250000)
		f = 0x4;
	else if (fint > 1250000 && fint <= 1500000)
		f = 0x5;
	else if (fint > 1500000 && fint <= 1750000)
		f = 0x6;
	else if (fint > 1750000 && fint <= 2100000)
		f = 0x7;
	else if (fint > 7500000 && fint <= 10000000)
		f = 0xB;
	else if (fint > 10000000 && fint <= 12500000)
		f = 0xC;
	else if (fint > 12500000 && fint <= 15000000)
		f = 0xD;
	else if (fint > 15000000 && fint <= 17500000)
		f = 0xE;
	else if (fint > 17500000 && fint <= 21000000)
		f = 0xF;
	else
		pr_debug("clock: unknown freqsel setting for %d\n", n);

	return f;
}




static int _omap3_noncore_dpll_lock(struct clk *clk)
{
	u8 ai;
	int r;

	if (clk == &dpll3_ck)
		return -EINVAL;

	pr_debug("clock: locking DPLL %s\n", clk->name);

	ai = omap3_dpll_autoidle_read(clk);

	omap3_dpll_deny_idle(clk);

	_omap3_dpll_write_clken(clk, DPLL_LOCKED);

	r = _omap3_wait_dpll_status(clk, 1);

	if (ai)
		omap3_dpll_allow_idle(clk);

	return r;
}


static int _omap3_noncore_dpll_bypass(struct clk *clk)
{
	int r;
	u8 ai;

	if (clk == &dpll3_ck)
		return -EINVAL;

	if (!(clk->dpll_data->modes & (1 << DPLL_LOW_POWER_BYPASS)))
		return -EINVAL;

	pr_debug("clock: configuring DPLL %s for low-power bypass\n",
		 clk->name);

	ai = omap3_dpll_autoidle_read(clk);

	_omap3_dpll_write_clken(clk, DPLL_LOW_POWER_BYPASS);

	r = _omap3_wait_dpll_status(clk, 0);

	if (ai)
		omap3_dpll_allow_idle(clk);
	else
		omap3_dpll_deny_idle(clk);

	return r;
}


static int _omap3_noncore_dpll_stop(struct clk *clk)
{
	u8 ai;

	if (clk == &dpll3_ck)
		return -EINVAL;

	if (!(clk->dpll_data->modes & (1 << DPLL_LOW_POWER_STOP)))
		return -EINVAL;

	pr_debug("clock: stopping DPLL %s\n", clk->name);

	ai = omap3_dpll_autoidle_read(clk);

	_omap3_dpll_write_clken(clk, DPLL_LOW_POWER_STOP);

	if (ai)
		omap3_dpll_allow_idle(clk);
	else
		omap3_dpll_deny_idle(clk);

	return 0;
}


static int omap3_noncore_dpll_enable(struct clk *clk)
{
	int r;
	struct dpll_data *dd;

	if (clk == &dpll3_ck)
		return -EINVAL;

	dd = clk->dpll_data;
	if (!dd)
		return -EINVAL;

	if (clk->rate == dd->clk_bypass->rate) {
		WARN_ON(clk->parent != dd->clk_bypass);
		r = _omap3_noncore_dpll_bypass(clk);
	} else {
		WARN_ON(clk->parent != dd->clk_ref);
		r = _omap3_noncore_dpll_lock(clk);
	}
	
	if (!r)
		clk->rate = omap2_get_dpll_rate(clk);

	return r;
}


static void omap3_noncore_dpll_disable(struct clk *clk)
{
	if (clk == &dpll3_ck)
		return;

	_omap3_noncore_dpll_stop(clk);
}





static int omap3_noncore_dpll_program(struct clk *clk, u16 m, u8 n, u16 freqsel)
{
	struct dpll_data *dd = clk->dpll_data;
	u32 v;

	
	_omap3_noncore_dpll_bypass(clk);

	
	v = __raw_readl(dd->control_reg);
	v &= ~dd->freqsel_mask;
	v |= freqsel << __ffs(dd->freqsel_mask);
	__raw_writel(v, dd->control_reg);

	
	v = __raw_readl(dd->mult_div1_reg);
	v &= ~(dd->mult_mask | dd->div1_mask);
	v |= m << __ffs(dd->mult_mask);
	v |= (n - 1) << __ffs(dd->div1_mask);
	__raw_writel(v, dd->mult_div1_reg);

	

	

	_omap3_noncore_dpll_lock(clk);

	return 0;
}


static int omap3_noncore_dpll_set_rate(struct clk *clk, unsigned long rate)
{
	struct clk *new_parent = NULL;
	u16 freqsel;
	struct dpll_data *dd;
	int ret;

	if (!clk || !rate)
		return -EINVAL;

	dd = clk->dpll_data;
	if (!dd)
		return -EINVAL;

	if (rate == omap2_get_dpll_rate(clk))
		return 0;

	
	omap2_clk_enable(dd->clk_bypass);
	omap2_clk_enable(dd->clk_ref);

	if (dd->clk_bypass->rate == rate &&
	    (clk->dpll_data->modes & (1 << DPLL_LOW_POWER_BYPASS))) {
		pr_debug("clock: %s: set rate: entering bypass.\n", clk->name);

		ret = _omap3_noncore_dpll_bypass(clk);
		if (!ret)
			new_parent = dd->clk_bypass;
	} else {
		if (dd->last_rounded_rate != rate)
			omap2_dpll_round_rate(clk, rate);

		if (dd->last_rounded_rate == 0)
			return -EINVAL;

		freqsel = _omap3_dpll_compute_freqsel(clk, dd->last_rounded_n);
		if (!freqsel)
			WARN_ON(1);

		pr_debug("clock: %s: set rate: locking rate to %lu.\n",
			 clk->name, rate);

		ret = omap3_noncore_dpll_program(clk, dd->last_rounded_m,
						 dd->last_rounded_n, freqsel);
		if (!ret)
			new_parent = dd->clk_ref;
	}
	if (!ret) {
		
		if (clk->usecount) {
			omap2_clk_enable(new_parent);
			omap2_clk_disable(clk->parent);
		}
		clk_reparent(clk, new_parent);
		clk->rate = rate;
	}
	omap2_clk_disable(dd->clk_ref);
	omap2_clk_disable(dd->clk_bypass);

	return 0;
}

static int omap3_dpll4_set_rate(struct clk *clk, unsigned long rate)
{
	
	if (omap_rev() == OMAP3430_REV_ES1_0) {
		printk(KERN_ERR "clock: DPLL4 cannot change rate due to "
		       "silicon 'Limitation 2.5' on 3430ES1.\n");
		return -EINVAL;
	}
	return omap3_noncore_dpll_set_rate(clk, rate);
}





static int omap3_core_dpll_m2_set_rate(struct clk *clk, unsigned long rate)
{
	u32 new_div = 0;
	u32 unlock_dll = 0;
	u32 c;
	unsigned long validrate, sdrcrate, mpurate;
	struct omap_sdrc_params *sdrc_cs0;
	struct omap_sdrc_params *sdrc_cs1;
	int ret;

	if (!clk || !rate)
		return -EINVAL;

	if (clk != &dpll3_m2_ck)
		return -EINVAL;

	validrate = omap2_clksel_round_rate_div(clk, rate, &new_div);
	if (validrate != rate)
		return -EINVAL;

	sdrcrate = sdrc_ick.rate;
	if (rate > clk->rate)
		sdrcrate <<= ((rate / clk->rate) >> 1);
	else
		sdrcrate >>= ((clk->rate / rate) >> 1);

	ret = omap2_sdrc_get_params(sdrcrate, &sdrc_cs0, &sdrc_cs1);
	if (ret)
		return -EINVAL;

	if (sdrcrate < MIN_SDRC_DLL_LOCK_FREQ) {
		pr_debug("clock: will unlock SDRC DLL\n");
		unlock_dll = 1;
	}

	
	mpurate = arm_fck.rate / CYCLES_PER_MHZ;
	c = (mpurate << SDRC_MPURATE_SCALE) >> SDRC_MPURATE_BASE_SHIFT;
	c += 1;  
	c *= SDRC_MPURATE_LOOPS;
	c >>= SDRC_MPURATE_SCALE;
	if (c == 0)
		c = 1;

	pr_debug("clock: changing CORE DPLL rate from %lu to %lu\n", clk->rate,
		 validrate);
	pr_debug("clock: SDRC CS0 timing params used:"
		 " RFR %08x CTRLA %08x CTRLB %08x MR %08x\n",
		 sdrc_cs0->rfr_ctrl, sdrc_cs0->actim_ctrla,
		 sdrc_cs0->actim_ctrlb, sdrc_cs0->mr);
	if (sdrc_cs1)
		pr_debug("clock: SDRC CS1 timing params used: "
		 " RFR %08x CTRLA %08x CTRLB %08x MR %08x\n",
		 sdrc_cs1->rfr_ctrl, sdrc_cs1->actim_ctrla,
		 sdrc_cs1->actim_ctrlb, sdrc_cs1->mr);

	if (sdrc_cs1)
		omap3_configure_core_dpll(
				  new_div, unlock_dll, c, rate > clk->rate,
				  sdrc_cs0->rfr_ctrl, sdrc_cs0->actim_ctrla,
				  sdrc_cs0->actim_ctrlb, sdrc_cs0->mr,
				  sdrc_cs1->rfr_ctrl, sdrc_cs1->actim_ctrla,
				  sdrc_cs1->actim_ctrlb, sdrc_cs1->mr);
	else
		omap3_configure_core_dpll(
				  new_div, unlock_dll, c, rate > clk->rate,
				  sdrc_cs0->rfr_ctrl, sdrc_cs0->actim_ctrla,
				  sdrc_cs0->actim_ctrlb, sdrc_cs0->mr,
				  0, 0, 0, 0);

	return 0;
}


static const struct clkops clkops_noncore_dpll_ops = {
	.enable		= &omap3_noncore_dpll_enable,
	.disable	= &omap3_noncore_dpll_disable,
};





static u32 omap3_dpll_autoidle_read(struct clk *clk)
{
	const struct dpll_data *dd;
	u32 v;

	if (!clk || !clk->dpll_data)
		return -EINVAL;

	dd = clk->dpll_data;

	v = __raw_readl(dd->autoidle_reg);
	v &= dd->autoidle_mask;
	v >>= __ffs(dd->autoidle_mask);

	return v;
}


static void omap3_dpll_allow_idle(struct clk *clk)
{
	const struct dpll_data *dd;
	u32 v;

	if (!clk || !clk->dpll_data)
		return;

	dd = clk->dpll_data;

	
	v = __raw_readl(dd->autoidle_reg);
	v &= ~dd->autoidle_mask;
	v |= DPLL_AUTOIDLE_LOW_POWER_STOP << __ffs(dd->autoidle_mask);
	__raw_writel(v, dd->autoidle_reg);
}


static void omap3_dpll_deny_idle(struct clk *clk)
{
	const struct dpll_data *dd;
	u32 v;

	if (!clk || !clk->dpll_data)
		return;

	dd = clk->dpll_data;

	v = __raw_readl(dd->autoidle_reg);
	v &= ~dd->autoidle_mask;
	v |= DPLL_AUTOIDLE_DISABLE << __ffs(dd->autoidle_mask);
	__raw_writel(v, dd->autoidle_reg);
}




static unsigned long omap3_clkoutx2_recalc(struct clk *clk)
{
	const struct dpll_data *dd;
	unsigned long rate;
	u32 v;
	struct clk *pclk;

	
	pclk = clk->parent;
	while (pclk && !pclk->dpll_data)
		pclk = pclk->parent;

	
	WARN_ON(!pclk);

	dd = pclk->dpll_data;

	WARN_ON(!dd->enable_mask);

	v = __raw_readl(dd->control_reg) & dd->enable_mask;
	v >>= __ffs(dd->enable_mask);
	if (v != OMAP3XXX_EN_DPLL_LOCKED)
		rate = clk->parent->rate;
	else
		rate = clk->parent->rate * 2;
	return rate;
}




#if defined(CONFIG_ARCH_OMAP3)

static struct clk_functions omap2_clk_functions = {
	.clk_enable		= omap2_clk_enable,
	.clk_disable		= omap2_clk_disable,
	.clk_round_rate		= omap2_clk_round_rate,
	.clk_set_rate		= omap2_clk_set_rate,
	.clk_set_parent		= omap2_clk_set_parent,
	.clk_disable_unused	= omap2_clk_disable_unused,
};


void omap2_clk_prepare_for_reboot(void)
{
	
#if 0
	u32 rate;

	if (vclk == NULL || sclk == NULL)
		return;

	rate = clk_get_rate(sclk);
	clk_set_rate(vclk, rate);
#endif
}

static void omap3_clk_lock_dpll5(void)
{
	struct clk *dpll5_clk;
	struct clk *dpll5_m2_clk;

	dpll5_clk = clk_get(NULL, "dpll5_ck");
	clk_set_rate(dpll5_clk, DPLL5_FREQ_FOR_USBHOST);
	clk_enable(dpll5_clk);

	
	omap3_dpll_allow_idle(dpll5_clk);

	
	dpll5_m2_clk = clk_get(NULL, "dpll5_m2_ck");
	clk_enable(dpll5_m2_clk);
	clk_set_rate(dpll5_m2_clk, DPLL5_FREQ_FOR_USBHOST);

	clk_disable(dpll5_m2_clk);
	clk_disable(dpll5_clk);
	return;
}




static int __init omap2_clk_arch_init(void)
{
	if (!mpurate)
		return -EINVAL;

	
	if (clk_set_rate(&dpll1_ck, mpurate))
		printk(KERN_ERR "*** Unable to set MPU rate\n");

	recalculate_root_clocks();

	printk(KERN_INFO "Switched to new clocking rate (Crystal/Core/MPU): "
	       "%ld.%01ld/%ld/%ld MHz\n",
	       (osc_sys_ck.rate / 1000000), ((osc_sys_ck.rate / 100000) % 10),
	       (core_ck.rate / 1000000), (arm_fck.rate / 1000000)) ;

	calibrate_delay();

	return 0;
}
arch_initcall(omap2_clk_arch_init);

int __init omap2_clk_init(void)
{
	
	struct omap_clk *c;
	
	u32 cpu_clkflg;

	if (cpu_is_omap34xx()) {
		cpu_mask = RATE_IN_343X;
		cpu_clkflg = CK_343X;

		
		if (omap_rev() == OMAP3430_REV_ES1_0) {
			
			cpu_clkflg |= CK_3430ES1;
		} else {
			cpu_mask |= RATE_IN_3430ES2;
			cpu_clkflg |= CK_3430ES2;
		}
	}

	clk_init(&omap2_clk_functions);

	for (c = omap34xx_clks; c < omap34xx_clks + ARRAY_SIZE(omap34xx_clks); c++)
		clk_preinit(c->lk.clk);

	for (c = omap34xx_clks; c < omap34xx_clks + ARRAY_SIZE(omap34xx_clks); c++)
		if (c->cpu & cpu_clkflg) {
			clkdev_add(&c->lk);
			clk_register(c->lk.clk);
			omap2_init_clk_clkdm(c->lk.clk);
		}

	
#if 0
	
	clkrate = omap2_get_dpll_rate_24xx(&dpll_ck);
	for (prcm = rate_table; prcm->mpu_speed; prcm++) {
		if (!(prcm->flags & cpu_mask))
			continue;
		if (prcm->xtal_speed != sys_ck.rate)
			continue;
		if (prcm->dpll_speed <= clkrate)
			 break;
	}
	curr_prcm_set = prcm;
#endif

	recalculate_root_clocks();

	printk(KERN_INFO "Clocking rate (Crystal/Core/MPU): "
	       "%ld.%01ld/%ld/%ld MHz\n",
	       (osc_sys_ck.rate / 1000000), (osc_sys_ck.rate / 100000) % 10,
	       (core_ck.rate / 1000000), (arm_fck.rate / 1000000));

	
	clk_enable_init_clocks();

	
	if (omap_rev() >= OMAP3430_REV_ES2_0)
		omap3_clk_lock_dpll5();

	
	
#if 0
	vclk = clk_get(NULL, "virt_prcm_set");
	sclk = clk_get(NULL, "sys_ck");
#endif
	return 0;
}

#endif
