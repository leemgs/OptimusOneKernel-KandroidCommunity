

#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>



#define TIME_REMOTE_MTOA_PROG 0x3000005d
#if CONFIG_MSM_AMSS_VERSION==6210
#define TIME_REMOTE_MTOA_VERS 0
#elif (CONFIG_MSM_AMSS_VERSION==6220) || (CONFIG_MSM_AMSS_VERSION==6225)
#define TIME_REMOTE_MTOA_VERS 0x9202a8e4
#elif CONFIG_MSM_AMSS_VERSION==6350
#define TIME_REMOTE_MTOA_VERS 0x00010000
#else
#error "Unknown AMSS version"
#endif
#define RPC_TIME_REMOTE_MTOA_NULL   0
#define RPC_TIME_TOD_SET_APPS_BASES 2

struct rpc_time_tod_set_apps_bases_args {
	uint32_t tick;
	uint64_t stamp;
};

static int handle_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	switch (req->procedure) {
	case RPC_TIME_REMOTE_MTOA_NULL:
		return 0;

	case RPC_TIME_TOD_SET_APPS_BASES: {
		struct rpc_time_tod_set_apps_bases_args *args;
		args = (struct rpc_time_tod_set_apps_bases_args *)(req + 1);
		args->tick = be32_to_cpu(args->tick);
		args->stamp = be64_to_cpu(args->stamp);
		printk(KERN_INFO "RPC_TIME_TOD_SET_APPS_BASES:\n"
		       "\ttick = %d\n"
		       "\tstamp = %lld\n",
		       args->tick, args->stamp);
		return 0;
	}
	default:
		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server = {
	.prog = TIME_REMOTE_MTOA_PROG,
	.vers = TIME_REMOTE_MTOA_VERS,
	.rpc_call = handle_rpc_call,
};

static int __init rpc_server_init(void)
{
	
	return msm_rpc_create_server(&rpc_server);
}


module_init(rpc_server_init);
