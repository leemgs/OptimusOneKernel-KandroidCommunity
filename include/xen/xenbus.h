

#ifndef _XEN_XENBUS_H
#define _XEN_XENBUS_H

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/init.h>
#include <xen/interface/xen.h>
#include <xen/interface/grant_table.h>
#include <xen/interface/io/xenbus.h>
#include <xen/interface/io/xs_wire.h>


struct xenbus_watch
{
	struct list_head list;

	
	const char *node;

	
	void (*callback)(struct xenbus_watch *,
			 const char **vec, unsigned int len);
};



struct xenbus_device {
	const char *devicetype;
	const char *nodename;
	const char *otherend;
	int otherend_id;
	struct xenbus_watch otherend_watch;
	struct device dev;
	enum xenbus_state state;
	struct completion down;
};

static inline struct xenbus_device *to_xenbus_device(struct device *dev)
{
	return container_of(dev, struct xenbus_device, dev);
}

struct xenbus_device_id
{
	
	char devicetype[32]; 	
};


struct xenbus_driver {
	char *name;
	struct module *owner;
	const struct xenbus_device_id *ids;
	int (*probe)(struct xenbus_device *dev,
		     const struct xenbus_device_id *id);
	void (*otherend_changed)(struct xenbus_device *dev,
				 enum xenbus_state backend_state);
	int (*remove)(struct xenbus_device *dev);
	int (*suspend)(struct xenbus_device *dev, pm_message_t state);
	int (*resume)(struct xenbus_device *dev);
	int (*uevent)(struct xenbus_device *, char **, int, char *, int);
	struct device_driver driver;
	int (*read_otherend_details)(struct xenbus_device *dev);
	int (*is_ready)(struct xenbus_device *dev);
};

static inline struct xenbus_driver *to_xenbus_driver(struct device_driver *drv)
{
	return container_of(drv, struct xenbus_driver, driver);
}

int __must_check __xenbus_register_frontend(struct xenbus_driver *drv,
					    struct module *owner,
					    const char *mod_name);

static inline int __must_check
xenbus_register_frontend(struct xenbus_driver *drv)
{
	WARN_ON(drv->owner != THIS_MODULE);
	return __xenbus_register_frontend(drv, THIS_MODULE, KBUILD_MODNAME);
}

int __must_check __xenbus_register_backend(struct xenbus_driver *drv,
					   struct module *owner,
					   const char *mod_name);
static inline int __must_check
xenbus_register_backend(struct xenbus_driver *drv)
{
	WARN_ON(drv->owner != THIS_MODULE);
	return __xenbus_register_backend(drv, THIS_MODULE, KBUILD_MODNAME);
}

void xenbus_unregister_driver(struct xenbus_driver *drv);

struct xenbus_transaction
{
	u32 id;
};


#define XBT_NIL ((struct xenbus_transaction) { 0 })

char **xenbus_directory(struct xenbus_transaction t,
			const char *dir, const char *node, unsigned int *num);
void *xenbus_read(struct xenbus_transaction t,
		  const char *dir, const char *node, unsigned int *len);
int xenbus_write(struct xenbus_transaction t,
		 const char *dir, const char *node, const char *string);
int xenbus_mkdir(struct xenbus_transaction t,
		 const char *dir, const char *node);
int xenbus_exists(struct xenbus_transaction t,
		  const char *dir, const char *node);
int xenbus_rm(struct xenbus_transaction t, const char *dir, const char *node);
int xenbus_transaction_start(struct xenbus_transaction *t);
int xenbus_transaction_end(struct xenbus_transaction t, int abort);


int xenbus_scanf(struct xenbus_transaction t,
		 const char *dir, const char *node, const char *fmt, ...)
	__attribute__((format(scanf, 4, 5)));


int xenbus_printf(struct xenbus_transaction t,
		  const char *dir, const char *node, const char *fmt, ...)
	__attribute__((format(printf, 4, 5)));


int xenbus_gather(struct xenbus_transaction t, const char *dir, ...);


extern int xenstored_ready;
int register_xenstore_notifier(struct notifier_block *nb);
void unregister_xenstore_notifier(struct notifier_block *nb);

int register_xenbus_watch(struct xenbus_watch *watch);
void unregister_xenbus_watch(struct xenbus_watch *watch);
void xs_suspend(void);
void xs_resume(void);
void xs_suspend_cancel(void);


void *xenbus_dev_request_and_reply(struct xsd_sockmsg *msg);

struct work_struct;


void xenbus_suspend(void);
void xenbus_resume(void);
void xenbus_probe(struct work_struct *);
void xenbus_suspend_cancel(void);

#define XENBUS_IS_ERR_READ(str) ({			\
	if (!IS_ERR(str) && strlen(str) == 0) {		\
		kfree(str);				\
		str = ERR_PTR(-ERANGE);			\
	}						\
	IS_ERR(str);					\
})

#define XENBUS_EXIST_ERR(err) ((err) == -ENOENT || (err) == -ERANGE)

int xenbus_watch_path(struct xenbus_device *dev, const char *path,
		      struct xenbus_watch *watch,
		      void (*callback)(struct xenbus_watch *,
				       const char **, unsigned int));
int xenbus_watch_pathfmt(struct xenbus_device *dev, struct xenbus_watch *watch,
			 void (*callback)(struct xenbus_watch *,
					  const char **, unsigned int),
			 const char *pathfmt, ...)
	__attribute__ ((format (printf, 4, 5)));

int xenbus_switch_state(struct xenbus_device *dev, enum xenbus_state new_state);
int xenbus_grant_ring(struct xenbus_device *dev, unsigned long ring_mfn);
int xenbus_map_ring_valloc(struct xenbus_device *dev,
			   int gnt_ref, void **vaddr);
int xenbus_map_ring(struct xenbus_device *dev, int gnt_ref,
			   grant_handle_t *handle, void *vaddr);

int xenbus_unmap_ring_vfree(struct xenbus_device *dev, void *vaddr);
int xenbus_unmap_ring(struct xenbus_device *dev,
		      grant_handle_t handle, void *vaddr);

int xenbus_alloc_evtchn(struct xenbus_device *dev, int *port);
int xenbus_bind_evtchn(struct xenbus_device *dev, int remote_port, int *port);
int xenbus_free_evtchn(struct xenbus_device *dev, int port);

enum xenbus_state xenbus_read_driver_state(const char *path);

void xenbus_dev_error(struct xenbus_device *dev, int err, const char *fmt, ...);
void xenbus_dev_fatal(struct xenbus_device *dev, int err, const char *fmt, ...);

const char *xenbus_strstate(enum xenbus_state state);
int xenbus_dev_is_online(struct xenbus_device *dev);
int xenbus_frontend_closed(struct xenbus_device *dev);

#endif 
