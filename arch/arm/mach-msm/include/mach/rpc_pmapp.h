

#ifndef __ASM_ARCH_MSM_RPC_PMAPP_H
#define __ASM_ARCH_MSM_RPC_PMAPP_H

#include <mach/msm_rpcrouter.h>


enum {
	PMAPP_CLOCK_ID_DO = 0,
	PMAPP_CLOCK_ID_D1,
	PMAPP_CLOCK_ID_A0,
	PMAPP_CLOCK_ID_A1,
};


enum {
	PMAPP_CLOCK_VOTE_OFF = 0,
	PMAPP_CLOCK_VOTE_ON,
	PMAPP_CLOCK_VOTE_PIN_CTRL,
};


enum {
	PMAPP_VREG_S3 = 21,
	PMAPP_VREG_S2 = 23,
	PMAPP_VREG_S4 = 24,
};


enum {
	PMAPP_SMPS_CLK_VOTE_DONTCARE = 0,
	PMAPP_SMPS_CLK_VOTE_2P74,	
	PMAPP_SMPS_CLK_VOTE_1P6,	
};


enum {
	PMAPP_SMPS_MODE_VOTE_DONTCARE = 0,
	PMAPP_SMPS_MODE_VOTE_PWM,
	PMAPP_SMPS_MODE_VOTE_PFM,
	PMAPP_SMPS_MODE_VOTE_AUTO
};

int msm_pm_app_rpc_init(void);
void msm_pm_app_rpc_deinit(void);
int msm_pm_app_register_vbus_sn(void (*callback)(int online));
void msm_pm_app_unregister_vbus_sn(void (*callback)(int online));
int msm_pm_app_enable_usb_ldo(int);

int pmapp_display_clock_config(uint enable);

int pmapp_clock_vote(const char *voter_id, uint clock_id, uint vote);
int pmapp_smps_clock_vote(const char *voter_id, uint vreg_id, uint vote);
int pmapp_vreg_level_vote(const char *voter_id, uint vreg_id, uint level);
int pmapp_smps_mode_vote(const char *voter_id, uint vreg_id, uint mode);

#endif
