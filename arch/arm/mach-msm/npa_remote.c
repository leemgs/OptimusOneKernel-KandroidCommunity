



#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>

#include "npa_remote.h"


enum npa_data_type {
	NPA_CLIENT_TYPE = 10,
	NPA_EVENT_TYPE = 11,
	NPA_QUERY_TYPE = 12,
};

struct npa_node_info {
	struct work_struct work;
	const char *resource_name;
	struct npa_node_definition *node;
	unsigned int init_state;
	npa_cb_fn user_callback;
	void *user_data;
};

static struct workqueue_struct *npa_remote_wq;


static int npa_get_remote_type(enum npa_data_type enum_type, int type)
{
	struct enum_match_type {
		int local;
		int remote;
	};

	static struct enum_match_type enum_match[] = {
		{NPA_CLIENT_RESERVED1, NPA_REMOTE_CLIENT_RESERVED1},
		{NPA_CLIENT_RESERVED2, NPA_REMOTE_CLIENT_RESERVED2},
		{NPA_CLIENT_CUSTOM1, NPA_REMOTE_CLIENT_CUSTOM1},
		{NPA_CLIENT_CUSTOM2, NPA_REMOTE_CLIENT_CUSTOM2},
		{NPA_CLIENT_CUSTOM3, NPA_REMOTE_CLIENT_CUSTOM3},
		{NPA_CLIENT_CUSTOM4, NPA_REMOTE_CLIENT_CUSTOM4},
		{NPA_CLIENT_REQUIRED, NPA_REMOTE_CLIENT_REQUIRED},
		{NPA_CLIENT_ISOCHRONOUS, NPA_REMOTE_CLIENT_ISOCHRONOUS},
		{NPA_CLIENT_IMPULSE, NPA_REMOTE_CLIENT_IMPULSE},
		{NPA_CLIENT_LIMIT_MAX, NPA_REMOTE_CLIENT_LIMIT_MAX},
		{NPA_CLIENT_TYPE, 0},
		{NPA_EVENT_TYPE, 0},
		{NPA_QUERY_TYPE, 0},
	};

	int result = type;
	int i = 0;
	int type_last = -1;

	if (enum_type == NPA_QUERY_TYPE)
		type_last = NPA_EVENT_TYPE;
	else if (enum_type == NPA_EVENT_TYPE)
		type_last = NPA_CLIENT_TYPE;

	for (i = enum_type - 1; i > type_last; i--) {
		if (enum_match[i].local == type) {
			result = enum_match[i].remote;
			break;
		}
	}

	return result;
}

static void create_local_resource(struct work_struct *work)
{
	int err = 0;
	struct npa_node_info *node_info =
		container_of(work, struct npa_node_info, work);

	

	npa_log(NPA_LOG_MASK_RESOURCE, NULL,
			"NPA Remote: Remote resource [%s] is available, "
			"creating local resource\n",
			node_info->resource_name);

	err = npa_define_node(node_info->node, &node_info->init_state,
			node_info->user_callback, node_info->user_data);

	BUG_ON(err);
	kfree(node_info);
}

static int npa_remote_resource_available_cb(void *context, unsigned int type,
		int *data, unsigned int size)
{
	struct npa_node_info *node_info =
		(struct npa_node_info *)context;

	
	queue_work(npa_remote_wq, &node_info->work);

	return 0;
}

static void npa_define_deferred_nodes(void *context, unsigned int event_type,
		void *data, unsigned int data_size)
{
	struct npa_node_info *node_info = context;

	npa_log(NPA_LOG_MASK_RESOURCE, NULL,
			"NPA Remote: Protocol [%s] for resource [%s] is "
			"available. Checking resource availability\n",
			"/protocols/modem/oncrpc/1.0.0",
			node_info->resource_name);

	npa_remote_resource_available(node_info->resource_name,
				npa_remote_resource_available_cb,
				(void *)node_info);
}

int npa_remote_define_node(struct npa_node_definition *node,
		unsigned int init_state, npa_cb_fn callback, void *data)
{
	int ret = 0;
	struct npa_node_info *node_info = NULL;

	node_info = kzalloc(sizeof(struct npa_node_info), GFP_KERNEL);

	
	BUG_ON(node->resource_count != 1);

	node_info->resource_name = node->resources[0].name;
	node_info->node = node;
	node_info->init_state = init_state;
	node_info->user_callback = callback;
	node_info->user_data = data;
	INIT_WORK(&node_info->work, create_local_resource);

	

	npa_log(NPA_LOG_MASK_RESOURCE, NULL,
			"NPA Remote: Checking if protocol [%s] for resource"
			"[%s] is available\n", "/protocols/modem/oncrpc/1.0.0",
			node_info->resource_name);

	npa_resource_available("/protocols/modem/oncrpc/1.0.0",
			npa_define_deferred_nodes, node_info);

	return ret;

}
EXPORT_SYMBOL(npa_remote_define_node);

unsigned int npa_local_agg_driver_fn(struct npa_resource *resource,
		struct npa_client *client, unsigned int state)
{
	int result = 0;
	void *handle = NULL;
	unsigned int new_state = 0;

	
	if (!resource->definition->data) {
		npa_log(NPA_LOG_MASK_CLIENT, resource,
			"NPA Remote: Creating remote sync client [%s] "
			"of resource [%s]\n",
			client->name, resource->definition->name);

		result = npa_remote_create_sync_client(
					client->resource_name,
					client->name,
					npa_get_remote_type(
						NPA_CLIENT_TYPE,
						client->type),
					&handle);

		BUG_ON(result);
		resource->definition->data = handle;
	}

	npa_log(NPA_LOG_MASK_CLIENT, resource,
			"NPA Remote: Issuing remote request to resource [%s] "
			"by client [%s] with state [%u]\n",
			resource->definition->name, client->name,
			PENDING_STATE(client));

	result = npa_remote_issue_required_request(resource->definition->data,
			PENDING_STATE(client), &new_state);

	npa_log(NPA_LOG_MASK_CLIENT, resource,
			"NPA Remote: Local resource[%s] new state [%u]\n",
			resource->definition->name, new_state);

	BUG_ON(result);
	resource->active_state = new_state;

	return resource->active_state;
}
EXPORT_SYMBOL(npa_local_agg_driver_fn);

unsigned int npa_remote_agg_driver_fn(struct npa_resource *resource,
		struct npa_client *client, unsigned int state)
{
	
	return resource->active_state;
}
EXPORT_SYMBOL(npa_remote_agg_driver_fn);


static void npa_remote_create_client_cb_fn(struct npa_client *client)
{
	int result = 0;
	void *handle = NULL;

	

	npa_log(NPA_LOG_MASK_CLIENT, client->resource,
			"NPA Remote: Creating remote sync client [%s] "
			"of resource [%s]\n",
			client->name, client->resource->definition->name);

	result = npa_remote_create_sync_client(
			client->resource->definition->name,
			client->name,
			npa_get_remote_type(NPA_CLIENT_TYPE, client->type),
			&handle);

	BUG_ON(result);
	client->resource_data = handle;
}

static void npa_remote_destroy_client_cb_fn(struct npa_client *client)
{
	npa_log(NPA_LOG_MASK_CLIENT, client->resource,
			"NPA Remote: Destroying remote client [%s] "
			"of resource [%s]\n",
			client->name, client->resource->definition->name);

	npa_remote_destroy_client(client->resource_data);
}

unsigned int npa_remote_agg_update_fn(struct npa_resource *resource,
		struct npa_client *client)
{
	int result = 0;
	unsigned int new_state = 0;

	
	npa_log(NPA_LOG_MASK_CLIENT, resource,
			"NPA Remote: Issuing remote request to resource [%s] "
			"by client [%s] with state [%u]\n",
			resource->definition->name, client->name,
			PENDING_STATE(client));

	result = npa_remote_issue_required_request(client->resource_data,
			PENDING_STATE(client), &new_state);

	npa_log(NPA_LOG_MASK_CLIENT, resource,
			"NPA Remote: Local resource[%s] new state [%u]\n",
			resource->definition->name, new_state);

	BUG_ON(result);
	resource->active_state = new_state;

	return resource->active_state;
}

const struct npa_resource_plugin_ops npa_remote_agg_plugin = {
	.update_fn              = npa_remote_agg_update_fn,
	.supported_clients      = NPA_CLIENT_REQUIRED,
	.create_client_fn       = npa_remote_create_client_cb_fn,
	.destroy_client_fn      = npa_remote_destroy_client_cb_fn,
};
EXPORT_SYMBOL(npa_remote_agg_plugin);

static int npa_remote_cmd_init(void)
{
	npa_remote_wq = create_workqueue("npa-remote");
	BUG_ON(!npa_remote_wq);

	pr_info("NPA Remote: Init done.\n");

	return 0;
}
postcore_initcall(npa_remote_cmd_init);
