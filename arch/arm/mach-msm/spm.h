

#ifndef __ARCH_ARM_MACH_MSM_SPM_H
#define __ARCH_ARM_MACH_MSM_SPM_H

enum {
	MSM_SPM_MODE_CLOCK_GATING,
	MSM_SPM_MODE_POWER_RETENTION,
	MSM_SPM_MODE_POWER_COLLAPSE,
	MSM_SPM_MODE_NR
};

enum {
	MSM_SPM_REG_SAW_CFG,
	MSM_SPM_REG_SAW_SPM_CTL,
	MSM_SPM_REG_SAW_SPM_SLP_TMR_DLY,
	MSM_SPM_REG_SAW_SPM_WAKE_TMR_DLY,
	MSM_SPM_REG_SAW_SLP_CLK_EN,
	MSM_SPM_REG_SAW_SLP_HSFS_PRECLMP_EN,
	MSM_SPM_REG_SAW_SLP_HSFS_POSTCLMP_EN,
	MSM_SPM_REG_SAW_SLP_CLMP_EN,
	MSM_SPM_REG_SAW_SLP_RST_EN,
	MSM_SPM_REG_SAW_SPM_MPM_CFG,
	MSM_SPM_REG_NR_INITIALIZE,

	MSM_SPM_REG_SAW_VCTL = MSM_SPM_REG_NR_INITIALIZE,
	MSM_SPM_REG_SAW_STS,
	MSM_SPM_REG_SAW_SPM_PMIC_CTL,
	MSM_SPM_REG_NR
};

struct msm_spm_platform_data {
	void __iomem *reg_base_addr;
	uint32_t reg_init_values[MSM_SPM_REG_NR_INITIALIZE];

	uint8_t awake_vlevel;
	uint8_t retention_vlevel;
	uint8_t collapse_vlevel;
	uint8_t retention_mid_vlevel;
	uint8_t collapse_mid_vlevel;

	uint32_t vctl_timeout_us;
};

#if defined(CONFIG_ARCH_MSM7X30) || defined(CONFIG_ARCH_MSM8X60)

int msm_spm_set_low_power_mode(unsigned int mode, bool notify_rpm);
int msm_spm_set_vdd(unsigned int vlevel);
void msm_spm_reinit(void);
int msm_spm_init(struct msm_spm_platform_data *data, int nr_devs);

#else

static inline int msm_spm_set_low_power_mode(unsigned int mode, bool notify_rpm)
{
	return -ENOSYS;
}

static inline int msm_spm_set_vdd(unsigned int vlevel)
{
	return -ENOSYS;
}

static inline void msm_spm_reinit(void)
{
	
}

static inline int msm_spm_init(struct msm_spm_platform_data *data, int nr_devs)
{
	return -ENOSYS;
}

#endif  

#endif  
