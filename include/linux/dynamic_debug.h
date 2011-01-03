#ifndef _DYNAMIC_DEBUG_H
#define _DYNAMIC_DEBUG_H


extern long long dynamic_debug_enabled;
extern long long dynamic_debug_enabled2;


struct _ddebug {
	
	const char *modname;
	const char *function;
	const char *filename;
	const char *format;
	char primary_hash;
	char secondary_hash;
	unsigned int lineno:24;
	
#define _DPRINTK_FLAGS_PRINT   (1<<0)  
#define _DPRINTK_FLAGS_DEFAULT 0
	unsigned int flags:8;
} __attribute__((aligned(8)));


int ddebug_add_module(struct _ddebug *tab, unsigned int n,
				const char *modname);

#if defined(CONFIG_DYNAMIC_DEBUG)
extern int ddebug_remove_module(char *mod_name);

#define __dynamic_dbg_enabled(dd)  ({	     \
	int __ret = 0;							     \
	if (unlikely((dynamic_debug_enabled & (1LL << DEBUG_HASH)) &&	     \
			(dynamic_debug_enabled2 & (1LL << DEBUG_HASH2))))   \
				if (unlikely(dd.flags))			     \
					__ret = 1;			     \
	__ret; })

#define dynamic_pr_debug(fmt, ...) do {					\
	static struct _ddebug descriptor				\
	__used								\
	__attribute__((section("__verbose"), aligned(8))) =		\
	{ KBUILD_MODNAME, __func__, __FILE__, fmt, DEBUG_HASH,	\
		DEBUG_HASH2, __LINE__, _DPRINTK_FLAGS_DEFAULT };	\
	if (__dynamic_dbg_enabled(descriptor))				\
		printk(KERN_DEBUG KBUILD_MODNAME ":" pr_fmt(fmt),	\
				##__VA_ARGS__);				\
	} while (0)


#define dynamic_dev_dbg(dev, fmt, ...) do {				\
	static struct _ddebug descriptor				\
	__used								\
	__attribute__((section("__verbose"), aligned(8))) =		\
	{ KBUILD_MODNAME, __func__, __FILE__, fmt, DEBUG_HASH,	\
		DEBUG_HASH2, __LINE__, _DPRINTK_FLAGS_DEFAULT };	\
	if (__dynamic_dbg_enabled(descriptor))				\
			dev_printk(KERN_DEBUG, dev,			\
					KBUILD_MODNAME ": " fmt,	\
					##__VA_ARGS__);			\
	} while (0)

#else

static inline int ddebug_remove_module(char *mod)
{
	return 0;
}

#define dynamic_pr_debug(fmt, ...)  do { } while (0)
#define dynamic_dev_dbg(dev, format, ...)  do { } while (0)
#endif

#endif
