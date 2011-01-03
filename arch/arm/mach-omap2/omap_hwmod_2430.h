
#ifndef __ARCH_ARM_PLAT_OMAP_INCLUDE_MACH_OMAP_HWMOD2430_H
#define __ARCH_ARM_PLAT_OMAP_INCLUDE_MACH_OMAP_HWMOD2430_H

#ifdef CONFIG_ARCH_OMAP2430

#include <mach/omap_hwmod.h>
#include <mach/irqs.h>
#include <mach/cpu.h>
#include <mach/dma.h>

#include "prm-regbits-24xx.h"

static struct omap_hwmod omap2430_mpu_hwmod;
static struct omap_hwmod omap2430_l3_hwmod;
static struct omap_hwmod omap2430_l4_core_hwmod;


static struct omap_hwmod_ocp_if omap2430_l3__l4_core = {
	.master	= &omap2430_l3_hwmod,
	.slave	= &omap2430_l4_core_hwmod,
	.user	= OCP_USER_MPU | OCP_USER_SDMA,
};


static struct omap_hwmod_ocp_if omap2430_mpu__l3 = {
	.master = &omap2430_mpu_hwmod,
	.slave	= &omap2430_l3_hwmod,
	.user	= OCP_USER_MPU,
};


static struct omap_hwmod_ocp_if *omap2430_l3_slaves[] = {
	&omap2430_mpu__l3,
};


static struct omap_hwmod_ocp_if *omap2430_l3_masters[] = {
	&omap2430_l3__l4_core,
};


static struct omap_hwmod omap2430_l3_hwmod = {
	.name		= "l3_hwmod",
	.masters	= omap2430_l3_masters,
	.masters_cnt	= ARRAY_SIZE(omap2430_l3_masters),
	.slaves		= omap2430_l3_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap2430_l3_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP2430)
};

static struct omap_hwmod omap2430_l4_wkup_hwmod;
static struct omap_hwmod omap2430_mmc1_hwmod;
static struct omap_hwmod omap2430_mmc2_hwmod;


static struct omap_hwmod_ocp_if omap2430_l4_core__l4_wkup = {
	.master	= &omap2430_l4_core_hwmod,
	.slave	= &omap2430_l4_wkup_hwmod,
	.user	= OCP_USER_MPU | OCP_USER_SDMA,
};


static struct omap_hwmod_ocp_if *omap2430_l4_core_slaves[] = {
	&omap2430_l3__l4_core,
};


static struct omap_hwmod_ocp_if *omap2430_l4_core_masters[] = {
	&omap2430_l4_core__l4_wkup,
};


static struct omap_hwmod omap2430_l4_core_hwmod = {
	.name		= "l4_core_hwmod",
	.masters	= omap2430_l4_core_masters,
	.masters_cnt	= ARRAY_SIZE(omap2430_l4_core_masters),
	.slaves		= omap2430_l4_core_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap2430_l4_core_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP2430)
};


static struct omap_hwmod_ocp_if *omap2430_l4_wkup_slaves[] = {
	&omap2430_l4_core__l4_wkup,
};


static struct omap_hwmod_ocp_if *omap2430_l4_wkup_masters[] = {
};


static struct omap_hwmod omap2430_l4_wkup_hwmod = {
	.name		= "l4_wkup_hwmod",
	.masters	= omap2430_l4_wkup_masters,
	.masters_cnt	= ARRAY_SIZE(omap2430_l4_wkup_masters),
	.slaves		= omap2430_l4_wkup_slaves,
	.slaves_cnt	= ARRAY_SIZE(omap2430_l4_wkup_slaves),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP2430)
};


static struct omap_hwmod_ocp_if *omap2430_mpu_masters[] = {
	&omap2430_mpu__l3,
};


static struct omap_hwmod omap2430_mpu_hwmod = {
	.name		= "mpu_hwmod",
	.clkdev_dev_id	= NULL,
	.clkdev_con_id	= "mpu_ck",
	.masters	= omap2430_mpu_masters,
	.masters_cnt	= ARRAY_SIZE(omap2430_mpu_masters),
	.omap_chip	= OMAP_CHIP_INIT(CHIP_IS_OMAP2430),
};

static __initdata struct omap_hwmod *omap2430_hwmods[] = {
	&omap2430_l3_hwmod,
	&omap2430_l4_core_hwmod,
	&omap2430_l4_wkup_hwmod,
	&omap2430_mpu_hwmod,
	NULL,
};

#else
# define omap2430_hwmods		0
#endif

#endif


