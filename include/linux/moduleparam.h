#ifndef _LINUX_MODULE_PARAMS_H
#define _LINUX_MODULE_PARAMS_H

#include <linux/init.h>
#include <linux/stringify.h>
#include <linux/kernel.h>


#ifdef MODULE
#define MODULE_PARAM_PREFIX 
#else
#define MODULE_PARAM_PREFIX KBUILD_MODNAME "."
#endif


#define MAX_PARAM_PREFIX_LEN (64 - sizeof(unsigned long))

#ifdef MODULE
#define ___module_cat(a,b) __mod_ ## a ## b
#define __module_cat(a,b) ___module_cat(a,b)
#define __MODULE_INFO(tag, name, info)					  \
static const char __module_cat(name,__LINE__)[]				  \
  __used								  \
  __attribute__((section(".modinfo"),unused)) = __stringify(tag) "=" info
#else  
#define __MODULE_INFO(tag, name, info)
#endif
#define __MODULE_PARM_TYPE(name, _type)					  \
  __MODULE_INFO(parmtype, name##type, #name ":" _type)

struct kernel_param;


typedef int (*param_set_fn)(const char *val, struct kernel_param *kp);

typedef int (*param_get_fn)(char *buffer, struct kernel_param *kp);


#define KPARAM_ISBOOL		2

struct kernel_param {
	const char *name;
	u16 perm;
	u16 flags;
	param_set_fn set;
	param_get_fn get;
	union {
		void *arg;
		const struct kparam_string *str;
		const struct kparam_array *arr;
	};
};


struct kparam_string {
	unsigned int maxlen;
	char *string;
};


struct kparam_array
{
	unsigned int max;
	unsigned int *num;
	param_set_fn set;
	param_get_fn get;
	unsigned int elemsize;
	void *elem;
};


#if defined(CONFIG_ALPHA) || defined(CONFIG_IA64) || defined(CONFIG_PPC64)
#define __moduleparam_const
#else
#define __moduleparam_const const
#endif


#define __module_param_call(prefix, name, set, get, arg, isbool, perm)	\
				\
	static int __param_perm_check_##name __attribute__((unused)) =	\
	BUILD_BUG_ON_ZERO((perm) < 0 || (perm) > 0777 || ((perm) & 2))	\
	+ BUILD_BUG_ON_ZERO(sizeof(""prefix) > MAX_PARAM_PREFIX_LEN);	\
	static const char __param_str_##name[] = prefix #name;		\
	static struct kernel_param __moduleparam_const __param_##name	\
	__used								\
    __attribute__ ((unused,__section__ ("__param"),aligned(sizeof(void *)))) \
	= { __param_str_##name, perm, isbool ? KPARAM_ISBOOL : 0,	\
	    set, get, { arg } }

#define module_param_call(name, set, get, arg, perm)			      \
	__module_param_call(MODULE_PARAM_PREFIX,			      \
			    name, set, get, arg,			      \
			    __same_type(*(arg), bool), perm)


#define module_param_named(name, value, type, perm)			   \
	param_check_##type(name, &(value));				   \
	module_param_call(name, param_set_##type, param_get_##type, &value, perm); \
	__MODULE_PARM_TYPE(name, #type)

#define module_param(name, type, perm)				\
	module_param_named(name, name, type, perm)

#ifndef MODULE

#define core_param(name, var, type, perm)				\
	param_check_##type(name, &(var));				\
	__module_param_call("", name, param_set_##type, param_get_##type, \
			    &var, __same_type(var, bool), perm)
#endif 


#define module_param_string(name, string, len, perm)			\
	static const struct kparam_string __param_string_##name		\
		= { len, string };					\
	__module_param_call(MODULE_PARAM_PREFIX, name,			\
			    param_set_copystring, param_get_string,	\
			    .str = &__param_string_##name, 0, perm);	\
	__MODULE_PARM_TYPE(name, "string")


extern int parse_args(const char *name,
		      char *args,
		      struct kernel_param *params,
		      unsigned num,
		      int (*unknown)(char *param, char *val));


#ifdef CONFIG_SYSFS
extern void destroy_params(const struct kernel_param *params, unsigned num);
#else
static inline void destroy_params(const struct kernel_param *params,
				  unsigned num)
{
}
#endif 



#define __param_check(name, p, type) \
	static inline type *__check_##name(void) { return(p); }

extern int param_set_byte(const char *val, struct kernel_param *kp);
extern int param_get_byte(char *buffer, struct kernel_param *kp);
#define param_check_byte(name, p) __param_check(name, p, unsigned char)

extern int param_set_short(const char *val, struct kernel_param *kp);
extern int param_get_short(char *buffer, struct kernel_param *kp);
#define param_check_short(name, p) __param_check(name, p, short)

extern int param_set_ushort(const char *val, struct kernel_param *kp);
extern int param_get_ushort(char *buffer, struct kernel_param *kp);
#define param_check_ushort(name, p) __param_check(name, p, unsigned short)

extern int param_set_int(const char *val, struct kernel_param *kp);
extern int param_get_int(char *buffer, struct kernel_param *kp);
#define param_check_int(name, p) __param_check(name, p, int)

extern int param_set_uint(const char *val, struct kernel_param *kp);
extern int param_get_uint(char *buffer, struct kernel_param *kp);
#define param_check_uint(name, p) __param_check(name, p, unsigned int)

extern int param_set_long(const char *val, struct kernel_param *kp);
extern int param_get_long(char *buffer, struct kernel_param *kp);
#define param_check_long(name, p) __param_check(name, p, long)

extern int param_set_ulong(const char *val, struct kernel_param *kp);
extern int param_get_ulong(char *buffer, struct kernel_param *kp);
#define param_check_ulong(name, p) __param_check(name, p, unsigned long)

extern int param_set_charp(const char *val, struct kernel_param *kp);
extern int param_get_charp(char *buffer, struct kernel_param *kp);
#define param_check_charp(name, p) __param_check(name, p, char *)


extern int param_set_bool(const char *val, struct kernel_param *kp);
extern int param_get_bool(char *buffer, struct kernel_param *kp);
#define param_check_bool(name, p)					\
	static inline void __check_##name(void)				\
	{								\
		BUILD_BUG_ON(!__same_type(*(p), bool) &&		\
			     !__same_type(*(p), unsigned int) &&	\
			     !__same_type(*(p), int));			\
	}

extern int param_set_invbool(const char *val, struct kernel_param *kp);
extern int param_get_invbool(char *buffer, struct kernel_param *kp);
#define param_check_invbool(name, p) __param_check(name, p, bool)


#define module_param_array_named(name, array, type, nump, perm)		\
	static const struct kparam_array __param_arr_##name		\
	= { ARRAY_SIZE(array), nump, param_set_##type, param_get_##type,\
	    sizeof(array[0]), array };					\
	__module_param_call(MODULE_PARAM_PREFIX, name,			\
			    param_array_set, param_array_get,		\
			    .arr = &__param_arr_##name,			\
			    __same_type(array[0], bool), perm);		\
	__MODULE_PARM_TYPE(name, "array of " #type)

#define module_param_array(name, type, nump, perm)		\
	module_param_array_named(name, name, type, nump, perm)

extern int param_array_set(const char *val, struct kernel_param *kp);
extern int param_array_get(char *buffer, struct kernel_param *kp);

extern int param_set_copystring(const char *val, struct kernel_param *kp);
extern int param_get_string(char *buffer, struct kernel_param *kp);



struct module;

#if defined(CONFIG_SYSFS) && defined(CONFIG_MODULES)
extern int module_param_sysfs_setup(struct module *mod,
				    struct kernel_param *kparam,
				    unsigned int num_params);

extern void module_param_sysfs_remove(struct module *mod);
#else
static inline int module_param_sysfs_setup(struct module *mod,
			     struct kernel_param *kparam,
			     unsigned int num_params)
{
	return 0;
}

static inline void module_param_sysfs_remove(struct module *mod)
{ }
#endif

#endif 
