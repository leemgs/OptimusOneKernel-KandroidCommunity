



#ifndef NPA_H
#define NPA_H

#include <linux/err.h>
#include <linux/errno.h>


#define NPA_NAME_MAX 64


enum npa_client_type {
	NPA_CLIENT_RESERVED1 	= (1<<0),
	NPA_CLIENT_RESERVED2 	= (1<<1),
	NPA_CLIENT_CUSTOM1 	= (1<<2),
	NPA_CLIENT_CUSTOM2 	= (1<<3),
	NPA_CLIENT_CUSTOM3 	= (1<<4),
	NPA_CLIENT_CUSTOM4 	= (1<<5),
	NPA_CLIENT_REQUIRED 	= (1<<6),
	NPA_CLIENT_ISOCHRONOUS 	= (1<<7),
	NPA_CLIENT_IMPULSE 	= (1<<8),
	NPA_CLIENT_LIMIT_MAX	= (1<<9),
};


enum npa_event_type {
	NPA_EVENT_RESERVED1,	
	NPA_EVENT_LO_WATERMARK,	
	NPA_EVENT_HI_WATERMARK,	
	NPA_EVENT_CHANGE,	
	NPA_NUM_EVENT_TYPES
};


struct npa_event_data {
	const char		*resource_name;
	unsigned int		state;
	int			headroom; 
};


typedef void (*npa_cb_fn)(void *, unsigned int, void *, unsigned int);

#ifdef CONFIG_MSM_NPA





struct npa_client *npa_create_sync_client(const char *resource_name,
		const char *handler_name, enum npa_client_type type);


void npa_destroy_client(struct npa_client *client);


int npa_issue_required_request(struct npa_client *client, unsigned int state);


int npa_modify_required_request(struct npa_client *client, int delta);


int npa_issue_impulse_request(struct npa_client *client);


int npa_issue_isoc_request(struct npa_client *client, unsigned int duration,
		unsigned int level);


int npa_issue_limit_max_request(struct npa_client *client, unsigned int max);


void npa_complete_request(struct npa_client *client);


void npa_cancel_request(struct npa_client *client);


unsigned int npa_get_state(struct npa_client *client);


int npa_resource_available(const char *resource_name,
		npa_cb_fn callback, void *user_data);


struct npa_event *npa_create_change_event(const char *resource_name,
		const char *handler_name, npa_cb_fn event_cb, void *user_data);


struct npa_event *npa_create_watermark_event(const char *resource_name,
		const char *handler_name, npa_cb_fn event_cb, void *user_data);


void npa_destroy_event(struct npa_event *event);


int npa_set_event_watermarks(struct npa_event *event,
		int lo_watermark, int hi_watermark);

#else
static inline struct npa_client *npa_create_sync_client(
		const char *resource_name,
		const char *handler_name,
		enum npa_client_type type) { return ERR_PTR(-ENOSYS); }
static inline void npa_destroy_client(struct npa_client *client) {}
static inline int npa_issue_required_request(struct npa_client *client,
		unsigned int state) { return -ENOSYS; }
static inline int npa_modify_required_request(struct npa_client *client,
		int delta) { return -ENOSYS; }
static inline int npa_issue_impulse_request(struct npa_client *client)
{ return -ENOSYS; }
static inline int npa_issue_isoc_request(struct npa_client *client,
		unsigned int duration,
		unsigned int level) { return -ENOSYS; }
static inline int npa_issue_limit_max_request(struct npa_client *client,
		unsigned int max) { return -ENOSYS; }
static inline void npa_complete_request(struct npa_client *client) {}
static inline void npa_cancel_request(struct npa_client *client) {}
static inline unsigned int npa_get_state(struct npa_client *client)
{ return 0; }
static inline int npa_resource_available(const char *resource_name,
		npa_cb_fn callback, void *user_data) { return -ENOSYS; }
static inline struct npa_event *npa_create_change_event(
		const char *resource_name,
		const char *handler_name, npa_cb_fn event_cb, void *user_data)
{ return ERR_PTR(-ENOSYS); }
static inline struct npa_event *npa_create_watermark_event(
		const char *resource_name,
		const char *handler_name, npa_cb_fn event_cb, void *user_data)
{ return ERR_PTR(-ENOSYS); }
static inline void npa_destroy_event(struct npa_event *event) {}
static inline int npa_set_event_watermarks(struct npa_event *event,
		int lo_watermark, int hi_watermark) { return -ENOSYS; }

#endif
#endif
