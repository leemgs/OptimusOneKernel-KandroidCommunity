

#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>



#define DOG_KEEPALIVE_PROG 0x30000015
#if CONFIG_MSM_AMSS_VERSION==6210
#define DOG_KEEPALIVE_VERS 0
#define RPC_DOG_KEEPALIVE_BEACON 1
#elif (CONFIG_MSM_AMSS_VERSION==6220) || (CONFIG_MSM_AMSS_VERSION==6225)
#define DOG_KEEPALIVE_VERS 0x731fa727
#define RPC_DOG_KEEPALIVE_BEACON 2
#else
#error "Unsupported AMSS version"
#endif
#define DOG_KEEPALIVE_VERS_COMP 0x00010001
#define RPC_DOG_KEEPALIVE_NULL 0




static int handle_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	switch (req->procedure) {
	case RPC_DOG_KEEPALIVE_NULL:
		return 0;
	case RPC_DOG_KEEPALIVE_BEACON:
		return 0;
	default:
		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server[] = {
	{
		.prog = DOG_KEEPALIVE_PROG,
		.vers = DOG_KEEPALIVE_VERS,
		.rpc_call = handle_rpc_call,
	},
	{
		.prog = DOG_KEEPALIVE_PROG,
		.vers = DOG_KEEPALIVE_VERS_COMP,
		.rpc_call = handle_rpc_call,
	},
};

static int __init rpc_server_init(void)
{
	
	int ret;
	ret = msm_rpc_create_server(&rpc_server[1]);
	if (ret < 0)
		return ret;
	return msm_rpc_create_server(&rpc_server[0]);
}


module_init(rpc_server_init);
