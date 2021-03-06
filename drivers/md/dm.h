

#ifndef DM_INTERNAL_H
#define DM_INTERNAL_H

#include <linux/fs.h>
#include <linux/device-mapper.h>
#include <linux/list.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>


#define DM_SUSPEND_LOCKFS_FLAG		(1 << 0)
#define DM_SUSPEND_NOFLUSH_FLAG		(1 << 1)


#define DM_TYPE_NONE		0
#define DM_TYPE_BIO_BASED	1
#define DM_TYPE_REQUEST_BASED	2


struct dm_dev_internal {
	struct list_head list;
	atomic_t count;
	struct dm_dev dm_dev;
};

struct dm_table;
struct dm_md_mempools;


void dm_table_destroy(struct dm_table *t);
void dm_table_event_callback(struct dm_table *t,
			     void (*fn)(void *), void *context);
struct dm_target *dm_table_get_target(struct dm_table *t, unsigned int index);
struct dm_target *dm_table_find_target(struct dm_table *t, sector_t sector);
int dm_calculate_queue_limits(struct dm_table *table,
			      struct queue_limits *limits);
void dm_table_set_restrictions(struct dm_table *t, struct request_queue *q,
			       struct queue_limits *limits);
struct list_head *dm_table_get_devices(struct dm_table *t);
void dm_table_presuspend_targets(struct dm_table *t);
void dm_table_postsuspend_targets(struct dm_table *t);
int dm_table_resume_targets(struct dm_table *t);
int dm_table_any_congested(struct dm_table *t, int bdi_bits);
int dm_table_any_busy_target(struct dm_table *t);
int dm_table_set_type(struct dm_table *t);
unsigned dm_table_get_type(struct dm_table *t);
bool dm_table_request_based(struct dm_table *t);
int dm_table_alloc_md_mempools(struct dm_table *t);
void dm_table_free_md_mempools(struct dm_table *t);
struct dm_md_mempools *dm_table_get_md_mempools(struct dm_table *t);


#define dm_target_is_valid(t) ((t)->table)


#define dm_target_request_based(t) ((t)->type->map_rq != NULL)


int dm_target_init(void);
void dm_target_exit(void);
struct target_type *dm_get_target_type(const char *name);
void dm_put_target_type(struct target_type *tt);
int dm_target_iterate(void (*iter_func)(struct target_type *tt,
					void *param), void *param);

int dm_split_args(int *argc, char ***argvp, char *input);


int dm_interface_init(void);
void dm_interface_exit(void);


int dm_sysfs_init(struct mapped_device *md);
void dm_sysfs_exit(struct mapped_device *md);
struct kobject *dm_kobject(struct mapped_device *md);
struct mapped_device *dm_get_from_kobject(struct kobject *kobj);


int dm_linear_init(void);
void dm_linear_exit(void);

int dm_stripe_init(void);
void dm_stripe_exit(void);

int dm_open_count(struct mapped_device *md);
int dm_lock_for_deletion(struct mapped_device *md);

void dm_kobject_uevent(struct mapped_device *md, enum kobject_action action,
		       unsigned cookie);

int dm_kcopyd_init(void);
void dm_kcopyd_exit(void);


struct dm_md_mempools *dm_alloc_md_mempools(unsigned type);
void dm_free_md_mempools(struct dm_md_mempools *pools);

#endif
