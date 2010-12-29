



#ifndef NPA_REMOTE_H
#define NPA_REMOTE_H

#include <linux/errno.h>

#include "npa.h"
#include "npa_resource.h"

#ifdef CONFIG_MSM_NPA_REMOTE

#define NPA_REMOTE_VERSION_MAJOR	1
#define NPA_REMOTE_VERSION_MINOR	0
#define NPA_REMOTE_VERSION_BUILD	0


#define NPA_REMOTE_PROTOCOL_FAILURE	-1
#define NPA_REMOTE_FAILURE		1

enum npa_remote_client_type {
	NPA_REMOTE_CLIENT_RESERVED1,
	NPA_REMOTE_CLIENT_RESERVED2,
	NPA_REMOTE_CLIENT_CUSTOM1,
	NPA_REMOTE_CLIENT_CUSTOM2,
	NPA_REMOTE_CLIENT_CUSTOM3,
	NPA_REMOTE_CLIENT_CUSTOM4,
	NPA_REMOTE_CLIENT_REQUIRED,
	NPA_REMOTE_CLIENT_ISOCHRONOUS,
	NPA_REMOTE_CLIENT_IMPULSE,
	NPA_REMOTE_CLIENT_LIMIT_MAX,
	NPA_REMOTE_CLIENT_SIZE = 0x7FFFFFFF, 
};

typedef int (*npa_remote_callback)(void *context, unsigned int event_type,
		int *data, unsigned int data_size);




int npa_remote_null(void);


int npa_remote_init(unsigned int major, unsigned int minor, unsigned int build,
		npa_remote_callback callback, void *context);


int npa_remote_resource_available(const char *resource_name,
		npa_remote_callback callback, void *context);


int npa_remote_create_sync_client(const char *resource_name,
		const char *client_name,
		enum npa_remote_client_type client_type,
		void **handle);


int npa_remote_destroy_client(void *handle);


int npa_remote_issue_required_request(void *handle, unsigned int state,
		unsigned int *new_state);




unsigned int npa_remote_agg_driver_fn(struct npa_resource *resource,
		struct npa_client *client, unsigned int state);
unsigned int npa_local_agg_driver_fn(struct npa_resource *resource,
		struct npa_client *client, unsigned int state);


extern const struct npa_resource_plugin_ops npa_remote_agg_plugin;

#define DECLARE_RESOURCE_LOCAL_AGGREGATION(n, r, r_name, u_name, val, p) \
	struct npa_resource_definition r = { \
		.name = r_name, \
		.units = u_name, \
		.attributes = NPA_RESOURCE_DEFAULT, \
		.max = val, \
		.plugin = &p, \
		.data = NULL, \
	}; \
	struct npa_node_definition n = { \
		.name = r_name, \
		.attributes = NPA_NODE_DEFAULT, \
		.driver_fn = npa_local_agg_driver_fn, \
		.dependencies = NULL, \
		.dependency_count = 0, \
		.resources = &r, \
		.resource_count = 1, \
	};

#define DECLARE_RESOURCE_REMOTE_AGGREGATION(n, r, r_name, u_name, val) \
	struct npa_resource_definition r = { \
		.name = r_name, \
		.units = u_name, \
		.attributes = NPA_RESOURCE_DEFAULT, \
		.max = val, \
		.plugin = &npa_remote_agg_plugin, \
		.data = NULL, \
	}; \
	struct npa_node_definition n = {\
		.name = r_name, \
		.attributes = NPA_NODE_DEFAULT, \
		.driver_fn = npa_remote_agg_driver_fn, \
		.dependencies = NULL, \
		.dependency_count = 0, \
		.resources = &r, \
		.resource_count = 1, \
	};


int npa_remote_define_node(struct npa_node_definition *node,
		unsigned int init_state, npa_cb_fn callback, void *data);

#else

static inline int npa_remote_define_node(struct npa_node_definition *node,
		unsigned int init_state, npa_cb_fn callback, void *data)
{
	return -ENOSYS;
}

#endif

#endif
