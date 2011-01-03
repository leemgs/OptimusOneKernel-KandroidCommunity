#include <linux/suspend.h>
#include <linux/suspend_ioctls.h>
#include <linux/utsname.h>
#include <linux/freezer.h>

struct swsusp_info {
	struct new_utsname	uts;
	u32			version_code;
	unsigned long		num_physpages;
	int			cpus;
	unsigned long		image_pages;
	unsigned long		pages;
	unsigned long		size;
} __attribute__((aligned(PAGE_SIZE)));

#ifdef CONFIG_HIBERNATION
#ifdef CONFIG_ARCH_HIBERNATION_HEADER

#define MAX_ARCH_HEADER_SIZE	(sizeof(struct new_utsname) + 4)

extern int arch_hibernation_header_save(void *addr, unsigned int max_size);
extern int arch_hibernation_header_restore(void *addr);

static inline int init_header_complete(struct swsusp_info *info)
{
	return arch_hibernation_header_save(info, MAX_ARCH_HEADER_SIZE);
}

static inline char *check_image_kernel(struct swsusp_info *info)
{
	return arch_hibernation_header_restore(info) ?
			"architecture specific data" : NULL;
}
#endif 


#define PAGES_FOR_IO	((4096 * 1024) >> PAGE_SHIFT)


#define SPARE_PAGES	((1024 * 1024) >> PAGE_SHIFT)


extern int hibernation_snapshot(int platform_mode);
extern int hibernation_restore(int platform_mode);
extern int hibernation_platform_enter(void);
#endif

extern int pfn_is_nosave(unsigned long);

#define power_attr(_name) \
static struct kobj_attribute _name##_attr = {	\
	.attr	= {				\
		.name = __stringify(_name),	\
		.mode = 0644,			\
	},					\
	.show	= _name##_show,			\
	.store	= _name##_store,		\
}


extern unsigned long image_size;
extern int in_suspend;
extern dev_t swsusp_resume_device;
extern sector_t swsusp_resume_block;

extern asmlinkage int swsusp_arch_suspend(void);
extern asmlinkage int swsusp_arch_resume(void);

extern int create_basic_memory_bitmaps(void);
extern void free_basic_memory_bitmaps(void);
extern int hibernate_preallocate_memory(void);



struct snapshot_handle {
	loff_t		offset;	
	unsigned int	cur;	
	unsigned int	cur_offset;	
	unsigned int	prev;	
	void		*buffer;	
	unsigned int	buf_offset;	
	int		sync_read;	
};


#define data_of(handle)	((handle).buffer + (handle).buf_offset)

extern unsigned int snapshot_additional_pages(struct zone *zone);
extern unsigned long snapshot_get_image_size(void);
extern int snapshot_read_next(struct snapshot_handle *handle, size_t count);
extern int snapshot_write_next(struct snapshot_handle *handle, size_t count);
extern void snapshot_write_finalize(struct snapshot_handle *handle);
extern int snapshot_image_loaded(struct snapshot_handle *handle);


extern atomic_t snapshot_device_available;

extern sector_t alloc_swapdev_block(int swap);
extern void free_all_swap_pages(int swap);
extern int swsusp_swap_in_use(void);


#define SF_PLATFORM_MODE	1


extern int swsusp_check(void);
extern void swsusp_free(void);
extern int swsusp_read(unsigned int *flags_p);
extern int swsusp_write(unsigned int flags);
extern void swsusp_close(fmode_t);

struct timeval;

extern void swsusp_show_speed(struct timeval *, struct timeval *,
				unsigned int, char *);

#ifdef CONFIG_SUSPEND

extern const char *const pm_states[];

extern bool valid_state(suspend_state_t state);
extern int suspend_devices_and_enter(suspend_state_t state);
extern int enter_state(suspend_state_t state);
#else 
static inline int suspend_devices_and_enter(suspend_state_t state)
{
	return -ENOSYS;
}
static inline int enter_state(suspend_state_t state) { return -ENOSYS; }
static inline bool valid_state(suspend_state_t state) { return false; }
#endif 

#ifdef CONFIG_PM_TEST_SUSPEND

extern void suspend_test_start(void);
extern void suspend_test_finish(const char *label);
#else 
static inline void suspend_test_start(void) {}
static inline void suspend_test_finish(const char *label) {}
#endif 

#ifdef CONFIG_PM_SLEEP

extern int pm_notifier_call_chain(unsigned long val);
#endif

#ifdef CONFIG_HIGHMEM
int restore_highmem(void);
#else
static inline unsigned int count_highmem_pages(void) { return 0; }
static inline int restore_highmem(void) { return 0; }
#endif


enum {
	
	TEST_NONE,
	TEST_CORE,
	TEST_CPUS,
	TEST_PLATFORM,
	TEST_DEVICES,
	TEST_FREEZER,
	
	__TEST_AFTER_LAST
};

#define TEST_FIRST	TEST_NONE
#define TEST_MAX	(__TEST_AFTER_LAST - 1)

extern int pm_test_level;

#ifdef CONFIG_SUSPEND_FREEZER
static inline int suspend_freeze_processes(void)
{
	return freeze_processes();
}

static inline void suspend_thaw_processes(void)
{
	thaw_processes();
}
#else
static inline int suspend_freeze_processes(void)
{
	return 0;
}

static inline void suspend_thaw_processes(void)
{
}
#endif

#ifdef CONFIG_WAKELOCK

extern struct workqueue_struct *suspend_work_queue;
extern struct wake_lock main_wake_lock;
extern suspend_state_t requested_suspend_state;
#endif

#ifdef CONFIG_USER_WAKELOCK
ssize_t wake_lock_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
ssize_t wake_lock_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t n);
ssize_t wake_unlock_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf);
ssize_t  wake_unlock_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t n);
#endif

#ifdef CONFIG_EARLYSUSPEND

void request_suspend_state(suspend_state_t state);
suspend_state_t get_suspend_state(void);
#endif
