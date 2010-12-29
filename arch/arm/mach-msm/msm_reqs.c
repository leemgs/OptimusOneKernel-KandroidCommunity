

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <mach/msm_reqs.h>
#include "npa_remote.h"

struct request {
	atomic_t creation_pending;
	struct npa_client *client;
	char *client_name;
	char *resource_name;
};


static void npa_res_available_cb(void *u_data, unsigned t, void *d, unsigned n)
{
	struct request *request = u_data;

	
	if (!atomic_dec_and_test(&request->creation_pending)) {
		kfree(request);
		return;
	}

	
	request->client = npa_create_sync_client(request->resource_name,
				request->client_name, NPA_CLIENT_REQUIRED);
	if (IS_ERR(request->client)) {
		pr_crit("npa_req: Failed to create NPA client '%s' "
			"for resource '%s'. (Error %ld)\n",
			request->client_name, request->resource_name,
			PTR_ERR(request->client));
		BUG();
	}

	return;
}


void *msm_req_add(char *res_name, char *req_name)
{
	struct request *request;

	request = kmalloc(sizeof(*request), GFP_KERNEL);
	if (!request)
		return ERR_PTR(-ENOMEM);

	
	request->client_name = req_name;
	request->resource_name = res_name;

	
	atomic_set(&request->creation_pending, 1);

	
	npa_resource_available(res_name, npa_res_available_cb, request);

	return request;
}


int msm_req_update(void *req, s32 value)
{
	struct request *request = req;
	int rc = 0;

	if (atomic_read(&request->creation_pending)) {
		pr_err("%s: Error: No client '%s' for resource '%s'.\n",
			__func__, request->client_name, request->resource_name);
		return -ENXIO;
	}

	if (value == MSM_REQ_DEFAULT_VALUE)
		npa_complete_request(request->client);
	else
		rc = npa_issue_required_request(request->client, value);

	return rc;
}


int msm_req_remove(void *req)
{
	struct request *request = req;

	
	if (!atomic_dec_and_test(&request->creation_pending)) {
		npa_cancel_request(request->client);
		npa_destroy_client(request->client);
		kfree(request);
	}

	return 0;
}

