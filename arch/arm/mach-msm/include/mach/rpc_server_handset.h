

#ifndef __ASM_ARCH_MSM_RPC_SERVER_HANDSET_H
#define __ASM_ARCH_MSM_RPC_SERVER_HANDSET_H

struct msm_handset_platform_data {
	const char *hs_name;
	uint32_t pwr_key_delay_ms; 
};

void report_headset_status(bool connected);

#if defined(CONFIG_MACH_LGE)
void rpc_server_hs_register_callback(void *callback_func);
#endif
#endif
