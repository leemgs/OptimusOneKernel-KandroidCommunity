
#ifndef __debug__h__
#define __debug__h__

#include <linux/types.h>
#include <linux/device.h>





static inline
void __d_head(char *head, size_t head_size,
	      struct device *dev)
{
	if (dev == NULL)
		head[0] = 0;
	else if ((unsigned long)dev < 4096) {
		printk(KERN_ERR "E: Corrupt dev %p\n", dev);
		WARN_ON(1);
	} else
		snprintf(head, head_size, "%s %s: ",
			 dev_driver_string(dev), dev_name(dev));
}



#define _d_printf(l, tag, dev, f, a...)					\
do {									\
	char head[64];							\
	if (!d_test(l))							\
		break;							\
	__d_head(head, sizeof(head), dev);				\
	printk(KERN_ERR "%s%s%s: " f, head, __func__, tag, ##a);	\
} while (0)



#define __D_PASTE__(varname, modulename) varname##_##modulename
#define __D_PASTE(varname, modulename) (__D_PASTE__(varname, modulename))
#define _D_SUBMODULE_INDEX(_name) (D_SUBMODULE_DECLARE(_name))



struct d_level {
	u8 level;
	const char *name;
};



#define D_LEVEL __D_PASTE(d_level, D_MODULENAME)
#define D_LEVEL_SIZE __D_PASTE(d_level_size, D_MODULENAME)

extern struct d_level D_LEVEL[];
extern size_t D_LEVEL_SIZE;




#ifndef D_MODULENAME
#error D_MODULENAME is not defined in your debug-levels.h file

#define D_MODULENAME undefined_modulename
#endif


#ifndef D_MASTER
#warning D_MASTER not defined, but debug.h included! [see docs]

#define D_MASTER 0
#endif

#ifndef D_SUBMODULE
#error D_SUBMODULE not defined, but debug.h included! [see docs]

#define D_SUBMODULE undefined_module
#endif



#define D_SUBMODULE_DECLARE(_name) __D_SUBMODULE_##_name



#define D_SUBMODULE_DEFINE(_name)		\
[__D_SUBMODULE_##_name] = {			\
	.level = 0,				\
	.name = #_name				\
}







#define d_test(l)							\
({									\
	unsigned __l = l;				\
	(D_MASTER) >= __l						\
	&& ({								\
		BUG_ON(_D_SUBMODULE_INDEX(D_SUBMODULE) >= D_LEVEL_SIZE);\
		D_LEVEL[_D_SUBMODULE_INDEX(D_SUBMODULE)].level >= __l;	\
	});								\
})



#define d_fnstart(l, _dev, f, a...) _d_printf(l, " FNSTART", _dev, f, ## a)



#define d_fnend(l, _dev, f, a...) _d_printf(l, " FNEND", _dev, f, ## a)



#define d_printf(l, _dev, f, a...) _d_printf(l, "", _dev, f, ## a)



#define d_dump(l, dev, ptr, size)			\
do {							\
	char head[64];					\
	if (!d_test(l))					\
		break;					\
	__d_head(head, sizeof(head), dev);		\
	print_hex_dump(KERN_ERR, head, 0, 16, 1,	\
		       ((void *) ptr), (size), 0);	\
} while (0)



#define d_level_register_debugfs(prefix, name, parent)			\
({									\
	int rc;								\
	struct dentry *fd;						\
	struct dentry *verify_parent_type = parent;			\
	fd = debugfs_create_u8(						\
		prefix #name, 0600, verify_parent_type,			\
		&(D_LEVEL[__D_SUBMODULE_ ## name].level));		\
	rc = PTR_ERR(fd);						\
	if (IS_ERR(fd) && rc != -ENODEV)				\
		printk(KERN_ERR "%s: Can't create debugfs entry %s: "	\
		       "%d\n", __func__, prefix #name, rc);		\
	else								\
		rc = 0;							\
	rc;								\
})


#endif 
