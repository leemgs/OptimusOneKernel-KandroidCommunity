

#ifndef __ASM__ARCH_OEM_RAPI_CLIENT_H
#define __ASM__ARCH_OEM_RAPI_CLIENT_H



#include <linux/types.h>
#include <mach/msm_rpcrouter.h>

enum {
	OEM_RAPI_CLIENT_EVENT_NONE = 0,

	

#if defined (CONFIG_LGE_SUPPORT_RAPI)
	
	LG_FW_RAPI_START = 100,
	LG_FW_RAPI_CLIENT_EVENT_GET_LINE_TYPE = LG_FW_RAPI_START,
	LG_FW_TESTMODE_EVENT_FROM_ARM11 = LG_FW_RAPI_START + 1,
	LG_FW_A2M_BATT_INFO_GET = LG_FW_RAPI_START + 2,
	LG_FW_A2M_PSEUDO_BATT_INFO_SET = LG_FW_RAPI_START + 3,
	LG_FW_MEID_GET = LG_FW_RAPI_START + 4,
	
	LG_FW_SET_OPERATIN_MODE = LG_FW_RAPI_START + 5,
	LG_FW_A2M_BLOCK_CHARGING_SET = LG_FW_RAPI_START + 6,
#endif
	OEM_RAPI_CLIENT_EVENT_MAX

};

struct oem_rapi_client_streaming_func_cb_arg {
	uint32_t  event;
	void      *handle;
	uint32_t  in_len;
	char      *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_cb_ret {
	uint32_t *out_len;
	char *output;
};

struct oem_rapi_client_streaming_func_arg {
	uint32_t event;
	int (*cb_func)(struct oem_rapi_client_streaming_func_cb_arg *,
		       struct oem_rapi_client_streaming_func_cb_ret *);
	void *handle;
	uint32_t in_len;
	char *input;
	uint32_t out_len_valid;
	uint32_t output_valid;
	uint32_t output_size;
};

struct oem_rapi_client_streaming_func_ret {
	uint32_t *out_len;
	char *output;
};

int oem_rapi_client_streaming_function(
	struct msm_rpc_client *client,
	struct oem_rapi_client_streaming_func_arg *arg,
	struct oem_rapi_client_streaming_func_ret *ret);

int oem_rapi_client_close(void);

struct msm_rpc_client *oem_rapi_client_init(void);

#endif
