

#ifndef _INTERNAL_POWER_RAIL_H
#define _INTERNAL_POWER_RAIL_H


#define PWR_RAIL_GRP_CLK	8
#define PWR_RAIL_GRP_2D_CLK	58
#define PWR_RAIL_MDP_CLK	14
#define PWR_RAIL_MFC_CLK	68
#define PWR_RAIL_ROTATOR_CLK	90
#define PWR_RAIL_VDC_CLK	39
#define PWR_RAIL_VFE_CLK	41
#define PWR_RAIL_VPE_CLK	76

enum rail_ctl_mode {
	PWR_RAIL_CTL_AUTO = 0,
	PWR_RAIL_CTL_MANUAL,
};

#ifdef CONFIG_ARCH_MSM8X60
static inline int __maybe_unused internal_pwr_rail_ctl(unsigned rail_id,
						       bool enable)
{
	
	return 0;
}
static inline int __maybe_unused internal_pwr_rail_mode(unsigned rail_id,
							enum rail_ctl_mode mode)
{
	
	return 0;
}
#else
int internal_pwr_rail_ctl(unsigned rail_id, bool enable);
int internal_pwr_rail_mode(unsigned rail_id, enum rail_ctl_mode mode);
#endif

#endif 

