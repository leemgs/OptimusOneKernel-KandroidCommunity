

#include <linux/err.h>
#include <mach/clk.h>

#include "clock.h"
#include "clock-rpm.h"

int rpm_clk_enable(unsigned id)
{
	
	return -EPERM;
}

void rpm_clk_disable(unsigned id)
{
	
}

int rpm_clk_reset(unsigned id, enum clk_reset_action action)
{
	
	return -EPERM;
}

int rpm_clk_set_rate(unsigned id, unsigned rate)
{
	
	return -EPERM;
}

int rpm_clk_set_min_rate(unsigned id, unsigned rate)
{
	 
	if (id == R_EBI1_CLK)
		return 0;

	
	return -EPERM;
}

int rpm_clk_set_max_rate(unsigned id, unsigned rate)
{
	
	return -EPERM;
}

int rpm_clk_set_flags(unsigned id, unsigned flags)
{
	
	return -EPERM;
}

unsigned rpm_clk_get_rate(unsigned id)
{
	
	return 0;
}

signed rpm_clk_measure_rate(unsigned id)
{
	
	return -EPERM;
}

unsigned rpm_clk_is_enabled(unsigned id)
{
	
	return 0;
}

long rpm_clk_round_rate(unsigned id, unsigned rate)
{
	
	return rate;
}

struct clk_ops clk_ops_remote = {
	.enable = rpm_clk_enable,
	.disable = rpm_clk_disable,
	.auto_off = rpm_clk_disable,
	.reset = rpm_clk_reset,
	.set_rate = rpm_clk_set_rate,
	.set_min_rate = rpm_clk_set_min_rate,
	.set_max_rate = rpm_clk_set_max_rate,
	.set_flags = rpm_clk_set_flags,
	.get_rate = rpm_clk_get_rate,
	.measure_rate = rpm_clk_measure_rate,
	.is_enabled = rpm_clk_is_enabled,
	.round_rate = rpm_clk_round_rate,
};
