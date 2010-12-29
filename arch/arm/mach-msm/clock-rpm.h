

#ifndef __ARCH_ARM_MACH_MSM_CLOCK_RPM_H
#define __ARCH_ARM_MACH_MSM_CLOCK_RPM_H


#define R_EBI1_CLK	0

struct clk_ops;
extern struct clk_ops clk_ops_remote;

#define CLK_RPM(clk_name, clk_id, clk_dev, clk_flags) {	\
	.name = clk_name, \
	.id = R_##clk_id, \
	.remote_id = R_##clk_id, \
	.ops = &clk_ops_remote, \
	.flags = clk_flags, \
	.dev = clk_dev, \
	.dbg_name = #clk_id, \
	}

#endif
