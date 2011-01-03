

#ifndef _LINUX_KEY_TYPE_H
#define _LINUX_KEY_TYPE_H

#include <linux/key.h>

#ifdef CONFIG_KEYS


struct key_construction {
	struct key	*key;	
	struct key	*authkey;
};

typedef int (*request_key_actor_t)(struct key_construction *key,
				   const char *op, void *aux);


struct key_type {
	
	const char *name;

	
	size_t def_datalen;

	
	int (*instantiate)(struct key *key, const void *data, size_t datalen);

	
	int (*update)(struct key *key, const void *data, size_t datalen);

	
	int (*match)(const struct key *key, const void *desc);

	
	void (*revoke)(struct key *key);

	
	void (*destroy)(struct key *key);

	
	void (*describe)(const struct key *key, struct seq_file *p);

	
	long (*read)(const struct key *key, char __user *buffer, size_t buflen);

	
	request_key_actor_t request_key;

	
	struct list_head	link;		
};

extern struct key_type key_type_keyring;

extern int register_key_type(struct key_type *ktype);
extern void unregister_key_type(struct key_type *ktype);

extern int key_payload_reserve(struct key *key, size_t datalen);
extern int key_instantiate_and_link(struct key *key,
				    const void *data,
				    size_t datalen,
				    struct key *keyring,
				    struct key *instkey);
extern int key_negate_and_link(struct key *key,
			       unsigned timeout,
			       struct key *keyring,
			       struct key *instkey);
extern void complete_request_key(struct key_construction *cons, int error);

#endif 
#endif 
