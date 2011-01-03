
#include <linux/debugfs.h>
#include "kgsl_log.h"
#include "kgsl_device.h"
#include "kgsl.h"


#define KGSL_LOG_LEVEL_DEFAULT 3
#define KGSL_LOG_LEVEL_MAX     7
unsigned int kgsl_drv_log = KGSL_LOG_LEVEL_DEFAULT;
unsigned int kgsl_cmd_log = KGSL_LOG_LEVEL_DEFAULT;
unsigned int kgsl_ctxt_log = KGSL_LOG_LEVEL_DEFAULT;
unsigned int kgsl_mem_log = KGSL_LOG_LEVEL_DEFAULT;

#ifdef CONFIG_MSM_KGSL_MMU
unsigned int kgsl_cache_enable;
#endif

#ifdef CONFIG_DEBUG_FS
static int kgsl_log_set(unsigned int *log_val, void *data, u64 val)
{
	*log_val = min((unsigned int)val, (unsigned int)KGSL_LOG_LEVEL_MAX);
	return 0;
}

static int kgsl_drv_log_set(void *data, u64 val)
{
	return kgsl_log_set(&kgsl_drv_log, data, val);
}

static int kgsl_drv_log_get(void *data, u64 *val)
{
	*val = kgsl_drv_log;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kgsl_drv_log_fops, kgsl_drv_log_get,
			kgsl_drv_log_set, "%llu\n");

static int kgsl_cmd_log_set(void *data, u64 val)
{
	return kgsl_log_set(&kgsl_cmd_log, data, val);
}

static int kgsl_cmd_log_get(void *data, u64 *val)
{
	*val = kgsl_cmd_log;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kgsl_cmd_log_fops, kgsl_cmd_log_get,
			kgsl_cmd_log_set, "%llu\n");

static int kgsl_ctxt_log_set(void *data, u64 val)
{
	return kgsl_log_set(&kgsl_ctxt_log, data, val);
}

static int kgsl_ctxt_log_get(void *data, u64 *val)
{
	*val = kgsl_ctxt_log;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kgsl_ctxt_log_fops, kgsl_ctxt_log_get,
			kgsl_ctxt_log_set, "%llu\n");

static int kgsl_mem_log_set(void *data, u64 val)
{
	return kgsl_log_set(&kgsl_mem_log, data, val);
}

static int kgsl_mem_log_get(void *data, u64 *val)
{
	*val = kgsl_mem_log;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kgsl_mem_log_fops, kgsl_mem_log_get,
			kgsl_mem_log_set, "%llu\n");

#ifdef CONFIG_MSM_KGSL_MMU
static int kgsl_cache_enable_set(void *data, u64 val)
{
	kgsl_cache_enable = (val != 0);
	return 0;
}

static int kgsl_cache_enable_get(void *data, u64 *val)
{
	*val = kgsl_cache_enable;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(kgsl_cache_enable_fops, kgsl_cache_enable_get,
			kgsl_cache_enable_set, "%llu\n");
#endif 

#endif 

int kgsl_debug_init(void)
{
#ifdef CONFIG_DEBUG_FS
	struct dentry *dent;
	dent = debugfs_create_dir("kgsl", 0);
	if (IS_ERR(dent))
		return 0;

	debugfs_create_file("log_level_cmd", 0644, dent, 0,
				&kgsl_cmd_log_fops);
	debugfs_create_file("log_level_ctxt", 0644, dent, 0,
				&kgsl_ctxt_log_fops);
	debugfs_create_file("log_level_drv", 0644, dent, 0,
				&kgsl_drv_log_fops);
	debugfs_create_file("log_level_mem", 0644, dent, 0,
				&kgsl_mem_log_fops);

#ifdef CONFIG_MSM_KGSL_MMU
    debugfs_create_file("cache_enable", 0644, dent, 0,
				&kgsl_cache_enable_fops);
#endif

#endif 
	return 0;
}
