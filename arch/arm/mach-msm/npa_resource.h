



#ifndef NPA_RESOURCE_H
#define NPA_RESOURCE_H

#include <linux/workqueue.h>
#include "npa.h"

#define ACTIVE_REQUEST		0
#define PENDING_REQUEST		1
#define ACTIVE_STATE(client)	((client)->work[ACTIVE_REQUEST].state)
#define PENDING_STATE(client)	((client)->work[PENDING_REQUEST].state)

#define NPA_LOG_MASK_NO_LOG	0
#define NPA_LOG_MASK_RESOURCE	(1<<0)
#define NPA_LOG_MASK_CLIENT	(1<<1)
#define NPA_LOG_MASK_EVENT	(1<<2)
#define NPA_LOG_MASK_LIST	(1<<3)
#define NPA_LOG_MASK_PLUGIN	(1<<4)
#define NPA_LOG_MASK_LOCKS	(1<<5)

#define npa_log(lm, res, f, ...) _npa_log(lm, res, KERN_INFO f, ##__VA_ARGS__)

struct npa_client;
struct npa_event;
struct npa_event_data;
struct npa_node_definition;
struct npa_resource_definition;
struct npa_node_dependency;
struct npa_resource;


enum npa_resource_attribute {
	NPA_RESOURCE_DEFAULT,
	NPA_RESOURCE_REPORT_ALL_REQS,
	NPA_RESOURCE_SINGLE_CLIENT,  
};


enum npa_node_attribute {
	NPA_NODE_DEFAULT,
};


typedef unsigned int (*npa_resource_update_fn)(
		struct npa_resource *resource, struct npa_client *client);


typedef unsigned int (*npa_resource_driver_fn)(
		struct npa_resource *resource,
		struct npa_client *client, unsigned int state);


struct npa_resource_plugin_ops {
	npa_resource_update_fn		update_fn;
	unsigned int			supported_clients;
	void (*create_client_fn) (struct npa_client *);
	void (*destroy_client_fn) (struct npa_client *);
};




extern const struct npa_resource_plugin_ops npa_binary_plugin;


extern const struct npa_resource_plugin_ops npa_max_plugin;


extern const struct npa_resource_plugin_ops npa_min_plugin;


extern const struct npa_resource_plugin_ops npa_sum_plugin;


extern const struct npa_resource_plugin_ops npa_always_on_plugin;


struct npa_node_dependency {
	const char 			*name;
	enum npa_client_type 		client_type;
	struct npa_client 		*handle;
};


struct npa_resource_definition {
	const char			*name;
	const char			*units;
	unsigned int			attributes;
	unsigned int			max;
	const struct npa_resource_plugin_ops *plugin;
	void				*data;
	struct npa_resource		*resource;
};


struct npa_node_definition {
	const char			*name;
	unsigned int			attributes;
	npa_resource_driver_fn		driver_fn;
	struct npa_node_dependency	*dependencies;
	unsigned int			dependency_count;
	struct npa_resource_definition	*resources;
	unsigned int			resource_count;
	void				*data;
};


struct npa_resource {
	struct npa_resource_definition  *definition;
	struct npa_node_definition	*node; 
	struct list_head		list; 
	struct list_head		clients; 
	struct list_head		events;  
	struct list_head		watermarks; 
	unsigned int			requested_state; 
	unsigned int			internal_state[4]; 
	unsigned int			active_state; 
	unsigned int			active_max; 
	int				active_headroom;
	const struct npa_resource_plugin_ops *active_plugin; 
	struct mutex			*resource_lock; 
	unsigned int			level; 
	struct work_struct		work;
};


struct npa_work_request {
	unsigned int			state;
	int				start;
	int				end;
};



struct npa_client {
	const char 			*name;		
	const char 			*resource_name; 
	struct npa_resource 		*resource;
	struct list_head		list; 
	enum npa_client_type 		type;
	struct npa_work_request 	work[2]; 	
	void 				*resource_data; 
};


struct npa_event {
	enum npa_event_type		type;
	const char			*handler_name;
	struct list_head		list; 
	struct npa_resource		*resource;
	int				lo_watermark;
	int				hi_watermark;
	npa_cb_fn			callback;
	void				*user_data;
	struct work_struct		work;
};

#ifdef CONFIG_MSM_NPA



int npa_define_node(struct npa_node_definition *node,
		unsigned int initial_state[],
		npa_cb_fn callback, void *user_data);


int npa_alias_resource(const char *resource_name, const char *alias_name,
			npa_cb_fn callback, void *user_data);


int npa_assign_resource_state(struct npa_resource *resource,
			unsigned int state);

#else
int npa_define_node(struct npa_node_definition *node,
		unsigned int initial_state[],
		npa_cb_fn callback, void *user_data)
{ return -ENOSYS; }
int npa_alias_resource(const char *resource_name, const char *alias_name,
			npa_cb_fn callback, void *user_data)
{ return -ENOSYS; }
int npa_assign_resource_state(struct npa_resource *resource,
			unsigned int state)
{ return -ENOSYS; }

#endif

#ifdef CONFIG_MSM_NPA_LOG

extern int npa_log_mask;
extern char npa_log_resource_name[];
extern struct npa_resource *npa_log_resource;
extern int npa_log_reset;

void _npa_log(int log_mask, struct npa_resource *res, const char *fmt, ...);



void __print_resource(struct npa_resource *r);
void __print_resources(void);
void __print_client_states(struct npa_resource *resource);
void __print_aliases(void);
#else
static inline void _npa_log(int log_mask, struct npa_resource *res,
		const char *fmt, ...) {}
static inline void __print_resource(struct npa_resource *r) {}
static inline void __print_resources(void) {}
static inline void __print_client_states(struct npa_resource *resource) {}
static inline void __print_aliases(void) {}
#endif

#ifdef CONFIG_MSM_NPA_DEBUG

void npa_reset(void);
#else
static inline void npa_reset(void) {}
#endif

#endif
