



#if defined(CONFIG_NFS_V4_1)

#include <linux/module.h>

#include <linux/sunrpc/xprt.h>
#include <linux/sunrpc/sched.h>
#include <linux/sunrpc/bc_xprt.h>

#define RPCDBG_FACILITY	RPCDBG_SVCDSP

void bc_release_request(struct rpc_task *task)
{
	struct rpc_rqst *req = task->tk_rqstp;

	dprintk("RPC:       bc_release_request: task= %p\n", task);

	
	if (!bc_prealloc(req))
		return;
	xprt_free_bc_request(req);
}


static const struct rpc_call_ops nfs41_callback_ops = {
};



int bc_send(struct rpc_rqst *req)
{
	struct rpc_task *task;
	int ret;

	dprintk("RPC:       bc_send req= %p\n", req);
	task = rpc_run_bc_task(req, &nfs41_callback_ops);
	if (IS_ERR(task))
		ret = PTR_ERR(task);
	else {
		BUG_ON(atomic_read(&task->tk_count) != 1);
		ret = task->tk_status;
		rpc_put_task(task);
	}
	return ret;
	dprintk("RPC:       bc_send ret= %d \n", ret);
}

#endif 
